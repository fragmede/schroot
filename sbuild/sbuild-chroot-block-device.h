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

#ifndef SBUILD_CHROOT_BLOCK_DEVICE_H
#define SBUILD_CHROOT_BLOCK_DEVICE_H

#include <sbuild/sbuild-chroot.h>

namespace sbuild
{

  /**
   * A chroot stored on an unmounted block device.
   *
   * The device will be mounted on demand.
   */
  class chroot_block_device : virtual public chroot
  {
  protected:
    /// The constructor.
    chroot_block_device ();

    friend class chroot;

  public:
    /// The destructor.
    virtual ~chroot_block_device ();

    virtual chroot::ptr
    clone () const;

    /**
     * Get the block device of the chroot.
     *
     * @returns the device.
     */
    std::string const&
    get_device () const;

    /**
     * Set the block device of the chroot.  This is the "source" device.
     * It may be the case that the real device is different (for
     * example, an LVM snapshot PV), but by default will be the device
     * to mount.
     *
     * @param device the device.
     */
    void
    set_device (std::string const& device);

    virtual std::string const&
    get_mount_device () const;

    /**
     * Get the filesystem mount_options of the chroot block device.
     *
     * @returns the mount options.
     */
    std::string const&
    get_mount_options () const;

    /**
     * Set the filesystem mount_options of the chroot block device.
     *
     * @param mount_options the mount options.
     */
    void
    set_mount_options (std::string const& mount_options);

    /**
     * Get the location.  This is a path to the chroot directory
     * inside the LV (absolute path from the LV root).
     *
     * @returns the location.
     */
    virtual std::string const&
    get_location () const;

    /**
     * Set the location.  This is a path to the chroot directory
     * inside the LV (absolute path from the LV root).
     *
     * @param location the location.
     */
    virtual void
    set_location (std::string const& location);

    virtual std::string const&
    get_chroot_type () const;

    virtual void
    setup_env (environment& env);

    virtual session_flags
    get_session_flags () const;

  protected:
    virtual void
    setup_lock (chroot::setup_type type,
		bool               lock,
		int                status);

    virtual void
    get_details (format_detail& detail) const;

    virtual void
    get_keyfile (keyfile& keyfile) const;

    virtual void
    set_keyfile (keyfile const& keyfile);

  private:
    /// The block device to use.
    std::string device;
    /// The options to mount the device with.
    std::string mount_options;
  };

}

#endif /* SBUILD_CHROOT_BLOCK_DEVICE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
