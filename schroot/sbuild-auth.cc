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

/**
 * SECTION:sbuild-auth
 * @short_description: authentication handler
 * @title: SbuildAuth
 *
 * SbuildAuth handles user authentication, authorisation and session
 * management using the Pluggable Authentication Modules (PAM)
 * library.  It is essentially a GObject wrapper around PAM.
 *
 * In order to use PAM correctly, it is important to call several of
 * the methods in the correct order.  For example, it is not possible
 * to authorise a user before authenticating a user, and a session may
 * not be started before either of these have occured.
 *
 * The correct order is
 * - sbuild_auth_start
 * - sbuild_auth_authenticate
 * - sbuild_auth_setupenv
 * - sbuild_auth_account
 * - sbuild_auth_cred_establish
 * - sbuild_auth_close_session
 *
 * After the session has finished, or if an error occured, the
 * corresponding cleanup methods should be called
 * - sbuild_auth_close_session
 * - sbuild
 * - sbuild_auth_cred_delete
 * - sbuild_auth_stop
 *
 * The function sbuild_auth_run will handle all this.  The session_run
 * vfunc or "session-run" signal should be used to provide a session
 * handler to open and close the session for the user.
 * sbuild_auth_open_session and sbuild_auth_close_session must still
 * be used.
 */

#include <config.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <syslog.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "sbuild-auth.h"
#include "sbuild-auth-message.h"
#include "sbuild-error.h"

namespace
{
  /* This is the glue to link PAM user interaction with SbuildAuthConv. */
  static int
  auth_conv (int                        num_msg,
	     const struct pam_message **msgm,
	     struct pam_response      **response,
	     void                      *appdata_ptr)
  {
    if (appdata_ptr == 0)
      return PAM_CONV_ERR;

    SbuildAuthConv *conv = static_cast<SbuildAuthConv *>(appdata_ptr);
    g_assert (conv != 0);

    /* Construct a message vector */
    SbuildAuthConv::message_list messages;
    for (guint i = 0; i < num_msg; ++i)
      {
	const struct pam_message *source = msgm[i];

	SbuildAuthMessage message(static_cast<SbuildAuthMessageType>(source->msg_style),
				  source->msg);
	messages.push_back(message);

      }

    /* Do the conversation */
    gboolean status = conv->conversation(messages);

    if (status == TRUE)
      {
	/* Copy response into **reponse */
	struct pam_response *reply = g_new0(struct pam_response, num_msg);

	for (guint i = 0; i < num_msg; ++i)
	  {
	    reply[i].resp_retcode = 0;
	    reply[i].resp = g_strdup(messages[i].response.c_str());
	  }

	*response = reply;
	reply = 0;
      }

    if (status == TRUE)
      return PAM_SUCCESS;
    else
      return PAM_CONV_ERR;
  }

  std::string
  env_string(const std::string& key,
	     const std::string& value)
  {
    return key + "=" + value;
  }

  SbuildAuth::env_list
  env_strv_to_env_list (char **env)
  {
    SbuildAuth::env_list ret;

    if (env)
      {
	for (char *ev = env[0]; ev != 0; ++ev)
	  {
	    std::string evs(ev);
	    std::string::size_type pos = evs.find('=');
	    if (pos != std::string::npos && pos != 0)
	      {
		std::string key = evs.substr(0, pos);
		std::string value = evs.substr(pos + 1);
		ret.push_back(std::make_pair(key, value));
	      }
	  }
      }

    return ret;
  }

}


SbuildAuth::SbuildAuth(const std::string& service_name):
  service(service_name),
  uid(0),
  gid(0),
  user(),
  command(),
  home(),
  shell(),
  environment(),
  ruid(),
  ruser(),
  conv(dynamic_cast<SbuildAuthConv *>(new SbuildAuthConvTty)),
  pam(),
  verbosity(SBUILD_AUTH_VERBOSITY_NORMAL)
{
  this->ruid = getuid();
  struct passwd *pwent = getpwuid(this->ruid);
  if (pwent == 0)
    {
      g_printerr(_("%lu: user not found: %s\n"), (unsigned long) this->ruid,
		 g_strerror(errno));
      exit (EXIT_FAILURE);
    }
  this->ruser = pwent->pw_name;

  /* By default, the auth user is the same as the remote user. */
  set_user(this->ruser);
}

