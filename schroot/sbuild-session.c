/* sbuild-session - sbuild session object
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
 * SECTION:sbuild-session
 * @short_description: session object
 * @title: SbuildSession
 *
 * This object provides the session handling for schroot.  It derives
 * from SbuildAuth, which performs all the necessary PAM actions,
 * specialising it by overriding its virtual functions.  This allows
 * more sophisticated handling of user authorisation (groups and
 * root-groups membership in the configuration file) and session
 * management (setting up the session, entering the chroot and running
 * the requested commands or shell).
 */

#include <config.h>

#define _GNU_SOURCE
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <syslog.h>

#include <glib.h>
#include <glib/gi18n.h>

#include <uuid/uuid.h>

#include "sbuild-typebuiltins.h"
#include "sbuild-session.h"
#include "sbuild-chroot-lvm-snapshot.h"

/**
 * sbuild_session_error_quark:
 *
 * Get the SBUILD_SESSION_ERROR domain number.
 *
 * Returns the domain.
 */
GQuark
sbuild_session_error_quark (void)
{
  static GQuark error_quark = 0;

  if (error_quark == 0)
    error_quark = g_quark_from_static_string ("sbuild-session-error-quark");

  return error_quark;
}

enum
{
  PROP_0,
  PROP_CONFIG,
  PROP_CHROOTS,
  PROP_OPERATION,
  PROP_SESSION_ID,
  PROP_FORCE,
  PROP_CHILD_STATUS
};

static GObjectClass *parent_class;

G_DEFINE_TYPE(SbuildSession, sbuild_session, SBUILD_TYPE_AUTH)

/**
 * sbuild_session_new:
 * @service: the PAM service name
 * @config: an #SbuildConfig
 * @operation: the #SbuildSessionOperation to perform
 * @chroots: the chroots to use
 *
 * Creates a new #SbuildSession.  The session will use the provided
 * configuration data, and will run in the list of chroots specified.
 * @service MUST be a constant string literal, for security reasons;
 * the application service name should be hard-coded.
 *
 * Returns the newly created #SbuildSession.
 */
SbuildSession *
sbuild_session_new(const char              *service,
		   SbuildConfig            *config,
		   SbuildSessionOperation   operation,
		   char                   **chroots)
{
  return (SbuildSession *) g_object_new(SBUILD_TYPE_SESSION,
					"service", service,
					"config", config,
					"operation", operation,
					"chroots", chroots,
					NULL);
}

/**
 * sbuild_session_get_config:
 * @session: an #SbuildSession
 *
 * Get the configuration associated with @session.
 *
 * Returns an #SbuildConfig.
 */
SbuildConfig *
sbuild_session_get_config (const SbuildSession *restrict session)
{
  g_return_val_if_fail(SBUILD_IS_SESSION(session), NULL);

  return session->config;
}

/**
 * sbuild_session_set_config:
 * @session: an #SbuildSession
 * @config: an #SbuildConfig
 *
 * Set the configuration associated with @session.
 */
void
sbuild_session_set_config (SbuildSession *session,
			   SbuildConfig  *config)
{
  g_return_if_fail(SBUILD_IS_SESSION(session));

  if (session->config)
    {
      g_object_unref(G_OBJECT(session->config));
    }
  session->config = config;
  g_object_ref(G_OBJECT(session->config));
  g_object_notify(G_OBJECT(session), "config");
}

/**
 * sbuild_session_get_operation:
 * @session: an #SbuildSession
 *
 * Get the operation @session will perform.
 *
 * Returns an #SbuildConfig.
 */
SbuildSessionOperation
sbuild_session_get_operation (const SbuildSession  *restrict session)
{
  g_return_val_if_fail(SBUILD_IS_SESSION(session), SBUILD_SESSION_OPERATION_AUTOMATIC);

  return session->operation;
}

/**
 * sbuild_session_set_operation:
 * @session: an #SbuildSession
 * @operation: an #SbuildSessionOperation
 *
 * Set the operation @session will perform.
 */
void
sbuild_session_set_operation (SbuildSession          *session,
			      SbuildSessionOperation operation)
{
  g_return_if_fail(SBUILD_IS_SESSION(session));

  session->operation = operation;
  g_object_notify(G_OBJECT(session), "operation");
}


/**
 * sbuild_session_get_session_id:
 * @session: an #SbuildSession
 *
 * Get the session identifier.  The session identifier is a unique
 * string to identify a session.
 *
 * Returns a string.  The string must be freed by the caller.
 */
gchar *
sbuild_session_get_session_id (const SbuildSession  *restrict session)
{
  g_return_val_if_fail(SBUILD_IS_SESSION(session), NULL);

  return session->session_id;
}

/**
 * sbuild_session_set_session_id:
 * @session: an #SbuildSession
 * @session_id: an string containing a valid session id
 *
 * Set the session identifier for @session.
 */
void
sbuild_session_set_session_id (SbuildSession  *session,
			       const gchar    *session_id)
{
  g_return_if_fail(SBUILD_IS_SESSION(session));

  if (session->session_id)
    {
      g_free(session->session_id);
    }
  session->session_id = g_strdup(session_id);
  g_object_notify(G_OBJECT(session), "session-id");
}

/**
 * sbuild_session_get_force:
 * @session: an #SbuildSession
 *
 * Get the force status of @session.
 *
 * Returns TRUE if operation will be forced, otherwise FALSE.
 */
gboolean
sbuild_session_get_force (const SbuildSession *restrict session)
{
  g_return_val_if_fail(SBUILD_IS_SESSION(session), FALSE);

  return session->force;
}

/**
 * sbuild_session_set_force:
 * @session: an #SbuildSession
 * @force: TRUE to force session operation, otherwise FALSE
 *
 * Set the force status of @session.
 */
void
sbuild_session_set_force (SbuildSession *session,
			  gboolean       force)
{
  g_return_if_fail(SBUILD_IS_SESSION(session));

  session->force = force;
  g_object_notify(G_OBJECT(session), "force");
}

/**
 * sbuild_session_get_chroots:
 * @session: an #SbuildSession
 *
 * Get the chroots to use in @session.
 *
 * Returns a string vector. This string vector points to internally
 * allocated storage in the chroot and must not be freed, modified or
 * stored.
 */
char **
sbuild_session_get_chroots (const SbuildSession *restrict session)
{
  g_return_val_if_fail(SBUILD_IS_SESSION(session), NULL);

  return session->chroots;
}

/**
 * sbuild_session_set_chroots:
 * @session: an #SbuildSession
 * @chroots: the chroots to use
 *
 * Set the chroots to use in @session.
 */
void
sbuild_session_set_chroots (SbuildSession  *session,
			    char         **chroots)
{
  g_return_if_fail(SBUILD_IS_SESSION(session));

  if (session->chroots)
    {
      g_strfreev(session->chroots);
    }
  session->chroots = g_strdupv(chroots);
  g_object_notify(G_OBJECT(session), "chroots");
}

/**
 * is_group_member:
 * @group: the group to check for
 *
 * Check group membership.
 *
 * Returns TRUE if the user is a member of @group, otherwise FALSE.
 */
static gboolean
is_group_member (const char *group)
{
  errno = 0;
  struct group *groupbuf = getgrnam(group);
  if (groupbuf == NULL)
    {
      if (errno == 0)
	g_printerr(_("%s: group not found\n"), group);
      else
	g_printerr(_("%s: group not found: %s\n"), group, g_strerror(errno));
      exit (EXIT_FAILURE);
    }

  int supp_group_count = getgroups(0, NULL);
  if (supp_group_count < 0)
    {
      g_printerr(_("can't get supplementary group count: %s\n"), g_strerror(errno));
      exit (EXIT_FAILURE);
    }
  gid_t supp_groups[supp_group_count];
  if (getgroups(supp_group_count, supp_groups) < 1)
    {
      g_printerr(_("can't get supplementary groups: %s\n"), g_strerror(errno));
      exit (EXIT_FAILURE);
    }

  gboolean group_member = FALSE;

  for (int i = 0; i < supp_group_count; ++i)
    {
      if (groupbuf->gr_gid == supp_groups[i])
	group_member = TRUE;
    }

  return group_member;
}

/**
 * sbuild_session_get_child_status:
 * @session: an #SbuildSession
 *
 * Get the exit (wait) status of the last child process to run in this
 * session.
 *
 * Returns the exit status.
 */
int
sbuild_session_get_child_status (SbuildSession *session)
{
  g_return_val_if_fail(SBUILD_IS_SESSION(session), EXIT_FAILURE);

  return session->child_status;
}

/**
 * sbuild_session_require_auth:
 * @session: an #SbuildSession
 *
 * Check if authentication is required for @session.  Group membership
 * is checked for all chroots, and depending on which user will be run
 * in the chroot, password authentication or no authentication may be
 * required.
 *
 * Returns the authentication type.
 */
static SbuildAuthStatus
sbuild_session_require_auth (SbuildSession *session)
{
  g_return_val_if_fail(SBUILD_IS_SESSION(session), SBUILD_AUTH_STATUS_FAIL);
  g_return_val_if_fail(session->chroots != NULL, SBUILD_AUTH_STATUS_FAIL);
  g_return_val_if_fail(session->config != NULL, SBUILD_AUTH_STATUS_FAIL);

  SbuildAuth *auth = SBUILD_AUTH(session);
  SbuildAuthStatus status = SBUILD_AUTH_STATUS_NONE;

  for (guint i=0; session->chroots[i] != NULL; ++i)
    {
      SbuildChroot *chroot = sbuild_config_find_alias(session->config,
						      session->chroots[i]);
      if (chroot == NULL) // Should never happen, but cater for it anyway.
	{
	  g_warning(_("No chroot found matching alias '%s'"), session->chroots[i]);
	  status = sbuild_auth_change_auth(status, SBUILD_AUTH_STATUS_FAIL);
	}

      char **groups = sbuild_chroot_get_groups(chroot);
      char **root_groups = sbuild_chroot_get_root_groups(chroot);

      if (groups != NULL)
	{
	  gboolean in_groups = FALSE;
	  gboolean in_root_groups = FALSE;

	  if (groups != NULL)
	    {
	      for (guint y=0; groups[y] != 0; ++y)
		if (is_group_member(groups[y]) == TRUE)
		  in_groups = TRUE;
	    }

	  if (root_groups != NULL)
	    {
	      for (guint y=0; root_groups[y] != 0; ++y)
		if (is_group_member(root_groups[y]) == TRUE)
		  in_root_groups = TRUE;
	    }

	  /*
	   * No auth required if in root groups and changing to root,
	   * or if the uid is not changing.  If not in a group,
	   * authentication fails immediately.
	   */
	  if (in_groups == TRUE &&
	      ((auth->uid == 0 && in_root_groups == TRUE) ||
	       (auth->ruid == auth->uid)))
	    {
	      status = sbuild_auth_change_auth(status, SBUILD_AUTH_STATUS_NONE);
	    }
	  else if (in_groups == TRUE) // Auth required if not in root group
	    {
	      status = sbuild_auth_change_auth(status, SBUILD_AUTH_STATUS_USER);
	    }
	  else // Not in any groups
	    {
	      status = sbuild_auth_change_auth(status, SBUILD_AUTH_STATUS_FAIL);
	    }
	}
      else // No available groups entries means no access to anyone
	{
	  status = sbuild_auth_change_auth(status, SBUILD_AUTH_STATUS_FAIL);
	}
    }

  return status;
}

/**
 * sbuild_session_setup_chroot_child_setup:
 * @session: an #SbuildSession
 *
 * A helper to make sure the child process is real and effective root
 * before running run-parts.
 */
void
sbuild_session_setup_chroot_child_setup (SbuildSession *session)
{
  /* This is required to ensure the scripts run with uid=0 and gid=0,
     otherwise setuid programs such as mount(8) will fail.  This
     should always succeed, because our euid=0 and egid=0.*/
  setuid(0);
  setgid(0);
  initgroups("root", 0);
}

/**
 * sbuild_session_setup_chroot:
 * @session: an #SbuildSession
 * @session_chroot: an #SbuildChroot (which must be present in the @session configuration)
 * @setup_type: an #SbuildChrootSetupType
 * @error: a #GError
 *
 * Setup a chroot.  This runs all of the commands in setup.d.
 *
 * The environment variables CHROOT_NAME, CHROOT_DESCRIPTION,
 * CHROOT_LOCATION, AUTH_USER and AUTH_VERBOSITY are set for use in
 * setup scripts.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
static gboolean
sbuild_session_setup_chroot (SbuildSession          *session,
			     SbuildChroot           *session_chroot,
			     SbuildChrootSetupType   setup_type,
			     GError                **error)
{
  g_return_val_if_fail(SBUILD_IS_SESSION(session), FALSE);
  g_return_val_if_fail(SBUILD_IS_CHROOT(session_chroot), FALSE);
  g_return_val_if_fail(sbuild_chroot_get_name(session_chroot) != NULL, FALSE);
  g_return_val_if_fail(setup_type == SBUILD_CHROOT_SETUP_START ||
		       setup_type == SBUILD_CHROOT_SETUP_RECOVER ||
		       setup_type == SBUILD_CHROOT_SETUP_STOP ||
		       setup_type == SBUILD_CHROOT_RUN_START ||
		       setup_type == SBUILD_CHROOT_RUN_STOP, FALSE);

  GError *setup_lock_error = NULL;
  if (sbuild_chroot_setup_lock(session_chroot, setup_type,
			       TRUE, &setup_lock_error) == FALSE)
    {
      g_set_error(error,
		  SBUILD_SESSION_ERROR, SBUILD_SESSION_ERROR_CHROOT_SETUP,
		  _("Chroot setup failed to lock chroot: %s"),
		  setup_lock_error->message);
      g_error_free(setup_lock_error);
      return FALSE;
    }

  gchar *setup_type_string = NULL;
  if (setup_type == SBUILD_CHROOT_SETUP_START)
    setup_type_string = "setup-start";
  else if (setup_type == SBUILD_CHROOT_SETUP_RECOVER)
    setup_type_string = "setup-recover";
  else if (setup_type == SBUILD_CHROOT_SETUP_STOP)
    setup_type_string = "setup-stop";
  else if (setup_type == SBUILD_CHROOT_RUN_START)
    setup_type_string = "run-start";
  else if (setup_type == SBUILD_CHROOT_RUN_STOP)
    setup_type_string = "run-stop";

  gchar **argv = g_new(gchar *, 8);
  {
    guint i = 0;
    argv[i++] = g_strdup(RUN_PARTS); // Run run-parts(8)
    if (sbuild_auth_get_verbosity(SBUILD_AUTH(session)) ==
	SBUILD_AUTH_VERBOSITY_VERBOSE)
      argv[i++] = g_strdup("--verbose");
    argv[i++] = g_strdup("--lsbsysinit");
    argv[i++] = g_strdup("--exit-on-error");
    if (setup_type == SBUILD_CHROOT_SETUP_STOP ||
	setup_type == SBUILD_CHROOT_RUN_STOP)
      argv[i++] = g_strdup("--reverse");
    argv[i++] = g_strdup_printf("--arg=%s", setup_type_string);
    if (setup_type == SBUILD_CHROOT_SETUP_START ||
	setup_type == SBUILD_CHROOT_SETUP_RECOVER ||
	setup_type == SBUILD_CHROOT_SETUP_STOP)
      argv[i++] = g_strdup(SCHROOT_CONF_SETUP_D); // Setup directory
    else
      argv[i++] = g_strdup(SCHROOT_CONF_RUN_D); // Run directory
    argv[i++] = NULL;
  }

  /* Get a complete list of environment variables to set.  We need to
     query the chroot here, since this can vary depending upon the
     chroot type. */
  GList *env = NULL;
  sbuild_chroot_setup_env(session_chroot, &env);
  {
    env = g_list_append(env,
			g_strdup_printf("AUTH_USER=%s",
					sbuild_auth_get_user(SBUILD_AUTH(session))));

    const char *verbosity = NULL;
    SbuildAuthVerbosity v = sbuild_auth_get_verbosity(SBUILD_AUTH(session));
    switch (v)
      {
      case SBUILD_AUTH_VERBOSITY_QUIET:
	verbosity = "quiet";
	break;
      case SBUILD_AUTH_VERBOSITY_NORMAL:
	verbosity = "normal";
	break;
      case SBUILD_AUTH_VERBOSITY_VERBOSE:
	verbosity = "verbose";
	break;
      default:
	g_warning(_("Invalid verbosity level: %d, falling back to \"normal\""), v);
	verbosity = "normal";
	break;
      }

    env = g_list_append(env,
			g_strdup_printf("AUTH_VERBOSITY=%s", verbosity));
  }

  env = g_list_append(env,
		      g_strdup_printf("MOUNT_DIR=%s", SCHROOT_MOUNT_DIR));
  env = g_list_append(env,
		      g_strdup_printf("LIBEXEC_DIR=%s", SCHROOT_LIBEXEC_DIR));
  env = g_list_append(env,
		      g_strdup_printf("PID=%d", getpid()));
  env = g_list_append(env,
		      g_strdup_printf("SESSION_ID=%s", session->session_id));

  /* Move the strings into the envp vector. */
  gchar **envp = NULL;
  if (env)
    {
      guint num_vars = g_list_length(env);
      envp = g_new(gchar *, num_vars + 1);

      for (guint i = 0; i < num_vars; ++i)
	{
	  GList *node = g_list_nth(env, i);
	  g_assert(node != NULL);
	  envp[i] = (gchar *) node->data;
	}
      envp[num_vars] = NULL;
    }
  g_list_free(env);

  gint exit_status = 0;
  GError *tmp_error = NULL;

  g_spawn_sync("/",          // working directory
	       argv,         // arguments
	       envp,         // environment
	       0,            // spawn flags
	       (GSpawnChildSetupFunc)
	         sbuild_session_setup_chroot_child_setup, // child_setup
	       (gpointer) session, // child_setup user_data
	       NULL,         // standard_output
	       NULL,         // standard_error
	       &exit_status, // child exit status
	       &tmp_error);  // error

  g_strfreev(argv);
  argv = NULL;
  g_strfreev(envp);
  envp = NULL;

  GError *setup_unlock_error = NULL;
  if (sbuild_chroot_setup_lock(session_chroot, setup_type,
			       FALSE, &setup_unlock_error) == FALSE)
    {
      if (tmp_error == NULL)
	g_set_error(&tmp_error,
		    SBUILD_SESSION_ERROR, SBUILD_SESSION_ERROR_CHROOT_SETUP,
		    _("Chroot setup failed to unlock chroot: %s"),
		    setup_unlock_error->message);
      g_error_free(setup_unlock_error);
    }

  if (tmp_error != NULL)
    {
      g_propagate_error(error, tmp_error);
      return FALSE;
    }

  if (exit_status != 0)
    {
      g_set_error(error,
		  SBUILD_SESSION_ERROR, SBUILD_SESSION_ERROR_CHROOT_SETUP,
		  _("Chroot setup failed during chroot %s"),
		  setup_type_string);
      return FALSE;
    }

  return TRUE;
}

/**
 * sbuild_session_run_child:
 * @session: an #SbuildSession
 * @session_chroot: an #SbuildChroot (which must be present in the @session configuration)
 *
 * Run a command or login shell as a child process in the specified chroot.
 *
 */
static void
sbuild_session_run_child (SbuildSession  *session,
			  SbuildChroot   *session_chroot)
{
  g_assert(SBUILD_IS_SESSION(session));
  g_assert(SBUILD_IS_CHROOT(session_chroot));
  g_assert(sbuild_chroot_get_name(session_chroot) != NULL);
  g_assert(sbuild_chroot_get_mount_location(session_chroot) != NULL);

  SbuildAuth *auth = SBUILD_AUTH(session);

  g_assert(sbuild_auth_get_user(auth) != NULL);
  g_assert(sbuild_auth_get_shell(auth) != NULL);
  g_assert(auth->pam != NULL); // PAM must be initialised

  uid_t uid = sbuild_auth_get_uid(auth);
  gid_t gid = sbuild_auth_get_gid(auth);
  const char *user = sbuild_auth_get_user(auth);
  uid_t ruid = sbuild_auth_get_ruid(auth);
  const char *ruser = sbuild_auth_get_ruser(auth);
  const char *shell = sbuild_auth_get_shell(auth);
  char **command = g_strdupv(sbuild_auth_get_command(auth));
  char **environment = sbuild_auth_get_environment(auth);

  const char *location = sbuild_chroot_get_mount_location(session_chroot);
  char *cwd = g_get_current_dir();
  /* Child errors result in immediate exit().  Errors are not
     propagated back via a GError, because there is no longer any
     higher-level handler to catch them. */
  GError *pam_error = NULL;
  sbuild_auth_open_session(SBUILD_AUTH(session), &pam_error);
  if (pam_error != NULL)
    {
      g_printerr(_("PAM error: %s\n"), pam_error->message);
      exit (EXIT_FAILURE);
    }

  /* Set group ID and supplementary groups */
  if (setgid (gid))
    {
      fprintf (stderr, _("Could not set gid to '%lu'\n"), (unsigned long) gid);
      exit (EXIT_FAILURE);
    }
  if (initgroups (user, gid))
    {
      fprintf (stderr, _("Could not set supplementary group IDs\n"));
      exit (EXIT_FAILURE);
    }

  /* Enter the chroot */
  if (chdir (location))
    {
      fprintf (stderr, _("Could not chdir to '%s': %s\n"), location,
	       g_strerror (errno));
      exit (EXIT_FAILURE);
    }
  if (chroot (location))
    {
      fprintf (stderr, _("Could not chroot to '%s': %s\n"), location,
	       g_strerror (errno));
      exit (EXIT_FAILURE);
    }

  /* Set uid and check we are not still root */
  if (setuid (uid))
    {
      fprintf (stderr, _("Could not set uid to '%lu'\n"), (unsigned long) uid);
      exit (EXIT_FAILURE);
    }
  if (!setuid (0) && uid)
    {
      fprintf (stderr, _("Failed to drop root permissions.\n"));
      exit (EXIT_FAILURE);
    }

  /* chdir to current directory */
  if (chdir (cwd))
    {
      fprintf (stderr, _("warning: Could not chdir to '%s': %s\n"), cwd,
	       g_strerror (errno));
    }
  g_free(cwd);

  /* Set up environment */
  char **env = sbuild_auth_get_pam_environment(auth);
  g_assert (env != NULL); // Can this fail?  Make sure we don't find out the hard way.
  for (guint i=0; env[i] != NULL; ++i)
    g_debug("Set environment: %s", env[i]);

  /* Run login shell */
  char *file = NULL;

  if ((command == NULL ||
       command[0] == NULL)) // No command
    {
      g_assert (shell != NULL);

      g_strfreev(command);
      command = g_new(char *, 2);
      file = g_strdup(shell);
      if (environment == NULL) // Not keeping environment; login shell
	{
	  char *shellbase = g_path_get_basename(shell);
	  char *loginshell = g_strconcat("-", shellbase, NULL);
	  g_free(shellbase);
	  command[0] = loginshell;
	  g_debug("Login shell: %s", command[1]);
	}
      else
	{
	  command[0] = g_strdup(shell);
	}
      command[1] = NULL;

      if (environment == NULL)
	{
	  g_debug("Running login shell: %s", shell);
	  syslog(LOG_USER|LOG_NOTICE, "[%s chroot] (%s->%s) Running login shell: \"%s\"",
		 sbuild_chroot_get_name(session_chroot), ruser, user, shell);
	}
      else
	{
	  g_debug("Running shell: %s", shell);
	  syslog(LOG_USER|LOG_NOTICE, "[%s chroot] (%s->%s) Running shell: \"%s\"",
		 sbuild_chroot_get_name(session_chroot), ruser, user, shell);
	}

      if (sbuild_auth_get_verbosity(auth) != SBUILD_AUTH_VERBOSITY_QUIET)
	{
	  if (ruid == uid)
	    {
	      if (environment == NULL)
		g_printerr(_("[%s chroot] Running login shell: \"%s\"\n"),
			   sbuild_chroot_get_name(session_chroot), shell);
	      else
		g_printerr(_("[%s chroot] Running shell: \"%s\"\n"),
			   sbuild_chroot_get_name(session_chroot), shell);
	    }
	  else
	    {
	      if (environment == NULL)
		g_printerr(_("[%s chroot] (%s->%s) Running login shell: \"%s\"\n"),
			   sbuild_chroot_get_name(session_chroot), ruser, user, shell);
	      else
		g_printerr(_("[%s chroot] (%s->%s) Running shell: \"%s\"\n"),
			   sbuild_chroot_get_name(session_chroot), ruser, user, shell);
	    }
	}
    }
  else
    {
      /* Search for program in path. */
      file = g_find_program_in_path(command[0]);
      if (file == NULL)
	file = g_strdup(command[0]);
      char *commandstring = g_strjoinv(" ", command);
      g_debug("Running command: %s", commandstring);
      syslog(LOG_USER|LOG_NOTICE, "[%s chroot] (%s->%s) Running command: \"%s\"",
	     sbuild_chroot_get_name(session_chroot), ruser, user, commandstring);
      if (sbuild_auth_get_verbosity(auth) != SBUILD_AUTH_VERBOSITY_QUIET)
	{
	  if (ruid == uid)
	    g_printerr(_("[%s chroot] Running command: \"%s\"\n"),
		       sbuild_chroot_get_name(session_chroot), commandstring);
	  else
	    g_printerr(_("[%s chroot] (%s->%s) Running command: \"%s\"\n"),
		       sbuild_chroot_get_name(session_chroot),
		       ruser, user, commandstring);
	}
      g_free(commandstring);
    }

  /* Execute */
  if (execve (file, command, env))
    {
      fprintf (stderr, _("Could not exec \"%s\": %s\n"), command[0],
	       g_strerror (errno));
      exit (EXIT_FAILURE);
    }
  /* This should never be reached */
  exit(EXIT_FAILURE);
}

