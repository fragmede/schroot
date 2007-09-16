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

#ifndef SCHROOT_OPTIONS_BASE_H
#define SCHROOT_OPTIONS_BASE_H

#include <sbuild/sbuild-session.h>
#include <sbuild/sbuild-types.h>

#include <schroot-base/schroot-base-options.h>

#include <string>

#ifdef HAVE_TR1_MEMORY
#include <tr1/memory>
#elif HAVE_BOOST_SHARED_PTR_HPP
#include <boost/shared_ptr.hpp>
namespace std { namespace tr1 { using boost::shared_ptr; } }
#else
#error A shared_ptr implementation is not available
#endif

#include <boost/program_options.hpp>

namespace schroot
{

  /**
   * Basic schroot command-line options.  This is specialised by the
   * frontends to suit their particular command-line options and
   * behaviour.  This class contains functionality and options common
   * to all schroot programs (schroot, dchroot, dchroot-dsa).
   */
  class options_base : public schroot_base::options
  {
  public:
    /// Begin, run and end a session.
    static const action_type ACTION_SESSION_AUTO;
    /// Begin a session.
    static const action_type ACTION_SESSION_BEGIN;
    /// Recover an existing session.
    static const action_type ACTION_SESSION_RECOVER;
    /// Run an existing session.
    static const action_type ACTION_SESSION_RUN;
    /// End an existing session.
    static const action_type ACTION_SESSION_END;
    /// Display a list of chroots.
    static const action_type ACTION_LIST;
    /// Display chroot information.
    static const action_type ACTION_INFO;
    /// Display chroot location information.
    static const action_type ACTION_LOCATION;
    /// Display chroot configuration.
    static const action_type ACTION_CONFIG;

    /// A shared_ptr to an options_base object.
    typedef std::tr1::shared_ptr<options_base> ptr;

    /// The constructor.
    options_base ();

    /// The destructor.
    virtual ~options_base ();

    /// Chroots to use.
    sbuild::string_list  chroots;
    /// Chroot to print path.
    std::string          chroot_path;
    /// Command to run.
    sbuild::string_list  command;
    /// Directory to use.
    std::string          directory;
    /// User to run as.
    std::string          user;
    /// Preserve environment.
    bool                 preserve;
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
    /// Session name.
    std::string          session_name;
    /// Force session operations.
    bool                 session_force;

  protected:
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
    add_option_groups ();

    virtual void
    check_options ();

    virtual void
    check_actions ();

    /// Chroot options group.
    boost::program_options::options_description chroot;
    /// Chroot environment options group.
    boost::program_options::options_description chrootenv;
    /// Session actions group.
    boost::program_options::options_description session_actions;
    /// Session options group.
    boost::program_options::options_description session_options;
  };

}

#endif /* SCHROOT_OPTIONS_BASE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */

