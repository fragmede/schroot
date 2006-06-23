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

#ifndef SCHROOT_OPTIONS_BASE_H
#define SCHROOT_OPTIONS_BASE_H

#include <string>
#include <vector>

#ifdef HAVE_TR1_MEMORY
#include <tr1/memory>
#elif HAVE_BOOST_SHARED_PTR_HPP
#include <boost/shared_ptr.hpp>
namespace std { namespace tr1 { using boost::shared_ptr; } }
#else
#error A shared_ptr implementation is not available
#endif

#include <boost/program_options.hpp>

#include "sbuild-session.h"
#include "sbuild-types.h"

namespace schroot
{

  /**
   * Basic schroot command-line options.  This is specialised by the
   * frontends to suit their particular command-line options and
   * behaviour.
   */
  class options_base
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

    /// A shared_ptr to an options_base object.
    typedef std::tr1::shared_ptr<options_base> ptr;

    /**
     * The constructor.
     *
     * @param argc the number of arguments.
     * @param argv the list of arguments.
     * @param compat the compatibility mode.
     */
    options_base (int                 argc,
		  char               *argv[]);

    /// The destructor.
    virtual ~options_base ();

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

  protected:
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

    virtual void
    add_options ();

    virtual void
    parse_options (int   argc,
		   char *argv[]);

    virtual void
    check_options ();

    virtual void
    check_actions ();

    boost::program_options::options_description            general;
    boost::program_options::options_description            chroot;
    boost::program_options::options_description            chrootenv;
    boost::program_options::options_description            session;
    boost::program_options::options_description            hidden;
    boost::program_options::positional_options_description positional;
    boost::program_options::options_description            visible;
    boost::program_options::options_description            global;
    boost::program_options::variables_map                  vm;
  };

}

#endif /* SCHROOT_OPTIONS_BASE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */

