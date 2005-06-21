/* sbuild-session - sbuild session object
 *
 * Copyright (C) 2005  Roger Leigh <rleigh@debian.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *********************************************************************/

/**
 * SECTION:sbuild-session
 * @short_description: session object
 * @title: SbuildSession
 *
 */

#define _GNU_SOURCE 1
#include <errno.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#include "sbuild-session.h"

enum
{
  PROP_0,
  PROP_USER,
  PROP_COMMAND,
  PROP_CONFIG,
  PROP_CHROOTS
};

static GObjectClass *parent_class;

G_DEFINE_TYPE(SbuildSession, sbuild_session, G_TYPE_OBJECT)

static const struct pam_conv sbuild_session_pam_conv = {
        misc_conv,
        NULL
};

/**
 * sbuild_session_new:
 *
 * Creates a new #SbuildSession.
 *
 * Returns the newly created #SbuildSession.
 */
SbuildSession *
sbuild_session_new(SbuildConfig  *config,
		   char         **chroots)
{
  return (SbuildSession *) g_object_new(SBUILD_TYPE_SESSION,
					"config", config,
					"chroots", chroots,
					NULL);
}

const char *
sbuild_session_get_user (const SbuildSession *restrict session)
{
  g_return_val_if_fail(SBUILD_IS_SESSION(session), NULL);

  return session->user;
}

void
sbuild_session_set_user (SbuildSession *session,
			 const char   *user)
{
  g_return_if_fail(SBUILD_IS_SESSION(session));

  if (session->user)
    {
      g_free(session->user);
    }
  session->user = g_strdup(user);
  if (session->shell)
    {
      g_free(session->shell);
      session->shell = NULL;
    }

  if (user != NULL)
    {
      struct passwd *pwent = getpwnam(session->user);
      if (pwent == NULL)
	{
	  g_printerr("%s: user not found: %s\n", session->user, g_strerror(errno));
	  exit (EXIT_FAILURE);
	}
      session->uid = pwent->pw_uid;
      session->gid = pwent->pw_gid;
      session->shell = g_strdup(pwent->pw_shell);
      g_debug("session uid = %lu, gid = %lu\n", (unsigned long) session->uid,
	      (unsigned long) session->gid);
    }
  else
    {
      session->uid = 0;
      session->gid = 0;
      session->shell = g_strdup("/bin/false");
    }

  g_object_notify(G_OBJECT(session), "user");
}

char **
sbuild_session_get_command (const SbuildSession *restrict session)
{
  g_return_val_if_fail(SBUILD_IS_SESSION(session), NULL);

  return session->command;
}

void
sbuild_session_set_command (SbuildSession  *session,
			    char          **command)
{
  g_return_if_fail(SBUILD_IS_SESSION(session));

  if (session->command)
    {
      g_strfreev(session->command);
    }
  session->command = g_strdupv(command);
  g_object_notify(G_OBJECT(session), "command");
}

SbuildConfig *
sbuild_session_get_config (const SbuildSession *restrict session)
{
  g_return_val_if_fail(SBUILD_IS_SESSION(session), NULL);

  return session->config;
}

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

char **
sbuild_session_get_chroots (const SbuildSession *restrict session)
{
  g_return_val_if_fail(SBUILD_IS_SESSION(session), NULL);

  return session->chroots;
}

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

