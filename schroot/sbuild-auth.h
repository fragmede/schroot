/* sbuild-auth - sbuild auth object
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

#include <sigc++/sigc++.h>

#include <security/pam_appl.h>
#include <security/pam_misc.h>

#include <glib.h>
#include <glib/gprintf.h>

#include "sbuild-auth-conv.h"
#include "sbuild-auth-conv-tty.h"
#include "sbuild-config.h"
#include "sbuild-error.h"

namespace sbuild
{

  class Auth
  {
  public:
    typedef std::vector<std::string> string_list;
    typedef std::pair<std::string,std::string> env;
    typedef std::vector<env> env_list;

    typedef enum
      {
	STATUS_NONE,
	STATUS_USER,
	STATUS_FAIL
      } Status;

    typedef enum
      {
	VERBOSITY_QUIET,
	VERBOSITY_NORMAL,
	VERBOSITY_VERBOSE
      } Verbosity;

    typedef enum
      {
	ERROR_PAM_STARTUP,
	ERROR_PAM_SET_ITEM,
	ERROR_HOSTNAME,
	ERROR_PAM_AUTHENTICATE,
	ERROR_PAM_PUTENV,
	ERROR_PAM_ACCOUNT,
	ERROR_PAM_CREDENTIALS,
	ERROR_PAM_SESSION_OPEN,
	ERROR_PAM_SESSION_CLOSE,
	ERROR_PAM_DELETE_CREDENTIALS,
	ERROR_PAM_SHUTDOWN,
      } ErrorCode;

    typedef Exception<ErrorCode> error;

    Auth(const std::string& service_name);
    virtual ~Auth();

    const std::string&
    get_service () const;

    uid_t
    get_uid () const;

    gid_t
    get_gid () const;

    const std::string&
    get_user () const;

    void
    set_user (const std::string& user);

    const string_list&
    get_command () const;

    void
    set_command (const string_list& command);

    const std::string&
    get_home () const;

    const std::string&
    get_shell () const;

    const env_list&
    get_environment () const;

    void
    set_environment (char **environment);

    void
    set_environment (const env_list& environment);

    env_list
    get_pam_environment () const;

    uid_t
    get_ruid () const;

    const std::string&
    get_ruser () const;

    Verbosity
    get_verbosity () const;

    void
    set_verbosity (Verbosity  verbosity);

    std::tr1::shared_ptr<AuthConv>&
    get_conv ();

    void
    set_conv (std::tr1::shared_ptr<AuthConv>& conv);

    void
    run ();

    void
    start ();

    void
    stop ();

    void
    authenticate ();

    void
    setupenv ();

    void
    account ();

    void
    cred_establish ();

    void
    cred_delete ();

    void
    open_session ();

    void
    close_session ();

protected:
    virtual Status
    get_auth_status () const;

    virtual void
    run_impl () = 0;

  public:
    /**
     * change_auth:
     * @oldauth: the current authentication status
     * @newauth: the new authentication status
     *
     * Set new authentication status.  If @newauth > @oldauth, @newauth is
     * returned, otherwise @oldauth is returned.  This is to ensure the
     * authentication status can never be decreased.
     *
     * Returns the new authentication status.
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
    pam_handle_t        *pam;

  private:
    const std::string    service;
    uid_t                uid;
    gid_t                gid;
    std::string          user;
    string_list          command;
    std::string          home;
    std::string          shell;
    env_list             environment;
    uid_t                ruid;
    std::string          ruser;
    std::tr1::shared_ptr<AuthConv> conv;
    Verbosity            verbosity;
  };

}

#endif /* SBUILD_AUTH_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
