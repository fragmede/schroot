/* Copyright © 2005-2013  Roger Leigh <rleigh@debian.org>
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

#include "sbuild-log.h"

#include "csbuild-main.h"

#include <cerrno>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <locale>
#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <boost/format.hpp>

using std::endl;
using boost::format;
using sbuild::_;
using sbuild::N_;
using namespace csbuild;

main::main (options::ptr& options):
  schroot_base::main("csbuild",
                     // TRANSLATORS: '...' is an ellipsis e.g. U+2026,
                     // and '-' is an em-dash.
                     _("[OPTION…] — build Debian packages from source"),
                     options,
                     false),
  opts(options)
{
}

main::~main ()
{
}

void
main::action_build ()
{
  sbuild::string_list command;
  sbuild::environment env(environ);

  /// @todo Make sbuild binary configurable.
  command.push_back("/usr/bin/sbuild");

  /// @todo Handle incrementing debug level.
  if (sbuild::debug_log_level != sbuild::DEBUG_NONE)
    command.push_back("--debug");

  if (opts->nolog)
    command.push_back("--nolog");

  if (opts->batch_mode)
    command.push_back("--batch");

  for (const auto& buildopt : opts->deb_build_options)
    {
      std::string bopt("--debbuildopt=");
      bopt += buildopt;
      command.push_back(bopt);
    }

  if (!opts->distribution.empty())
    {
      std::string dist("--dist=");
      dist += opts->distribution;
      command.push_back(dist);
    }

  if (!opts->archive.empty())
    {
      std::string archive("--archive=");
      archive += opts->archive;
      command.push_back(archive);
    }

  if (!opts->build_arch.empty())
    {
      std::string arch("--arch=");
      arch += opts->build_arch;
      command.push_back(arch);
    }

  if (opts->build_arch_all)
      command.push_back("--arch-all");

  if (opts->build_source)
      command.push_back("--source");

  if (opts->force_orig_source)
      command.push_back("--force-orig-source");

  if (opts->bin_nmu)
    {
      std::string binnmu1("--make-binNMU=");
      binnmu1 += opts->bin_nmu_changelog;
      command.push_back(binnmu1);

      std::ostringstream binnmu2;
      binnmu2.imbue(std::locale::classic());
      binnmu2 << "--binNMU=";
      binnmu2 << opts->bin_nmu_version;
      command.push_back(binnmu2.str());
    }

  if (!opts->append_version.empty())
    {
      std::string append("--append-to-version=");
      append += opts->append_version;
      command.push_back(append);
    }

  if (opts->apt_update)
    command.push_back("apt-update");

  if (!opts->chroot.empty())
    {
      std::string chroot("--chroot=");
      chroot += opts->chroot;
      command.push_back(chroot);
    }

  if (!opts->purge_string.empty())
    {
      std::string purge("--purge=");
      purge += opts->purge_string;
      command.push_back(purge);
    }

  if (!opts->purge_deps_string.empty())
    {
      std::string purge_deps("--purge-deps=");
      purge_deps += opts->purge_deps_string;
      command.push_back(purge_deps);
    }

  if (!opts->setup_hook_script.empty())
    {
      std::string setup_hook("--setup-hook=");
      setup_hook += opts->setup_hook_script;
      command.push_back(setup_hook);
    }

  if (!opts->keyid.empty())
    {
      std::string keyid("--keyid=");
      keyid += opts->keyid;
      command.push_back(keyid);
    }

  if (!opts->maintainer.empty())
    {
      std::string maintainer("--maintainer=");
      maintainer += opts->maintainer;
      command.push_back(maintainer);
    }

  if (!opts->uploader.empty())
    {
      std::string uploader("--uploader=");
      uploader += opts->uploader;
      command.push_back(uploader);
    }

  for (const auto& bd : opts->build_depends)
    {
      std::string dep("--add-depends=");
      dep += bd;
      command.push_back(dep);
    }

  for (const auto& bc : opts->build_conflicts)
    {
      std::string dep("--add-conflicts=");
      dep += bc;
      command.push_back(dep);
    }

  for (const auto& bdi : opts->build_depends_indep)
    {
      std::string dep("--add-depends=");
      dep += bdi;
      command.push_back(dep);
    }

  for (const auto& bci : opts->build_conflicts_indep)
    {
      std::string dep("--add-conflicts=");
      dep += bci;
      command.push_back(dep);
    }

  if (!opts->depends_algorithm.empty())
    {
      std::string algo("check-depends-algorithm=");
      algo += opts->depends_algorithm;
      command.push_back(algo);
    }

  if (opts->gcc_snapshot)
    command.push_back("--use-snapshot");

  std::copy(opts->packages.begin(), opts->packages.end(),
            std::back_inserter(command));

  sbuild::log_debug(sbuild::DEBUG_NOTICE)
    << "command="
    << sbuild::string_list_to_string(command, ", ")
    << std::endl;

  exec(command[0], command, env);

  // This should never be reached.
  exit(EXIT_FAILURE);
}

int
main::run_impl ()
{
  if (this->opts->action == options::ACTION_HELP)
    action_help(std::cerr);
  else if (this->opts->action == options::ACTION_VERSION)
    action_version(std::cerr);
  else if (this->opts->action == options::ACTION_BUILD)
    action_build();
  else
    assert(0); // Invalid action.

  return EXIT_SUCCESS;
}
