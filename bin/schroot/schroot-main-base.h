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

#ifndef SCHROOT_MAIN_BASE_H
#define SCHROOT_MAIN_BASE_H

#include <schroot-base/schroot-base-main.h>
#include <schroot/schroot-options-base.h>

#include <sbuild/sbuild-chroot-config.h>
#include <sbuild/sbuild-custom-error.h>

namespace schroot
{

  /**
   * Frontend base for schroot programs.  This class is used to "run"
   * schroot programs.  This class contains functionality common to
   * all schroot programs (schroot, dchroot, dchroot-dsa).
   */
  class main_base : public schroot_base::main
  {
  public:
    /// Error codes.
    enum error_code
      {
	CHROOTS_NOTFOUND,  ///< Chroots not found.
	CHROOT_FILE,       ///< No chroots are defined in ....
	CHROOT_FILE2,      ///< No chroots are defined in ... or ....
	CHROOT_NOTDEFINED, ///< The specified chroots are not defined.
	CHROOT_NOTFOUND,   ///< Chroot not found.
	SESSION_INVALID    ///< Invalid session name.
      };

    /// Exception type.
    typedef sbuild::custom_error<error_code> error;

    /**
     * The constructor.
     *
     * @param program_name the program name.
     * @param program_usage the program usage message.
     * @param options the command-line options to use.
     * @param use_syslog whether to open a connection to the system
     * logger.
     */
    main_base (std::string const& program_name,
	       std::string const& program_usage,
	       options_base::ptr& options,
	       bool               use_syslog);

    /// The destructor.
    virtual ~main_base ();

    virtual void
    action_version (std::ostream& stream);

    /**
     * List chroots.
     */
    virtual void
    action_list () = 0;

    /**
     * Print detailed information about chroots.
     */
    virtual void
    action_info ();

    /**
     * Print location of chroots.
     */
    virtual void
    action_location ();

    /**
     * Dump configuration file for chroots.
     */
    virtual void
    action_config () = 0;

  protected:
    /**
     * Run the program.  This is the program-specific run method which
     * must be implemented in a derived class.
     *
     * @returns 0 on success, 1 on failure or the exit status of the
     * chroot command.
     */
    virtual int
    run_impl ();

    /**
     * Get a list of chroots based on the specified options (--all, --chroot).
     *
     * @returns a list of chroots.
     */
    virtual sbuild::string_list
    get_chroot_options ();

    /**
     * Check compatibility.  Does nothing, but derived classes may use
     * it as they see fit.
     */
    virtual void
    compat_check ();

    /**
     * Load configuration.
     */
    virtual void
    load_config ();

    /**
     * Create a session.  This sets the session member.
     *
     * @param sess_op the session operation to perform.
     */
    virtual void
    create_session (sbuild::session::operation sess_op) = 0;

    /**
     * Add PAM authentication handler to the session.
     */
    virtual void
    add_session_auth ();

  protected:
    /// The program options.
    options_base::ptr            options;
    /// The chroot configuration.
    sbuild::chroot_config::ptr   config;
    /// The chroots to use.
    sbuild::string_list          chroots;
    /// The chroots to use.
    sbuild::session::chroot_list chroot_objects;
    /// The session.
    sbuild::session::ptr         session;
  };

}

#endif /* SCHROOT_MAIN_BASE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
