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

#include "schroot-options.h"

#include <cstdlib>
#include <iostream>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

using std::endl;
using boost::format;
namespace opt = boost::program_options;
using namespace schroot_base;

options::options ():
  quiet(false),
  verbose(false),
  general(_("General options")),
  hidden(_("Hidden options")),
  positional(),
  visible(),
  global(),
  vm()
{
}

options::~options ()
{
}

boost::program_options::options_description const&
options::get_visible_options() const
{
  return this->visible;
}

void
options::parse (int   argc,
		char *argv[])
{
  add_options();
  add_option_groups();

  opt::store(opt::command_line_parser(argc, argv).
	     options(global).positional(positional).run(), vm);
  opt::notify(vm);

  check_options();
  check_actions();
}

void
options::add_options ()
{
  general.add_options()
    ("help,h",
     _("Show help options"))
    ("version,V",
     _("Print version information"))
    ("quiet,q",
     _("Show less output"))
    ("verbose,v",
     _("Show more output"));

  hidden.add_options()
    ("debug", opt::value<std::string>(&this->debug_level),
     _("Enable debugging messages"));
}

void
options::add_option_groups ()
{
  if (!general.options().empty())
    {
      visible.add(general);
      global.add(general);
    }
  if (!hidden.options().empty())
    global.add(hidden);
}

void
options::check_options ()
{
  if (vm.count("quiet"))
    this->quiet = true;
  if (vm.count("verbose"))
    this->verbose = true;

  if (vm.count("debug"))
    {
      if (this->debug_level == "none")
	sbuild::debug_level = sbuild::DEBUG_NONE;
      else if (this->debug_level == "notice")
	sbuild::debug_level = sbuild::DEBUG_NOTICE;
      else if (this->debug_level == "info")
	sbuild::debug_level = sbuild::DEBUG_INFO;
      else if (this->debug_level == "warning")
	sbuild::debug_level = sbuild::DEBUG_WARNING;
      else if (this->debug_level == "critical")
	sbuild::debug_level = sbuild::DEBUG_CRITICAL;
      else
	throw opt::validation_error(_("Invalid debug level"));
    }
  else
    sbuild::debug_level = sbuild::DEBUG_NONE;
}

void
options::check_actions ()
{
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
