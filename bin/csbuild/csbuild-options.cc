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
  packages(),
  nolog(false),
  batch_mode(false),
  deb_build_options(),
  deb_build_options_string(),
  distribution(),
  archive(),
  build_arch(),
  build_arch_all(false),
  build_source(false),
  force_orig_source(false),
  bin_nmu(false),
  bin_nmu_changelog(),
  bin_nmu_version(0),
  append_version(),
  apt_update(false),
  chroot(),
  purge_string(),
  purge(PURGE_ALWAYS),
  purge_deps_string(),
  purge_deps(PURGE_ALWAYS),
  setup_hook_script(),
  keyid(),
  maintainer(),
  uploader(),
  build_depends(),
  build_conflicts(),
  build_depends_indep(),
  build_conflicts_indep(),
  depends_algorithm(),
  gcc_snapshot(false),
  build(_("Build options")),
  version(_("Package version options")),
  chrootopt(_("Build environment options")),
  user(_("User options")),
  depends(_("Build dependency override options")),
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
    ("nolog,n", _("Don't log program output"))
    ("batch,b", _("Run in batch mode"));

  build.add_options()
    ("debbuildopt", opt::value<sbuild::string_list>(&this->deb_build_options),
     _("dpkg-buildpackage option"))
    ("debbuildopts", opt::value<std::string>(&this->deb_build_options_string),
     _("dpkg-buildpackage options (space-separated)"))
    ("dist,d", opt::value<std::string>(&this->distribution),
     _("Distribution to build for"))
    ("archive", opt::value<std::string>(&this->archive),
     _("Archive to build for"))
    ("arch", opt::value<std::string>(&this->build_arch),
     _("Build architecture"))
    ("arch-all,A",
     _("Build architecture \"all\" packages"))
    ("source,s",
     _("Build a source package"))
    ("force-orig-source",
     _("Force building of a source package, irrespective of Debian version"));

  version.add_options()
    ("make-binNMU", opt::value<std::string>(&this->bin_nmu_changelog),
     _("Make a binary non-maintainer upload (changelog entry)"))
    ("binNMU", opt::value<unsigned int>(&this->bin_nmu_version),
     _("Make a binary non-maintainer upload (binNMU number)"))
    ("append-to-version", opt::value<std::string>(&this->append_version),
     _("Append version suffix"));

  chrootopt.add_options()
    ("apt-update", _("Update chroot environment"))
    ("chroot,c", opt::value<std::string>(&this->chroot),
     _("Chroot environment to build in"))
    ("purge,p", opt::value<std::string>(&this->purge_string),
     _("Purge build mode"))
    ("purge-deps", opt::value<std::string>(&this->purge_deps_string),
     _("Purge dependencies mode"))
    ("setup-hook", opt::value<std::string>(&this->setup_hook_script),
     _("Run setup hook script in chroot prior to building"));

  user.add_options()
    ("keyid,k", opt::value<std::string>(&this->keyid),
     _("GPG key identifier"))
    ("maintainer,m", opt::value<std::string>(&this->maintainer),
     _("Package maintainer"))
    ("uploader,u", opt::value<std::string>(&this->uploader),
     _("Package uploader"));

  depends.add_options()
    ("add-depends", opt::value<string_list>(&this->build_depends),
     _("Add a build dependency"))
    ("add-conflicts", opt::value<string_list>(&this->build_conflicts),
     _("Add a build conflict"))
    ("add-depends-indep", opt::value<string_list>(&this->build_depends_indep),
     _("Add an architecture-independent build dependency"))
    ("add-conflicts-indep", opt::value<string_list>(&this->build_conflicts_indep),
     _("Add an architecture-independent build conflict"));

  special.add_options()
    ("check-depends-algorithm,C", opt::value<std::string>(&this->depends_algorithm),
     _("Specify algorithm for dependency checking"))
    ("gcc-snapshot,G",
     _("Build using the current GCC development snapshot"));

  hidden.add_options()
    ("package", opt::value<sbuild::string_list>(&this->packages),
     _("Package to build"));

  positional.add("package", -1);
}

void
options::add_option_groups ()
{
  // Chain up to add basic option groups.
  schroot_base::options::add_option_groups();

  visible.add(build);
  global.add(build);

  visible.add(version);
  global.add(version);

  visible.add(chrootopt);
  global.add(chrootopt);

  visible.add(user);
  global.add(user);

  visible.add(depends);
  global.add(depends);

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

  if (vm.count("nolog"))
    this->nolog = true;

  if (vm.count("batch"))
    this->batch_mode = true;

  if (vm.count("arch-all"))
    this->build_arch_all = true;

  if (vm.count("source"))
    this->build_source = true;

  if (vm.count("force-orig-source"))
    this->force_orig_source = true;

  if (vm.count("binNMU") && vm.count("make-binNMU"))
    this->bin_nmu = true;
  else if (vm.count("binNMU"))
    throw error
      (_("--makebinNMU missing"));
  else if (vm.count("make-binNMU"))
    throw error
      (_("--binNMU missing"));

  if (!deb_build_options_string.empty())
    {
      string_list bopts = sbuild::split_string(deb_build_options_string,
                                               std::string(1, ' '));

      for (const auto& bopt : bopts)
        {
          if (!bopt.empty())
            deb_build_options.push_back(bopt);
        }
    }

  if (vm.count("apt-update"))
    this->apt_update = true;

  if (vm.count("use-snapshot"))
    this->gcc_snapshot = true;
}
