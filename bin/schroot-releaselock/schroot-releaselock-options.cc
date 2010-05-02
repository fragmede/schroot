/* Copyright © 2005-2007  Roger Leigh <rleigh@debian.org>
 *
 * schroot is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * schroot is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 *********************************************************************/

#include <config.h>

#include <sbuild/sbuild-i18n.h>

#include "schroot-releaselock-options.h"

#include <cstdlib>
#include <iostream>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

using std::endl;
using boost::format;
using sbuild::_;
namespace opt = boost::program_options;
using namespace schroot_releaselock;

const options::action_type options::ACTION_RELEASELOCK ("releaselock");

options::options ():
  schroot_base::options(),
  device(),
  pid(0),
  lock(_("Lock"))
{
}

options::~options ()
{
}

void
options::add_options ()
{
  // Chain up to add basic options.
  schroot_base::options::add_options();

  action.add(ACTION_RELEASELOCK);
  action.set_default(ACTION_RELEASELOCK);

  lock.add_options()
    ("device,d", opt::value<std::string>(&this->device),
     _("Device to unlock (full path)"))
    ("pid,p", opt::value<int>(&this->pid),
     _("Process ID owning the lock"));
}

void
options::add_option_groups ()
{
  // Chain up to add basic option groups.
  schroot_base::options::add_option_groups();

#ifndef BOOST_PROGRAM_OPTIONS_DESCRIPTION_OLD
  if (!lock.options().empty())
#else
  if (!lock.primary_keys().empty())
#endif
    {
      visible.add(lock);
      global.add(lock);
    }
}

void
options::check_options ()
{
  // Chain up to check basic options.
  schroot_base::options::check_options();

  if (this->action == ACTION_RELEASELOCK &&
      this->device.empty())
	throw opt::validation_error
	  (
#ifndef BOOST_PROGRAM_OPTIONS_VALIDATION_ERROR_OLD
	   opt::validation_error::at_least_one_value_required,
#endif
	   _("No device specified"));
}
