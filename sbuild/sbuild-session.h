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

#ifndef SBUILD_SESSION_H
#define SBUILD_SESSION_H

#include <sbuild/sbuild-auth.h>
#include <sbuild/sbuild-chroot-config.h>
#include <sbuild/sbuild-custom-error.h>

#include <string>

#include <signal.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

namespace sbuild
{

  /**
   * Session handler.
   *
   * This class provides the session handling for schroot.  It uses
   * auth, which performs all the necessary PAM actions.  This allows
   * more sophisticated handling of user authorisation (users, groups,
   * root-users and root-groups membership in the configuration file)
   * and session management (setting up the session, entering the
   * chroot and running the requested command or shell).
   */
  class session
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

    /// Error codes.
    enum error_code
      {
	CHDIR,          ///< Failed to change to directory.
	CHDIR_FB,       ///< Falling back to directory.
	CHILD_CORE,     ///< Child dumped core.
	CHILD_FAIL,     ///< Child exited abnormally (reason unknown)
	CHILD_FORK,     ///< Failed to fork child.
	CHILD_SIGNAL,   ///< Child terminated by signal.
	CHILD_WAIT,     ///< Wait for child failed.
	CHROOT,         ///< Failed to change root to directory.
	CHROOT_ALIAS,   ///< No chroot found matching alias.
	CHROOT_LOCK,    ///< Failed to lock chroot.
	CHROOT_SETUP,   ///< Setup failed.
	CHROOT_UNKNOWN, ///< Failed to find chroot.
	CHROOT_UNLOCK,  ///< Failed to unlock chroot.
	COMMAND_ABS,    ///< Command must have an absolute path.
	EXEC,           ///< Failed to execute.
	GROUP_GET_SUP,  ///< Failed to get supplementary groups.
	GROUP_GET_SUPC, ///< Failed to get supplementary group count
	GROUP_SET,      ///< Failed to set group.
	GROUP_SET_SUP,  ///< Failed to set supplementary groups.
	GROUP_UNKNOWN,  ///< Group not found.
	PAM,            ///< PAM error.
	ROOT_DROP,      ///< Failed to drop root permissions.
        SET_SESSION_ID, ///< Chroot does not support setting a session ID.
	SHELL,          ///< Shell not available.
	SHELL_FB,       ///< Falling back to shell.
	SIGNAL_CATCH,   ///< Caught signal.
	SIGNAL_SET,     ///< Failed to set signal handler.
	USER_SET,       ///< Failed to set user.
	USER_SWITCH     ///< User switching is not permitted.
      };

    /// Exception type.
    typedef custom_error<error_code> error;

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
     * Get the authentication state associated with this session.
     *
     * @returns a shared_ptr to the authentication state.
     */
    auth::ptr const&
    get_auth () const;

    /**
     * Set the authentication state associated with this session.
     *
     * @param auth a shared_ptr to the authentication state.
     */
    void
    set_auth (auth::ptr& auth);

    /**
     * Get the configuration associated with this session.
     *
     * @returns a shared_ptr to the configuration.
     */
    config_ptr const&
    get_config () const;

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
     * Save terminal state.
     */
    void
    save_termios ();

    /**
     * Restore terminal state.
     */
    void
    restore_termios ();

    /**
     * Get the exit (wait) status of the last child process to run in this
     * session.
     *
     * @returns the exit status.
     */
    int
    get_child_status () const;

  protected:
    /**
     * Get the chroot authentication properties the user is included in.
     */
    void
    get_chroot_membership (chroot::ptr const& chroot,
			   bool&              in_users,
			   bool&              in_root_users,
			   bool&              in_groups,
			   bool&              in_root_groups) const;

    /**
     * Check if authentication is required for a single chroot, taking
     * users, groups, root-users and root-groups membership into
     * account.
     */
    virtual auth::status
    get_chroot_auth_status (auth::status status,
			    chroot::ptr const& chroot) const;

  public:
    /**
     * Check if authentication is required, taking users, groups,
     * root-users and root-groups membership of all chroots specified
     * into account.
     */
    virtual sbuild::auth::status
    get_auth_status () const;