SbuildAuth::~SbuildAuth()
{
  // TODO: Shutdown PAM?
}

/**
 * sbuild_auth_get_service:
 * @auth: an #SbuildAuth
 *
 * Get the PAM service name.  This is passed to pam_start() when
 * initialising PAM.  It must be set during object construction, and
 * MUST be a hard-coded (constant) string literal for security
 * reasons.
 *
 * Returns a string. This string points to internally allocated
 * storage  and must not be freed, modified or stored.
 */
const std::string&
SbuildAuth::get_service () const
{
  return this->service;
}

/**
 * sbuild_auth_get_uid:
 * @auth: an #SbuildAuth
 *
 * Get the uid of the user.  This is the uid to run as in the
 * session.
 *
 * Returns a uid.  This will be 0 if no user was set, or the user is
 * uid 0.
 */
uid_t
SbuildAuth::get_uid () const
{
  return this->uid;
}

/**
 * sbuild_auth_get_gid:
 * @auth: an #SbuildAuth
 *
 * Get the gid of the user.  This is the gid to run as in the
 * session.
 *
 * Returns a gid.  This will be 0 if no user was set, or the user is
 * gid 0.
 */
gid_t
SbuildAuth::get_gid () const
{
  return this->gid;
}

/**
 * sbuild_auth_get_user:
 * @auth: an #SbuildAuth
 *
 * Get the name of the user.  This is the user to run as in the
 * session.
 *
 * Returns a string.  This string points to internally allocated
 * storage and must not be freed, modified or stored.
 */
const std::string&
SbuildAuth::get_user () const
{
  return this->user;
}

/**
 * sbuild_auth_set_user:
 * @auth: an #SbuildAuth.
 * @user: the name to set.
 *
 * Get the name of the user.  This is the user to run as in the
 * session.
 *
 * As a side effect, the "uid", "gid", "home" and "shell" properties
 * will also be set.
 */
void
SbuildAuth::set_user (const std::string& user)
{
  this->uid = 0;
  this->gid = 0;
  this->home = "/";
  this->shell = "/bin/false";

  this->user = user;

  struct passwd *pwent = getpwnam(this->user.c_str());
  if (pwent == 0)
    {
      g_printerr(_("%s: user not found: %s\n"), this->user.c_str(), g_strerror(errno));
      exit (EXIT_FAILURE);
    }
  this->uid = pwent->pw_uid;
  this->gid = pwent->pw_gid;
  this->home = pwent->pw_dir;
  this->shell = pwent->pw_shell;
  g_debug("auth uid = %lu, gid = %lu", (unsigned long) this->uid,
	  (unsigned long) this->gid);
}

/**
 * sbuild_auth_get_command:
 * @auth: an #SbuildAuth
 *
 * Get the command to run in the session.
 *
 * Returns a string vector.  This string vector points to internally
 * allocated storage and must not be freed, modified or stored.
 */
const SbuildAuth::string_list&
SbuildAuth::get_command () const
{
  return this->command;
}

/**
 * sbuild_auth_set_command:
 * @auth: an #SbuildAuth.
 * @command: the command to set.
 *
 * Set the command to run in the session.
 */
void
SbuildAuth::set_command (const SbuildAuth::string_list& command)
{
  this->command = command;
}

/**
 * sbuild_auth_get_home:
 * @auth: an #SbuildAuth
 *
 * Get the home directory.  This is the $HOME to set in the session,
 * if the user environment is not being preserved.
 *
 * Returns a string.  This string points to internally allocated
 * storage and must not be freed, modified or stored.
 */
const std::string&
SbuildAuth::get_home () const
{
  return this->home;
}

/**
 * sbuild_auth_get_shell:
 * @auth: an #SbuildAuth
 *
 * Get the name of the shell.  This is the shell to run as in the
 * session.
 *
 * Returns a string.  This string points to internally allocated
 * storage and must not be freed, modified or stored.
 */
const std::string&
SbuildAuth::get_shell () const
{
  return this->shell;
}

