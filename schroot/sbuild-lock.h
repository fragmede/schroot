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

#ifndef SBUILD_LOCK_H
#define SBUILD_LOCK_H

#include <string>

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include "sbuild-error.h"

GQuark
sbuild_auth_error_quark (void);


namespace sbuild
{

  class Lock
  {
  public:
    typedef enum
      {
	LOCK_SHARED    = F_RDLCK,
	LOCK_EXCLUSIVE = F_WRLCK,
	LOCK_NONE      = F_UNLCK
      } Type;

    typedef enum
      {
	LOCK_ERROR_SETUP,
	LOCK_ERROR_TIMEOUT,
	LOCK_ERROR_FAIL
      } ErrorCode;

    typedef Exception<ErrorCode> error;

    virtual void
    set_lock (Type  lock_type,
	      guint timeout) = 0;

    virtual void
    unset_lock () = 0;

  protected:
    Lock();
    virtual ~Lock();

    void
    set_alarm ();

    void
    clear_alarm ();

    void
    set_timer(struct itimerval const& timer);

    void
    unset_timer();

  private:
    struct sigaction saved_signals;
  };

  class FileLock : public Lock
  {
  public:
    FileLock (int fd);
    virtual ~FileLock();

    void
    set_lock (Type  lock_type,
	      guint timeout);

    void
    unset_lock ();

  private:
    int fd;
  };

  class DeviceLock : public Lock
  {
  public:
    DeviceLock (const std::string& device);
    virtual ~DeviceLock();

    void
    set_lock (Type  lock_type,
	      guint timeout);

    void
    unset_lock ();

  private:
    std::string device;
  };

}

#endif /* SBUILD_LOCK_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
