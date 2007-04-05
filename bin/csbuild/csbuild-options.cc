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
  build(_("Build options")),
  user(_("User options")),
  special(_("Special options"))
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

  actions.add_options()
    ("build",
     _("Build source packages (default)"));

  general.add_options()
    ("nolog,n", opt::value<bool>(&this->nolog),
     _("Don't log program output"));

  build.add_options()
    ("distribution,d", opt::value<std::string>(&this->distribution),
     _("Distribution to build for"))
    ("arch-all,A", opt::value<bool>(&this->build_arch_all),
     _("Build architecture \"all\" packages"))
    ("source,s", opt::value<bool>(&this->build_source),
     _("Build a source package"))
    ("force-orig-source,f", opt::value<bool>(&this->force_orig_source),
     _("Force building of a source package, irrespective of Debian version"))
    ("binary-nmu,B", opt::value<bool>(&this->bin_nmu),
     _("Make a binary non-maintainer upload"))
    ("purge,p", opt::value<std::string>(&this->purge_string),
     _("Purge mode"));

  user.add_options()
    ("keyid,k", opt::value<std::string>(&this->keyid),
     _("GPG key identifier"))
    ("maintainer,m", opt::value<std::string>(&this->maintainer),
     _("Package maintainer"))
    ("uploader,u", opt::value<std::string>(&this->uploader),
     _("Package uploader"));


  special.add_options()
    ("add-depends", opt::value<string_list>(&this->additional_dependencies),
     _("Add a build dependency"))
    ("force-depends", opt::value<string_list>(&this->forced_dependencies),
     _("Force a build dependency"))
    ("gcc-snapshot,G", opt::value<bool>(&this->gcc_snapshot),
     _("Build using the current GCC development snapshot"));


}

void
options::add_option_groups ()
{
  // Chain up to add basic option groups.
  schroot_base::options::add_option_groups();

  visible.add(build);
  global.add(build);

  visible.add(user);
  global.add(user);

  visible.add(special);
  global.add(special);
}

void
options::check_options ()
{
  // Chain up to check basic options.
  schroot_base::options::check_options();

  if (vm.count("build"))
    this->action = ACTION_BUILD;
}
