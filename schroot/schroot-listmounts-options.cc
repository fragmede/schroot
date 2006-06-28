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

#include "schroot-listmounts-options.h"

#include <cstdlib>
#include <iostream>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

using std::endl;
using boost::format;
namespace opt = boost::program_options;
using namespace schroot_listmounts;

options::options (int   argc,
		  char *argv[]):
  schroot_base::options(argc, argv),
  action(ACTION_LISTMOUNTS),
  mountpoint(),
  mount(_("Mount"))
{
  add_options();
  parse_options(argc, argv);
  check_options();
  check_actions();
}

options::~options ()
{
}

void
options::add_options ()
{
  schroot_base::options::add_options();

  mount.add_options()
    ("mountpoint,m", opt::value<std::string>(&this->mountpoint),
     _("Mountpoint to check (full path)"));
}

void
options::parse_options (int   argc,
			char *argv[])
{
  if (!general.options().empty())
    {
      visible.add(general);
      global.add(general);
    }
  if (!mount.options().empty())
    {
      visible.add(mount);
      global.add(mount);
    }
  if (!hidden.options().empty())
    global.add(hidden);

  opt::store(opt::command_line_parser(argc, argv).
	     options(global).positional(positional).run(), vm);
  opt::notify(vm);
}

void
options::check_options ()
{
  schroot_base::options::check_options();

  if (vm.count("help"))
    set_action(ACTION_HELP);

  if (vm.count("version"))
    set_action(ACTION_VERSION);

  if (this->mountpoint.empty() &&
      this->action != ACTION_HELP &&
      this->action != ACTION_VERSION)
    throw opt::validation_error(_("No mount point specified"));
}

void
options::set_action (action_type action)
{
  if (this->action != ACTION_LISTMOUNTS)
    throw opt::validation_error(_("Only one action may be specified"));

  this->action = action;
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
