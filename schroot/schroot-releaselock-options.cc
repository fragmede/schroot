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

#include <iostream>

#include <stdlib.h>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include "sbuild.h"

#include "schroot-releaselock-options.h"

using std::endl;
using boost::format;
namespace opt = boost::program_options;
using namespace schroot_releaselock;

options::options(int   argc,
		 char *argv[]):
  device(),
  pid(0),
  version(0)
{
  opt::options_description general(_("General options"));
  general.add_options()
    ("help,?", _("Show help options"))
    ("version,V",
     _("Print version information"));

  opt::options_description lock(_("Lock options"));
  lock.add_options()
    ("device,d", opt::value<std::string>(&this->device),
     _("Device to unlock (full path)"))
    ("pid,p", opt::value<int>(&this->pid),
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
    this->version = true;
}

options::~options()
{
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
