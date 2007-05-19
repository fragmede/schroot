/* Copyright © 2005-2007  Roger Leigh <rleigh@debian.org>
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

#ifndef SCHROOT_BASE_RUN_H
#define SCHROOT_BASE_RUN_H

#include <schroot-base/schroot-base-options.h>

#include <sbuild/sbuild-config.h>

#include <cstdlib>
#include <iostream>
#include <string>

namespace schroot_base
{

  /**
   * Main routine.
   *
   * @param argc the number of arguments
   * @param argv argument vector
   *
   * @returns 0 on success, 1 on failure or the exit status of the
   * chroot command.
   */
  template<class O, class M>
  static int
  run (int   argc,
       char *argv[])
  {
    typedef O options_type;
    typedef M main_type;

    try
      {
	// Set up locale.
	try
	  {
	    std::locale::global(std::locale(""));
	  }
	catch (std::runtime_error const& e) // Invalid locale
	  {
	    std::locale::global(std::locale::classic());
	  }
	std::cout.imbue(std::locale());
	std::cerr.imbue(std::locale());

	bindtextdomain (SBUILD_MESSAGE_CATALOGUE, LOCALEDIR);
	textdomain (SBUILD_MESSAGE_CATALOGUE);

	typename options_type::ptr opts(new options_type);
	main_type kit(opts);
	exit (kit.run(argc, argv));
      }
    catch (std::exception const& e)
      {
	sbuild::log_exception_error(e);
	exit(EXIT_FAILURE);
      }
    catch (...)
      {
	sbuild::log_unknown_exception_error();
	exit(EXIT_FAILURE);
      }
  }

}

#endif /* SCHROOT_BASE_RUN_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
