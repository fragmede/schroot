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

#include "csbuild-options.h"

#include <cstdlib>
#include <iostream>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

using std::endl;
using sbuild::_;
using boost::format;
namespace opt = boost::program_options;
using namespace csbuild;

options::options ():
  schroot::options_base()
{
}

options::~options ()
{
}

void
options::add_options ()
{
  schroot::options_base::add_options();

  general.add_options()
    ("path,p", opt::value<std::string>(&this->chroot_path),
     _("Print path to selected chroot"));

  chroot.add_options()
    ("all,a",
     _("Select all chroots"));

  chrootenv.add_options()
    ("directory", opt::value<std::string>(&this->directory),
     _("Directory to use"))
    ("preserve-environment,d",
     _("Preserve user environment"));
}

void
options::check_options ()
{
  schroot::options_base::check_options();

  if (vm.count("path"))
    set_action(ACTION_LOCATION);

  if (vm.count("all"))
    {
      this->all = false;
      this->all_chroots = true;
      this->all_sessions = false;
    }

  if (vm.count("preserve-environment"))
    this->preserve = true;

  if (this->quiet && this->verbose)
    {
      sbuild::log_warning()
	<< _("--quiet and --verbose may not be used at the same time")
	<< endl;
      sbuild::log_info() << _("Using verbose output") << endl;
    }

  if (!this->chroots.empty() && all_used())
    {
      sbuild::log_warning()
	<< _("--chroot and --all may not be used at the same time")
	<< endl;
      sbuild::log_info() << _("Using --chroots only") << endl;
      this->all = this->all_chroots = this->all_sessions = false;
    }

  if (this->all == true)
    {
      this->all_chroots = true;
      this->all_sessions = true;
    }
}
