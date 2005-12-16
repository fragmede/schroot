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

#include <unistd.h>
#include <fcntl.h>

#include <glib.h>

typedef enum
{
  SBUILD_LOCK_ERROR_SETUP,
  SBUILD_LOCK_ERROR_TIMEOUT,
  SBUILD_LOCK_ERROR_FAIL
} SbuildLockError;

#define SBUILD_LOCK_ERROR sbuild_lock_error_quark()

GQuark
sbuild_auth_error_quark (void);

typedef enum
{
  SBUILD_LOCK_SHARED    = F_RDLCK,
  SBUILD_LOCK_EXCLUSIVE = F_WRLCK,
  SBUILD_LOCK_NONE      = F_UNLCK
} SbuildLockType;

namespace sbuild
{
  class Lock
  {
    virtual bool
    set_lock (SbuildLockType   lock_type,
	      guint            timeout,
	      GError         **error) = 0;

    virtual bool
    unset_lock (GError **error) = 0;

  protected:
    Lock();
    virtual ~Lock();
  };

  class FileLock : public Lock
  {
  public:
    FileLock (int fd);
    virtual ~FileLock();

    bool
      set_lock (SbuildLockType   lock_type,
		guint            timeout,
		GError         **error);

    bool
      unset_lock (GError **error);

  private:
    int fd;
  };

  class DeviceLock : public Lock
  {
  public:
    DeviceLock (const std::string& device);
    virtual ~DeviceLock();

    bool
      set_lock (SbuildLockType   lock_type,
		guint            timeout,
		GError         **error);

    bool
      unset_lock (GError **error);

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
