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

#define _GNU_SOURCE
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <syslog.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "sbuild-auth.h"
#include "sbuild-error.h"
#include "sbuild-marshallers.h"
#include "sbuild-typebuiltins.h"

static gboolean
sbuild_auth_require_auth_accumulator (GSignalInvocationHint *ihint,
				      GValue                *return_accu,
				      const GValue          *handler_return,
				      gpointer               data);

static gboolean
sbuild_auth_boolean_accumulator (GSignalInvocationHint *ihint,
				 GValue                *return_accu,
				 const GValue          *handler_return,
				 gpointer               data);

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

enum
{
  PROP_0,
  PROP_SERVICE,
  PROP_UID,
  PROP_GID,
  PROP_USER,
  PROP_COMMAND,
  PROP_HOME,
  PROP_SHELL,
  PROP_ENV,
  PROP_RUID,
  PROP_RUSER,
  PROP_QUIET
};

enum
{
  SIGNAL_REQUIRE_AUTH,
  SIGNAL_SESSION_RUN,
  SIGNAL_LAST
};

static guint auth_signals[SIGNAL_LAST];

static GObjectClass *parent_class;

G_DEFINE_TYPE(SbuildAuth, sbuild_auth, G_TYPE_OBJECT)

/**
 * sbuild_auth_new:
 *
 * Creates a new #SbuildAuth.
 *
 * Returns the newly created #SbuildAuth.
 */
SbuildAuth *
sbuild_auth_new (void)
{
  /* TODO: add service property */
  return (SbuildAuth *) g_object_new(SBUILD_TYPE_AUTH,
				     NULL);
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
 * storage in the chroot and must not be freed, modified or stored.
 */
const gchar *
sbuild_auth_get_service (const SbuildAuth *restrict auth)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), 0);

  return auth->service;
}

/**
 * sbuild_auth_set_service:
 * @auth: an #SbuildAuth
 *
 * Set the PAM service name.  This is passed to pam_start() when
 * initialising PAM.  It must be set during object construction, and
 * MUST be a hard-coded (constant) string literal for security
 * reasons.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
static void
sbuild_auth_set_service (SbuildAuth  *auth,
			 const gchar *service)
{
  g_return_if_fail(SBUILD_IS_AUTH(auth));

  if (auth->service)
    g_free(auth->service);
  auth->service = g_strdup(service);
  g_object_notify(G_OBJECT(auth), "service");
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
sbuild_auth_get_uid (const SbuildAuth *restrict auth)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), 0);

  return auth->uid;
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
sbuild_auth_get_gid (const SbuildAuth *restrict auth)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), 0);

  return auth->gid;
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
const char *
sbuild_auth_get_user (const SbuildAuth *restrict auth)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), NULL);

  return auth->user;
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
sbuild_auth_set_user (SbuildAuth *auth,
		      const char *user)
{
  g_return_if_fail(SBUILD_IS_AUTH(auth));

  if (auth->user)
    {
      g_free(auth->user);
    }
  auth->user = g_strdup(user);
  if (auth->shell)
    {
      g_free(auth->shell);
      auth->shell = NULL;
    }

  if (user != NULL)
    {
      struct passwd *pwent = getpwnam(auth->user);
      if (pwent == NULL)
	{
	  g_printerr(_("%s: user not found: %s\n"), auth->user, g_strerror(errno));
	  exit (EXIT_FAILURE);
	}
      auth->uid = pwent->pw_uid;
      auth->gid = pwent->pw_gid;
      auth->home = g_strdup(pwent->pw_dir);
      auth->shell = g_strdup(pwent->pw_shell);
      g_debug("auth uid = %lu, gid = %lu", (unsigned long) auth->uid,
	      (unsigned long) auth->gid);
    }
  else
    {
      auth->uid = 0;
      auth->gid = 0;
      auth->home = g_strdup("/");
      auth->shell = g_strdup("/bin/false");
    }

  g_object_notify(G_OBJECT(auth), "uid");
  g_object_notify(G_OBJECT(auth), "gid");
  g_object_notify(G_OBJECT(auth), "user");
  g_object_notify(G_OBJECT(auth), "home");
  g_object_notify(G_OBJECT(auth), "shell");
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
char **
sbuild_auth_get_command (const SbuildAuth *restrict auth)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), NULL);

  return auth->command;
}

/**
 * sbuild_auth_set_command:
 * @auth: an #SbuildAuth.
 * @command: the command to set.
 *
 * Set the command to run in the session.
 */