/**
 * sbuild_session_wait_for_child:
 * @session: an #SbuildSession
 * @error: a #GError
 *
 * Wait for a child process to complete, and check its exit status.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
static gboolean
sbuild_session_wait_for_child (SbuildSession  *session,
			       int             pid,
			       GError        **error)
{
  g_return_val_if_fail(SBUILD_IS_SESSION(session), FALSE);

  session->child_status = EXIT_FAILURE; // Default exit status

  int status;
  if (wait(&status) != pid)
    {
      g_set_error(error,
		  SBUILD_SESSION_ERROR, SBUILD_SESSION_ERROR_CHILD,
		  _("wait for child failed: %s\n"), g_strerror (errno));
      return FALSE;
    }

  GError *pam_error = NULL;
  sbuild_auth_close_session(SBUILD_AUTH(session), &pam_error);
  if (pam_error != NULL)
    {
      g_propagate_error(error, pam_error);
      return FALSE;
    }

  if (!WIFEXITED(status))
    {
      if (WIFSIGNALED(status))
	g_set_error(error,
		    SBUILD_SESSION_ERROR, SBUILD_SESSION_ERROR_CHILD,
		    _("Child terminated by signal '%s'"),
		    strsignal(WTERMSIG(status)));
      else if (WCOREDUMP(status))
	g_set_error(error,
		    SBUILD_SESSION_ERROR, SBUILD_SESSION_ERROR_CHILD,
		    _("Child dumped core"));
      else
	g_set_error(error,
		    SBUILD_SESSION_ERROR, SBUILD_SESSION_ERROR_CHILD,
		    _("Child exited abnormally (reason unknown; not a signal or core dump)"));
      return FALSE;
    }

  session->child_status = WEXITSTATUS(status);
  g_object_notify(G_OBJECT(session), "child-status");

  if (session->child_status)
    {
      g_set_error(error,
		  SBUILD_SESSION_ERROR, SBUILD_SESSION_ERROR_CHILD,
		  _("Child exited abnormally with status '%d'"),
		  session->child_status);
      return FALSE;
    }

  return TRUE;

  /* Should never be reached. */
  return TRUE;
}

