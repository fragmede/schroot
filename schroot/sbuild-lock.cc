/* sbuild-lock - sbuild advisory locking
 *
 * Copyright © 2005  Roger Leigh <rleigh@debian.org>
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

/**
 * SECTION:sbuild-lock
 * @short_description: advisory locking
 * @title: Advisory Locking
 *
 * These functions implement simple whole-file shared and exclusive
 * advisory locking based upon POSIX fcntl byte region locks.
 */

#include <config.h>

#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#include <lockdev.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "sbuild-lock.h"

/**
 * sbuild_lock_error_quark:
 *
 * Get the SBUILD_LOCK_ERROR domain number.
 *
 * Returns the domain.
 */
GQuark
sbuild_lock_error_quark (void)
{
  static GQuark error_quark = 0;

  if (error_quark == 0)
    error_quark = g_quark_from_static_string ("sbuild-lock-error-quark");

  return error_quark;
}

namespace
{

  volatile bool lock_timeout = false;

  /**
   * sbuild_lock_alarm_handler:
   * @ignore: the signal number.
   *
   * Handle the SIGALRM signal.
   */
  static void
  sbuild_lock_alarm_handler (int ignore)
  {
    /* This exists so that system calls get interrupted. */
    /* lock_timeout is used for polling for a timeout, rather than
       interruption. */
    lock_timeout = true;
  }

  /**
   * sbuild_lock_clear_alarm:
   * @orig_sa: the original signal handler.
   *
   * Restore the state of SIGALRM prior to starting lock acquisition.
   */
  static void
  sbuild_lock_clear_alarm (struct sigaction *orig_sa)
  {
    /* Restore original handler */
    sigaction (SIGALRM, orig_sa, NULL);
  }

  /**
   * sbuild_lock_set_alarm:
   * @orig_sa: the original signal handler
   *
   * Set the SIGALARM handler.  The old signal handler is stored in
   * @orig_sa.
   */
  static gboolean
  sbuild_lock_set_alarm (struct sigaction *orig_sa)
  {
    struct sigaction new_sa;
    sigemptyset(&new_sa.sa_mask);
    new_sa.sa_flags = 0;
    new_sa.sa_handler = sbuild_lock_alarm_handler;

    if (sigaction(SIGALRM, &new_sa, orig_sa) != 0)
      {
	return FALSE;
      }

    return TRUE;
  }

}

namespace sbuild
{

  Lock::Lock()
  {
  }

  Lock::~Lock()
  {
  }

  FileLock::FileLock (int fd):
    Lock(),
    fd(fd)
  {
  }

  FileLock::~FileLock()
  {
  }