/**
 * sbuild_auth_get_environment:
 * @auth: an #SbuildAuth
 *
 * Get the environment to use in @auth.
 *
 * Returns a string vector.  This string vector points to internally
 * allocated storage and must not be freed, modified or stored.
 */
const SbuildAuth::env_list&
SbuildAuth::get_environment () const
{
  return this->environment;
}

void
SbuildAuth::set_environment (char **environment)
{
  set_environment(env_strv_to_env_list(environment));
}

/**
 * sbuild_auth_set_environment:
 * @auth: an #SbuildAuth
 * @environment: the environment to use
 *
 * Set the environment to use in @auth.
 */
void
SbuildAuth::set_environment (const SbuildAuth::env_list& environment)
{
  this->environment = environment;
}

/**
 * sbuild_auth_get_pam_environment:
 * @auth: an #SbuildAuth
 *
 * Get the PAM environment from @auth.
 *
 * Returns a string vector.  This string vector points to internally
 * allocated storage and must not be freed, modified or stored.
 */
SbuildAuth::env_list
SbuildAuth::get_pam_environment () const
{
  char **env =  pam_getenvlist(this->pam);

  return env_strv_to_env_list(env);
}

/**
 * sbuild_auth_get_ruid:
 * @auth: an #SbuildAuth
 *
 * Get the "remote uid" of the user.  This is the uid which is
 * requesting authentication.
 *
 * Returns a uid.
 */
uid_t
SbuildAuth::get_ruid () const
{
  return this->ruid;
}

/**
 * sbuild_auth_get_ruser:
 * @auth: an #SbuildAuth
 *
 * Get the "remote" name of the user.  This is the user which is
 * requesting authentication.
 *
 * Returns a string.  This string points to internally allocated
 * storage and must not be freed, modified or stored.
 */
const std::string&
SbuildAuth::get_ruser () const
{
  return this->ruser;
}

/**
 * sbuild_auth_get_verbosity:
 * @auth: an #SbuildAuth
 *
 * Get the message verbosity of @auth.
 *
 * Returns the #SbuildAuthVerbosity verbosity level.
 */
SbuildAuthVerbosity
SbuildAuth::get_verbosity () const
{
  return this->verbosity;
}

/**
 * sbuild_auth_set_verbosity:
 * @auth: an #SbuildAuth
 * @verbosity: the verbosity level to use
 *
 * Set the message verbosity of @auth.
 */
void
SbuildAuth::set_verbosity (SbuildAuthVerbosity  verbosity)
{
  this->verbosity = verbosity;
}

/**
 * sbuild_auth_get_conv:
 * @auth: an #SbuildAuth
 *
 * Get the conversation handler for @auth.
 *
 * Returns the handler object.
 */
std::tr1::shared_ptr<SbuildAuthConv>&
SbuildAuth::get_conv ()
{
  return this->conv;
}

/**
 * sbuild_auth_set_conv:
 * @auth: an #SbuildAuth
 * @conv: the conversation handler to use
 *
 * Set the conversation handler for @auth.
 */
void
SbuildAuth::set_conv (std::tr1::shared_ptr<SbuildAuthConv>& conv)
{
  this->conv = conv;
}

gboolean
SbuildAuth::run (GError     **error)
{
  gboolean retval;
  GError *tmp_error = 0;

  start(&tmp_error);
  if (tmp_error == 0)
    {
      authenticate(&tmp_error);
      if (tmp_error == 0)
	{
	  setupenv(&tmp_error);
	  if (tmp_error == 0)
	    {
	      account(&tmp_error);
	      if (tmp_error == 0)
		{
		  cred_establish(&tmp_error);
		  if (tmp_error == 0)
		    {
		      const char *authuser = 0;
		      pam_get_item(this->pam, PAM_USER, (const void **) &authuser);
		      g_debug("PAM authentication succeeded for user %s\n", authuser);

		      /* TODO: emit a signal */
		      retval = run_impl(&tmp_error);

		      /* The session is now finished, either
			 successfully or not.  All PAM operations are
			 now for cleanup and shutdown, and we must
			 clean up whether or not errors were raised at
			 any previous point.  This means only the
			 first error is reported back to the user. */

		      /* Don't cope with failure, since we are now
			 already bailing out, and an error may already
			 have been raised */
		      cred_delete((tmp_error != 0) ? 0 : &tmp_error);

		    } // pam_cred_establish
		} // pam_account
	    } // pam_setupenv
	} // pam_auth
      /* Don't cope with failure, since we are now already bailing out,
	 and an error may already have been raised */
      stop((tmp_error != 0) ? 0 : &tmp_error);
    } // pam_start

  if (tmp_error != 0)
    {
      g_propagate_error(error, tmp_error);
      return FALSE;
    }
  else
    return TRUE;
}

