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

#ifndef SBUILD_CHROOT_LVM_SNAPSHOT_H
#define SBUILD_CHROOT_LVM_SNAPSHOT_H

#include <sbuild/sbuild-chroot-block-device-base.h>

namespace sbuild
{

  /**
   * A chroot stored on an LVM logical volume (LV).
   *
   * A snapshot LV will be created and mounted on demand.
   */
  class chroot_lvm_snapshot : public chroot_block_device_base
  {
  protected:
    /// The constructor.
    chroot_lvm_snapshot ();

    /// The copy constructor.
    chroot_lvm_snapshot (const chroot_lvm_snapshot& rhs);

    friend class chroot;

  public:
    /// The destructor.
    virtual ~chroot_lvm_snapshot ();

    virtual chroot::ptr
    clone () const;

    virtual chroot::ptr
    clone_session (std::string const& session_id) const;

    virtual chroot::ptr
    clone_source () const;

    /**
     * Get the logical volume snapshot device name.  This is used by
     * lvcreate.
     *
     * @returns the device name.
     */
    std::string const&
    get_snapshot_device () const;

    /**
     * Set the logical volume snapshot device name.  This is used by
     * lvcreate.
     *
     * @param snapshot_device the device name.
     */
    void
    set_snapshot_device (std::string const& snapshot_device);

    /**
     * Get the logical volume snapshot options.  These are used by
     * lvcreate.
     *
     * @returns the options.
     */
    std::string const&
    get_snapshot_options () const;

    /**
     * Set the logical volume snapshot options.  These are used by
     * lvcreate.
     *
     * @param snapshot_options the options.
     */
    void
    set_snapshot_options (std::string const& snapshot_options);

    virtual std::string const&
    get_chroot_type () const;

    virtual void
    setup_env (chroot const& chroot,
	       environment&  env) const;

    virtual session_flags
    get_session_flags (chroot const& chroot) const;

  protected:
    virtual void
    setup_lock (chroot::setup_type type,
		bool               lock,
		int                status);

    virtual void
    get_details (chroot const& chroot,
		 format_detail& detail) const;

    virtual void
    get_keyfile (chroot const& chroot,
		 keyfile& keyfile) const;

    virtual void
    set_keyfile (chroot&        chroot,
		 keyfile const& keyfile,
		 string_list&   used_keys);

  private:
    /// LVM snapshot device name for lvcreate.
    std::string snapshot_device;
    /// LVM snapshot options for lvcreate.
    std::string snapshot_options;
  };

}

#endif /* SBUILD_CHROOT_LVM_SNAPSHOT_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
