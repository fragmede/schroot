/* Copyright © 2005-2006  Roger Leigh <rleigh@debian.org>
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

#ifndef SBUILD_CHROOT_BLOCK_DEVICE_H
#define SBUILD_CHROOT_BLOCK_DEVICE_H

#include "sbuild-chroot.h"

namespace sbuild
{

  /**
   * A chroot stored on an unmounted block device.  The device will be
   * mounted on demand.
   */
  class ChrootBlockDevice : public Chroot
  {
  protected:
    /// The constructor.
    ChrootBlockDevice();

    /**
     * The constructor.  Initialise from an open keyfile.
     *
     * @param keyfile the configuration file
     * @param group the keyfile group (chroot name)
     */
    ChrootBlockDevice (keyfile const&     keyfile,
		       std::string const& group);

    friend class Chroot;

  public:
    /// The destructor.
    virtual ~ChrootBlockDevice();

    virtual Chroot::chroot_ptr
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

    virtual std::string const&
    get_chroot_type () const;

    virtual void
    setup_env (environment& env);

    virtual void
    setup_lock (SetupType type,
		bool      lock);

    virtual SessionFlags
    get_session_flags () const;

  protected:
    virtual void
    print_details (std::ostream& stream) const;

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