  /**
   * sbuild_lock_set_lock:
   * @fd: the file descriptor to lock.
   * @lock_type: the type of lock to set.
   * @timeout: the time in seconds to wait for the lock.
   * @error: a #GError.
   *
   * Set an advisory lock on a file.  A byte region lock is placed on
   * the entire file, regardless of size, using fcntl.
   *
   * Returns TRUE on success, FALSE on failure (@error will be set to
   * indicate the cause of the failure).
   */
  bool
  FileLock::set_lock (SbuildLockType   lock_type,
		      guint            timeout,
		      GError         **error)
  {
    struct flock read_lock =
      {
	lock_type,
	SEEK_SET,
	0,
	0 // Lock entire file
      };

    struct itimerval timeout_timer;
    timeout_timer.it_interval.tv_sec = timeout_timer.it_interval.tv_usec = 0;
    timeout_timer.it_value.tv_sec = timeout;
    timeout_timer.it_value.tv_usec = 0;

    struct itimerval disable_timer;
    disable_timer.it_interval.tv_sec = disable_timer.it_interval.tv_usec = 0;
    disable_timer.it_value.tv_sec = disable_timer.it_value.tv_usec = 0;

    struct sigaction saved_signals;
    if (sbuild_lock_set_alarm(&saved_signals) == FALSE)
      {
	g_set_error(error,
		    SBUILD_LOCK_ERROR, SBUILD_LOCK_ERROR_SETUP,
		    _("failed to set timeout handler: %s\n"),
		    g_strerror(errno));
	return FALSE;
      }

    if (setitimer(ITIMER_REAL, &timeout_timer, NULL) == -1)
      {
	g_set_error(error,
		    SBUILD_LOCK_ERROR, SBUILD_LOCK_ERROR_SETUP,
		    _("failed to set timeout: %s\n"),
		    g_strerror(errno));
	return FALSE;
      }

    /* Now the signal handler and itimer are set, the function can't
       return without stopping the timer and restoring the signal
       handler to its original state. */

    /* Wait on lock until interrupted by a signal if a timeout was set,
       otherwise return immediately. */
    if (fcntl(this->fd,
	      (timeout != 0) ? F_SETLKW : F_SETLK,
	      &read_lock) == -1)
      {
	if (errno == EINTR)
	  g_set_error(error,
		      SBUILD_LOCK_ERROR, SBUILD_LOCK_ERROR_TIMEOUT,
		      _("failed to acquire lock (timeout after %u seconds)\n"), timeout);
	else
	  g_set_error(error,
		      SBUILD_LOCK_ERROR, SBUILD_LOCK_ERROR_FAIL,
		      _("failed to acquire lock: %s\n"), g_strerror(errno));
      }

    if (setitimer(ITIMER_REAL, &disable_timer, NULL) == -1)
      {
	if (*error == NULL) /* Don't set error if already set. */
	  g_set_error(error,
		      SBUILD_LOCK_ERROR, SBUILD_LOCK_ERROR_SETUP,
		      _("failed to unset timeout: %s\n"),
		      g_strerror(errno));
      }

    sbuild_lock_clear_alarm(&saved_signals);

    if (*error)
      return FALSE;
    else
      return TRUE;
  }

  /**
   * sbuild_lock_unset_lock:
   * @fd: the file descriptor to unlock.
   * @error: a #GError.
   *
   * Remove an advisory lock on a file.  This is equivalent to calling
   * sbuild_lock_set_lock with a lock type of SBUILD_LOCK_NONE and a
   * timeout of 0.
   *
   * Returns TRUE on success, FALSE on failure (@error will be set to
   * indicate the cause of the failure).
   */
  bool
  FileLock::unset_lock (GError **error)
  {
    return set_lock(SBUILD_LOCK_NONE, 0, error);
  }

  DeviceLock::DeviceLock (const std::string& device):
    Lock(),
    device(device)
  {
  }

  DeviceLock::~DeviceLock()
  {
  }

