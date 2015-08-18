/* Copyright © 2005-2007  Roger Leigh <rleigh@codelibre.net>
 *
 * schroot is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * schroot is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 *********************************************************************/

#ifndef CSBUILD_MAIN_H
#define CSBUILD_MAIN_H

#include <csbuild/csbuild-options.h>

#include <schroot-base/schroot-base-main.h>

#include <sbuild/sbuild-custom-error.h>

namespace csbuild
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
     * Build packages.
     */
    virtual void
    action_build ();

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

#endif /* CSBUILD_MAIN_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