/**
 * sbuild_auth_start:
 * @auth: an #SbuildAuth
 * @error: a #GError
 *
 * Start the PAM system.  No other PAM functions may be called before
 * calling this function.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
gboolean
SbuildAuth::start (GError     **error)
{
  g_return_val_if_fail(this->user.empty(), FALSE);

  if (this->pam != 0)
    {
      g_set_error(error,
		  SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_STARTUP,
		  _("PAM error: PAM is already initialised"));
      g_debug("pam_start FAIL (already initialised)");
      return FALSE;
    }

  struct pam_conv conv_hook =
    {
      auth_conv,
      static_cast<void *>(this->conv.get())
    };

  int pam_status;
  if ((pam_status =
       pam_start(this->service.c_str(), this->user.c_str(),
		 &conv_hook, &this->pam)) != PAM_SUCCESS)
    {
      g_set_error(error,
		  SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_STARTUP,
		  _("PAM error: %s"), pam_strerror(this->pam, pam_status));
      g_debug("pam_start FAIL");
      return FALSE;
    }
  g_debug("pam_start OK");
  return TRUE;
}

/**
 * sbuild_auth_stop:
 * @auth: an #SbuildAuth
 * @error: a #GError
 *
 * Stop the PAM system.  No other PAM functions may be used after
 * calling this function.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
gboolean
SbuildAuth::stop (GError     **error)
{
  g_return_val_if_fail(this->pam != 0, FALSE); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_end(this->pam, PAM_SUCCESS)) != PAM_SUCCESS)
    {
      g_set_error(error,
		  SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_SHUTDOWN,
		  _("PAM error: %s"), pam_strerror(this->pam, pam_status));
      g_debug("pam_end FAIL");
      return FALSE;
    }

  g_debug("pam_end OK");
  return TRUE;
}

/**
 * sbuild_auth_authenticate:
 * @auth: an #SbuildAuth
 * @error: a #GError
 *
 * Perform PAM authentication.  If required, the user will be prompted
 * to authenticate themselves.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
gboolean
SbuildAuth::authenticate (GError     **error)
{
  g_return_val_if_fail(this->user.empty(), FALSE);
  g_return_val_if_fail(this->pam != 0, FALSE); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_set_item(this->pam, PAM_RUSER, this->ruser.c_str())) != PAM_SUCCESS)
    {
      g_set_error(error,
		  SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_SET_ITEM,
		  _("PAM set RUSER error: %s"), pam_strerror(this->pam, pam_status));
      g_debug("pam_set_item (PAM_RUSER) FAIL");
      return FALSE;
    }

  long hl = 256; /* sysconf(_SC_HOST_NAME_MAX); BROKEN with Debian libc6 2.3.2.ds1-22 */

  char *hostname = g_new(char, hl);
  if (gethostname(hostname, hl) != 0)
    {
      g_set_error(error,
		  SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_HOSTNAME,
		  _("Failed to get hostname: %s\n"), g_strerror(errno));
      g_debug("gethostname FAIL");
      return FALSE;
    }

  if ((pam_status =
       pam_set_item(this->pam, PAM_RHOST, hostname)) != PAM_SUCCESS)
    {
      g_set_error(error,
		  SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_SET_ITEM,
		  _("PAM set RHOST error: %s"), pam_strerror(this->pam, pam_status));
      g_debug("pam_set_item (PAM_RHOST) FAIL");
      return FALSE;
    }

  g_free(hostname);
  hostname = 0;

  const char *tty = ttyname(STDIN_FILENO);
  if (tty)
    {
      if ((pam_status =
	   pam_set_item(this->pam, PAM_TTY, tty)) != PAM_SUCCESS)
	{
	  g_set_error(error,
		      SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_SET_ITEM,
		      _("PAM set TTY error: %s"), pam_strerror(this->pam, pam_status));
	  g_debug("pam_set_item (PAM_TTY) FAIL");
	  return FALSE;
	}
    }

  /* Authenticate as required. */
  switch (get_auth_status())
    {
    case SBUILD_AUTH_STATUS_NONE:
      if ((pam_status =
	   pam_set_item(this->pam, PAM_USER, this->user.c_str())) != PAM_SUCCESS)
	{
	  g_set_error(error,
		      SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_SET_ITEM,
		      _("PAM set USER error: %s"), pam_strerror(this->pam, pam_status));
	  g_debug("pam_set_item (PAM_USER) FAIL");
	  return FALSE;
	}
      break;

    case SBUILD_AUTH_STATUS_USER:
      if ((pam_status =
	   pam_authenticate(this->pam, 0)) != PAM_SUCCESS)
	{
	  g_set_error(error,
		      SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_AUTHENTICATE,
		      _("PAM authentication failed: %s\n"), pam_strerror(this->pam, pam_status));
	  g_debug("pam_authenticate FAIL");
	  syslog(LOG_AUTH|LOG_WARNING, "%s->%s Authentication failure",
		 this->ruser.c_str(), this->user.c_str());
	  return FALSE;
	}
      g_debug("pam_authenticate OK");
      break;

    case SBUILD_AUTH_STATUS_FAIL:
	{
	  g_set_error(error,
		      SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_AUTHENTICATE,
		      _("access not authorised"));
	  g_debug("PAM auth premature FAIL");
	  g_printerr(_("You do not have permission to access the %s service.\n"),
		     this->service.c_str());
	  g_printerr(_("This failure will be reported.\n"));
	  syslog(LOG_AUTH|LOG_WARNING,
		 "%s->%s Unauthorised",
		 this->ruser.c_str(), this->user.c_str());
	  return FALSE;
	}
    default:
      break;
    }

  return TRUE;
}

