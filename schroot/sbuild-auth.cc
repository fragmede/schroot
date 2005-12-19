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

#include <iostream>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <syslog.h>

#include <glib.h>

#include "sbuild-i18n.h"
#include "sbuild-auth.h"
#include "sbuild-auth-message.h"
#include "sbuild-error.h"
#include "sbuild-util.h"

using std::cerr;
using std::endl;
using namespace sbuild;

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

    AuthConv *conv = static_cast<AuthConv *>(appdata_ptr);
    g_assert (conv != 0);

    /* Construct a message vector */
    AuthConv::message_list messages;
    for (guint i = 0; i < num_msg; ++i)
      {
	const struct pam_message *source = msgm[i];

	AuthMessage message(static_cast<AuthMessage::MessageType>(source->msg_style),
				  source->msg);
	messages.push_back(message);

      }

    /* Do the conversation */
    bool status = conv->conversation(messages);

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

  Auth::env_list
  env_strv_to_env_list (char **env)
  {
    Auth::env_list ret;

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


Auth::Auth(const std::string& service_name):
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
  conv(dynamic_cast<AuthConv *>(new AuthConvTty)),
  pam(),
  verbosity(VERBOSITY_NORMAL)
{
  this->ruid = getuid();
  struct passwd *pwent = getpwuid(this->ruid);
  if (pwent == 0)
    {
      cerr << format_string(_("%lu: user not found: %s\n"),
			    (unsigned long) this->ruid,
			    g_strerror(errno))
	   << endl;
      exit (EXIT_FAILURE);
    }
  this->ruser = pwent->pw_name;

  /* By default, the auth user is the same as the remote user. */
  set_user(this->ruser);
}

Auth::~Auth()
{
  // TODO: Shutdown PAM?
}

/**
 * sbuild_auth_get_service:
 * @auth: an #Auth
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
Auth::get_service () const
{
  return this->service;
}

/**
 * sbuild_auth_get_uid:
 * @auth: an #Auth
 *
 * Get the uid of the user.  This is the uid to run as in the
 * session.
 *
 * Returns a uid.  This will be 0 if no user was set, or the user is
 * uid 0.
 */
uid_t
Auth::get_uid () const
{
  return this->uid;
}

/**
 * sbuild_auth_get_gid:
 * @auth: an #Auth
 *
 * Get the gid of the user.  This is the gid to run as in the
 * session.
 *
 * Returns a gid.  This will be 0 if no user was set, or the user is
 * gid 0.
 */
gid_t
Auth::get_gid () const
{
  return this->gid;
}

/**
 * sbuild_auth_get_user:
 * @auth: an #Auth
 *
 * Get the name of the user.  This is the user to run as in the
 * session.
 *
 * Returns a string.  This string points to internally allocated
 * storage and must not be freed, modified or stored.
 */
const std::string&
Auth::get_user () const
{
  return this->user;
}

/**
 * sbuild_auth_set_user:
 * @auth: an #Auth.
 * @user: the name to set.
 *
 * Get the name of the user.  This is the user to run as in the
 * session.
 *
 * As a side effect, the "uid", "gid", "home" and "shell" properties
 * will also be set.
 */
void
Auth::set_user (const std::string& user)
{
  this->uid = 0;
  this->gid = 0;
  this->home = "/";
  this->shell = "/bin/false";

  this->user = user;

  struct passwd *pwent = getpwnam(this->user.c_str());
  if (pwent == 0)
    {
      cerr << format_string(_("%s: user not found: %s\n"),
			    this->user.c_str(),
			    g_strerror(errno))
	   << endl;
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
 * @auth: an #Auth
 *
 * Get the command to run in the session.
 *
 * Returns a string vector.  This string vector points to internally
 * allocated storage and must not be freed, modified or stored.
 */
const Auth::string_list&
Auth::get_command () const
{
  return this->command;
}

/**
 * sbuild_auth_set_command:
 * @auth: an #Auth.
 * @command: the command to set.
 *
 * Set the command to run in the session.
 */
void
Auth::set_command (const Auth::string_list& command)
{
  this->command = command;
}

/**
 * sbuild_auth_get_home:
 * @auth: an #Auth
 *
 * Get the home directory.  This is the $HOME to set in the session,
 * if the user environment is not being preserved.
 *
 * Returns a string.  This string points to internally allocated
 * storage and must not be freed, modified or stored.
 */
const std::string&
Auth::get_home () const
{
  return this->home;
}

/**
 * sbuild_auth_get_shell:
 * @auth: an #Auth
 *
 * Get the name of the shell.  This is the shell to run as in the
 * session.
 *
 * Returns a string.  This string points to internally allocated
 * storage and must not be freed, modified or stored.
 */
const std::string&
Auth::get_shell () const
{
  return this->shell;
}

/**
 * sbuild_auth_get_environment:
 * @auth: an #Auth
 *
 * Get the environment to use in @auth.
 *
 * Returns a string vector.  This string vector points to internally
 * allocated storage and must not be freed, modified or stored.
 */
const Auth::env_list&
Auth::get_environment () const
{
  return this->environment;
}

void
Auth::set_environment (char **environment)
{
  set_environment(env_strv_to_env_list(environment));
}

/**
 * sbuild_auth_set_environment:
 * @auth: an #Auth
 * @environment: the environment to use
 *
 * Set the environment to use in @auth.
 */
void
Auth::set_environment (const Auth::env_list& environment)
{
  this->environment = environment;
}

/**
 * sbuild_auth_get_pam_environment:
 * @auth: an #Auth
 *
 * Get the PAM environment from @auth.
 *
 * Returns a string vector.  This string vector points to internally
 * allocated storage and must not be freed, modified or stored.
 */
Auth::env_list
Auth::get_pam_environment () const
{
  char **env =  pam_getenvlist(this->pam);

  return env_strv_to_env_list(env);
}

/**
 * sbuild_auth_get_ruid:
 * @auth: an #Auth
 *
 * Get the "remote uid" of the user.  This is the uid which is
 * requesting authentication.
 *
 * Returns a uid.
 */
uid_t
Auth::get_ruid () const
{
  return this->ruid;
}

/**
 * sbuild_auth_get_ruser:
 * @auth: an #Auth
 *
 * Get the "remote" name of the user.  This is the user which is
 * requesting authentication.
 *
 * Returns a string.  This string points to internally allocated
 * storage and must not be freed, modified or stored.
 */
const std::string&
Auth::get_ruser () const
{
  return this->ruser;
}

/**
 * sbuild_auth_get_verbosity:
 * @auth: an #Auth
 *
 * Get the message verbosity of @auth.
 *
 * Returns the #Verbosity verbosity level.
 */
Auth::Verbosity
Auth::get_verbosity () const
{
  return this->verbosity;
}

/**
 * sbuild_auth_set_verbosity:
 * @auth: an #Auth
 * @verbosity: the verbosity level to use
 *
 * Set the message verbosity of @auth.
 */
void
Auth::set_verbosity (Auth::Verbosity verbosity)
{
  this->verbosity = verbosity;
}

/**
 * sbuild_auth_get_conv:
 * @auth: an #Auth
 *
 * Get the conversation handler for @auth.
 *
 * Returns the handler object.
 */
std::tr1::shared_ptr<AuthConv>&
Auth::get_conv ()
{
  return this->conv;
}

/**
 * sbuild_auth_set_conv:
 * @auth: an #Auth
 * @conv: the conversation handler to use
 *
 * Set the conversation handler for @auth.
 */
void
Auth::set_conv (std::tr1::shared_ptr<AuthConv>& conv)
{
  this->conv = conv;
}

void
Auth::run ()
{
  try
    {
      start();
      authenticate();
      setupenv();
      account();
      try
	{
	  cred_establish();

	  const char *authuser = 0;
	  pam_get_item(this->pam, PAM_USER, (const void **) &authuser);
	  g_debug("PAM authentication succeeded for user %s\n", authuser);

	  run_impl();

	  /* The session is now finished, either
	     successfully or not.  All PAM operations are
	     now for cleanup and shutdown, and we must
	     clean up whether or not errors were raised at
	     any previous point.  This means only the
	     first error is reported back to the user. */

	  /* Don't cope with failure, since we are now
	     already bailing out, and an error may already
	     have been raised */
	}
      catch (const error& e)
	{
	  try
	    {
	      cred_delete();
	    }
	  catch (const error& discard)
	    {
	      throw error(e);
	    }
	}
    }
  catch (const error& e)
    {
      try
	{
	  /* Don't cope with failure, since we are now already bailing out,
	     and an error may already have been raised */
	  stop();
	}
      catch (const error& discard)
	{
	  throw error(e);
	}
    }
}

/**
 * sbuild_auth_start:
 * @auth: an #Auth
 * @error: a #GError
 *
 * Start the PAM system.  No other PAM functions may be called before
 * calling this function.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
void
Auth::start ()
{
  g_return_if_fail(this->user.empty());

  if (this->pam != 0)
    {
      g_debug("pam_start FAIL (already initialised)");
      throw error(_("PAM error: PAM is already initialised"),
		  ERROR_PAM_STARTUP);
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
      g_debug("pam_start FAIL");
      throw error(std::string(_("PAM error: ")) + pam_strerror(this->pam, pam_status),
		  ERROR_PAM_STARTUP);
    }

  g_debug("pam_start OK");
}

/**
 * sbuild_auth_stop:
 * @auth: an #Auth
 * @error: a #GError
 *
 * Stop the PAM system.  No other PAM functions may be used after
 * calling this function.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
void
Auth::stop ()
{
  g_return_if_fail(this->pam != 0); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_end(this->pam, PAM_SUCCESS)) != PAM_SUCCESS)
    {
      g_debug("pam_end FAIL");
      throw error(std::string(_("PAM error: ")) + pam_strerror(this->pam, pam_status),
		  ERROR_PAM_SHUTDOWN);
    }

  g_debug("pam_end OK");
}

/**
 * sbuild_auth_authenticate:
 * @auth: an #Auth
 * @error: a #GError
 *
 * Perform PAM authentication.  If required, the user will be prompted
 * to authenticate themselves.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
void
Auth::authenticate ()
{
  g_return_if_fail(this->user.empty());
  g_return_if_fail(this->pam != 0); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_set_item(this->pam, PAM_RUSER, this->ruser.c_str())) != PAM_SUCCESS)
    {
      g_debug("pam_set_item (PAM_RUSER) FAIL");
      throw error(std::string(_("PAM set RUSER error: ")) + pam_strerror(this->pam, pam_status),
		  ERROR_PAM_SET_ITEM);
    }

  long hl = 256; /* sysconf(_SC_HOST_NAME_MAX); BROKEN with Debian libc6 2.3.2.ds1-22 */

  char *hostname = g_new(char, hl);
  if (gethostname(hostname, hl) != 0)
    {
      g_debug("gethostname FAIL");
      throw error(std::string(_("Failed to get hostname: ")) + g_strerror(errno),
		  ERROR_HOSTNAME);
    }

  if ((pam_status =
       pam_set_item(this->pam, PAM_RHOST, hostname)) != PAM_SUCCESS)
    {
      g_debug("pam_set_item (PAM_RHOST) FAIL");
      throw error(std::string(_("PAM set RHOST error: ")) + pam_strerror(this->pam, pam_status),
		  ERROR_PAM_SET_ITEM);
    }

  g_free(hostname);
  hostname = 0;

  const char *tty = ttyname(STDIN_FILENO);
  if (tty)
    {
      if ((pam_status =
	   pam_set_item(this->pam, PAM_TTY, tty)) != PAM_SUCCESS)
	{
	  g_debug("pam_set_item (PAM_TTY) FAIL");
	  throw error(std::string(_("PAM set TTY error: ")) + pam_strerror(this->pam, pam_status),
		      ERROR_PAM_SET_ITEM);
	}
    }

  /* Authenticate as required. */
  switch (get_auth_status())
    {
    case STATUS_NONE:
      if ((pam_status =
	   pam_set_item(this->pam, PAM_USER, this->user.c_str())) != PAM_SUCCESS)
	{
	  g_debug("pam_set_item (PAM_USER) FAIL");
	  throw error(std::string(_("PAM set USER error: ")) + pam_strerror(this->pam, pam_status),
		      ERROR_PAM_SET_ITEM);
	}
      break;

    case STATUS_USER:
      if ((pam_status =
	   pam_authenticate(this->pam, 0)) != PAM_SUCCESS)
	{
	  g_debug("pam_authenticate FAIL");
	  syslog(LOG_AUTH|LOG_WARNING, "%s->%s Authentication failure",
		 this->ruser.c_str(), this->user.c_str());
	  throw error(std::string(_("PAM authentication failed: ")) + pam_strerror(this->pam, pam_status),
		      ERROR_PAM_AUTHENTICATE);
	}
      g_debug("pam_authenticate OK");
      break;

    case STATUS_FAIL:
	{
	  g_debug("PAM auth premature FAIL");
	  cerr << format_string(_("You do not have permission to access the %s service.\n"),
				this->service.c_str())
	       << "\n"
	       << _("This failure will be reported.")
	       << endl;
	  syslog(LOG_AUTH|LOG_WARNING,
		 "%s->%s Unauthorised",
		 this->ruser.c_str(), this->user.c_str());
	  throw error(_("access not authorised"),
		      ERROR_PAM_AUTHENTICATE);
	}
    default:
      break;
    }
}

/**
 * sbuild_auth_setupenv:
 * @auth: an #Auth
 * @error: a #GError
 *
 * Import the user environment into PAM.  If no environment was
 * specified with #sbuild_auth_set_environment, a minimal environment
 * will be created containing HOME, LOGNAME, PATH, TERM and LOGNAME.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
void
Auth::setupenv ()
{
  g_return_if_fail(this->pam != 0); // PAM must be initialised

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
	  g_debug("pam_putenv FAIL");
	  throw error(std::string(_("PAM error: ")) + pam_strerror(this->pam, pam_status),
		      ERROR_PAM_PUTENV);
	}
      g_debug("pam_putenv: set %s=%s", cur->first.c_str(), cur->second.c_str());
    }

  g_debug("pam_putenv OK");
}

/**
 * sbuild_auth_account:
 * @auth: an #Auth
 * @error: a #GError
 *
 * Do PAM account management (authorisation).
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
void
Auth::account ()
{
  g_return_if_fail(this->pam != 0); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_acct_mgmt(this->pam, 0)) != PAM_SUCCESS)
    {
      /* We don't handle changing expired passwords here, since we are
	 not login or ssh. */
      g_debug("pam_acct_mgmt FAIL");
      throw error(std::string(_("PAM error: ")) + pam_strerror(this->pam, pam_status),
		  ERROR_PAM_ACCOUNT);
    }

  g_debug("pam_acct_mgmt OK");
}

