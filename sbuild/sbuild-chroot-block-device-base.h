/* Copyright © 2005-2008  Roger Leigh <rleigh@debian.org>
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

#ifndef SBUILD_CHROOT_BLOCK_DEVICE_BASE_H
#define SBUILD_CHROOT_BLOCK_DEVICE_BASE_H

#include <sbuild/sbuild-chroot.h>

namespace sbuild
{

  /**
   * A base class for block-device chroots.
   *
   * This class doesn't implement a chroot (get_chroot_type
   * is not implemented).
   *
   * Originally lvm-snapshot inherited from the block-device chroot,
   * but this was changed when union support was introduced.  This
   * design prevents lvm-snapshot offering union based sessions.
   */
  class chroot_block_device_base : public chroot
  {
  protected:
    /// The constructor.
    chroot_block_device_base ();

    /// The copy constructor.
    chroot_block_device_base (const chroot_block_device_base& rhs);

    friend class chroot;

  public:
    /// The destructor.
    virtual ~chroot_block_device_base ();

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

    virtual std::string
    get_path () const;

    virtual void
    setup_env (chroot const& chroot,
	       environment& env) const;

    virtual session_flags
    get_session_flags (chroot const& chroot) const;

  protected:
    virtual void
    get_details (chroot const& chroot,
		 format_detail& detail) const;

    virtual void
    get_used_keys (string_list& used_keys) const;

    virtual void
    get_keyfile (chroot const& chroot,
		 keyfile& keyfile) const;

    virtual void
    set_keyfile (chroot&        chroot,
		 keyfile const& keyfile);

    /// The block device to use.
    std::string device;
  };

}

#endif /* SBUILD_CHROOT_BLOCK_DEVICE_BASE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
