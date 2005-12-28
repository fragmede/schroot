/* sbuild-chroot-lvm-snapshot - sbuild chroot lvm snapshot object
 *
 * Copyright © 2005  Roger Leigh <rleigh@debian.org>
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

#ifndef SBUILD_CHROOT_LVM_SNAPSHOT_H
#define SBUILD_CHROOT_LVM_SNAPSHOT_H

#include "sbuild-chroot-block-device.h"

namespace sbuild
{

  /**
   * A chroot stored on an LVM logical volume (LV).  A snapshot LV
   * will be created and mounted on demand.
   */
  class ChrootLvmSnapshot : public ChrootBlockDevice
  {
  public:
    /// The constructor.
    ChrootLvmSnapshot();

    /**
     * The constructor.  Initialise from an open keyfile.
     *
     * @param keyfile the configuration file
     * @param group the keyfile group (chroot name)
     */
    ChrootLvmSnapshot (const keyfile&      keyfile,
		       const std::string&  group);

    /// The destructor.
    virtual ~ChrootLvmSnapshot();

    virtual Chroot *
    clone () const;

    /**
     * Get the logical volume snapshot device name.  This is used by
     * lvcreate.
     *
     * @returns the device name.
     */
    const std::string&
    get_snapshot_device () const;

    /**
     * Set the logical volume snapshot device name.  This is used by
     * lvcreate.
     *
     * @param snapshot_device the device name.
     */
    void
    set_snapshot_device (const std::string& snapshot_device);

    virtual const std::string&
    get_mount_device () const;

    /**
     * Get the logical volume snapshot options.  These are used by
     * lvcreate.
     *
     * @returns the options.
     */
    const std::string&
    get_snapshot_options () const;

    /**
     * Set the logical volume snapshot options.  These are used by
     * lvcreate.
     *
     * @param snapshot_options the options.
     */
    void
    set_snapshot_options (const std::string& snapshot_options);

    virtual const std::string&
    get_chroot_type () const;

    virtual void
    setup_env (env_list& env);

    virtual void
    setup_lock (SetupType type,
		bool      lock);

    virtual SessionFlags
    get_session_flags () const;

    virtual void
    print_details (std::ostream& stream) const;

    virtual void
    print_config (std::ostream& stream) const;

  private:
    /**
     * Read chroot configuration from a keyfile.
     *
     * @param keyfile the configuration file
     * @param group the keyfile group (chroot name)
     */
    void
    read_keyfile (const keyfile&      keyfile,
		  const std::string&  group);

    /**
     * Set up persistent session information.
     *
     * @param start true if startion, or false if ending a session.
     */
    void
    setup_session_info (bool start);

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