  /**
   * sbuild_lock_set_device_lock:
   * @device: the device to lock (full pathname).
   * @lock_type: the type of lock to set.
   * @timeout: the time in seconds to wait for the lock.
   * @error: a #GError.
   *
   * Set an advisory lock on a device.  The lock is acquired using
   * liblockdev lock_dev().  Note that a @lock_type of
   * SBUILD_LOCK_SHARED is equivalent to SBUILD_LOCK_EXCLUSIVE, because
   * this lock type does not support shared locks.
   *
   * Returns TRUE on success, FALSE on failure (@error will be set to
   * indicate the cause of the failure).
   */
  bool
  DeviceLock::set_lock (SbuildLockType    lock_type,
			guint            timeout,
			GError         **error)
  {
    struct itimerval timeout_timer;
    timeout_timer.it_interval.tv_sec = timeout_timer.it_interval.tv_usec = 0;
    timeout_timer.it_value.tv_sec = timeout;
    timeout_timer.it_value.tv_usec = 0;

    struct itimerval disable_timer;
    disable_timer.it_interval.tv_sec = disable_timer.it_interval.tv_usec = 0;
    disable_timer.it_value.tv_sec = disable_timer.it_value.tv_usec = 0;

    lock_timeout = false;

    struct sigaction saved_signals;
    if (sbuild_lock_set_alarm(&saved_signals) == FALSE)
      {
	g_set_error(error,
		    SBUILD_LOCK_ERROR, SBUILD_LOCK_ERROR_SETUP,
		    _("failed to set timeout handler: %s\n"),
		    g_strerror(errno));
	return FALSE;
      }

    if (setitimer(ITIMER_REAL, &timeout_timer, NULL) == -1)
      {
	g_set_error(error,
		    SBUILD_LOCK_ERROR, SBUILD_LOCK_ERROR_SETUP,
		    _("failed to set timeout: %s\n"),
		    g_strerror(errno));
	return FALSE;
      }

    /* Now the signal handler and itimer are set, the function can't
       return without stopping the timer and restoring the signal
       handler to its original state. */

    /* Wait on lock until interrupted by a signal if a timeout was set,
       otherwise return immediately. */
    pid_t status = 0;
    while (lock_timeout == false)
      {
	if (lock_type == SBUILD_LOCK_SHARED || lock_type == SBUILD_LOCK_EXCLUSIVE)
	  {
	    status = dev_lock(this->device.c_str());
	    if (status == 0) // Success
	      break;
	    else if (status < 0) // Failure
	      {
		g_set_error(error,
			    SBUILD_LOCK_ERROR, SBUILD_LOCK_ERROR_FAIL,
			    _("failed to acquire device lock"));
		break;
	      }
	  }
	else
	  {
	    pid_t cur_lock_pid = dev_testlock(this->device.c_str());
	    if (cur_lock_pid < 0) // Test failure
	      {
		g_set_error(error,
			    SBUILD_LOCK_ERROR, SBUILD_LOCK_ERROR_FAIL,
			    _("failed to test device lock"));
		break;
	      }
	    else if (cur_lock_pid > 0 && cur_lock_pid != getpid()) // Another process owns the lock
	      {
		g_set_error(error,
			    SBUILD_LOCK_ERROR, SBUILD_LOCK_ERROR_FAIL,
			    _("failed to release device lock held by pid %d"),
			    cur_lock_pid);
		break;
	      }
	    status = dev_unlock(this->device.c_str(), getpid());
	    if (status == 0) // Success
	      break;
	    else if (status < 0) // Failure
	      {
		g_set_error(error,
			    SBUILD_LOCK_ERROR, SBUILD_LOCK_ERROR_FAIL,
			    _("failed to release device lock"));
		break;
	      }
	  }
      }

    if (lock_timeout)
      {
	g_set_error(error,
		    SBUILD_LOCK_ERROR, SBUILD_LOCK_ERROR_TIMEOUT,
		    (lock_type == SBUILD_LOCK_SHARED ||
		     lock_type == SBUILD_LOCK_EXCLUSIVE) ?
		    _("failed to acquire device lock held by pid %d "
		      "(timeout after %u seconds)\n") :
		    _("failed to release device lock held by pid %d "
		      "(timeout after %u seconds)\n"),
		    status, timeout);
      }

    if (setitimer(ITIMER_REAL, &disable_timer, NULL) == -1)
      {
	if (*error == NULL) /* Don't set error if already set. */
	  g_set_error(error,
		      SBUILD_LOCK_ERROR, SBUILD_LOCK_ERROR_SETUP,
		      _("failed to unset timeout: %s\n"),
		      g_strerror(errno));
      }

    sbuild_lock_clear_alarm(&saved_signals);

    if (*error)
      return FALSE;
    else
      return TRUE;
  }

  /**
   * sbuild_lock_unset_device_lock:
   * @device: the device to unlock (full pathname).
   * @error: a #GError.
   *
   * Remove an advisory lock on a device.  This is equivalent to calling
   * sbuild_lock_set_lock with a lock type of SBUILD_LOCK_NONE and a
   * timeout of 0.
   *
   * Returns TRUE on success, FALSE on failure (@error will be set to
   * indicate the cause of the failure).
   */
  bool
  DeviceLock::unset_lock (GError **error)
  {
    return set_lock(SBUILD_LOCK_NONE, 0, error);
  }

}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
