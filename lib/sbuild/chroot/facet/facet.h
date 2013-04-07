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

#ifndef SBUILD_CHROOT_FACET_FACET_H
#define SBUILD_CHROOT_FACET_FACET_H

#include <sbuild/environment.h>
#include <sbuild/format-detail.h>
#include <sbuild/keyfile.h>
#include <sbuild/types.h>
#include <sbuild/chroot/chroot.h>

#include <string>

namespace sbuild
{
  namespace chroot
  {
    namespace facet
    {

  /**
   * Common chroot data.  This class contains all of the metadata
   * associated with a single chroot, for all chroot types.  This is
   * the in-core representation of a chroot definition in the
   * configuration file, and may be initialised directly from an open
   * keyfile.
   */
  class facet
  {
  public:
    /// A shared_ptr to a chroot facet object.
    typedef std::shared_ptr<facet> ptr;

    /// A shared_ptr to a const chroot facet object.
    typedef std::shared_ptr<const facet> const_ptr;

  protected:
    /// The constructor.
    facet(): owner(0) {};

    /**
     * Set containing chroot.
     *
     * @param chroot the chroot containing this facet.
     */
    void
    set_chroot(chroot& chroot)
    {
      this->owner = &chroot;
    }

    friend class ::sbuild::chroot::chroot;

  public:
    /// The destructor.
    virtual ~facet () {};

    /**
     * Copy the chroot facet.  This is a virtual copy constructor.
     *
     * @returns a shared_ptr to the new copy of the chroot facet.
     */
    virtual ptr
    clone () const = 0;

    /**
     * Get the name of the chroot facet.
     *
     * @returns the chroot facet name.
     */
    virtual std::string const&
    get_name () const = 0;

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
     * Get a list of the keys used during keyfile parsing.
     *
     * @returns a list of key names.
     */
    virtual void
    get_used_keys (string_list& used_keys) const = 0;

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
    set_keyfile (chroot&         chroot,
                 keyfile const&  keyfile) = 0;

  protected:
    /// Chroot owning this facet.
    chroot *owner;
  };

}
  }
}

#endif /* SBUILD_CHROOT_FACET_FACET_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
