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

#include <iostream>

#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <boost/format.hpp>

#include <lockdev.h>

#include <glib.h>

#include "sbuild-i18n.h"
#include "sbuild-log.h"
#include "sbuild-util.h"

using std::endl;
using boost::format;

/* Stored command-line options. */
struct options {
  char *device;
  int   pid;
  bool  version;
};

options opt =
  {
    NULL,
    0,
    FALSE
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
      sbuild::log_error()
	<< format(_("Error parsing options: %1%")) % error->message << endl;
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
print_version (std::ostream& stream)
{
  stream << format(_("schroot-releaselock (Debian sbuild) %1%\n")) % VERSION
	 << _("Written by Roger Leigh\n\n")
	 << _("Copyright (C) 2004-2005 Roger Leigh\n")
	 << _("This is free software; see the source for copying conditions.  There is NO\n"
	      "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n")
	 << std::flush;
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
  sbuild::debug_level = sbuild::DEBUG_CRITICAL;
#else
  sbuild::debug_level = sbuild::DEBUG_NONE;
#endif

  /* Parse command-line options into opt structure. */
  parse_options(argc, argv);

  if (opt.version == TRUE)
    {
      print_version(std::cerr);
      exit(EXIT_SUCCESS);
    }

  if (opt.device == NULL)
    {
      sbuild::log_error() << _("No device specified") << endl;
      exit (EXIT_FAILURE);
    }

  if (opt.pid == 0)
    {
      sbuild::log_error() << _("No pid specified; forcing release of lock")
			  << endl;
    }

  struct stat statbuf;

  if (stat(opt.device, &statbuf) == -1)
    {
      sbuild::log_error()
	<< format(_("Failed to stat device %1%: %2%"))
	% opt.device % strerror(errno)
	<< endl;
      exit (EXIT_FAILURE);
    }
  if (!S_ISBLK(statbuf.st_mode))
    {
      sbuild::log_error()
	<< format(_("%1% is not a block device")) % opt.device << endl;
      exit (EXIT_FAILURE);
    }

  pid_t status = dev_unlock(opt.device, opt.pid);
  if (status < 0) // Failure
    {
      sbuild::log_error()
	<< format(_("%1%: failed to release device lock")) % opt.device
	<< endl;
      exit (EXIT_FAILURE);
    }
  else if (status > 0) // Owned
    {
      sbuild::log_error()
	<< format(_("%1%: failed to release device lock owned by pid %2%"))
	% opt.device % status
	<< endl;
      exit (EXIT_FAILURE);
    }

  exit (EXIT_SUCCESS);
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