void
sbuild_auth_set_command (SbuildAuth  *auth,
			 char          **command)
{
  g_return_if_fail(SBUILD_IS_AUTH(auth));

  if (auth->command)
    {
      g_strfreev(auth->command);
    }
  auth->command = g_strdupv(command);
  g_object_notify(G_OBJECT(auth), "command");
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
const char *
sbuild_auth_get_home (const SbuildAuth *restrict auth)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), NULL);

  return auth->home;
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
const char *
sbuild_auth_get_shell (const SbuildAuth *restrict auth)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), NULL);

  return auth->shell;
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
char **
sbuild_auth_get_environment (const SbuildAuth *restrict auth)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), NULL);

  return auth->environment;
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
char **
sbuild_auth_get_pam_environment (const SbuildAuth *restrict auth)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), NULL);
  g_return_val_if_fail(auth->pam != NULL, NULL);

  return pam_getenvlist(auth->pam);
}

/**
 * sbuild_auth_set_environment:
 * @auth: an #SbuildAuth
 * @environment: the environment to use
 *
 * Set the environment to use in @auth.
 */
void
sbuild_auth_set_environment (SbuildAuth  *auth,
			     char          **environment)
{
  g_return_if_fail(SBUILD_IS_AUTH(auth));

  if (auth->environment)
    {
      g_strfreev(auth->environment);
    }
  auth->environment = g_strdupv(environment);
  g_object_notify(G_OBJECT(auth), "environment");
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
sbuild_auth_get_ruid (const SbuildAuth *restrict auth)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), 0);

  return auth->ruid;
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
const char *
sbuild_auth_get_ruser (const SbuildAuth *restrict auth)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), NULL);

  return auth->ruser;
}

/**
 * sbuild_auth_get_quiet:
 * @auth: an #SbuildAuth
 *
 * Get the message verbosity of @auth.
 *
 * Returns TRUE if quiet messages are enabled, otherwise FALSE.
 */
gboolean
sbuild_auth_get_quiet (const SbuildAuth *restrict auth)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), FALSE);

  return auth->quiet;
}

/**
 * sbuild_auth_set_quiet:
 * @auth: an #SbuildAuth
 * @quiet: the quiet to use
 *
 * Set the message verbosity of @auth.
 */
void
sbuild_auth_set_quiet (SbuildAuth  *auth,
		       gboolean        quiet)
{
  g_return_if_fail(SBUILD_IS_AUTH(auth));

  auth->quiet = quiet;
  g_object_notify(G_OBJECT(auth), "quiet");
}

/**
 * sbuild_auth_require_auth_impl:
 * @auth: an #SbuildAuth
 *
 * Check if authentication is required for @auth.  If the user is root
 * (uid 0), no authentication is required, otherwise authentication is
 * always required.
 *
 * Returns the authentication type.
 */
static SbuildAuthStatus
sbuild_auth_require_auth_impl (SbuildAuth *auth)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), SBUILD_AUTH_STATUS_FAIL);

  SbuildAuthStatus authtype = SBUILD_AUTH_STATUS_NONE;

  if (auth->ruid == 0) // root has universal access
    authtype = sbuild_auth_change_auth(authtype, SBUILD_AUTH_STATUS_NONE);
  else if (auth->uid == 0) // changing to root
    authtype = sbuild_auth_change_auth(authtype, SBUILD_AUTH_STATUS_USER);
  else // non-root access
    authtype = sbuild_auth_change_auth(authtype, SBUILD_AUTH_STATUS_USER);

  return authtype;
}

/**
 * sbuild_auth_require_auth:
 * @auth: an #SbuildAuth
 *
 * Check if authentication is required for @auth.  If the user is root
 * (uid 0), no authentication is required, otherwise authentication is
 * always required.  This is a virtual function which may be
 * overridden in derived classes.
 *
 * The default behaviour is if the user is root (uid 0), no
 * authentication is required, otherwise authentication is always
 * required.
 *
 * Returns the authentication type.
 */