/* Run command in chroot */
static int
sbuild_session_run_chroot (SbuildSession *session,
			   SbuildChroot  *session_chroot)
{
  g_return_val_if_fail(SBUILD_IS_SESSION(session), -1);
  g_return_val_if_fail(SBUILD_IS_CHROOT(session_chroot), -1);

  g_assert(session->user != NULL);
  g_assert(session->shell != NULL);

  pid_t pid;
  if ((pid = fork()) == -1)
    {
      fprintf (stderr, "Could not fork child: %s\n", g_strerror(errno));
      exit (EXIT_FAILURE);
    }
  else if (pid == 0)
    {
      const char *location = sbuild_chroot_get_location(session_chroot);
      char *cwd = g_get_current_dir();

      /* Set group ID and supplementary groups */
      if (setgid (session->gid))
	{
	  fprintf (stderr, "Could not set gid to %lu\n", (unsigned long) session->gid);
	  exit (EXIT_FAILURE);
	}
      if (initgroups (session->user, session->gid))
	{
	  fprintf (stderr, "Could not set supplementary group IDs\n");
	  exit (EXIT_FAILURE);
	}

      /* Enter the chroot */
      if (chdir (location))
	{
	  fprintf (stderr, "Could not chdir to %s: %s\n", location,
		   g_strerror (errno));
	  exit (EXIT_FAILURE);
	}
      if (chroot (location))
	{
	  fprintf (stderr, "Could not chroot to %s: %s\n", location,
		   g_strerror (errno));
	  exit (EXIT_FAILURE);
	}
      /* printf ("Entered chroot: %s\n", location); */

      /* Set uid and check we are not still root */
      if (setuid (session->uid))
	{
	  fprintf (stderr, "Could not set uid to %lu\n", (unsigned long) session->uid);
	  exit (EXIT_FAILURE);
	}
      if (!setuid (0) && session->uid)
	{
	  fprintf (stderr, "Failed to drop root permissions.\n");
	  exit (EXIT_FAILURE);
	}

      /* Set up environment */
/*       if (pass->pw_dir) */
/* 	setenv("HOME", pass->pw_dir, 1); */
/*       else */
/* 	setenv("HOME", "/", 1); */

      /* chdir to current directory */
      if (chdir (cwd))
	{
	  fprintf (stderr, "warning: Could not chdir to %s: %s\n", cwd,
		   g_strerror (errno));
	}
      g_free(cwd);

      char **env = pam_getenvlist(session->pam);
      // Can this fail?
      g_assert (env != NULL);

      /* Run login shell */
      if ((session->command == NULL ||
	   session->command[0] == NULL)) // No command
	{
	  g_assert (session->shell != NULL);

	  session->command = g_new(char *, 2);
	  session->command[0] = g_strdup(session->shell);
	  session->command[1] = NULL;

	  g_debug("Running login shell: %s", session->shell);
	}
      else
	g_debug("Running command: %s", session->command[0]);

      /* Execute */
      if (execve (session->command[0], session->command, env))
	{
	  fprintf (stderr, "Could not exec %s: %s\n", session->command[0],
		   g_strerror (errno));
	  exit (EXIT_FAILURE);
	}
      /* This should never be reached */
      exit(EXIT_FAILURE);
    }
  else
    {
      int status;
      if (wait(&status) != pid)
	{
	  fprintf (stderr, "wait for child failed: %s\n", g_strerror (errno));
	  exit (EXIT_FAILURE);
	}

      if (!WIFEXITED(status))
	{
	  g_print("Child exited abnormally\n");
	  return -1;
	}

      if (WEXITSTATUS(status))
	{
	  g_print("Child exited abnormally\n");
	}

      return WEXITSTATUS(status);
    }
}

