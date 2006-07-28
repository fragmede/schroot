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

#include "schroot-main.h"
#include "schroot-options.h"

#include <cstdlib>
#include <iostream>

#include <boost/format.hpp>

#include <syslog.h>

using std::endl;
using boost::format;
using namespace schroot;

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

      schroot::options::ptr opts(new schroot::options);
      schroot::main kit(opts);
      exit (kit.run(argc, argv));
    }
  catch (std::exception const& e)
    {
      sbuild::log_exception_error(e);
      exit(EXIT_FAILURE);
    }
  catch (...)
    {
      sbuild::log_error() << _("An unknown exception occurred") << endl;
      exit(EXIT_FAILURE);
    }
}