static SbuildAuthStatus
sbuild_auth_require_auth (SbuildAuth *auth)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), SBUILD_AUTH_STATUS_FAIL);
  g_return_val_if_fail(auth->user != NULL, SBUILD_AUTH_STATUS_FAIL);

  SbuildAuthStatus status;
  g_signal_emit(auth, auth_signals[SIGNAL_REQUIRE_AUTH],
		0, &status);

  return status;
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
sbuild_auth_start (SbuildAuth  *auth,
		   GError     **error)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), FALSE);
  g_return_val_if_fail(auth->user != NULL, FALSE);

  if (auth->pam != NULL)
    {
      g_set_error(error,
		  SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_STARTUP,
		  _("PAM error: PAM is already initialised"));
      g_debug("pam_start FAIL (already initialised)");
      return FALSE;
    }

  if (auth->service == NULL)
    {
      g_set_error(error,
		  SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_STARTUP,
		  _("PAM error: No service specified"));
      g_debug("pam_start FAIL (no service specified)");
      return FALSE;
    }

  int pam_status;
  if ((pam_status =
       pam_start(auth->service, auth->user,
		 &auth->conv, &auth->pam)) != PAM_SUCCESS)
    {
      g_set_error(error,
		  SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_STARTUP,
		  _("PAM error: %s"), pam_strerror(auth->pam, pam_status));
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
sbuild_auth_stop (SbuildAuth  *auth,
		  GError     **error)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), FALSE);
  g_return_val_if_fail(auth->pam != NULL, FALSE); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_end(auth->pam, PAM_SUCCESS)) != PAM_SUCCESS)
    {
      g_set_error(error,
		  SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_SHUTDOWN,
		  _("PAM error: %s"), pam_strerror(auth->pam, pam_status));
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
sbuild_auth_authenticate (SbuildAuth  *auth,
			  GError     **error)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), FALSE);
  g_return_val_if_fail(auth->user != NULL, FALSE);
  g_return_val_if_fail(auth->pam != NULL, FALSE); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_set_item(auth->pam, PAM_RUSER, auth->ruser)) != PAM_SUCCESS)
    {
      g_set_error(error,
		  SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_SET_ITEM,
		  _("PAM set RUSER error: %s"), pam_strerror(auth->pam, pam_status));
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
       pam_set_item(auth->pam, PAM_RHOST, hostname)) != PAM_SUCCESS)
    {
      g_set_error(error,
		  SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_SET_ITEM,
		  _("PAM set RHOST error: %s"), pam_strerror(auth->pam, pam_status));
      g_debug("pam_set_item (PAM_RHOST) FAIL");
      return FALSE;
    }

  g_free(hostname);
  hostname = NULL;

  const char *tty = ttyname(STDIN_FILENO);
  if (tty)
    {
      if ((pam_status =
	   pam_set_item(auth->pam, PAM_TTY, tty)) != PAM_SUCCESS)
	{
	  g_set_error(error,
		      SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_SET_ITEM,
		      _("PAM set TTY error: %s"), pam_strerror(auth->pam, pam_status));
	  g_debug("pam_set_item (PAM_TTY) FAIL");
	  return FALSE;
	}
    }

  /* Authenticate as required. */
  switch (sbuild_auth_require_auth (auth))
    {
    case SBUILD_AUTH_STATUS_NONE:
      if ((pam_status =
	   pam_set_item(auth->pam, PAM_USER, auth->user)) != PAM_SUCCESS)
	{
	  g_set_error(error,
		      SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_SET_ITEM,
		      _("PAM set USER error: %s"), pam_strerror(auth->pam, pam_status));
	  g_debug("pam_set_item (PAM_USER) FAIL");
	  return FALSE;
	}
      break;

    case SBUILD_AUTH_STATUS_USER:
      if ((pam_status =
	   pam_authenticate(auth->pam, 0)) != PAM_SUCCESS)
	{
	  g_set_error(error,
		      SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_AUTHENTICATE,
		      _("PAM authentication failed: %s\n"), pam_strerror(auth->pam, pam_status));
	  g_debug("pam_authenticate FAIL");
	  syslog(LOG_AUTH|LOG_WARNING, "%s->%s Authentication failure",
		 auth->ruser, auth->user);
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
	  g_printerr(_("You do not have permission to access the specified chroots.\n"));
	  g_printerr(_("This failure will be reported.\n"));
	  syslog(LOG_AUTH|LOG_WARNING,
		 "%s->%s Unauthorised",
		 auth->ruser, auth->user);
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
sbuild_auth_setupenv (SbuildAuth  *auth,
		      GError     **error)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), FALSE);
  g_return_val_if_fail(auth->pam != NULL, FALSE); // PAM must be initialised

  int pam_status;

  /* Initial environment to set, used if the environment was not
     specified. */
  char **newenv = g_new(char *, 6); // Size must be at least max envvars + 1
  guint i = 0;

  if (auth->uid == 0)
    newenv[i++] = g_strdup_printf("PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/bin/X11");
  else
    newenv[i++] = g_strdup("PATH=/usr/local/bin:/usr/bin:/bin:/usr/bin/X11:/usr/games");

  if (auth->home)
    newenv[i++] = g_strdup_printf("HOME=%s", auth->home);
  else
    newenv[i++] = g_strdup_printf("HOME=%s", "/");
  if (auth->user)
    {
      newenv[i++] = g_strdup_printf("LOGNAME=%s", auth->user);
      newenv[i++] = g_strdup_printf("USER=%s", auth->user);
    }
  {
    const char *term = g_getenv("TERM");
    if (term)
      newenv[i++] = g_strdup_printf("TERM=%s", term);
  }

  newenv[i] = NULL;

  char **environment = auth->environment != NULL ? auth->environment : newenv;
  for (guint j=0; environment[j] != NULL; ++j)
    {
      if ((pam_status =
	   pam_putenv(auth->pam, environment[j])) != PAM_SUCCESS)
	{
	  g_set_error(error,
		      SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_PUTENV,
		      _("PAM error: %s\n"), pam_strerror(auth->pam, pam_status));
	  g_debug("pam_putenv FAIL");
	  return FALSE;
	}
      g_debug("pam_putenv: set %s", environment[j]);
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
sbuild_auth_account (SbuildAuth  *auth,
		     GError     **error)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), FALSE);
  g_return_val_if_fail(auth->pam != NULL, FALSE); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_acct_mgmt(auth->pam, 0)) != PAM_SUCCESS)
    {
      /* We don't handle changing expired passwords here, since we are
	 not login or ssh. */
      g_set_error(error,
		  SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_ACCOUNT,
		  _("PAM error: %s\n"), pam_strerror(auth->pam, pam_status));
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
sbuild_auth_cred_establish (SbuildAuth  *auth,
			    GError     **error)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), FALSE);
  g_return_val_if_fail(auth->pam != NULL, FALSE); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_setcred(auth->pam, PAM_ESTABLISH_CRED)) != PAM_SUCCESS)
    {
      g_set_error(error,
		  SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_CREDENTIALS,
		  _("PAM error: %s\n"), pam_strerror(auth->pam, pam_status));
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
sbuild_auth_cred_delete (SbuildAuth  *auth,
			 GError     **error)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), FALSE);
  g_return_val_if_fail(auth->pam != NULL, FALSE); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_setcred(auth->pam, PAM_DELETE_CRED)) != PAM_SUCCESS)
    {
      g_set_error(error,
		  SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_DELETE_CREDENTIALS,
		  _("PAM error: %s"), pam_strerror(auth->pam, pam_status));
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
sbuild_auth_open_session (SbuildAuth  *auth,
			  GError     **error)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), FALSE);
  g_return_val_if_fail(auth->pam != NULL, FALSE); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_open_session(auth->pam, 0)) != PAM_SUCCESS)
    {
      g_set_error(error,
		  SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_SESSION_OPEN,
		  _("PAM error: %s"), pam_strerror(auth->pam, pam_status));
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
sbuild_auth_close_session (SbuildAuth  *auth,
			   GError     **error)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), FALSE);
  g_return_val_if_fail(auth->pam != NULL, FALSE); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_close_session(auth->pam, 0)) != PAM_SUCCESS)
    {
      g_set_error(error,
		  SBUILD_AUTH_ERROR, SBUILD_AUTH_ERROR_PAM_SESSION_CLOSE,
		  _("PAM error: %s"), pam_strerror(auth->pam, pam_status));
      g_debug("pam_close_session FAIL");
      return FALSE;
    }

  g_debug("pam_close_session OK");
  return TRUE;
}

