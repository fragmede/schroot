/* Copyright © 2005-2007  Roger Leigh <rleigh@debian.org>
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

#ifndef SCHROOT_RELEASELOCK_MAIN_H
#define SCHROOT_RELEASELOCK_MAIN_H

#include <schroot-base/schroot-base-main.h>

#include <schroot-releaselock/schroot-releaselock-options.h>

#include <sbuild/sbuild-custom-error.h>

namespace schroot_releaselock
{

  /**
   * Frontend for schroot.  This class is used to "run" schroot.
   */
  class main : public schroot_base::main
  {
  public:
    /// Error codes.
    enum error_code
      {
	DEVICE_NOTBLOCK, ///< File is not a block device.
	DEVICE_OWNED,    ///< Failed to release device lock (lock held by PID).
	DEVICE_RELEASE,  ///< Failed to release device lock.
	DEVICE_STAT      ///< Failed to stat device.
      };

    /// Exception type.
    typedef sbuild::custom_error<error_code> error;

    /**
     * The constructor.
     *
     * @param options the command-line options to use.
     */
    main (options::ptr& options);

    /// The destructor.
    virtual ~main ();

    /**
     * Release lock.
     */
    virtual void
    action_releaselock ();

    /**
     * Run the program.
     *
     * @returns 0 on success, 1 on failure or the exit status of the
     * chroot command.
     */
    virtual int
    run_impl ();

    /// The program options.
    options::ptr opts;
  };

}

#endif /* SCHROOT_RELEASELOCK_MAIN_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
