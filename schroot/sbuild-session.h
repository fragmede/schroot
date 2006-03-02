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

#ifndef SBUILD_SESSION_H
#define SBUILD_SESSION_H

#include <string>

#include <sys/types.h>
#include <sys/wait.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#include "sbuild-auth.h"
#include "sbuild-chroot-config.h"
#include "sbuild-error.h"

namespace sbuild
{

  /**
   * Session handler.
   *
   * This class provides the session handling for schroot.  It derives
   * from auth, which performs all the necessary PAM actions,
   * specialising it by overriding its virtual functions.  This allows
   * more sophisticated handling of user authorisation (groups and
   * root-groups membership in the configuration file) and session
   * management (setting up the session, entering the chroot and
   * running the requested command or shell).
   */
  class session : public auth
  {
  public:
    /// Session operations.
    enum operation
      {
	OPERATION_AUTOMATIC, ///< Begin, end and run a session automatically.
	OPERATION_BEGIN,     ///< Begin a session.
	OPERATION_RECOVER,   ///< Recover an existing (but inactive) session.
	OPERATION_END,       ///< End a session.
	OPERATION_RUN        ///< Run a command in an existing session.
      };

    /// Exception type.
    typedef runtime_error_custom<session> error;

    /// A shared_ptr to a chroot_config object.
    typedef std::tr1::shared_ptr<chroot_config> config_ptr;

    /// A shared_ptr to a session object.
    typedef std::tr1::shared_ptr<session> ptr;

    /**
     * The constructor.
     *
     * @param service the PAM service name.
     * @param config a shared_ptr to the chroot configuration.
     * @param operation the session operation to perform.
     * @param chroots the chroots to act upon.
     */
    session (std::string const& service,
	     config_ptr&        config,
	     operation          operation,
	     string_list const& chroots);

    /// The destructor.
    virtual ~session ();

    /**
     * Get the configuration associated with this session.
     *
     * @returns a shared_ptr to the configuration.
     */
    config_ptr&
    get_config ();

    /**
     * Set the configuration associated with this session.
     *
     * @param config a shared_ptr to the configuration.
     */
    void
    set_config (config_ptr& config);

    /**
     * Get the chroots to use in this session.
     *
     * @returns a list of chroots.
     */
    string_list const&
    get_chroots () const;

    /**
     * Set the chroots to use in this session.
     *
     * @param chroots a list of chroots.
     */
    void
    set_chroots (string_list const& chroots);

    /**
     * Get the operation this session will perform.
     *
     * @returns the operation.
     */
    operation
    get_operation () const;

    /**
     * Set the operation this session will perform.
     *
     * @param operation the operation.
     */
    void
    set_operation (operation operation);

    /**
     * Get the session identifier.  The session identifier is a unique
     * string to identify a session.
     *
     * @returns the session id.
     */
    std::string const&
    get_session_id () const;

    /**
     * Set the session identifier.  The session identifier is a unique
     * string to identify a session.
     *
     * @param session_id the session id.
     */
    void
    set_session_id (std::string const& session_id);

    /**
     * Get the force status of this session.
     *
     * @returns true if operation will be forced, otherwise false.
     */
    bool
    get_force () const;

    /**
     * Set the force status of this session.
     *
     * @param force true to force operation, otherwise false.
     */
    void
    set_force (bool force);

    /**
     * Get the exit (wait) status of the last child process to run in this
     * session.
     *
     * @returns the exit status.
     */
    int
    get_child_status () const;

    /**
     * Check if authentication is required, taking groups and
     * root-groups membership or all chroots specified into account.
     */
    virtual sbuild::auth::status
    get_auth_status () const;

    /**
     * Run a session.  If a command has been specified, this will be
     * run in each of the specified chroots.  If no command has been
     * specified, a login shell will run in the specified chroot.
     *
     * An error will be thrown on failure.
     */
    virtual void
    run_impl ();

  private:
    /**
     * execve wrapper.  Run the command specified by file (an absolute
     * pathname), using command and env as the argv and environment,
     * respectively.
     *
     * @param file the program to execute.
     * @param command the arguments to pass to the executable.
     * @param env the environment.
     * @returns the return value of the execve system call on failure.
     */
    int
    exec (std::string const& file,
	  string_list const& command,
	  environment const& env);
    /**
     * Setup a chroot.  This runs all of the commands in setup.d or run.d.
     *
     * The environment variables CHROOT_NAME, CHROOT_DESCRIPTION,
     * CHROOT_LOCATION, AUTH_USER and AUTH_VERBOSITY are set for use in
     * setup scripts.  See schroot-setup(5) for a complete list.
     *
     * An error will be thrown on failure.
     *
     * @param session_chroot the chroot to setup.  This must be
     * present in the chroot list and the chroot configuration object.
     * @param setup_type the type of setup to perform.
     */
    void
    setup_chroot (chroot::ptr&       session_chroot,
		  chroot::setup_type setup_type);

    /**
     * Run command or login shell in the specified chroot.
     *
     * An error will be thrown on failure.
     *
     * @param session_chroot the chroot to setup.  This must be
     * present in the chroot list and the chroot configuration object.
     */
    void
    run_chroot (chroot::ptr& session_chroot);

    /**
     * Run a command or login shell as a child process in the
     * specified chroot.  This method is only ever to be run in a
     * child process, and will never return.
     *
     * @param session_chroot the chroot to setup.  This must be
     * present in the chroot list and the chroot configuration object.
     */
    void
    run_child (chroot::ptr& session_chroot);

    /**
     * Wait for a child process to complete, and check its exit status.
     *
     * An error will be thrown on failure.
     *
     * @param pid the pid to wait for.
     * @param child_status the place to store the child exit status.
     */
    void
    wait_for_child (int  pid,
		    int& child_status);

    /**
     * Set the SIGHUP handler.
     *
     * An error will be thrown on failure.
     */
    void
    set_sighup_handler ();

    /**
     * Restore the state of SIGHUP prior to setting the handler.
     */
    void
    clear_sighup_handler ();

    /// The chroot configuration.
    config_ptr       config;
    /// The chroots to run the session operation in.
    string_list      chroots;
    /// The current chroot status.
    int              chroot_status;
    /// The child exit status.
    int              child_status;
    /// The session operation to perform.
    operation        session_operation;
    /// The session identifier.
    std::string      session_id;
    /// The session force status.
    bool             force;
    /// Signals saved while sighup handler is set.
    struct sigaction saved_signals;
  };

}

#endif /* SBUILD_SESSION_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
