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

#ifndef SCHROOT_LISTMOUNTS_MAIN_H
#define SCHROOT_LISTMOUNTS_MAIN_H

#include <schroot/schroot-base-main.h>
#include <schroot/schroot-listmounts-options.h>

namespace schroot_listmounts
{

  /**
   * Frontend for schroot.  This class is used to "run" schroot.
   */
  class main : public schroot_base::main
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
     * List mounts.
     *
     * @param mountfile the file containing the database of mounted filesystems.
     * @param mountpoint the mount point to check for.
     */
    sbuild::string_list
    list_mounts (std::string const& mountfile) const;

    /**
     * Release lock.
     *
     * @param stream the stream to output to.
     */
    virtual void
    action_listmounts ();

    /**
     * Run the program.
     *
     * @returns 0 on success, 1 on failure or the exit status of the
     * chroot command.
     */
    virtual int
    run_impl ();

    /// The program options.
    options::ptr options;
  };

}

#endif /* SCHROOT_LISTMOUNTS_MAIN_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
