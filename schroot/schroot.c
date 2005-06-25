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
#include <stdlib.h>
#include <stdio.h>

#include <syslog.h>

#include <libintl.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "sbuild-chroot.h"
#include "sbuild-config.h"
#include "sbuild-session.h"

/* Stored command-line options. */
static struct {
  char **chroots;
  char **command;
  char *user;
  gboolean preserve;
  gboolean quiet;
  gboolean list;
  gboolean info;
  gboolean all;
  gboolean version;
} opt =
  {
    .chroots = NULL,
    .command = NULL,
    .user = NULL,
    .preserve = FALSE,
    .quiet = FALSE,
    .list = FALSE,
    .info = FALSE,
    .all = FALSE,
    .version = FALSE
  };

/* Command-line options. */
static GOptionEntry entries[] =
{
  { "all", 'a', 0, G_OPTION_ARG_NONE, &opt.all, N_("Run command in all chroots"), NULL },
  { "chroot", 'c', 0, G_OPTION_ARG_STRING_ARRAY, &opt.chroots, N_("Use specified chroot"), "chroot" },
  { "user", 'u', 0, G_OPTION_ARG_STRING, &opt.user, N_("Username (default current user)"), "user" },
  { "list", 'l', 0, G_OPTION_ARG_NONE, &opt.list, N_("List available chroots"), NULL },
  { "info", 'i', 0, G_OPTION_ARG_NONE, &opt.info, N_("Show information about chroot"), NULL },
  { "preserve-environment", 'p', 0, G_OPTION_ARG_NONE, &opt.preserve, N_("Preserve user environment"), NULL },
  { "quiet", 'q', 0, G_OPTION_ARG_NONE, &opt.quiet, N_("Show less output"), NULL },
  { "version", 'V', 0, G_OPTION_ARG_NONE, &opt.version, N_("Print version information"), NULL },
  { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &opt.command, NULL, NULL }
};

/**
 * parse_options:
 * @argc: the number of arguments
 * @argv: argument vector
 *
 * Parse command-line options.  The options are places in the opt
 * structure.
 */
static void
parse_options(int   argc,
	      char *argv[])
{
  GError *error = NULL;

  GOptionContext *context = g_option_context_new (_("- run command or shell in a chroot"));
  g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);
  g_option_context_parse (context, &argc, &argv, &error);
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

  if (opt.all == TRUE)
    {
      const GList *list = sbuild_config_get_chroots(config);
      if (list != NULL)
	{
	  guint num_chroots = g_list_length((GList *) list);
	  chroots = g_new(char *, num_chroots + 1);
	  chroots[num_chroots] = NULL;
	  for (guint i=0; i < num_chroots; ++i)
	    {
	      GList *node = g_list_nth((GList *) list, i);
	      g_assert(node != NULL);
	      SbuildChroot *chroot = node->data;
	      g_assert(sbuild_chroot_get_name(chroot) != NULL);
	      chroots[i] = g_strdup(sbuild_chroot_get_name(chroot));
	    }
	}
    }
  else if (opt.chroots == NULL)
    {
      g_printerr(_("No chroot specified.  Use --chroot or --all.\n"));
      exit (EXIT_FAILURE);
    }
  else
    {
      if (sbuild_config_validate_chroots(config, opt.chroots) == FALSE)
	exit(EXIT_FAILURE);
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
  SbuildConfig *config = sbuild_config_new(SCHROOT_CONFIG_FILE);
  g_assert (config != NULL);

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
      g_printerr(_("No chroots defined in %s\n"), SCHROOT_CONFIG_FILE);
      exit (EXIT_FAILURE);
    }

  /* Print chroot information for specified chroots. */
  if (opt.info == TRUE)
    {
      for (guint i=0; chroots[i] != NULL; ++i)
	{
	  SbuildChroot *chroot = sbuild_config_find_alias(config, chroots[i]);
	  if (chroot)
	    sbuild_chroot_print_details(chroot, stdout);
	}
      exit (EXIT_SUCCESS);
    }

  /* Create and run a session. */
  SbuildSession *session = sbuild_session_new(config, chroots);
  if (opt.user)
    sbuild_session_set_user(session, opt.user);
  if (opt.command)
    sbuild_session_set_command(session, opt.command);
  if (opt.preserve)
    sbuild_session_set_environment(session, environ);
  if (opt.quiet)
    sbuild_session_set_quiet(session, TRUE);

  int child_status = 0;
  GError *session_error = NULL;
  sbuild_session_run(session, &child_status, &session_error);
  if (session_error)
    g_printerr(_("Session failure: %s\n"), session_error->message);

  g_object_unref(G_OBJECT(session));
  g_object_unref(G_OBJECT(config));

  closelog();

  exit (child_status);
}