/* Check group membership */
static gboolean
is_group_member (const char *group)
{
  errno = 0;
  struct group *groupbuf = getgrnam(group);
  if (groupbuf == NULL)
    {
      if (errno == 0)
	g_printerr("%s: group not found\n", group);
      else
	g_printerr("%s: group not found: %s\n", group, g_strerror(errno));
      exit (EXIT_FAILURE);
    }

  int supp_group_count = getgroups(0, NULL);
  if (supp_group_count < 0)
    {
      g_printerr("can't get supplementary group count: %s\n", g_strerror(errno));
      exit (EXIT_FAILURE);
    }
  gid_t supp_groups[supp_group_count];
  if (getgroups(supp_group_count, supp_groups) < 1)
    {
      g_printerr("can't get supplementary groups: %s\n", g_strerror(errno));
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

typedef enum
{
  SBUILD_SESSION_AUTH_NONE,
  SBUILD_SESSION_AUTH_USER,
  SBUILD_SESSION_AUTH_FAIL
} SbuildSessionAuthType;

static inline SbuildSessionAuthType
set_auth (SbuildSessionAuthType oldauth,
	  SbuildSessionAuthType newauth)
{
  /* Ensure auth level always escalates. */
  if (newauth > oldauth)
    return newauth;
  else
    return oldauth;
}

static SbuildSessionAuthType
sbuild_session_require_auth (SbuildSession *session)
{
  g_return_val_if_fail(SBUILD_IS_SESSION(session), SBUILD_SESSION_AUTH_FAIL);
  g_return_val_if_fail(session->chroots != NULL, SBUILD_SESSION_AUTH_FAIL);
  g_return_val_if_fail(session->user != NULL, SBUILD_SESSION_AUTH_FAIL);

  SbuildSessionAuthType auth = SBUILD_SESSION_AUTH_NONE;

  for (guint i=0; session->chroots[i] != NULL; ++i)
    {
      SbuildChroot *chroot = sbuild_config_find_alias(session->config,
						      session->chroots[i]);
      if (chroot == NULL) // Should never happen, but cater for it anyway.
	{
	  g_warning("No chroot found matching alias %s", session->chroots[i]);
	  auth = set_auth(auth, SBUILD_SESSION_AUTH_FAIL);
	}

      char **groups = NULL;
      if (session->uid == 0)
	groups = sbuild_chroot_get_root_groups(chroot);
      else
	groups = sbuild_chroot_get_groups(chroot);

      if (groups)
	{
	  for (guint y=0; groups[y] != 0; ++y)
	    {
	      if (is_group_member(groups[y]) == TRUE) // No auth required
		auth = set_auth(auth, SBUILD_SESSION_AUTH_NONE);
	      else
		auth = set_auth(auth, SBUILD_SESSION_AUTH_USER);
	    }
	}
    }

  return auth;
}

void
sbuild_session_run (SbuildSession *session)
{
  /* PAM setup. */

  int pam_status;
  if ((pam_status =
       pam_start("schroot", session->user,
		 &sbuild_session_pam_conv, &session->pam)) != PAM_SUCCESS)
    {
      g_printerr("PAM initialisation error: %s\n", pam_strerror(session->pam, pam_status));
      exit (EXIT_FAILURE);
    }

  if ((pam_status =
       pam_set_item(session->pam, PAM_RUSER, session->ruser)) != PAM_SUCCESS)
    {
      g_printerr("PAM set RUSER failed: %s\n", pam_strerror(session->pam, pam_status));
      exit (EXIT_FAILURE);
    }


  long hl = 256; /* BROKEN with libc6 sysconf(_SC_HOST_NAME_MAX); */

  char *hostname = g_new(char, hl);
  if (gethostname(hostname, hl) != 0)
    {
      g_printerr("failed to get hostname: %s\n", g_strerror(errno));
      exit (EXIT_FAILURE);
    }

  if ((pam_status =
       pam_set_item(session->pam, PAM_RHOST, hostname)) != PAM_SUCCESS)
    {
      g_printerr("PAM set RHOST failed: %s\n", pam_strerror(session->pam, pam_status));
      exit (EXIT_FAILURE);
    }

  g_free(hostname);
  hostname = NULL;

  const char *tty = ttyname(STDIN_FILENO);
  if (tty)
    {
      if ((pam_status =
	   pam_set_item(session->pam, PAM_TTY, tty)) != PAM_SUCCESS)
	{
	  g_printerr("PAM set TTY failed: %s\n", pam_strerror(session->pam, pam_status));
	  exit (EXIT_FAILURE);
	}
    }

  /* Authenticate as required. */
  switch (sbuild_session_require_auth (session))
    {
    case SBUILD_SESSION_AUTH_NONE:
      if ((pam_status =
	   pam_set_item(session->pam, PAM_USER, session->user)) != PAM_SUCCESS)
	{
	  g_printerr("PAM set USER failed: %s\n", pam_strerror(session->pam, pam_status));
	  exit (EXIT_FAILURE);
	}
      break;

    case SBUILD_SESSION_AUTH_USER:
      if ((pam_status =
	   pam_authenticate(session->pam, 0)) != PAM_SUCCESS)
	{
	  g_printerr("PAM authentication failed: %s\n", pam_strerror(session->pam, pam_status));
	  exit (EXIT_FAILURE);
	}
      break;

    case SBUILD_SESSION_AUTH_FAIL:
      g_printerr("PAM authentication failed prematurely due to configuration error\n");
      exit (EXIT_FAILURE);
    default:
      break;
    }

  if ((pam_status =
       pam_acct_mgmt(session->pam, 0)) != PAM_SUCCESS)
    {
      /* We don't handle changing expired passwords here, since we are
	 not login or ssh. */
      g_printerr("PAM account management failed: %s\n",
		 pam_strerror(session->pam, pam_status));
      exit (EXIT_FAILURE);
    }

  if ((pam_status =
       pam_setcred(session->pam, PAM_ESTABLISH_CRED)) != PAM_SUCCESS)
    {
      /* We don't handle changing expired passwords here, since we are
	 not login or ssh. */
      g_printerr("PAM user credentials setup failed: %s\n",
		 pam_strerror(session->pam, pam_status));
      exit (EXIT_FAILURE);
    }

  if ((pam_status =
       pam_open_session(session->pam, 0)) != PAM_SUCCESS)
    {
      g_printerr("PAM open session failed: %s\n",
		 pam_strerror(session->pam, pam_status));
      exit (EXIT_FAILURE);
    }

  const char *authuser = NULL;
  pam_get_item(session->pam, PAM_USER, (const void **) &authuser);
  g_printerr("PAM authentication succeeded for user %s\n", authuser);

  for (guint x=0; session->chroots[x] != 0; ++x)
    {
      g_printerr("Running session in %s chroot:\n", session->chroots[x]);
      SbuildChroot *chroot = sbuild_config_find_alias(session->config,
						      session->chroots[x]);
      sbuild_session_run_chroot(session, chroot);
    }

  if ((pam_status =
       pam_close_session(session->pam, 0)) != PAM_SUCCESS)
    {
      g_printerr("PAM close session failed: %s\n",
		 pam_strerror(session->pam, pam_status));
      exit (EXIT_FAILURE);
    }

  if ((pam_status =
       pam_setcred(session->pam, PAM_DELETE_CRED)) != PAM_SUCCESS)
    {
      g_printerr("PAM user credentials deletion failed: %s\n",
		 pam_strerror(session->pam, pam_status));
      exit (EXIT_FAILURE);
    }

  if ((pam_status =
       pam_end(session->pam, PAM_SUCCESS)) != PAM_SUCCESS)
    {
      g_printerr("PAM finalisation failed: %s\n",
		 pam_strerror(session->pam, pam_status));
      exit (EXIT_FAILURE);
    }
}

static void
sbuild_session_init (SbuildSession *session)
{
  g_return_if_fail(SBUILD_IS_SESSION(session));

  session->user = NULL;
  session->uid = 0;
  session->gid = 0;
  session->command = NULL;
  session->shell = NULL;
  session->config = NULL;
  session->chroots = NULL;
  session->pam = NULL;

  /* Current user's details. */
  session->ruid = getuid();
  struct passwd *pwent = getpwuid(session->ruid);
  if (pwent == NULL)
    {
      g_printerr("%lu: user not found: %s\n", (unsigned long) session->ruid,
		 g_strerror(errno));
      exit (EXIT_FAILURE);
    }
  session->ruser = g_strdup(pwent->pw_name);

  /* By default, the session user is the same as the remote user. */
  sbuild_session_set_user(session, session->ruser);
}

static void
sbuild_session_finalize (SbuildSession *session)
{
  g_return_if_fail(SBUILD_IS_SESSION(session));

  if (session->user)
    {
      g_free (session->user);
      session->user = NULL;
    }
  if (session->command)
    {
      g_strfreev (session->command);
      session->command = NULL;
    }
  if (session->shell)
    {
      g_free (session->shell);
      session->shell = NULL;
    }
  if (session->ruser)
    {
      g_free (session->ruser);
      session->ruser = NULL;
    }
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
    case PROP_USER:
      sbuild_session_set_user(session, g_value_get_string(value));
      break;
    case PROP_COMMAND:
      sbuild_session_set_command(session, g_value_get_boxed(value));
      break;
    case PROP_CONFIG:
      sbuild_session_set_config(session, g_value_get_object(value));
      break;
    case PROP_CHROOTS:
      sbuild_session_set_chroots(session, g_value_get_boxed(value));
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
    case PROP_USER:
      g_value_set_string(value, session->user);
      break;
    case PROP_COMMAND:
      g_value_set_boxed(value, session->command);
      break;
    case PROP_CONFIG:
      g_value_set_object(value, session->config);
      break;
    case PROP_CHROOTS:
      g_value_set_boxed(value, session->chroots);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
sbuild_session_class_init (SbuildSessionClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = (GObjectFinalizeFunc) sbuild_session_finalize;
  gobject_class->set_property = (GObjectSetPropertyFunc) sbuild_session_set_property;
  gobject_class->get_property = (GObjectGetPropertyFunc) sbuild_session_get_property;

  g_object_class_install_property
    (gobject_class,
     PROP_USER,
     g_param_spec_string ("user", "User",
			  "The user to run as in the chroot",
			  "",
			  (G_PARAM_READABLE | G_PARAM_WRITABLE)));

  g_object_class_install_property
    (gobject_class,
     PROP_COMMAND,
     g_param_spec_boxed ("command", "Command",
			 "The command to run in the chroot, or NULL for a login shell",
			 G_TYPE_STRV,
			 (G_PARAM_READABLE | G_PARAM_WRITABLE)));

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
}

/*
 * Local Variables:
 * mode:C
 * End:
 */
