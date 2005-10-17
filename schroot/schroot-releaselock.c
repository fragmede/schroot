/* schroot-releaselock - release a device lock
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
#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <lockdev.h>

#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>

static gboolean
parse_session_options(const gchar  *option_name,
		      const gchar  *value,
		      gpointer      data,
		      GError      **error);

/* Stored command-line options. */
static struct {
  char     *device;
  gint      pid;
  gboolean  version;
} opt =
  {
    .device = NULL,
    .pid = 0,
    .version = FALSE
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
      { "device", 'd', 0, G_OPTION_ARG_STRING, &opt.device,
	N_("Device to unlock (full path)"), "device" },
      { "pid", 'p', 0, G_OPTION_ARG_INT, &opt.pid,
	N_("Process owning the lock"), "pid" },
      { "version", 'V', 0, G_OPTION_ARG_NONE, &opt.version,
	N_("Print version information"), NULL },
      { NULL }
    };

  GError *error = NULL;

  GOptionContext *context = g_option_context_new (_("- release a device lock"));
  g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);
  g_option_context_parse (context, &argc, &argv, &error);
  g_option_context_free (context);
  if (error != NULL)
    {
      g_printerr(_("Error parsing options: %s\n"), error->message);
      exit (EXIT_FAILURE);
    }
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
  g_fprintf(file, _("schroot-releaselock (Debian sbuild) %s\n"), VERSION);
  g_fprintf(file, _("Written by Roger Leigh\n\n"));
  g_fprintf(file, _("Copyright (C) 2004-2005 Roger Leigh\n"));
  g_fprintf(file, _("This is free software; see the source for copying conditions.  There is NO\n"
		    "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"));
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
  setlocale (LC_ALL, "");

  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  textdomain (GETTEXT_PACKAGE);

#ifndef SBUILD_DEBUG
  /* Discard g_debug output for this logging domain. */
  g_log_set_handler (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, debug_logfunc, NULL);
#endif

  /* Parse command-line options into opt structure. */
  parse_options(argc, argv);

  if (opt.version == TRUE)
    {
      print_version(stdout);
      exit(EXIT_SUCCESS);
    }

  if (opt.device == NULL)
    {
      g_printerr(_("No device specified\n"));
      exit (EXIT_FAILURE);
    }

  if (opt.pid == 0)
    {
      g_printerr(_("No pid specified; forcing release of lock\n"));
    }

  struct stat statbuf;

  if (stat(opt.device, &statbuf) == -1)
    {
      g_printerr(_("Failed to stat device %s: %s\n"), opt.device,
		 g_strerror(errno));
      exit (EXIT_FAILURE);
    }
  if (!S_ISBLK(statbuf.st_mode))
    {
      g_printerr(_("%s is not a block device\n"), opt.device);
      exit (EXIT_FAILURE);
    }

  pid_t status = dev_unlock(opt.device, opt.pid);
  if (status < 0) // Failure
    {
      g_printerr(_("%s: failed to release device lock\n"), opt.device);
      exit (EXIT_FAILURE);
    }
  else if (status > 0) // Owned
    {
      g_printerr(_("%s: failed to release device lock owned by pid %d\n"),
		 opt.device, status);
      exit (EXIT_FAILURE);
    }

  exit (EXIT_SUCCESS);
}
