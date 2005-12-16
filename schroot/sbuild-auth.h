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

typedef enum
{
  SBUILD_AUTH_ERROR_PAM_STARTUP,
  SBUILD_AUTH_ERROR_PAM_SET_ITEM,
  SBUILD_AUTH_ERROR_HOSTNAME,
  SBUILD_AUTH_ERROR_PAM_AUTHENTICATE,
  SBUILD_AUTH_ERROR_PAM_PUTENV,
  SBUILD_AUTH_ERROR_PAM_ACCOUNT,
  SBUILD_AUTH_ERROR_PAM_CREDENTIALS,
  SBUILD_AUTH_ERROR_PAM_SESSION_OPEN,
  SBUILD_AUTH_ERROR_PAM_SESSION_CLOSE,
  SBUILD_AUTH_ERROR_PAM_DELETE_CREDENTIALS,
  SBUILD_AUTH_ERROR_PAM_SHUTDOWN,
} SbuildAuthError;

#define SBUILD_AUTH_ERROR sbuild_auth_error_quark()

GQuark
sbuild_auth_error_quark (void);

typedef enum
{
  SBUILD_AUTH_STATUS_NONE,
  SBUILD_AUTH_STATUS_USER,
  SBUILD_AUTH_STATUS_FAIL
} SbuildAuthStatus;

typedef enum
{
  SBUILD_AUTH_VERBOSITY_QUIET,
  SBUILD_AUTH_VERBOSITY_NORMAL,
  SBUILD_AUTH_VERBOSITY_VERBOSE
} SbuildAuthVerbosity;


class SbuildAuth
{
public:
  typedef std::vector<std::string> string_list;
  typedef std::pair<std::string,std::string> env;
  typedef std::vector<env> env_list;

  SbuildAuth(const std::string& service_name);
  virtual ~SbuildAuth();

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

  SbuildAuthVerbosity
  get_verbosity () const;

  void
  set_verbosity (SbuildAuthVerbosity  verbosity);

  std::tr1::shared_ptr<SbuildAuthConv>&
  get_conv ();

  void
  set_conv (std::tr1::shared_ptr<SbuildAuthConv>& conv);

  gboolean
  run (GError     **error);

  gboolean
  start (GError     **error);

  gboolean
  stop (GError     **error);

  gboolean
  authenticate (GError     **error);

  gboolean
  setupenv (GError     **error);

  gboolean
  account (GError     **error);

  gboolean
  cred_establish (GError     **error);

  gboolean
  cred_delete (GError     **error);

  gboolean
  open_session (GError     **error);

  gboolean
  close_session (GError     **error);

protected:
  virtual SbuildAuthStatus
  get_auth_status () const;

  virtual bool
  run_impl (GError **error) = 0;

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
  SbuildAuthStatus
  change_auth (SbuildAuthStatus oldauth,
	       SbuildAuthStatus newauth) const
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
  std::tr1::shared_ptr<SbuildAuthConv> conv;
  SbuildAuthVerbosity verbosity;
};

#endif /* SBUILD_AUTH_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