/**
 * sbuild_auth_setupenv:
 * @auth: an #SbuildAuth
 * @error: a #GError
 *
 * Import the user environment into PAM.  If no environment was
 * specified with #sbuild_auth_set_environment, a minimal environment
 * will be created containing HOME, LOGNAME, PATH, TERM and LOGNAME.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
gboolean
SbuildAuth::setupenv (GError     **error)
{
  g_return_val_if_fail(this->pam != 0, FALSE); // PAM must be initialised

  int pam_status;

  /* Initial environment to set, used if the environment was not
     specified. */
  env_list newenv;

  if (this->uid == 0)
    newenv.push_back(std::make_pair("PATH", "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/bin/X11"));
  else
    newenv.push_back(std::make_pair("PATH", "/usr/local/bin:/usr/bin:/bin:/usr/bin/X11:/usr/games"));

  if (!this->home.empty())
    newenv.push_back(std::make_pair("HOME", this->home));
  else
    newenv.push_back(std::make_pair("HOME", "/"));
  if (!this->user.empty())
    {
      newenv.push_back(std::make_pair("LOGNAME", this->user));
      newenv.push_back(std::make_pair("USER", this->user));
    }
  {
    const char *term = g_getenv("TERM");
    if (term)
      newenv.push_back(std::make_pair("TERM", term));
  }

  env_list environment = !this->environment.empty() ? this->environment : newenv;
  for (env_list::const_iterator cur = environment.begin();
       cur != environment.end();
       ++cur)
    {
      std::string env_string = cur->first + "=" + cur->second;
      if ((pam_status =
	   pam_putenv(this->pam, env_string.c_str())) != PAM_SUCCESS)
	{
	  g_set_error(error,
		      SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_PUTENV,
		      _("PAM error: %s\n"), pam_strerror(this->pam, pam_status));
	  g_debug("pam_putenv FAIL");
	  return FALSE;
	}
      g_debug("pam_putenv: set %s=%s", cur->first.c_str(), cur->second.c_str());
    }

  g_debug("pam_putenv OK");
  return TRUE;
}