/**
 * sbuild_auth_run:
 * @auth: an #SbuildAuth
 * @error: a #GError, or NULL to ignore errors
 *
 * Run the authentication process.
 *
 * If required, the user may be required to authenticate themselves.
 * This usually occurs when running as a different user.  The user
 * must be a member of the appropriate groups in order to satisfy the
 * groups and root-groups requirements in the chroot configuration.
 *
 * If authentication and authorisation succeed, the "session-run"
 * signal will be emitted.  Derived classes should override the
 * session_run vfunc, or connect signal handlers.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
gboolean
sbuild_auth_run (SbuildAuth  *auth,
		 GError     **error)
{
  g_return_val_if_fail(SBUILD_IS_AUTH(auth), FALSE);

  /* PAM setup. */
  gboolean retval;
  GError *tmp_error = NULL;

  sbuild_auth_start(auth, &tmp_error);
  if (tmp_error == NULL)
    {
      sbuild_auth_authenticate(auth, &tmp_error);
      if (tmp_error == NULL)
	{
	  sbuild_auth_setupenv(auth, &tmp_error);
	  if (tmp_error == NULL)
	    {
	      sbuild_auth_account(auth, &tmp_error);
	      if (tmp_error == NULL)
		{
		  sbuild_auth_cred_establish(auth, &tmp_error);
		  if (tmp_error == NULL)
		    {
		      const char *authuser = NULL;
		      pam_get_item(auth->pam, PAM_USER, (const void **) &authuser);
		      g_debug("PAM authentication succeeded for user %s\n", authuser);

		      g_signal_emit(auth, auth_signals[SIGNAL_SESSION_RUN],
				    0, &tmp_error, &retval);

		      /* The session is now finished, either
			 successfully or not.  All PAM operations are
			 now for cleanup and shutdown, and we must
			 clean up whether or not errors were raised at
			 any previous point.  This means only the
			 first error is reported back to the user. */

		      /* Don't cope with failure, since we are now
			 already bailing out, and an error may already
			 have been raised*/
		      sbuild_auth_cred_delete(auth,
					      (tmp_error != NULL) ? NULL : &tmp_error);

		    } // pam_cred_establish
		} // pam_account
	    } // pam_setupenv
	} // pam_auth
      /* Don't cope with failure, since we are now already bailing out,
	 and an error may already have been raised*/
      sbuild_auth_stop(auth,
		       (tmp_error != NULL) ? NULL : &tmp_error);
    } // pam_start

  if (tmp_error != NULL)
    {
      g_propagate_error(error, tmp_error);
      return FALSE;
    }
  else
    return TRUE;
}

