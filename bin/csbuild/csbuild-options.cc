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

#include <config.h>

#include <sbuild/sbuild-i18n.h>

#include "csbuild-options.h"

#include <cstdlib>
#include <iostream>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

using std::endl;
using boost::format;
using sbuild::string_list;
using sbuild::_;
namespace opt = boost::program_options;
using namespace csbuild;

const options::action_type options::ACTION_BUILD ("build");

options::options ():
  schroot_base::options(),
  user(_("User")),
  dependencies(_("Dependencies"))
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

  action.add(ACTION_BUILD);
  action.set_default(ACTION_BUILD);

  general.add_options()
    ("batchmode,b", opt::value<bool>(&this->batchmode),
     _("Use batch mode"))
    ("nolog,n", opt::value<bool>(&this->nolog),
     _("Don't log program output"))
    ("arch-all,A", opt::value<bool>(&this->build_arch_all),
     _("Build architecture \"all\" packages"))
    ("source,s", opt::value<bool>(&this->build_source),
     _("Build a source package"))
    ("force-orig-source,f", opt::value<bool>(&this->force_orig_source),
     _("Force building of a source package, irrespective of Debian version"))
    ("distribution,d", opt::value<std::string>(&this->distribution),
     _("Distribution to build for"))
    ("purge,p", opt::value<std::string>(&this->purge_string),
     _("Purge mode"))
    ("binary-nmu,B", opt::value<bool>(&this->bin_nmu),
     _("Make a binary non-maintainer upload"))
    ("gcc-snapshot,G", opt::value<bool>(&this->gcc_snapshot),
     _("Build using the current GCC development snapshot"));

  user.add_options()
    ("keyid,k", opt::value<std::string>(&this->keyid),
     _("GPG key identifier"))
    ("maintainer,m", opt::value<std::string>(&this->maintainer),
     _("Package maintainer"))
    ("uploader,u", opt::value<std::string>(&this->uploader),
     _("Package uploader"));


  dependencies.add_options()
    ("force-depends", opt::value<string_list>(&this->forced_dependencies),
     _("Force a build dependency"))
    ("add-depends", opt::value<string_list>(&this->additional_dependencies),
     _("Add a build dependency"));


}

void
options::add_option_groups ()
{
  // Chain up to add basic option groups.
  schroot_base::options::add_option_groups();

  visible.add(user);
  global.add(user);

  visible.add(dependencies);
  global.add(dependencies);
}

void
options::check_options ()
{
  // Chain up to check basic options.
  schroot_base::options::check_options();
}
