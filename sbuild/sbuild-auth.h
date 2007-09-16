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

#ifndef SBUILD_AUTH_H
#define SBUILD_AUTH_H

#include <sbuild/sbuild-auth-conv.h>
#include <sbuild/sbuild-custom-error.h>
#include <sbuild/sbuild-environment.h>
#include <sbuild/sbuild-types.h>
#include <sbuild/sbuild-tr1types.h>

#include <string>

#include <sys/types.h>
#include <sys/wait.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#include <security/pam_appl.h>

namespace sbuild
{

  /**
   * Authentication handler.
   *
   * auth handles user authentication, authorisation and session
   * management using the Pluggable Authentication Modules (PAM)
   * library.  It is essentially an object-oriented wrapper around PAM.
   *
   * In order to use PAM correctly, it is important to call several of
   * the methods in the correct order.  For example, it is not possible
   * to authorise a user before authenticating a user, and a session may
   * not be started before either of these have occurred.
   *
   * The correct order is
   * - start
   * - authenticate
   * - setupenv
   * - account
   * - cred_establish
   * - open_session
   *
   * After the session has finished, or if an error occurred, the
   * corresponding cleanup methods should be called
   * - close_session
   * - cred_delete
   * - stop
   *
   * The run method will handle all this.  The run_impl virtual
   * function should be used to provide a session handler to open and
   * close the session for the user.  open_session and close_session
   * must still be called.
   */
  class auth
  {
  public:
    /// Authentication status
    enum status
      {
	STATUS_NONE, ///< Authentication is not required.
	STATUS_USER, ///< Authentication is required by the user.
	STATUS_FAIL  ///< Authentication has failed.
      };

    /// Message verbosity
    enum verbosity
      {
	VERBOSITY_QUIET,  ///< Only print essential messages.
	VERBOSITY_NORMAL, ///< Print messages (the default).
	VERBOSITY_VERBOSE ///< Print all messages.
      };

    /// Error codes.
    enum error_code
      {
	HOSTNAME,        ///< Failed to get hostname.
	USER,            ///< User not found.
	GROUP,           ///< Group not found.
	AUTHENTICATION,  ///< Authentication failed.
	AUTHORISATION,   ///< Authorisation failed.
	PAM_DOUBLE_INIT, ///< PAM was already initialised.
	PAM              ///< PAM error.
      };

    /// Exception type.
    typedef custom_error<error_code> error;

    /// A shared_ptr to an auth_conv object.
    typedef std::tr1::shared_ptr<auth_conv> conv_ptr;

    /**
     * The constructor.
     *
     * @param service_name the PAM service name.  This should be a
     * hard-coded constant string literal for safety and security.
     * This is passed to pam_start() when initialising PAM, and is
     * used to load the correct configuration file from /etc/pam.d.
     */
    auth (std::string const& service_name);

    /**
     * The destructor.
     */
    virtual ~auth ();

    /**
     * Get the PAM service name.
     *
     * @returns the service name.
     */
    std::string const&
    get_service () const;

    /**
     * Get the uid of the user.  This is the uid to run as in the *
     * session.
     *
     * @returns a uid.  This will be 0 if no user was set, or the user
     * is uid 0.
     */
    uid_t
    get_uid () const;

    /**
     * Get the gid of the user.  This is the gid to run as in the
     * session.
     *
     * @returns a gid.  This will be 0 if no user was set, or the user
     * is gid 0.
     */
    gid_t
    get_gid () const;

    /**
     * Get the name of the user.  This is the user to run as in the
     * session.
     *
     * @returns the user's name.
     */
    std::string const&
    get_user () const;

    /**
     * Set the name of the user.  This is the user to run as in the
     * session.
     *
     * As a side effect, the uid, gid, home and shell member variables
     * will also be set, so calling the corresponding get methods will
     * now return meaningful values.
     *
     * @param user the name to set.
     */
    void
    set_user (std::string const& user);

    /**
     * Get the command to run in the session.
     *
     * @returns the command as string list, each item being a separate
     * argument.  If no command has been specified, the list will be
     * empty.
     */
    string_list const&
    get_command () const;

    /**
     * Set the command to run in the session.
     *
     * @param command the command to run.  This is a string list, each
     * item being a separate argument.
     */
    void
    set_command (string_list const& command);

    /**
     * Get the home directory.  This is the $HOME to set in the session,
     * if the user environment is not being preserved.
     *
     * @returns the home directory.
     */
    std::string const&
    get_home () const;

    /**
     * Get the working directory.  This is the working directory to
     * set in the session.
     *
     * @returns the current working directory.
     */
    std::string const&
    get_wd () const;

    /**
     * Set the working directory.  This is the working directory to
     * set in the session.
     *
     * @param cvs the current working directory.
     */
    void
    set_wd (std::string const& wd);

    /**
     * Get the name of the shell.  This is the shell to run in the
     * session.
     *
     * @returns the shell.  This is typically a full pathname, though
     * the executable name only should also work (the caller will have
     * to search for it).
     */
    std::string const&
    get_shell () const;

    /**
     * Get the environment to use in the session.
     *
     * @returns an environment list (a list of key-value pairs).
     */
    environment const&
    get_environment () const;

    /**
     * Set the environment to use in the session.
     *
     * @param environment an environ- or envp-like string vector
     * containing key=value pairs.
     */
    void
    set_environment (char **environment);

