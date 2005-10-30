/* schroot - securely enter a chroot
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

#include <config.h>

#define _GNU_SOURCE
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>

#include <syslog.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "sbuild-chroot.h"
#include "sbuild-config.h"
#include "sbuild-session.h"

static gboolean
parse_session_options(const gchar  *option_name,
		      const gchar  *value,
		      gpointer      data,
		      GError      **error);

/* Stored command-line options. */
static struct {
  char     **chroots;
  char     **command;
  char      *user;
  gboolean   preserve;
  gboolean   quiet;
  gboolean   verbose;
  gboolean   list;
  gboolean   info;
  gboolean   all;
  gboolean   all_chroots;
  gboolean   all_sessions;
  gboolean   version;
} opt =
  {
    .chroots = NULL,
    .command = NULL,
    .user = NULL,
    .preserve = FALSE,
    .quiet = FALSE,
    .verbose = FALSE,
    .list = FALSE,
    .info = FALSE,
    .all = FALSE,
    .all_chroots = FALSE,
    .all_sessions = FALSE,
    .version = FALSE,
  };

static struct {
  SbuildSessionOperation  operation;
  gboolean                force;
} session_opt =
  {
    .operation = SBUILD_SESSION_OPERATION_AUTOMATIC,
    .force = FALSE
  };

/**
 * parse_options:
 * @argc: the number of arguments
 * @argv: argument vector
 *
 * Parse command-line options.  The options are placed in the opt
 * structure.
 */
static void
parse_options(int   argc,
	      char *argv[])
{
  /* Command-line options. */
  static const GOptionEntry entries[] =
    {
      { "all", 'a', 0, G_OPTION_ARG_NONE, &opt.all,
	N_("Select all chroots and active sessions"), NULL },
      { "all-chroots", 0, 0, G_OPTION_ARG_NONE, &opt.all_chroots,
	N_("Select all chroots"), NULL },
      { "all-sessions", 0, 0, G_OPTION_ARG_NONE, &opt.all_sessions,
	N_("Select all active sessions"), NULL },
      { "chroot", 'c', 0, G_OPTION_ARG_STRING_ARRAY, &opt.chroots,
	N_("Use specified chroot"), "chroot" },
      { "user", 'u', 0, G_OPTION_ARG_STRING, &opt.user,
	N_("Username (default current user)"), "user" },
      { "list", 'l', 0, G_OPTION_ARG_NONE, &opt.list,
	N_("List available chroots"), NULL },
      { "info", 'i', 0, G_OPTION_ARG_NONE, &opt.info,
	N_("Show information about chroot"), NULL },
      { "preserve-environment", 'p', 0, G_OPTION_ARG_NONE, &opt.preserve,
	N_("Preserve user environment"), NULL },
      { "quiet", 'q', 0, G_OPTION_ARG_NONE, &opt.quiet,
	N_("Show less output"), NULL },
      { "verbose", 'v', 0, G_OPTION_ARG_NONE, &opt.verbose,
	N_("Show more output"), NULL },
      { "version", 'V', 0, G_OPTION_ARG_NONE, &opt.version,
	N_("Print version information"), NULL },
      { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &opt.command,
	NULL, NULL },
      { NULL }
    };

  static const GOptionEntry session_entries[] =
    {
      { "begin-session", 'b', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	parse_session_options,
	N_("Begin a session; returns a session UUID"), NULL },
      { "run-session", 'r', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	parse_session_options,
	N_("Run an existing session"), NULL },
      { "end-session", 'e', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	parse_session_options,
	N_("End an existing session"), NULL },
      { "force", 'f', 0, G_OPTION_ARG_NONE, &session_opt.force,
	N_("Force operation, even if it fails"), NULL },
      { NULL }
    };

  GError *error = NULL;

  GOptionContext *context = g_option_context_new (_("- run command or shell in a chroot"));
  g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);

  GOptionGroup* session_group =
    g_option_group_new("session", N_("Session Options"),
		       N_("Session management options"), NULL, NULL);
  g_option_group_set_translation_domain(session_group, GETTEXT_PACKAGE);
  g_option_group_add_entries (session_group, session_entries);
  g_option_context_add_group(context, session_group);

  g_option_context_parse (context, &argc, &argv, &error);
  g_option_context_free (context);
  if (error != NULL)
    {
      g_printerr(_("Error parsing options: %s\n"), error->message);
      exit (EXIT_FAILURE);
    }
}

/**
 * parse_session_options:
 *
 * Parse command-line session options.  The options are placed in the
 * session_opt structure.  This is a #GOptionArgFunc.
 *
 * Returns TRUE on success, FALSE on failure (and error will also be
 * set).
 */
