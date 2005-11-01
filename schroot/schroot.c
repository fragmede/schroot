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

#include "schroot-options.h"

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
 * @options: an #SchrootOptions
 *
 * Get a list of chroots based on the specified options (--all, --chroot).
 *
 * Returns a NULL-terminated string vector (GStrv).
 */
char **
get_chroot_options(SbuildConfig   *config,
		   SchrootOptions *options)
{
  char **chroots = NULL;

  if (options->all_chroots == TRUE || options->all_sessions == TRUE)
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
 	      if ((active == FALSE && options->all_chroots == FALSE) ||
 		  (active == TRUE && options->all_sessions == FALSE))
		continue;
	      chroots[pos++] = g_strdup(sbuild_chroot_get_name(chroot));
	    }
	  chroots[pos] = NULL;
	}
    }
  else
    {
      char **invalid_chroots =
	sbuild_config_validate_chroots(config, options->chroots);

      if (invalid_chroots)
	{
	  for (guint i = 0; invalid_chroots[i] != NULL; ++i)
	    g_printerr(_("%s: No such chroot\n"), invalid_chroots[i]);
	  g_strfreev(invalid_chroots);
	  exit(EXIT_FAILURE);
	}
      g_strfreev(invalid_chroots);
      chroots = g_strdupv(options->chroots);
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
  SchrootOptions *options = schroot_options_parse(argc, argv);

  if (options->version == TRUE)
    {
      print_version(stdout);
      exit(EXIT_SUCCESS);
    }

  /* Initialise chroot configuration. */
  SbuildConfig *config = sbuild_config_new();
  g_assert (config != NULL);

  /* The normal chroot list is used when starting a session or running
     any chroot type or session, or displaying chroot information. */
  if ((options->list == FALSE && options->info == FALSE) ||
      options->all_chroots == TRUE)
    sbuild_config_add_config_file(config, SCHROOT_CONF);
  /* The session chroot list is used when running or ending an
     existing session, or displaying chroot information. */
  if ((options->list == FALSE && options->info == FALSE) ||
      options->all_sessions == TRUE)
    sbuild_config_add_config_directory(config, SCHROOT_SESSION_DIR);

  if (sbuild_config_get_chroots(config) == NULL)
    {
      g_printerr(_("No chroots are defined in %s\n"), SCHROOT_CONF);
      exit (EXIT_FAILURE);
    }

  /* Print chroot list (including aliases). */
  if (options->list == TRUE)
    {
      sbuild_config_print_chroot_list(config, stdout);
      exit(EXIT_SUCCESS);
    }

  /* Get list of chroots to use */
  char **chroots = get_chroot_options(config, options);
  if (chroots == NULL)
    {
      g_printerr(_("The specified chroots are not defined in %s\n"), SCHROOT_CONF);
      exit (EXIT_FAILURE);
    }

  /* Print chroot information for specified chroots. */
  if (options->info == TRUE)
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

  if (options->session_operation == SBUILD_SESSION_OPERATION_BEGIN &&
      chroots[0] != NULL && chroots[1] != NULL)
    {
      g_printerr(_("Only one chroot may be specified when beginning a session\n"));
      exit (EXIT_FAILURE);
    }

  /* Create a session. */
  SbuildSession *session =
    sbuild_session_new("schroot",
		       config,
		       options->session_operation,
		       chroots);
  if (options->user)
    sbuild_auth_set_user(SBUILD_AUTH(session), options->user);
  if (options->command)
    sbuild_auth_set_command(SBUILD_AUTH(session), options->command);
  if (options->preserve)
    sbuild_auth_set_environment(SBUILD_AUTH(session), environ);
  sbuild_session_set_force(session, options->session_force);
  if (options->quiet && options->verbose)
    g_printerr(_("--quiet and --verbose may not be used at the same time!\nUsing verbose output.\n"));
  SbuildAuthVerbosity verbosity = SBUILD_AUTH_VERBOSITY_NORMAL;
  if (options->quiet)
    verbosity = SBUILD_AUTH_VERBOSITY_QUIET;
  else if (options->verbose)
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

  schroot_options_free(options);

  closelog();

  exit (exit_status);
}
