/* Copyright © 2005-2013  Roger Leigh <rleigh@debian.org>
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

#include <csbuild/options.h>

#include <bin-common/main.h>

namespace csbuild
{

  /**
   * Frontend for schroot.  This class is used to "run" schroot.
   */
  class main : public bin_common::main
  {
  public:
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
