/* Copyright © 2005-2007  Roger Leigh <rleigh@debian.org>
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

#ifndef SCHROOT_MOUNT_OPTIONS_H
#define SCHROOT_MOUNT_OPTIONS_H

#include <schroot-base/schroot-base-options.h>

#include <string>

namespace schroot_mount
{

  /**
   * schroot-mount command-line options.
   */
  class options : public schroot_base::options
  {
  public:
    /// A shared_ptr to an options object.
    typedef std::tr1::shared_ptr<options> ptr;

    /// Begin, run and end a session.
    static const action_type ACTION_MOUNT;

    /// The constructor.
    options ();

    /// The destructor.
    virtual ~options ();

    /// Dry run.
    bool dry_run;

    /// The fstab to read.
    std::string fstab;

    /// The mountpoint to check.
    std::string mountpoint;

  protected:
    virtual void
    add_options ();

    virtual void
    add_option_groups ();

    virtual void
    check_options ();

    /// Mount options group.
    boost::program_options::options_description mount;
  };

}

#endif /* SCHROOT_MOUNT_OPTIONS_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
