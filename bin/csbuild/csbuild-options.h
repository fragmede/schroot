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
    typedef std::tr1::shared_ptr<options> ptr;

    /// Begin, run and end a session.
    static const action_type ACTION_BUILD;

    /// The constructor.
    options ();

    /// The destructor.
    virtual ~options ();


    /// Batch mode
    bool  batchmode;

    /// No logging
    bool  nolog;

    /// Build architecture all packages
    bool build_arch_all;

    /// Build source package
    bool build_source;

    /// Force original source.  dpkg-buildpackage -sa
    bool force_orig_source;

    /// Distribution
    std::string distribution;

    /// Purge build directory
    std::string purge_string;

    /// Purge modes
    enum purge_mode
      {
	PURGE_ALWAYS,  ///< Always purge build.
	PURGE_SUCCESS, ///< Purge build on success only.
	PURGE_NEVER    ///< Never purge build.
      };

    /// Purge build directory
    purge_mode purge;

    /// Maintainer
    std::string maintainer;

    /// Key ID
    std::string keyid;

    /// Uploader
    std::string uploader;

    /// Forced dependencies
    sbuild::string_list forced_dependencies;

    /// Additional dependencies
    sbuild::string_list additional_dependencies;

    /// Make a binary non-maintainer upload
    bool bin_nmu;

    /// Use the current GCC snapshot to build
    bool gcc_snapshot;

  protected:
    virtual void
    add_options ();

    virtual void
    add_option_groups ();

    virtual void
    check_options ();

    /// User options group.
    boost::program_options::options_description user;

    /// User options group.
    boost::program_options::options_description dependencies;

  };

}

#endif /* CSBUILD_OPTIONS_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