/**
 * sbuild_session_run_command:
 * @session: an #SbuildSession
 * @session_chroot: an #SbuildChroot (which must be present in the @session configuration)
 * @error: a #GError
 *
 * Run the session command or login shell in the specified chroot.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
static gboolean
sbuild_session_run_chroot (SbuildSession  *session,
			   SbuildChroot   *session_chroot,
			   GError        **error)
{
  g_return_val_if_fail(SBUILD_IS_SESSION(session), FALSE);
  g_return_val_if_fail(SBUILD_IS_CHROOT(session_chroot), FALSE);
  g_return_val_if_fail(sbuild_chroot_get_name(session_chroot) != NULL, FALSE);

  pid_t pid;
  if ((pid = fork()) == -1)
    {
      g_set_error(error,
		  SBUILD_SESSION_ERROR, SBUILD_SESSION_ERROR_FORK,
		  _("Failed to fork child: %s"), g_strerror(errno));
      return FALSE;
    }
  else if (pid == 0)
    {
      sbuild_session_run_child(session, session_chroot);
      exit (EXIT_FAILURE); /* Should never be reached. */
    }
  else
    {
      GError *tmp_error = NULL;
      sbuild_session_wait_for_child(session, pid, &tmp_error);

      if (tmp_error != NULL)
	{
	  g_propagate_error(error, tmp_error);
	  return FALSE;
	}
      return TRUE;
    }
}

