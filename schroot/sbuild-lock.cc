/* Copyright © 2005-2006  Roger Leigh <rleigh@debian.org>
 *
 * schroot is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * schroot is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA  02111-1307  USA
 *
 *********************************************************************/

#include <config.h>

#include "sbuild.h"

#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#include <boost/format.hpp>

#include <lockdev.h>

using boost::format;
using namespace sbuild;

namespace
{

  volatile bool lock_timeout = false;

  /**
   * Handle the SIGALRM signal.
   *
   * @param ignore the signal number.
   */
  static void
  alarm_handler (int ignore)
  {
    /* This exists so that system calls get interrupted. */
    /* lock_timeout is used for polling for a timeout, rather than
       interruption. */
    lock_timeout = true;
  }
}

Lock::Lock():
  saved_signals()
{
}

Lock::~Lock()
{
}

void
Lock::set_alarm ()
{
  struct sigaction new_sa;
  sigemptyset(&new_sa.sa_mask);
  new_sa.sa_flags = 0;
  new_sa.sa_handler = alarm_handler;

  if (sigaction(SIGALRM, &new_sa, &this->saved_signals) != 0)
    {
      format fmt(_("failed to set timeout handler: %1%"));
      fmt % strerror(errno);
      throw error(fmt);
    }
}

void
Lock::clear_alarm ()
{
  /* Restore original handler */
  sigaction (SIGALRM, &this->saved_signals, NULL);
}

void
Lock::set_timer(struct itimerval const& timer)
{
  set_alarm();

  if (setitimer(ITIMER_REAL, &timer, NULL) == -1)
    {
      clear_alarm();
      format fmt(_("failed to set timeout: %1%"));
      fmt % strerror(errno);
      throw error(fmt);
    }
}

void
Lock::unset_timer()
{
  struct itimerval disable_timer;
  disable_timer.it_interval.tv_sec = disable_timer.it_interval.tv_usec = 0;
  disable_timer.it_value.tv_sec = disable_timer.it_value.tv_usec = 0;

  if (setitimer(ITIMER_REAL, &disable_timer, NULL) == -1)
    {
      clear_alarm();
      format fmt(_("failed to unset timeout: %1%"));
      fmt % strerror(errno);
      throw error(fmt);
    }

  clear_alarm();
}

FileLock::FileLock (int fd):
  Lock(),
  fd(fd)
{
}

FileLock::~FileLock()
{
}

void
FileLock::set_lock (Type         lock_type,
		    unsigned int timeout)
{
  try
    {
      struct itimerval timeout_timer;
      timeout_timer.it_interval.tv_sec = timeout_timer.it_interval.tv_usec = 0;
      timeout_timer.it_value.tv_sec = timeout;
      timeout_timer.it_value.tv_usec = 0;
      set_timer(timeout_timer);

      /* Now the signal handler and itimer are set, the function can't
	 return without stopping the timer and restoring the signal
	 handler to its original state. */

      /* Wait on lock until interrupted by a signal if a timeout was set,
	 otherwise return immediately. */
      struct flock read_lock =
	{
	  lock_type,
	  SEEK_SET,
	  0,
	  0, // Lock entire file
	  0
	};

      if (fcntl(this->fd,
		(timeout != 0) ? F_SETLKW : F_SETLK,
		&read_lock) == -1)
	{
	  if (errno == EINTR)
	    {
	      format fmt (_("failed to acquire lock (timeout after %1% seconds)"));
	      fmt % timeout;
	      throw error(fmt);
	    }
	  else
	    {
	      format fmt(_("failed to acquire lock: %1%"));
	      fmt % strerror(errno);
	      throw error(fmt);
	    }
	}
      unset_timer();
    }
  catch (error const& e)
    {
      unset_timer();
      throw;
    }
}

void
FileLock::unset_lock ()
{
  set_lock(LOCK_NONE, 0);
}

DeviceLock::DeviceLock (std::string const& device):
  Lock(),
  device(device)
{
}

DeviceLock::~DeviceLock()
{
}

void
DeviceLock::set_lock (Type         lock_type,
		      unsigned int timeout)
{
  try
    {
      lock_timeout = false;

      struct itimerval timeout_timer;
      timeout_timer.it_interval.tv_sec = timeout_timer.it_interval.tv_usec = 0;
      timeout_timer.it_value.tv_sec = timeout;
      timeout_timer.it_value.tv_usec = 0;
      set_timer(timeout_timer);

      /* Now the signal handler and itimer are set, the function can't
	 return without stopping the timer and restoring the signal
	 handler to its original state. */

      /* Wait on lock until interrupted by a signal if a timeout was set,
	 otherwise return immediately. */
      pid_t status = 0;
      while (lock_timeout == false)
	{
	  if (lock_type == LOCK_SHARED || lock_type == LOCK_EXCLUSIVE)
	    {
	      status = dev_lock(this->device.c_str());
	      if (status == 0) // Success
		break;
	      else if (status < 0) // Failure
		{
		  throw error(_("failed to acquire device lock"));
		}
	    }
	  else
	    {
	      pid_t cur_lock_pid = dev_testlock(this->device.c_str());
	      if (cur_lock_pid < 0) // Test failure
		{
		  throw error(_("failed to test device lock"));
		}
	      else if (cur_lock_pid > 0 && cur_lock_pid != getpid())
		{
		  // Another process owns the lock, so we successfully
		  // "drop" our nonexistent lock.
		  break;
		}
	      status = dev_unlock(this->device.c_str(), getpid());
	      if (status == 0) // Success
		break;
	      else if (status < 0) // Failure
		{
		  throw error(_("failed to release device lock"));
		}
	    }
	}

      if (lock_timeout)
	{
	  format fmt((lock_type == LOCK_SHARED ||
		      lock_type == LOCK_EXCLUSIVE) ?
		     _("failed to acquire device lock held by pid %1% "
		       "(timeout after %2% seconds)") :
		     _("failed to release device lock held by pid %1% "
		       "(timeout after %2% seconds)"));
	  fmt %	status % timeout;
	  throw error(fmt);
	}
      unset_timer();
    }
  catch (error const& e)
    {
      unset_timer();
      throw;
    }
}

void
DeviceLock::unset_lock ()
{
  set_lock(LOCK_NONE, 0);
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
