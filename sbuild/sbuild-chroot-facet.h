/* Copyright © 2005-2009  Roger Leigh <rleigh@debian.org>
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

#ifndef SBUILD_CHROOT_FACET_H
#define SBUILD_CHROOT_FACET_H

#include <sbuild/sbuild-environment.h>
#include <sbuild/sbuild-format-detail.h>
#include <sbuild/sbuild-keyfile.h>
#include <sbuild/sbuild-types.h>
#include <sbuild/sbuild-chroot.h>

#include <string>

namespace sbuild
{

  /**
   * Common chroot data.  This class contains all of the metadata
   * associated with a single chroot, for all chroot types.  This is
   * the in-core representation of a chroot definition in the
   * configuration file, and may be initialised directly from an open
   * keyfile.
   */
  class chroot_facet
  {
  protected:
    /// The constructor.
    chroot_facet () {};

  public:
    /// The destructor.
    virtual ~chroot_facet () {};

    /**
     * Set environment.  Set the environment that the setup scripts
     * will see during execution.
     *
     * @param chroot the chroot to use.
     * @param env the environment to set.
     */
    virtual void
    setup_env (chroot const& chroot,
	       environment&  env) const = 0;

    /**
     * Get the session flags of the chroot.  These determine how the
     * Session controlling the chroot will operate.
     *
     * @param chroot the chroot to use.
     * @returns the session flags.
     */
    virtual chroot::session_flags
    get_session_flags (chroot const& chroot) const = 0;

    /**
     * Get detailed information about the chroot for output.
     *
     * @param chroot the chroot to use.
     * @param detail the details to output to.
     */
    virtual void
    get_details (chroot const&  chroot,
		 format_detail& detail) const = 0;

    /**
     * Copy the chroot properties into a keyfile.  The keyfile group
     * with the name of the chroot will be set; if it already exists,
     * it will be removed before setting it.
     *
     * @param chroot the chroot to use.
     * @param keyfile the keyfile to use.
     */
    virtual void
    get_keyfile (chroot const& chroot,
		 keyfile&      keyfile) const = 0;

    /**
     * Set the chroot properties from a keyfile.  The chroot name must
     * have previously been set, so that the correct keyfile group may
     * be determined.
     *
     * @param chroot the chroot to use.
     * @param keyfile the keyfile to get the properties from.
     * @param used_keys a list of the keys used will be set.
     */
    virtual void
    set_keyfile (chroot&        chroot,
		 keyfile const& keyfile,
		 string_list&   used_keys) = 0;
  };

}

#endif /* SBUILD_CHROOT_FACET_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
