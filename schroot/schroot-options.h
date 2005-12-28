/* schroot-options - schroot options parser
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

#ifndef SBUILD_SCHROOT_OPTIONS_H
#define SBUILD_SCHROOT_OPTIONS_H

#include <string>
#include <vector>

#include "sbuild-session.h"
#include "sbuild-types.h"

namespace schroot
{

  /**
   * schroot command-line options.
   */
  class Options
  {
  public:
    typedef std::vector<std::string> string_list;

    /**
     * The constructor.
     *
     * @param argc the number of arguments.
     * @param argv the list of arguments.
     */
    Options(int   argc,
	    char *argv[]);

    /// The destructor.
    virtual ~Options();

    /// Chroots to use.
    sbuild::string_list  chroots;
    /// Command to run.
    sbuild::string_list  command;
    /// User to run as.
    std::string          user;
    /// Preserve environment.
    bool                 preserve;
    /// Quiet messages.
    bool                 quiet;
    /// Verbose messages.
    bool                 verbose;
    /// List chroots.
    bool                 list;
    /// Display chroot information.
    bool                 info;
    /// Use all chroots and sessions.
    bool                 all;
    /// Use all chroots.
    bool                 all_chroots;
    /// Use all sessions.
    bool                 all_sessions;
    /// Load chroots.
    bool                 load_chroots;
    /// Load sessions.
    bool                 load_sessions;
    /// Display version information.
    bool                 version;
    /// Session operation to perform.
    sbuild::Session::Operation  session_operation;
    /// Force session operations.
    bool                 session_force;

  private:
    /**
     * Set session operation.  This detects if an operation has
     * already been set (only one operation may be specified at once).
     *
     * @param operation the operation to set.
     */
    void
    set_session_operation (sbuild::Session::Operation operation);
  };

}

#endif /* SBUILD_SCHROOT_OPTIONS_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */

