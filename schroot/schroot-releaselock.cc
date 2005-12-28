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
#include <boost/program_options.hpp>

#include <lockdev.h>

#include "sbuild-i18n.h"
#include "sbuild-log.h"
#include "sbuild-util.h"

using std::endl;
using boost::format;
namespace opt = boost::program_options;

namespace schroot_releaselock
{

  /**
   * schroot-releaselock command-line options.
   * @todo Split out into a separate file.
   */
  struct options {
    /// The device to unlock.
    std::string device;
    /// The PID holding the lock.
    int         pid;
    /// Display version information.
    bool        version;
  };

  options opts =
    {
      "",
      0,
      false
    };
}

/*
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
  opt::options_description general(_("General options"));
  general.add_options()
    ("help,?", _("Show help options"))
    ("version,V",
     _("Print version information"));

  opt::options_description lock(_("Lock options"));
  lock.add_options()
    ("device,d", opt::value<std::string>(&opts.device),
     _("Device to unlock (full path)"))
    ("pid,p", opt::value<int>(&opts.pid),
     _("Process ID owning the lock"));


  opt::options_description global;
  global.add(general).add(lock);

  opt::variables_map vm;
  opt::store(opt::parse_command_line(argc, argv, global), vm);
  opt::notify(vm);

  if (vm.count("help"))
    {
      std::cout
	<< _("Usage:") << '\n'
	<< _("  schroot-releaselock [OPTION...] - release a device lock") << '\n'
	<< global << std::flush;
      exit(EXIT_SUCCESS);
}

  if (vm.count("version"))
    opts.version = true;
}

/*
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

/*
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

  if (opts.version)
    {
      print_version(std::cerr);
      exit(EXIT_SUCCESS);
    }

  if (opts.device.empty())
    {
      sbuild::log_error() << _("No device specified") << endl;
      exit (EXIT_FAILURE);
    }

  if (opts.pid == 0)
    {
      sbuild::log_error() << _("No pid specified; forcing release of lock")
			  << endl;
    }

  struct stat statbuf;

  if (stat(opts.device.c_str(), &statbuf) == -1)
    {
      sbuild::log_error()
	<< format(_("Failed to stat device %1%: %2%"))
	% opts.device % strerror(errno)
	<< endl;
      exit (EXIT_FAILURE);
    }
  if (!S_ISBLK(statbuf.st_mode))
    {
      sbuild::log_error()
	<< format(_("%1% is not a block device")) % opts.device << endl;
      exit (EXIT_FAILURE);
    }

  pid_t status = dev_unlock(opts.device.c_str(), opts.pid);
  if (status < 0) // Failure
    {
      sbuild::log_error()
	<< format(_("%1%: failed to release device lock")) % opts.device
	<< endl;
      exit (EXIT_FAILURE);
    }
  else if (status > 0) // Owned
    {
      sbuild::log_error()
	<< format(_("%1%: failed to release device lock owned by pid %2%"))
	% opts.device % status
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