/**
 * sbuild_auth_session_run_impl:
 * @error: a #GError, or NULL to ignore errors
 *
 * Default session implementation which does nothing.
 *
 * Returns TRUE (it can't fail).
 */
static gboolean
sbuild_auth_session_run_impl (SbuildAuth  *auth,
			      GError     **error)
{
  return TRUE;
}

static void
sbuild_auth_init (SbuildAuth *auth)
{
  g_return_if_fail(SBUILD_IS_AUTH(auth));

  auth->service = NULL;
  auth->user = NULL;
  auth->uid = 0;
  auth->gid = 0;
  auth->command = NULL;
  auth->home = NULL;
  auth->shell = NULL;
  auth->environment = NULL;
  auth->pam = NULL;
  auth->conv.conv = misc_conv;
  auth->conv.appdata_ptr = (gpointer) auth;
  auth->quiet = FALSE;

  /* Current user's details. */
  auth->ruid = getuid();
  struct passwd *pwent = getpwuid(auth->ruid);
  if (pwent == NULL)
    {
      g_printerr(_("%lu: user not found: %s\n"), (unsigned long) auth->ruid,
		 g_strerror(errno));
      exit (EXIT_FAILURE);
    }
  auth->ruser = g_strdup(pwent->pw_name);

  g_object_notify(G_OBJECT(auth), "ruid");
  g_object_notify(G_OBJECT(auth), "ruser");

  /* By default, the auth user is the same as the remote user. */
  sbuild_auth_set_user(auth, auth->ruser);
}

static void
sbuild_auth_finalize (SbuildAuth *auth)
{
  g_return_if_fail(SBUILD_IS_AUTH(auth));

  if (auth->service)
    {
      g_free(auth->service);
      auth->service = NULL;
    }
  if (auth->user)
    {
      g_free (auth->user);
      auth->user = NULL;
    }
  if (auth->command)
    {
      g_strfreev (auth->command);
      auth->command = NULL;
    }
  if (auth->home)
    {
      g_free (auth->home);
      auth->home = NULL;
    }
  if (auth->shell)
    {
      g_free (auth->shell);
      auth->shell = NULL;
    }
  if (auth->ruser)
    {
      g_free (auth->ruser);
      auth->ruser = NULL;
    }

  if (parent_class->finalize)
    parent_class->finalize(G_OBJECT(auth));
}

static void
sbuild_auth_set_property (GObject      *object,
			  guint         param_id,
			  const GValue *value,
			  GParamSpec   *pspec)
{
  SbuildAuth *auth;

  g_return_if_fail (object != NULL);
  g_return_if_fail (SBUILD_IS_AUTH (object));

  auth = SBUILD_AUTH(object);

  switch (param_id)
    {
    case PROP_SERVICE:
      sbuild_auth_set_service(auth, g_value_get_string(value));
      break;
    case PROP_USER:
      sbuild_auth_set_user(auth, g_value_get_string(value));
      break;
    case PROP_COMMAND:
      sbuild_auth_set_command(auth, g_value_get_boxed(value));
      break;
    case PROP_ENV:
      sbuild_auth_set_environment(auth, g_value_get_boxed(value));
      break;
    case PROP_QUIET:
      sbuild_auth_set_quiet(auth, g_value_get_boolean(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
sbuild_auth_get_property (GObject    *object,
			  guint       param_id,
			  GValue     *value,
			  GParamSpec *pspec)
{
  SbuildAuth *auth;

  g_return_if_fail (object != NULL);
  g_return_if_fail (SBUILD_IS_AUTH (object));

  auth = SBUILD_AUTH(object);

  switch (param_id)
    {
    case PROP_SERVICE:
      g_value_set_string(value, auth->service);
    case PROP_UID:
      g_value_set_int(value, auth->uid);
      break;
    case PROP_GID:
      g_value_set_int(value, auth->gid);
      break;
    case PROP_USER:
      g_value_set_string(value, auth->user);
      break;
    case PROP_COMMAND:
      g_value_set_boxed(value, auth->command);
      break;
    case PROP_HOME:
      g_value_set_string(value, auth->home);
      break;
    case PROP_SHELL:
      g_value_set_string(value, auth->shell);
      break;
    case PROP_ENV:
      g_value_set_boxed(value, auth->environment);
      break;
    case PROP_RUID:
      g_value_set_int(value, auth->ruid);
      break;
    case PROP_RUSER:
      g_value_set_string(value, auth->ruser);
      break;
    case PROP_QUIET:
      g_value_set_boolean(value, auth->quiet);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
sbuild_auth_class_init (SbuildAuthClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  parent_class = g_type_class_peek_parent (klass);

  klass->require_auth = sbuild_auth_require_auth_impl;
  klass->session_run = sbuild_auth_session_run_impl;

  gobject_class->finalize = (GObjectFinalizeFunc) sbuild_auth_finalize;
  gobject_class->set_property = (GObjectSetPropertyFunc) sbuild_auth_set_property;
  gobject_class->get_property = (GObjectGetPropertyFunc) sbuild_auth_get_property;

  g_object_class_install_property
    (gobject_class,
     PROP_SERVICE,
     g_param_spec_string ("service", "Service",
			  "The PAM service name",
			  "",
			  (G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY)));

  g_object_class_install_property
    (gobject_class,
     PROP_UID,
     g_param_spec_int ("uid", "UID",
		       "The UID to run as in the session",
		       0, G_MAXINT, 0,
		       G_PARAM_READABLE));

  g_object_class_install_property
    (gobject_class,
     PROP_GID,
     g_param_spec_int ("gid", "GID",
		       "The GID to run as in the session",
		       0, G_MAXINT, 0,
		       G_PARAM_READABLE));

  g_object_class_install_property
    (gobject_class,
     PROP_USER,
     g_param_spec_string ("user", "User",
			  "The user login name to run as in the session",
			  "",
			  (G_PARAM_READABLE | G_PARAM_WRITABLE)));

  g_object_class_install_property
    (gobject_class,
     PROP_COMMAND,
     g_param_spec_boxed ("command", "Command",
			 "The command to run in the session, or NULL for a login shell",
			 G_TYPE_STRV,
			 (G_PARAM_READABLE | G_PARAM_WRITABLE)));

  g_object_class_install_property
    (gobject_class,
     PROP_HOME,
     g_param_spec_string ("home", "Home",
			  "The home directory for the user in the session",
			  "/bin/false",
			  G_PARAM_READABLE));

  g_object_class_install_property
    (gobject_class,
     PROP_SHELL,
     g_param_spec_string ("shell", "Shell",
			  "The login shell for the user to run as in the session",
			  "/bin/false",
			  G_PARAM_READABLE));

  g_object_class_install_property
    (gobject_class,
     PROP_ENV,
     g_param_spec_boxed ("environment", "Environment",
                         "The user environment to set in the session",
                         G_TYPE_STRV,
                         (G_PARAM_READABLE | G_PARAM_WRITABLE)));

  g_object_class_install_property
    (gobject_class,
     PROP_RUID,
     g_param_spec_int ("ruid", "Remote UID",
		       "The UID of the current user",
		       0, G_MAXINT, 0,
		       G_PARAM_READABLE));

  g_object_class_install_property
    (gobject_class,
     PROP_RUSER,
     g_param_spec_string ("ruser", "Remote user",
			  "The current user's login name",
			  "",
			  G_PARAM_READABLE));

  g_object_class_install_property
    (gobject_class,
     PROP_QUIET,
     g_param_spec_boolean ("quiet", "Quiet",
			   "Suppress non-essential messages",
			   FALSE,
			   G_PARAM_READABLE | G_PARAM_WRITABLE));

  auth_signals[SIGNAL_REQUIRE_AUTH] =
    g_signal_new ("require-auth",
                  G_OBJECT_CLASS_TYPE(klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (SbuildAuthClass, require_auth),
		  sbuild_auth_require_auth_accumulator, NULL,
                  sbuild_cclosure_marshal_ENUM__VOID,
                  SBUILD_TYPE_AUTH_STATUS, 0);

  auth_signals[SIGNAL_SESSION_RUN] =
    g_signal_new ("session-run",
                  G_OBJECT_CLASS_TYPE(klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (SbuildAuthClass, session_run),
		  sbuild_auth_boolean_accumulator, NULL,
                  sbuild_cclosure_marshal_BOOLEAN__BOXED,
                  G_TYPE_BOOLEAN, 1, SBUILD_TYPE_ERROR_POINTER);
}

/**
 * sbuild_auth_require_auth_accumulator:
 * @ihint: an invocation hint
 * @return_accu: a GValue to return to the caller
 * @handler_return: a GValue containing the return value of the last signal handler.
 * @data: user data
 *
 * Accumulate the results of "require-auth" checking.  This uses
 * sbuild_auth_change_auth to alter the authentication status.
 *
 * Returns FALSE if @return_accu is set to SBUILD_AUTH_STATUS_FAIL,
 * because continuing the signal emission is pointless, otherwise it
 * returns TRUE to continue signal emission.
 *
 */
static gboolean
sbuild_auth_require_auth_accumulator (GSignalInvocationHint *ihint,
				      GValue                *return_accu,
				      const GValue          *handler_return,
				      gpointer               data)
{
  SbuildAuthStatus new_status = g_value_get_enum(handler_return);
  SbuildAuthStatus status = g_value_get_enum(handler_return);

  status = sbuild_auth_change_auth(status, new_status);

  g_value_set_enum(return_accu, status);

  /* Stop signal emission if we have already failed. */
  if (status == SBUILD_AUTH_STATUS_FAIL)
    return FALSE;
  else
    return TRUE;
}

/**
 * sbuild_auth_boolean_accumulator:
 * @ihint: an invocation hint
 * @return_accu: a GValue to return to the caller
 * @handler_return: a GValue containing the return value of the last signal handler.
 * @data: user data
 *
 * Check if signal emission should continue.
 *
 * Returns FALSE if @handler return is set to FALSE, ending signal
 * emission, or TRUE if @handler return is set to TRUE, continuing
 * signal emission.
 */
static gboolean
sbuild_auth_boolean_accumulator (GSignalInvocationHint *ihint,
				 GValue                *return_accu,
				 const GValue          *handler_return,
				 gpointer               data)
{
  gboolean signal_return = g_value_get_boolean(handler_return);
  g_value_set_boolean(return_accu, signal_return);

  return signal_return;
}



/*
 * Local Variables:
 * mode:C
 * End:
 */