static gboolean
parse_session_options(const gchar  *option_name,
		      const gchar  *value,
		      gpointer      data,
		      GError      **error)
{
  if (strcmp(option_name, "-b") == 0 ||
      strcmp(option_name, "--begin-session") == 0)
    {
      session_opt.operation = SBUILD_SESSION_OPERATION_BEGIN;
    }
  else if (strcmp(option_name, "-r") == 0 ||
	   strcmp(option_name, "--run-session") == 0)
    {
      session_opt.operation = SBUILD_SESSION_OPERATION_RUN;
    }
  else if (strcmp(option_name, "-e") == 0 ||
	   strcmp(option_name, "--end-session") == 0)
    {
      session_opt.operation = SBUILD_SESSION_OPERATION_END;
    }
  else
    {
      g_set_error(error,
		  G_OPTION_ERROR, G_OPTION_ERROR_FAILED,
		  _("Invalid session option %s"), option_name);
      return FALSE;
    }
  return TRUE;
}

/**
 * print_version:
 * @file: the file to print to
 *
 * Print version information.
 */
void
print_version (FILE *file)
{
  g_fprintf(file, _("schroot (Debian sbuild) %s\n"), VERSION);
  g_fprintf(file, _("Written by Roger Leigh\n\n"));
  g_fprintf(file, _("Copyright (C) 2004-2005 Roger Leigh\n"));
  g_fprintf(file, _("This is free software; see the source for copying conditions.  There is NO\n"
		    "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"));
}

/**
 * get_chroot_options:
 * @config: an #SbuildConfig
 *
 * Get a list of chroots based on the specified options (--all, --chroot).
 *
 * Returns a NULL-terminated string vector (GStrv).
 */
char **
get_chroot_options(SbuildConfig *config)
{
  char **chroots = NULL;

  if (opt.all == TRUE || opt.all_chroots == TRUE || opt.all_sessions == TRUE)
    {
      const GList *list = NULL;
      guint list_len = 0;

      list = sbuild_config_get_chroots(config);
      list_len = g_list_length((GList *) list);
      chroots = g_new(char *, list_len + 1);

      if (list != NULL)
	{
	  guint pos=0;
	  for (guint i=0; i < list_len; ++i)
	    {
	      GList *node = g_list_nth((GList *) list, i);
	      g_assert(node != NULL);
	      SbuildChroot *chroot = node->data;
	      g_assert(sbuild_chroot_get_name(chroot) != NULL);
	      gboolean active = sbuild_chroot_get_active(chroot);
 	      if ((active == FALSE && opt.all == FALSE && opt.all_chroots == FALSE) ||
 		  (active == TRUE && opt.all == FALSE && opt.all_sessions == FALSE))
		continue;
	      chroots[pos++] = g_strdup(sbuild_chroot_get_name(chroot));
	    }
	  chroots[pos] = NULL;
	}
    }
  else
    {
      /* If no chroot was specified, fall back to the "default" chroot. */
      if (opt.chroots == NULL)
	{
	  opt.chroots = g_new(char *, 2);
	  opt.chroots[0] = g_strdup("default");
	  opt.chroots[1] = NULL;
	}

      char **invalid_chroots =
	sbuild_config_validate_chroots(config, opt.chroots);

      if (invalid_chroots)
	{
	  for (guint i = 0; invalid_chroots[i] != NULL; ++i)
	    g_printerr(_("%s: No such chroot\n"), invalid_chroots[i]);
	  g_strfreev(invalid_chroots);
	  exit(EXIT_FAILURE);
	}
      g_strfreev(invalid_chroots);
      chroots = g_strdupv(opt.chroots);
    }

  return chroots;
}

/**
 * debug_logfunc:
 * @log_domain: the log domain
 * @log_level: the logging level
 * @message: the message to log
 * @user_data: extra detail
 *
 * Log a debugging message.  This is a "NULL" message handler that
 * does nothing, discarding all messages.
 */
void debug_logfunc (const gchar *log_domain,
		    GLogLevelFlags log_level,
		    const gchar *message,
		    gpointer user_data)
{
  /* Discard all messages. */
}

/**
 * main:
 * @argc: the number of arguments
 * @argv: argument vector
 *
 * Main routine.
 *
 * Returns 0 on success, 1 on failure.
 */