    /**
     * Run a session.  The user will be asked for authentication if
     * required, and then the run_impl virtual method will be called.
     *
     * An auth::error will be thrown on failure.
     */
    void
    run ();

  protected:
    /**
     * Run a session.  If a command has been specified, this will be
     * run in each of the specified chroots.  If no command has been
     * specified, a login shell will run in the specified chroot.
     *
     * An error will be thrown on failure.
     */
    virtual void
    run_impl ();

    /**
     * Get a list of directories to change to when running a login
     * shell.  Multiple directories are used as fallbacks.
     *
     * @returns a list of directories
     */
    virtual string_list
    get_login_directories () const;

    /**
     * Get a list of directories to change to when running a command
     * Multiple directories are used as fallbacks.
     *
     * @returns a list of directories
     */
    virtual string_list
    get_command_directories () const;

    /**
     * Get the shell to run.  This finds a suitable shell to run in
     * the chroot, falling back to /bin/sh if necessary.  Note that it
     * assumes it is inside the chroot when called.
     *
     * @returns the shell.
     */
    virtual std::string
    get_shell () const;

    /**
     * Get the command to run.
     *
     * @param session_chroot the chroot to setup.  This must be
     * present in the chroot list and the chroot configuration object.
     * @param file the filename to pass to execve(2).
     * @param command the argv to pass to execve(2).
     */
    virtual void
    get_command (chroot::ptr& session_chroot,
		 std::string& file,
		 string_list& command) const;

    /**
     * Get the command to run a login shell.
     *
     * @param session_chroot the chroot to setup.  This must be
     * present in the chroot list and the chroot configuration object.
     * @param file the filename to pass to execve(2).
     * @param command the argv to pass to execve(2).
     */
    virtual void
    get_login_command (chroot::ptr& session_chroot,
		       std::string& file,
		       string_list& command) const;

    /**
     * Get the command to run a user command.
     *
     * @param session_chroot the chroot to setup.  This must be
     * present in the chroot list and the chroot configuration object.
     * @param file the filename to pass to execve(2).
     * @param command the argv to pass to execve(2).
     */
    virtual void
    get_user_command (chroot::ptr& session_chroot,
		      std::string& file,
		      string_list& command) const;

  private:
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
    wait_for_child (pid_t pid,
		    int&  child_status);

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

    /**
     * Set the SIGTERM handler.
     *
     * An error will be thrown on failure.
     */
    void
    set_sigterm_handler ();

    /**
     * Restore the state of SIGTERM prior to setting the handler.
     */
    void
    clear_sigterm_handler ();

    /**
     * Set a signal handler.
     * An error will be thrown on failure.
     *
     * @param signal the signal number.
     * @param saved_signal the location to save the current handler.
     * @param handler the signal handler to install.
     */
    void
    set_signal_handler (int                signal,
			struct sigaction  *saved_signal,
			void             (*handler)(int));

    /**
     * Restore the state of the signal prior to setting the handler.
     *
     * @param signal the signal number.
     * @param saved_signal the location from which to restore the
     * saved handler.
     */
    void
    clear_signal_handler (int               signal,
			  struct sigaction *saved_signal);

    /// Authentication state.
    auth::ptr        authstat;
    /// The chroot configuration.
    config_ptr       config;
    /// The chroots to run the session operation in.
    string_list      chroots;
    /// The current chroot status.
    int              chroot_status;
    /// Lock status for locks acquired during chroot setup.
    bool lock_status;
    /// The child exit status.
    int              child_status;
    /// The session operation to perform.
    operation        session_operation;
    /// The session identifier.
    std::string      session_id;
    /// The session force status.
    bool             force;
    /// Signal saved while sighup handler is set.
    struct sigaction saved_sighup_signal;
    /// Signal saved while sigterm handler is set.
    struct sigaction saved_sigterm_signal;
    /// Saved terminal settings.
    struct termios saved_termios;
    /// Are the saved terminal settings valid?
    bool termios_ok;

  protected:
    /// Current working directory.
    std::string      cwd;
  };

}

#endif /* SBUILD_SESSION_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