/**
 * sbuild_session_run:
 * @session: an #SbuildSession
 * @error: a #GError, or NULL to ignore errors
 *
 * Run a session.  If a command has been specified, this will be run
 * in each of the specified chroots.  If no command has been
 * specified, a login shell will run in the specified chroot.
 *
 * If required, the user may be required to authenticate themselves.
 * This usually occurs when running as a different user.  The user
 * must be a member of the appropriate groups in order to satisfy the
 * groups and root-groups requirements in the chroot configuration.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
static gboolean
sbuild_session_run (SbuildSession  *session,
		    GError        **error)
{
  g_return_val_if_fail(SBUILD_IS_SESSION(session), FALSE);
  g_return_val_if_fail(session->config != NULL, FALSE);
  g_return_val_if_fail(session->chroots != NULL, FALSE);

  GError *tmp_error = NULL;

  for (guint x=0; session->chroots[x] != 0; ++x)
    {
      g_debug("Running session in %s chroot:\n", session->chroots[x]);
      SbuildChroot *chroot =
	sbuild_config_find_alias(session->config,
				 session->chroots[x]);

      if (chroot == NULL) // Should never happen, but cater for it anyway.
	{
	  g_set_error(&tmp_error,
		      SBUILD_SESSION_ERROR, SBUILD_SESSION_ERROR_CHROOT,
		      _("%s: Failed to find chroot"), session->chroots[x]);
	}
      else
	{
	  /* If restoring a session, set the session ID from the
	     chroot name, or else generate it.  Only chroots which
	     support session creation append a UUID to the session
	     ID. */
	  if (sbuild_chroot_get_active(chroot) ||
	      !(sbuild_chroot_get_session_flags(chroot) & SBUILD_CHROOT_SESSION_CREATE))
	    {
	      sbuild_session_set_session_id(session,
					    sbuild_chroot_get_name(chroot));
	    }
	  else
	    {
	      uuid_t uuid;
	      gchar uuid_str[37];
	      uuid_generate(uuid);
	      uuid_unparse(uuid, uuid_str);
	      uuid_clear(uuid);
	      gchar *session_id = g_strconcat(sbuild_chroot_get_name(chroot), "-", uuid_str, NULL);
	      sbuild_session_set_session_id(session, session_id);
	      g_free(session_id);
	    }

	  /* Activate chroot. */
	  sbuild_chroot_set_active(chroot, TRUE);

	  /* If a chroot mount location has not yet been set, set one
	     with the session id. */
	  if (sbuild_chroot_get_mount_location(chroot) == NULL)
	    {
	      gchar *location = g_strconcat(SCHROOT_MOUNT_DIR, "/",
					    session->session_id, NULL);
	      sbuild_chroot_set_mount_location(chroot, location);
	      g_free(location);
	    }

	  /* Chroot types which create a session (e.g. LVM devices)
	     need the chroot name respecifying. */
	  if (sbuild_chroot_get_session_flags(chroot) & SBUILD_CHROOT_SESSION_CREATE)
	      sbuild_chroot_set_name(chroot, session->session_id);

	  /* LVM devices need the snapshot device name specifying. */
	  if (SBUILD_IS_CHROOT_LVM_SNAPSHOT(chroot))
	  {
	      gchar *dir =
		g_path_get_dirname(sbuild_chroot_block_device_get_device
				   (SBUILD_CHROOT_BLOCK_DEVICE(chroot)));
	      gchar *device = g_strconcat(dir, "/",
					  session->session_id, NULL);
	      sbuild_chroot_lvm_snapshot_set_snapshot_device
		(SBUILD_CHROOT_LVM_SNAPSHOT(chroot), device);
	      g_free(device);
	      g_free(dir);
	    }

	  /* Run setup-start chroot setup scripts. */
	  if (session->operation == SBUILD_SESSION_OPERATION_AUTOMATIC ||
	      session->operation == SBUILD_SESSION_OPERATION_BEGIN)
	    {
	      if (sbuild_chroot_get_run_setup_scripts(chroot) == TRUE)
		sbuild_session_setup_chroot(session, chroot,
					    SBUILD_CHROOT_SETUP_START,
					    &tmp_error);
	      if (session->operation == SBUILD_SESSION_OPERATION_BEGIN)
		g_fprintf(stdout, "%s\n", session->session_id);
	    }

	  /* Run recover scripts. */
	  if (session->operation == SBUILD_SESSION_OPERATION_RECOVER)
	    {
	      if (sbuild_chroot_get_run_setup_scripts(chroot) == TRUE)
		sbuild_session_setup_chroot(session, chroot,
					    SBUILD_CHROOT_SETUP_RECOVER,
					    &tmp_error);
	    }

	  if (tmp_error == NULL &&
	      (session->operation == SBUILD_SESSION_OPERATION_AUTOMATIC ||
	       session->operation == SBUILD_SESSION_OPERATION_RUN))
	    {
	      /* Run run-start scripts. */
	      if (sbuild_chroot_get_run_session_scripts(chroot) == TRUE)
		sbuild_session_setup_chroot(session, chroot,
					    SBUILD_CHROOT_RUN_START,
					    &tmp_error);

	      /* Run session if setup succeeded. */
	      if (tmp_error == NULL)
		sbuild_session_run_chroot(session, chroot, &tmp_error);

	      /* Run run-stop scripts whether or not there was an
		 error. */
	      if (sbuild_chroot_get_run_session_scripts(chroot) == TRUE)
		sbuild_session_setup_chroot(session, chroot,
					    SBUILD_CHROOT_RUN_STOP,
					    (tmp_error != NULL) ? NULL : &tmp_error);
	    }

	  /* Run setup-stop chroot setup scripts whether or not there
	     was an error. */
	  if (session->operation == SBUILD_SESSION_OPERATION_AUTOMATIC ||
	      session->operation == SBUILD_SESSION_OPERATION_END)
	    {
	      if (sbuild_chroot_get_run_setup_scripts(chroot) == TRUE)
		sbuild_session_setup_chroot(session, chroot,
					    SBUILD_CHROOT_SETUP_STOP,
					    (tmp_error != NULL) ? NULL : &tmp_error);
	    }

	  /* Deactivate chroot. */
	  sbuild_chroot_set_active(chroot, FALSE);
	}

      if (tmp_error != NULL)
	break;
    }

  if (tmp_error != NULL)
    {
      g_propagate_error(error, tmp_error);
      return FALSE;
    }
  else
    return TRUE;
}