/**
 * sbuild_auth_account:
 * @auth: an #SbuildAuth
 * @error: a #GError
 *
 * Do PAM account management (authorisation).
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
gboolean
SbuildAuth::account (GError     **error)
{
  g_return_val_if_fail(this->pam != 0, FALSE); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_acct_mgmt(this->pam, 0)) != PAM_SUCCESS)
    {
      /* We don't handle changing expired passwords here, since we are
	 not login or ssh. */
      g_set_error(error,
		  SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_ACCOUNT,
		  _("PAM error: %s\n"), pam_strerror(this->pam, pam_status));
      g_debug("pam_acct_mgmt FAIL");
      return FALSE;
    }

  g_debug("pam_acct_mgmt OK");
  return TRUE;
}

/**
 * sbuild_auth_cred_establish:
 * @auth: an #SbuildAuth
 * @error: a #GError
 *
 * Use PAM to establish credentials.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
gboolean
SbuildAuth::cred_establish (GError     **error)
{
  g_return_val_if_fail(this->pam != 0, FALSE); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_setcred(this->pam, PAM_ESTABLISH_CRED)) != PAM_SUCCESS)
    {
      g_set_error(error,
		  SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_CREDENTIALS,
		  _("PAM error: %s\n"), pam_strerror(this->pam, pam_status));
      g_debug("pam_setcred FAIL");
      return FALSE;
    }

  g_debug("pam_setcred OK");
  return TRUE;
}

/**
 * sbuild_auth_cred_delete:
 * @auth: an #SbuildAuth
 * @error: a #GError
 *
 * Use PAM to delete credentials.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
gboolean
SbuildAuth::cred_delete (GError     **error)
{
  g_return_val_if_fail(this->pam != 0, FALSE); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_setcred(this->pam, PAM_DELETE_CRED)) != PAM_SUCCESS)
    {
      g_set_error(error,
		  SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_DELETE_CREDENTIALS,
		  _("PAM error: %s"), pam_strerror(this->pam, pam_status));
      g_debug("pam_setcred (delete) FAIL");
      return FALSE;
    }

      g_debug("pam_setcred (delete) OK");
  return TRUE;
}

/**
 * sbuild_auth_open_session:
 * @auth: an #SbuildAuth
 * @error: a #GError
 *
 * Open a PAM session.  This should be called in the child process.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
gboolean
SbuildAuth::open_session (GError     **error)
{
  g_return_val_if_fail(this->pam != 0, FALSE); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_open_session(this->pam, 0)) != PAM_SUCCESS)
    {
      g_set_error(error,
		  SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_SESSION_OPEN,
		  _("PAM error: %s"), pam_strerror(this->pam, pam_status));
      g_debug("pam_open_session FAIL");
      return FALSE;
    }

  g_debug("pam_open_session OK");
  return TRUE;
}

/**
 * sbuild_auth_close_session:
 * @auth: an #SbuildAuth
 * @error: a #GError
 *
 * Close a PAM session.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
gboolean
SbuildAuth::close_session (GError     **error)
{
  g_return_val_if_fail(this->pam != 0, FALSE); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_close_session(this->pam, 0)) != PAM_SUCCESS)
    {
      g_set_error(error,
		  SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_SESSION_CLOSE,
		  _("PAM error: %s"), pam_strerror(this->pam, pam_status));
      g_debug("pam_close_session FAIL");
      return FALSE;
    }

  g_debug("pam_close_session OK");
  return TRUE;
}

/**
 * sbuild_auth_require_auth_impl:
 * @auth: an #SbuildAuth
 *
 * Check if authentication is required for @auth.  This default
 * implementation always requires authentication.
 *
 * Returns the authentication type.
 */
SbuildAuthStatus
SbuildAuth::get_auth_status () const
{
  SbuildAuthStatus authtype = SBUILD_AUTH_STATUS_NONE;

  authtype = change_auth(authtype, SBUILD_AUTH_STATUS_USER);

  return authtype;
}

/**
 * sbuild_auth_error_quark:
 *
 * Get the SBUILD_AUTH_ERROR domain number.
 *
 * Returns the domain.
 */
GQuark
sbuild_auth_error_quark (void)
{
  static GQuark error_quark = 0;

  if (error_quark == 0)
    error_quark = g_quark_from_static_string ("sbuild-auth-error-quark");

  return error_quark;
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
