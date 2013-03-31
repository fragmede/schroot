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

#ifndef CSBUILD_OPTIONS_H
#define CSBUILD_OPTIONS_H

#include <schroot-base/schroot-base-options.h>

#include <sbuild/sbuild-types.h>

#include <string>

namespace csbuild
{

  /**
   * csbuild command-line options.
   */
  class options : public schroot_base::options
  {
  public:
    /// A shared_ptr to an options object.
    typedef std::shared_ptr<options> ptr;

    /// Begin, run and end a session.
    static const action_type ACTION_BUILD;

    /// The constructor.
    options ();

    /// The destructor.
    virtual ~options ();

    /// Packages to build
    sbuild::string_list packages;

    /// No logging.
    bool nolog;

    /// Batch mode.
    bool batch_mode;

    /// dpkg-buildpackage options.
    sbuild::string_list deb_build_options;

    // dpkg-buildpackage options (space-separated).
    std::string deb_build_options_string;

    /// Distribution.
    std::string distribution;

    /// Archive.
    std::string archive;

    /// Architecture.
    std::string build_arch;

    /// Build architecture all packages.
    bool build_arch_all;

    /// Build source package.
    bool build_source;

    /// Force original source.  dpkg-buildpackage -sa.
    bool force_orig_source;

    /// Make a binary non-maintainer upload.
    bool bin_nmu;

    /// binNMU changelog entry.
    std::string bin_nmu_changelog;

    /// binNMU version.
    unsigned int bin_nmu_version;

    /// Suffix to append to version.
    std::string append_version;


    /// Update APT in chroot?
    bool apt_update;

    /// Build chroot.
    std::string chroot;

    /// Purge modes.
    enum purge_mode
      {
        PURGE_ALWAYS,  ///< Always purge build.
        PURGE_SUCCESS, ///< Purge build on success only.
        PURGE_NEVER    ///< Never purge build.
      };

    /// Purge build directory.
    std::string purge_string;

    /// Purge build directory.
    purge_mode purge;

    /// Purge build dependencies.
    std::string purge_deps_string;

    /// Purge build dependencies.
    purge_mode purge_deps;

    /// Chroot setup hook script.
    std::string setup_hook_script;

    /// Key ID.
    std::string keyid;

    /// Maintainer.
    std::string maintainer;

    /// Uploader.
    std::string uploader;

    /// Manual build dependencies.
    sbuild::string_list build_depends;

    /// Manual build conflicts.
    sbuild::string_list build_conflicts;

    /// Manual architecture-independent build dependencies.
    sbuild::string_list build_depends_indep;

    /// Manual architecture-independent build conflicts.
    sbuild::string_list build_conflicts_indep;

    // Build dependency checking algorithm.
    std::string depends_algorithm;

    /// Use the current GCC snapshot to build.
    bool gcc_snapshot;

  protected:
    virtual void
    add_options ();

    virtual void
    add_option_groups ();

    virtual void
    check_options ();

    /// Build options group.
    boost::program_options::options_description build;

    /// Package version group.
    boost::program_options::options_description version;

    /// Chroot group.
    boost::program_options::options_description chrootopt;

    /// User options group.
    boost::program_options::options_description user;

    /// Manual depdenency options group.
    boost::program_options::options_description depends;

    /// Special options group.
    boost::program_options::options_description special;
  };

}

#endif /* CSBUILD_OPTIONS_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