static void
sbuild_session_init (SbuildSession *session)
{
  g_return_if_fail(SBUILD_IS_SESSION(session));

  session->config = NULL;
  session->chroots = NULL;
  session->session_id = NULL;
  session->operation = SBUILD_SESSION_OPERATION_AUTOMATIC;
  session->force = FALSE;
  session->child_status = EXIT_FAILURE;
}

static void
sbuild_session_finalize (SbuildSession *session)
{
  g_return_if_fail(SBUILD_IS_SESSION(session));

  if (session->config)
    {
      g_object_unref (session->config);
      session->config = NULL;
    }
  if (session->chroots)
    {
      g_strfreev(session->chroots);
      session->chroots = NULL;
    }
  if (session->session_id)
    {
      g_free(session->session_id);
      session->session_id = NULL;
    }

  if (parent_class->finalize)
    parent_class->finalize(G_OBJECT(session));
}

static void
sbuild_session_set_property (GObject      *object,
			     guint         param_id,
			     const GValue *value,
			     GParamSpec   *pspec)
{
  SbuildSession *session;

  g_return_if_fail (object != NULL);
  g_return_if_fail (SBUILD_IS_SESSION (object));

  session = SBUILD_SESSION(object);

  switch (param_id)
    {
    case PROP_CONFIG:
      sbuild_session_set_config(session, g_value_get_object(value));
      break;
    case PROP_CHROOTS:
      sbuild_session_set_chroots(session, g_value_get_boxed(value));
      break;
    case PROP_OPERATION:
      sbuild_session_set_operation(session, g_value_get_enum(value));
      break;
    case PROP_SESSION_ID:
      sbuild_session_set_session_id(session, g_value_get_string(value));
      break;
    case PROP_FORCE:
      sbuild_session_set_force(session, g_value_get_boolean(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
sbuild_session_get_property (GObject    *object,
			     guint       param_id,
			     GValue     *value,
			     GParamSpec *pspec)
{
  SbuildSession *session;

  g_return_if_fail (object != NULL);
  g_return_if_fail (SBUILD_IS_SESSION (object));

  session = SBUILD_SESSION(object);

  switch (param_id)
    {
    case PROP_CONFIG:
      g_value_set_object(value, session->config);
      break;
    case PROP_CHROOTS:
      g_value_set_boxed(value, session->chroots);
      break;
    case PROP_OPERATION:
      g_value_set_enum(value, session->operation);
      break;
    case PROP_SESSION_ID:
      g_value_set_string(value, session->session_id);
      break;
    case PROP_FORCE:
      g_value_set_boolean(value, session->force);
      break;
    case PROP_CHILD_STATUS:
      g_value_set_int(value, session->child_status);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
sbuild_session_class_init (SbuildSessionClass *klass)
{
  SbuildAuthClass *auth_class = SBUILD_AUTH_CLASS(klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  parent_class = g_type_class_peek_parent (klass);

  auth_class->require_auth = (SbuildAuthRequireAuthFunc) sbuild_session_require_auth;
  auth_class->session_run = (SbuildAuthSessionRunFunc) sbuild_session_run;

  gobject_class->finalize = (GObjectFinalizeFunc) sbuild_session_finalize;
  gobject_class->set_property = (GObjectSetPropertyFunc) sbuild_session_set_property;
  gobject_class->get_property = (GObjectGetPropertyFunc) sbuild_session_get_property;

  g_object_class_install_property
    (gobject_class,
     PROP_CONFIG,
     g_param_spec_object ("config", "Configuration",
			  "The chroot configuration data",
			  SBUILD_TYPE_CONFIG,
			  (G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT)));

  g_object_class_install_property
    (gobject_class,
     PROP_CHROOTS,
     g_param_spec_boxed ("chroots", "Chroots",
                         "The chroots to use",
                         G_TYPE_STRV,
                         (G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT)));

  g_object_class_install_property
    (gobject_class,
     PROP_OPERATION,
     g_param_spec_enum ("operation", "Operation",
			"The control operation for this session",
			SBUILD_TYPE_SESSION_OPERATION,
			SBUILD_SESSION_OPERATION_AUTOMATIC,
			(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT)));

  g_object_class_install_property
    (gobject_class,
     PROP_SESSION_ID,
     g_param_spec_string ("session-id", "Session ID",
			  "The unique session identifier for this session",
			  "",
			  (G_PARAM_READABLE | G_PARAM_WRITABLE)));

  g_object_class_install_property
    (gobject_class,
     PROP_FORCE,
     g_param_spec_boolean ("force", "Force",
			   "Force session operation?",
			   FALSE,
			   (G_PARAM_READABLE | G_PARAM_WRITABLE)));

  g_object_class_install_property
    (gobject_class,
     PROP_CHILD_STATUS,
     g_param_spec_int ("child-status", "Child Status",
		       "The exit (wait) status of the child process",
		       0, G_MAXINT, 0,
		       (G_PARAM_READABLE)));
}

/*
 * Local Variables:
 * mode:C
 * End:
 */
