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
using sbuild::_;
namespace opt = boost::program_options;
using namespace schroot_listmounts;

const options::action_type options::ACTION_LISTMOUNTS ("listmounts");

options::options ():
  schroot_base::options(),
  mountpoint(),
  mount(_("Mount"))
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

  action.add(ACTION_LISTMOUNTS);
  action.set_default(ACTION_LISTMOUNTS);

  mount.add_options()
    ("mountpoint,m", opt::value<std::string>(&this->mountpoint),
     _("Mountpoint to check (full path)"));
}

void
options::add_option_groups ()
{
  // Chain up to add basic option groups.
  schroot_base::options::add_option_groups();

#ifndef BOOST_PROGRAM_OPTIONS_DESCRIPTION_OLD
  if (!mount.options().empty())
#else
  if (!mount.primary_keys().empty())
#endif
    {
      visible.add(mount);
      global.add(mount);
    }
}

void
options::check_options ()
{
  // Chain up to check basic options.
  schroot_base::options::check_options();

  if (this->action == ACTION_LISTMOUNTS &&
      this->mountpoint.empty())
    throw opt::validation_error(_("No mount point specified"));
}
