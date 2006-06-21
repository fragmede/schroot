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

#ifndef SCHROOT_OPTIONS_H
#define SCHROOT_OPTIONS_H

#include <string>
#include <vector>

#include "sbuild-session.h"
#include "sbuild-types.h"

namespace schroot
{

  /**
   * schroot command-line options.
   */
  class options
  {
  public:
    /// The action to perform.
    enum action_type
      {
	ACTION_SESSION_AUTO,    ///< Begin, run and end a session.
	ACTION_SESSION_BEGIN,   ///< Begin a session.
	ACTION_SESSION_RECOVER, ///< Recover an existing session.
	ACTION_SESSION_RUN,     ///< Run an existing session.
	ACTION_SESSION_END,     ///< End an existing session.
	ACTION_VERSION,         ///< Display program version.
	ACTION_LIST,            ///< Display a list of chroots.
	ACTION_INFO,            ///< Display chroot information.
	ACTION_LOCATION,        ///< Display chroot location information.
	ACTION_CONFIG           ///< Display chroot configuration.
      };

    enum compatibility_type
      {
	COMPAT_SCHROOT,    ///< schroot compatibility.
	COMPAT_DCHROOT,    ///< dchroot compatibility.
	COMPAT_DCHROOT_DSA ///< dchroot-dsa compatibility.
      };

    /**
     * The constructor.
     *
     * @param argc the number of arguments.
     * @param argv the list of arguments.
     */
    options (int   argc,
	     char *argv[]);

    /// The destructor.
    virtual ~options ();

    /// The action to perform.
    action_type          action;
    /// Chroots to use.
    sbuild::string_list  chroots;
    /// Chroot to print path.
    std::string          chroot_path;
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
    /// Force session operations.
    bool                 session_force;
    /// dchroot(1) compatibility mode.
    compatibility_type   compat;

  private:
    /**
     * Set action.  This detects if an action has already been set
     * (only one action may be specified at once).
     *
     * @param action the action to set.
     */
    void
    set_action (action_type action);

    /**
     * Check if any of the --all options have been used.
     *
     * @returns true if any of the options have been used, otherwise
     * false.
     */
    bool
    all_used () const
    {
      return (this->all || this->all_chroots || this->all_sessions);
    }

  };

}

#endif /* SCHROOT_OPTIONS_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */

