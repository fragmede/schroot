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

#include <sys/types.h>
#include <sys/wait.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#include <security/pam_appl.h>
#include <security/pam_misc.h>

#include <glib.h>
#include <glib/gprintf.h>
#include <glib-object.h>

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

#define SBUILD_TYPE_AUTH		  (sbuild_auth_get_type ())
#define SBUILD_AUTH(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), SBUILD_TYPE_AUTH, SbuildAuth))
#define SBUILD_AUTH_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), SBUILD_TYPE_AUTH, SbuildAuthClass))
#define SBUILD_IS_AUTH(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SBUILD_TYPE_AUTH))
#define SBUILD_IS_AUTH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SBUILD_TYPE_AUTH))
#define SBUILD_AUTH_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SBUILD_TYPE_AUTH, SbuildAuthClass))

typedef struct _SbuildAuth SbuildAuth;
typedef struct _SbuildAuthClass SbuildAuthClass;

typedef SbuildAuthStatus (*SbuildAuthRequireAuthFunc)(SbuildAuth *auth);
typedef gboolean (*SbuildAuthSessionRunFunc)(SbuildAuth *auth, GError **error);

struct _SbuildAuth
{
  GObject           parent;
  gchar            *service;
  uid_t             uid;
  gid_t             gid;
  gchar            *user;
  gchar           **command;
  gchar            *home;
  gchar            *shell;
  gchar           **environment;
  uid_t             ruid;
  gchar            *ruser;
  pam_handle_t     *pam;
  struct pam_conv   conv;
  gboolean          quiet;
};

struct _SbuildAuthClass
{
  GObjectClass              parent;
  SbuildAuthRequireAuthFunc require_auth;
  SbuildAuthSessionRunFunc  session_run;
};


GType
sbuild_auth_get_type (void);

SbuildAuth *
sbuild_auth_new (void);

const gchar *
sbuild_auth_get_service (const SbuildAuth *restrict auth);

uid_t
sbuild_auth_get_uid (const SbuildAuth *restrict auth);

gid_t
sbuild_auth_get_gid (const SbuildAuth *restrict auth);

const char *
sbuild_auth_get_user (const SbuildAuth *restrict auth);

void
sbuild_auth_set_user (SbuildAuth *auth,
		      const char *user);

char **
sbuild_auth_get_command (const SbuildAuth *restrict auth);

void
sbuild_auth_set_command (SbuildAuth  *auth,
			 char       **command);

const char *
sbuild_auth_get_home (const SbuildAuth *restrict auth);

const char *
sbuild_auth_get_shell (const SbuildAuth *restrict auth);

char **
sbuild_auth_get_environment (const SbuildAuth *restrict auth);

void
sbuild_auth_set_environment (SbuildAuth  *auth,
			     char       **environment);

char **
sbuild_auth_get_pam_environment (const SbuildAuth *restrict auth);

uid_t
sbuild_auth_get_ruid (const SbuildAuth *restrict auth);

const char *
sbuild_auth_get_ruser (const SbuildAuth *restrict auth);

gboolean
sbuild_auth_get_quiet (const SbuildAuth *restrict auth);

void
sbuild_auth_set_quiet (SbuildAuth  *auth,
		       gboolean     quiet);

gboolean
sbuild_auth_run (SbuildAuth  *auth,
		 GError     **error);

gboolean
sbuild_auth_start (SbuildAuth  *auth,
		   GError     **error);

gboolean
sbuild_auth_stop (SbuildAuth  *auth,
		  GError     **error);

gboolean
sbuild_auth_authenticate (SbuildAuth  *auth,
			  GError     **error);

gboolean
sbuild_auth_setupenv (SbuildAuth  *auth,
		      GError     **error);

gboolean
sbuild_auth_account (SbuildAuth  *auth,
		     GError     **error);

gboolean
sbuild_auth_cred_establish (SbuildAuth  *auth,
			    GError     **error);

gboolean
sbuild_auth_cred_delete (SbuildAuth  *auth,
			 GError     **error);

gboolean
sbuild_auth_open_session (SbuildAuth  *auth,
			  GError     **error);

gboolean
sbuild_auth_close_session (SbuildAuth  *auth,
			   GError     **error);

/**
 * sbuild_auth_change_auth:
 * @oldauth: the current authentication status
 * @newauth: the new authentication status
 *
 * Set new authentication status.  If @newauth > @oldauth, @newauth is
 * returned, otherwise @oldauth is returned.  This is to ensure the
 * authentication status can never be decreased.
 *
 * Returns the new authentication status.
 */
static inline SbuildAuthStatus
sbuild_auth_change_auth (SbuildAuthStatus oldauth,
			 SbuildAuthStatus newauth)
{
  /* Ensure auth level always escalates. */
  if (newauth > oldauth)
    return newauth;
  else
    return oldauth;
}

#endif /* SBUILD_AUTH_H */

/*
 * Local Variables:
 * mode:C
 * End:
 */