/**
 * sbuild_auth_cred_establish:
 * @auth: an #Auth
 * @error: a #GError
 *
 * Use PAM to establish credentials.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
void
Auth::cred_establish ()
{
  g_return_if_fail(this->pam != 0); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_setcred(this->pam, PAM_ESTABLISH_CRED)) != PAM_SUCCESS)
    {
      g_debug("pam_setcred FAIL");
      throw error(std::string(_("PAM error: \n")) + pam_strerror(this->pam, pam_status),
		  ERROR_PAM_CREDENTIALS);
    }

  g_debug("pam_setcred OK");
}

/**
 * sbuild_auth_cred_delete:
 * @auth: an #Auth
 * @error: a #GError
 *
 * Use PAM to delete credentials.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
void
Auth::cred_delete ()
{
  g_return_if_fail(this->pam != 0); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_setcred(this->pam, PAM_DELETE_CRED)) != PAM_SUCCESS)
    {
      g_debug("pam_setcred (delete) FAIL");
      throw error(std::string(_("PAM error: ")) + pam_strerror(this->pam, pam_status),
		  ERROR_PAM_DELETE_CREDENTIALS);
    }

  g_debug("pam_setcred (delete) OK");
}

/**
 * sbuild_auth_open_session:
 * @auth: an #Auth
 * @error: a #GError
 *
 * Open a PAM session.  This should be called in the child process.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
void
Auth::open_session ()
{
  g_return_if_fail(this->pam != 0); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_open_session(this->pam, 0)) != PAM_SUCCESS)
    {
      g_debug("pam_open_session FAIL");
      throw error(std::string(_("PAM error: ")) + pam_strerror(this->pam, pam_status),
		  ERROR_PAM_SESSION_OPEN);
    }

  g_debug("pam_open_session OK");
}

/**
 * sbuild_auth_close_session:
 * @auth: an #Auth
 * @error: a #GError
 *
 * Close a PAM session.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
void
Auth::close_session ()
{
  g_return_if_fail(this->pam != 0); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_close_session(this->pam, 0)) != PAM_SUCCESS)
    {
      g_debug("pam_close_session FAIL");
      throw error(std::string(_("PAM error: ")) + pam_strerror(this->pam, pam_status),
		  ERROR_PAM_SESSION_CLOSE);
    }

  g_debug("pam_close_session OK");
}

/**
 * sbuild_auth_require_auth_impl:
 * @auth: an #Auth
 *
 * Check if authentication is required for @auth.  This default
 * implementation always requires authentication.
 *
 * Returns the authentication type.
 */
Auth::Status
Auth::get_auth_status () const
{
  Status authtype = STATUS_NONE;

  authtype = change_auth(authtype, STATUS_USER);

  return authtype;
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
