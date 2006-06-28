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
#include "schroot-releaselock-main.h"

#include <cstdlib>
#include <iostream>
#include <locale>

#include <boost/format.hpp>
#include <boost/program_options.hpp>


using std::endl;
using boost::format;
namespace opt = boost::program_options;

using namespace schroot_releaselock;

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

      schroot_releaselock::options::ptr opts
	(new schroot_releaselock::options(argc, argv));
      schroot_releaselock::main kit(opts);
      exit (kit.run());
    }
  catch (boost::program_options::error const& e)
    {
      sbuild::log_error() << e.what() << endl;
      sbuild::log_info()
	<< format(_("Run \"%1% --help\" to list usage example and all available options"))
	% argv[0]
	<< endl;
      exit(EXIT_FAILURE);
    }
  catch (std::exception const& e)
    {
      sbuild::log_error() << e.what() << endl;
      exit(EXIT_FAILURE);
    }
  catch (...)
    {
      sbuild::log_error() << _("An unknown exception occured") << endl;
      exit(EXIT_FAILURE);
    }
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
