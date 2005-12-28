/* Copyright © 2005  Roger Leigh <rleigh@debian.org>
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

#ifndef SBUILD_AUTH_H
#define SBUILD_AUTH_H

#include <string>
#include <vector>
#include <tr1/memory>

#include <sys/types.h>
#include <sys/wait.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#include <security/pam_appl.h>

#include "sbuild-auth-conv.h"
#include "sbuild-error.h"
#include "sbuild-types.h"

namespace sbuild
{

  /**
   * @brief Authentication handler.
   *
   * Auth handles user authentication, authorisation and session
   * management using the Pluggable Authentication Modules (PAM)
   * library.  It is essentially an object-oriented wrapper around PAM.
   *
   * In order to use PAM correctly, it is important to call several of
   * the methods in the correct order.  For example, it is not possible
   * to authorise a user before authenticating a user, and a session may
   * not be started before either of these have occured.
   *
   * The correct order is
   * - start
   * - authenticate
   * - setupenv
   * - account
   * - cred_establish
   * - open_session
   *
   * After the session has finished, or if an error occured, the
   * corresponding cleanup methods should be called
   * - close_session
   * - sbuild
   * - cred_delete
   * - stop
   *
   * The run method will handle all this.  The run_impl virtual
   * function should be used to provide a session handler to open and
   * close the session for the user.  open_session and close_session
   * must still be used.
   */
  class Auth
  {
  public:
    /// Authentication status
    enum Status
      {
	STATUS_NONE, ///< Authentication is not required.
	STATUS_USER, ///< Authentication is required by the user.
	STATUS_FAIL  ///< Authentication has failed.
      };

    /// Message verbosity
    enum Verbosity
      {
	VERBOSITY_QUIET,  ///< Only print essential messages.
	VERBOSITY_NORMAL, ///< Print messages (the default).
	VERBOSITY_VERBOSE ///< Print all messages.
      };

    /// Exception type.
    typedef runtime_error_custom<Auth> error;

    /// A shared_ptr to an AuthConv object.
    typedef std::tr1::shared_ptr<AuthConv> conv_ptr;

    /**
     * The constructor.
     *
     * @param service_name the PAM service name.  This should be a
     * hard-coded constant string literal for safety and security.
     * This is passed to pam_start() when initialising PAM, and is
     * used to load the correct configuration file from /etc/pam.d.
     */
    Auth(const std::string& service_name);

    /**
     * The destructor.
     *
     * @todo Shut down PAM if currently active.
     */
    virtual ~Auth();

    /**
     * Get the PAM service name.
     *
     * @returns the service name.
     */
    const std::string&
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
    const std::string&
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
     *
     * @todo Return an error or throw an exception if the user does
     * not exist, rather than silently setting a "sane" state.
     *
     * @todo If preserving the environment, respect $HOME when finding
     * the user's home directory, and $SHELL when finding the user's
     * shell.  Only do this when user == ruser (and uid != 0) ??
     */
    void
    set_user (const std::string& user);

    /**
     * Get the command to run in the session.
     *
     * @returns the command as string list, each item being a separate
     * argument.  If no command has been specified, the list will be
     * empty.
     */
    const string_list&
    get_command () const;

    /**
     * Set the command to run in the session.
     *
     * @param command the command to run.  This is a string list, each
     * item being a separate argument.
     */
    void
    set_command (const string_list& command);

    /**
     * Get the home directory.  This is the $HOME to set in the session,
     * if the user environment is not being preserved.
     *
     * @returns the home directory.
     */
    const std::string&
    get_home () const;

    /**
     * Get the name of the shell.  This is the shell to run in the
     * session.
     *
     * @returns the shell.  This is typically a full pathname, though
     * the executable name only should also work (the caller will have
     * to search for it).
     */
    const std::string&
    get_shell () const;

    /**
     * Get the environment to use in the session.
     *
     * @returns an environment list (a list of key-value pairs).
     *
     * @todo: env_list should be changed to be a std::map.
     */
    const env_list&
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
    set_environment (const env_list& environment);

    /**
     * Get the PAM environment.  This is the environment as set by PAM
     * modules.
     *
     * @returns an environment list.
     */
    env_list
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
     * Get the "remote" name of the user.  This is the user which is
     * requesting authentication.
     *
     * @returns a user name.
     */
    const std::string&
    get_ruser () const;

    /**
     * Get the message verbosity.
     *
     * Returns the verbosity level.
     */
    Verbosity
    get_verbosity () const;

    /**
     * Set the message verbosity.
     *
     * @param verbosity the verbosity level.
     */
    void
    set_verbosity (Verbosity verbosity);

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
     */
    void
    authenticate ();

    /**
     * Import the user environment into PAM.  If no environment was
     * specified with set_environment, a minimal environment will be
     * created containing HOME, LOGNAME, PATH, TERM and LOGNAME.
     *
     * An error will be thrown on failure.
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
    virtual Status
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
    Status
    change_auth (Status oldauth,
		 Status newauth) const
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
    /// The home directory to run in.
    std::string        home;
    /// The user shell to run.
    std::string        shell;
    /// The user environment to set.
    env_list           environment;
    /// The uid requesting authentication.
    uid_t              ruid;
    /// The user name requesting authentication.
    std::string        ruser;
    /// The PAM conversation handler.
    conv_ptr           conv;
    /// The message verbosity.
    Verbosity          verbosity;
  };

}

#endif /* SBUILD_AUTH_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
