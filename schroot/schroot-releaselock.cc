/* Copyright © 2005-2006  Roger Leigh <rleigh@debian.org>
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

#include <sbuild/sbuild-i18n.h>
#include <sbuild/sbuild-log.h>
#include <sbuild/sbuild-types.h>

#include "schroot-releaselock-options.h"

#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <locale>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <lockdev.h>

using std::endl;
using boost::format;
namespace opt = boost::program_options;

using namespace schroot_releaselock;

/**
 * Print version information.
 *
 * @param stream the stream to output to.
 */
void
print_version (std::ostream& stream)
{
  format fmt(_("%1% (Debian sbuild) %2% (%3%)\n"));
  fmt % "schroot-releaselock" % VERSION % sbuild::date(RELEASE_DATE);

  stream << fmt
	 << _("Written by Roger Leigh\n\n")
	 << _("Copyright (C) 2004-2006 Roger Leigh\n")
	 << _("This is free software; see the source for copying conditions.  There is NO\n"
	      "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n")
	 << std::flush;
}

/**
 * Main routine.
 *
 * @param argc the number of arguments
 * @param argv argument vector
 *
 * @returns 0 on success, 1 on failure or the exit status of the
 * chroot command.
 */
int
main (int   argc,
      char *argv[])
{
  try
    {
      // Set up locale.
      std::locale::global(std::locale(""));
      std::cout.imbue(std::locale());
      std::cerr.imbue(std::locale());

      bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
      textdomain (GETTEXT_PACKAGE);

#ifndef SBUILD_DEBUG
      sbuild::debug_level = sbuild::DEBUG_CRITICAL;
#endif

      // Parse command-line options.
      options opts(argc, argv);

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
  catch (std::exception const& e)
    {
      sbuild::log_error() << e.what() << endl;

      exit(EXIT_FAILURE);
    }
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