int
main (int   argc,
      char *argv[])
{
  g_type_init();

  setlocale (LC_ALL, "");

  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  textdomain (GETTEXT_PACKAGE);

#ifndef SBUILD_DEBUG
  /* Discard g_debug output for this logging domain. */
  g_log_set_handler (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, debug_logfunc, NULL);
#endif

  openlog("schroot", LOG_PID|LOG_NDELAY, LOG_AUTHPRIV);

  /* Parse command-line options into opt structure. */
  parse_options(argc, argv);

  if (opt.version == TRUE)
    {
      print_version(stdout);
      exit(EXIT_SUCCESS);
    }

  /* Initialise chroot configuration. */
  SbuildConfig *config = sbuild_config_new();
  g_assert (config != NULL);
  /* The normal chroot list is used when starting a session or running
     any chroot type or session, or displaying chroot information. */
  sbuild_config_add_config_file(config, SCHROOT_CONF);
  /* The session chroot list is used when running or ending an
     existing session, or displaying chroot information. */
  if (opt.list == TRUE || opt.info == TRUE ||
      session_opt.operation == SBUILD_SESSION_OPERATION_RUN ||
      session_opt.operation == SBUILD_SESSION_OPERATION_END)
    sbuild_config_add_config_directory(config, SCHROOT_SESSION_DIR);

  if (sbuild_config_get_chroots(config) == NULL)
    {
      g_printerr(_("No chroots are defined in %s\n"), SCHROOT_CONF);
      exit (EXIT_FAILURE);
    }

  /* Print chroot list (including aliases). */
  if (opt.list == TRUE)
    {
      sbuild_config_print_chroot_list(config, stdout);
      exit(EXIT_SUCCESS);
    }

  /* Get list of chroots to use */
  char **chroots = get_chroot_options(config);
  if (chroots == NULL)
    {
      g_printerr(_("The specified chroots are not defined in %s\n"), SCHROOT_CONF);
      exit (EXIT_FAILURE);
    }

  /* Print chroot information for specified chroots. */
  if (opt.info == TRUE)
    {
      for (guint i=0; chroots[i] != NULL; ++i)
	{
	  SbuildChroot *chroot = sbuild_config_find_alias(config, chroots[i]);
	  if (chroot)
	    {
	      sbuild_chroot_print_details(chroot, stdout);
	      if (chroots[i+1] != NULL)
		g_fprintf(stdout, "\n");
	    }
	}
      exit (EXIT_SUCCESS);
    }

  if (session_opt.operation == SBUILD_SESSION_OPERATION_BEGIN &&
      chroots[0] != NULL && chroots[1] != NULL)
    {
      g_printerr(_("Only one chroot may be specified when beginning a session\n"));
      exit (EXIT_FAILURE);
    }

  /* Create a session. */
  SbuildSession *session =
    sbuild_session_new("schroot",
		       config,
		       session_opt.operation,
		       chroots);
  if (opt.user)
    sbuild_auth_set_user(SBUILD_AUTH(session), opt.user);
  if (opt.command)
    sbuild_auth_set_command(SBUILD_AUTH(session), opt.command);
  if (opt.preserve)
    sbuild_auth_set_environment(SBUILD_AUTH(session), environ);
  sbuild_session_set_force(session, session_opt.force);
  if (opt.quiet && opt.verbose)
    g_printerr(_("--quiet and --verbose may not be used at the same time!\nUsing verbose output.\n"));
  SbuildAuthVerbosity verbosity = SBUILD_AUTH_VERBOSITY_NORMAL;
  if (opt.quiet)
    verbosity = SBUILD_AUTH_VERBOSITY_QUIET;
  else if (opt.verbose)
    verbosity = SBUILD_AUTH_VERBOSITY_VERBOSE;
  sbuild_auth_set_verbosity(SBUILD_AUTH(session), verbosity);

  /* Set up authentication timeouts. */
  SbuildAuthConv *conv = SBUILD_AUTH_CONV(sbuild_auth_conv_tty_new());
  time_t curtime = 0;
  time(&curtime);
  sbuild_auth_conv_set_warning_timeout(conv, curtime + 15);
  sbuild_auth_conv_set_fatal_timeout(conv, curtime + 20);
  sbuild_auth_set_conv(SBUILD_AUTH(session), conv);

  /* Run session. */
  GError *session_error = NULL;
  sbuild_auth_run(SBUILD_AUTH(session), &session_error);
  if (session_error)
    g_printerr(_("Session failure: %s\n"), session_error->message);

  int exit_status = sbuild_session_get_child_status(session);

  g_object_unref(G_OBJECT(session));
  g_object_unref(G_OBJECT(config));
  g_strfreev(chroots);

  closelog();

  exit (exit_status);
}