    /**
     * Set the environment to use in the session.
     *
     * @param environment an environment list.
     */
    void
    set_environment (environment const& environment);

    /**
     * Get the PAM environment.  This is the environment as set by PAM
     * modules.
     *
     * @returns an environment list.
     */
    environment
    get_pam_environment () const;

    /**
     * Get the "remote uid" of the user.  This is the uid which is
     * requesting authentication.
     *
     * @returns a uid.
     */
    uid_t
    get_ruid () const;

    /**
     * Get the "remote gid" of the user.  This is the gid which is
     * requesting authentication.
     *
     * @returns a gid.
     */
    gid_t
    get_rgid () const;

    /**
     * Get the "remote" name of the user.  This is the user which is
     * requesting authentication.
     *
     * @returns a user name.
     */
    std::string const&
    get_ruser () const;

    /**
     * Get the "remote" name of the group.  This is the group which is
     * requesting authentication.
     *
     * @returns a group name.
     */
    std::string const&
    get_rgroup () const;

    /**
     * Get the message verbosity.
     *
     * Returns the verbosity level.
     */
    verbosity
    get_verbosity () const;

    /**
     * Set the message verbosity.
     *
     * @param verbosity the verbosity level.
     */
    void
    set_verbosity (verbosity verbosity);

    /**
     * Get the conversation handler.
     *
     * @returns a shared_ptr to the handler.
     */
    conv_ptr&
    get_conv ();

    /**
     * Set the conversation handler.
     *
     * @param conv a shared_ptr to the handler.
     */
    void
    set_conv (conv_ptr& conv);

    /**
     * Run a session.  The user will be asked for authentication if
     * required, and then the run_impl virtual method will be called.
     *
     * An error will be thrown on failure.
     */
    void
    run ();

    /**
     * Start the PAM system.  No other PAM functions may be called before
     * calling this function.
     *
     * An error will be thrown on failure.
     */
    void
    start ();

    /**
     * Stop the PAM system.  No other PAM functions may be used after
     * calling this function.
     *
     * An error will be thrown on failure.
     */
    void
    stop ();

    /**
     * Perform PAM authentication.  If required, the user will be
     * prompted to authenticate themselves.
     *
     * An error will be thrown on failure.
     *
     * @todo Use sysconf(_SC_HOST_NAME_MAX) when libc in a stable
     * release supports it.
     */
    void
    authenticate ();

    /**
     * Import the user environment into PAM.  If no environment was
     * specified with set_environment, a minimal environment will be
     * created containing HOME, LOGNAME, PATH, TERM and LOGNAME.
     *
     * An error will be thrown on failure.
     *
     * Note that the environment is not sanitised in any way.  This is
     * the responsibility of the user.
     */
    void
    setupenv ();

    /**
     * Do PAM account management (authorisation).
     *
     * An error will be thrown on failure.
     */
    void
    account ();

    /**
     * Use PAM to establish credentials.
     *
     * An error will be thrown on failure.
     */
    void
    cred_establish ();

    /**
     * Use PAM to delete credentials.
     *
     * An error will be thrown on failure.
     */
    void
    cred_delete ();

    /**
     * Open a PAM session.
     *
     * An error will be thrown on failure.
     */
    void
    open_session ();

    /**
     * Close a PAM session.
     *
     * An error will be thrown on failure.
     */
    void
    close_session ();

protected:
    /**
     * Check if authentication is required.  This default
     * implementation always requires authentication.
     */
    virtual status
    get_auth_status () const;

    /**
     * Run session.  The code to run when authentication and
     * authorisation have been completed.
     */
    virtual void
    run_impl () = 0;

  public:
    /**
     * Set new authentication status.  If newauth > oldauth, newauth
     * is returned, otherwise oldauth is returned.  This is to ensure
     * the authentication status can never be decreased (relaxed).
     *
     * @param oldauth the current authentication status.
     * @param newauth the new authentication status.
     * @returns the new authentication status.
     */
    status
    change_auth (status oldauth,
		 status newauth) const
    {
      /* Ensure auth level always escalates. */
      if (newauth > oldauth)
	return newauth;
      else
	return oldauth;
    }

  protected:
    /// The PAM handle.
    pam_handle_t      *pam;

    /**
     * Get a description of a PAM error.
     *
     * @param pam_error the PAM error number.
     * @returns the description.
     */
    const char *
    pam_strerror (int pam_error);

  private:
    /// The PAM service name.
    const std::string  service;
    /// The uid to run as.
    uid_t              uid;
    /// The gid to run as.
    gid_t              gid;
    /// The user name to run as.
    std::string        user;
    /// The command to run.
    string_list        command;
    /// The home directory.
    std::string        home;
    /// The directory to run in.
    std::string        wd;
    /// The user shell to run.
    std::string        shell;
    /// The user environment to set.
    environment        user_environment;
    /// The uid requesting authentication.
    uid_t              ruid;
    /// The gid requesting authentication.
    gid_t              rgid;
    /// The user name requesting authentication.
    std::string        ruser;
    /// The group name requesting authentication.
    std::string        rgroup;
    /// The PAM conversation handler.
    conv_ptr           conv;
    /// The message verbosity.
    verbosity          message_verbosity;
  };

}

#endif /* SBUILD_AUTH_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
