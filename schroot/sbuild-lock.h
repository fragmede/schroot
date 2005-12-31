/* Copyright © 2005  Roger Leigh <rleigh@debian.org>
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

namespace sbuild
{

  /**
   * Advisory locking.  This class defines a simple interface for
   * shared and exclusive locks.
   */
  class Lock
  {
  public:
    /// Lock type.
    enum Type
      {
	LOCK_SHARED    = F_RDLCK, ///< A shared (read) lock.
	LOCK_EXCLUSIVE = F_WRLCK, ///< An exclusive (write) lock.
	LOCK_NONE      = F_UNLCK  ///< No lock.
      };

    /// Exception type.
    typedef runtime_error_custom<Lock> error;

    /**
     * Acquire a lock.
     *
     * @param lock_type the type of lock to acquire.
     * @param timeout the time in seconds to wait on the lock.
     */
    virtual void
    set_lock (Type         lock_type,
	      unsigned int timeout) = 0;

    /**
     * Release a lock.  This is equivalent to set_lock with a
     * lock_type of LOCK_NONE and a timeout of 0.
     */
    virtual void
    unset_lock () = 0;

  protected:
    /// The constructor.
    Lock();
    /// The destructor.
    virtual ~Lock();

    /**
     * Set the SIGALARM handler.
     *
     * An error will be thrown on failure.
     */
    void
    set_alarm ();

    /**
     * Restore the state of SIGALRM prior to starting lock
     * acquisition.
     */
    void
    clear_alarm ();

    /**
     * Set up an itimer for future expiry.  This is used to interrupt
     * system calls.  This will set a handler for SIGALRM as a side
     * effect (using set_alarm).
     *
     * An error will be thrown on failure.
     *
     * @param timer the timeout to set.
     */
    void
    set_timer(struct itimerval const& timer);

    /**
     * Remove any itimer currently set up.  This will clear any
     * SIGALRM handler (using clear_alarm).
     *
     * An error will be thrown on failure.
     */
    void
    unset_timer();

  private:
    /// Signals saved during timeout.
    struct sigaction saved_signals;
  };

  /**
   * File locks.  Simple whole-file shared and exclusive advisory
   * locking based upon POSIX fcntl byte region locks.
   */
  class FileLock : public Lock
  {
  public:
    /**
     * The constructor.
     *
     * @param fd the file descriptor to lock.
     */
    FileLock (int fd);

    /// The destructor.
    virtual ~FileLock();

    void
    set_lock (Type         lock_type,
	      unsigned int timeout);

    void
    unset_lock ();

  private:
    /// The file descriptor to lock.
    int fd;
  };

  /**
   * Set an advisory lock on a device.  The lock is acquired using
   * liblockdev lock_dev().  Note that a lock_type of LOCK_SHARED is
   * equivalent to LOCK_EXCLUSIVE, because this lock type does not
   * support shared locks.
   */
  class DeviceLock : public Lock
  {
  public:
    /**
     * The constructor.
     *
     * @param device the device to lock (full pathname).
     */
    DeviceLock (std::string const& device);
    virtual ~DeviceLock();

    void
    set_lock (Type         lock_type,
	      unsigned int timeout);

    void
    unset_lock ();

  private:
    /// The device to lock.
    std::string device;
  };

}

#endif /* SBUILD_LOCK_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
