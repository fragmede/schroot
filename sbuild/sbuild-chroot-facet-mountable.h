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

#ifndef SBUILD_CHROOT_FACET_MOUNTABLE_H
#define SBUILD_CHROOT_FACET_MOUNTABLE_H

#include <sbuild/sbuild-chroot-facet.h>

namespace sbuild
{

  /**
   * Chroot support for kernel personalities (execution domains).
   */
  class chroot_facet_mountable : public chroot_facet
  {
  public:
    /// A shared_ptr to a chroot facet object.
    typedef std::tr1::shared_ptr<chroot_facet_mountable> ptr;

    /// A shared_ptr to a const chroot facet object.
    typedef std::tr1::shared_ptr<const chroot_facet_mountable> const_ptr;

  private:
    /// The constructor.
    chroot_facet_mountable ();

  public:
    /// The destructor.
    virtual ~chroot_facet_mountable ();

    /**
     * Create a chroot facet.
     *
     * @returns a shared_ptr to the new chroot facet.
     */
    static ptr
    create ();

    virtual chroot_facet::ptr
    clone () const;

    virtual std::string const&
    get_name () const;

    /**
     * Get the device path of the chroot block device to mount.
     *
     * @returns the mount device.
     */
    virtual std::string const&
    get_mount_device () const;

    /**
     * Set the device path of the chroot block device to mount.
     *
     * @param mount_device the mount device.
     */
    virtual void
    set_mount_device (std::string const& mount_device);

    /**
     * Get the filesystem mount options of the chroot block device.
     *
     * @returns the mount options.
     */
    virtual std::string const&
    get_mount_options () const;

    /**
     * Set the filesystem mount options of the chroot block device.
     *
     * @param mount_options the mount options.
     */
    virtual void
    set_mount_options (std::string const& mount_options);

    /**
     * Get the location.  This is a path to the chroot directory
     * inside the device (absolute path from the device root).
     *
     * @returns the location.
     */
    virtual std::string const&
    get_location () const;

    /**
     * Set the location.  This is a path to the chroot directory
     * inside the device (absolute path from the device root).
     *
     * @param location the location.
     */
    virtual void
    set_location (std::string const& location);

    virtual void
    setup_env (chroot const& chroot,
	       environment&  env) const;

    virtual chroot::session_flags
    get_session_flags (chroot const& chroot) const;

    virtual void
    get_details (chroot const&  chroot,
		 format_detail& detail) const;

    virtual void
    get_keyfile (chroot const& chroot,
		 keyfile&      keyfile) const;

    virtual void
    set_keyfile (chroot&        chroot,
		 keyfile const& keyfile,
		 string_list&   used_keys);

  private:
    /// The device to mount.
    std::string mount_device;
    /// The options to mount the device with.
    std::string mount_options;
    /// Location inside the mount location root.
    std::string location;
  };

}

#endif /* SBUILD_CHROOT_FACET_MOUNTABLE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
