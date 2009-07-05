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

#ifndef SBUILD_CHROOT_BLOCK_DEVICE_H
#define SBUILD_CHROOT_BLOCK_DEVICE_H

#include <sbuild/sbuild-config.h>
#include <sbuild/sbuild-chroot-block-device-base.h>
#ifdef SBUILD_FEATURE_UNION
#include <sbuild/sbuild-chroot-union.h>
#include <sbuild/sbuild-chroot-lvm-snapshot.h>
#endif // SBUILD_FEATURE_UNION

namespace sbuild
{

  /**
   * A chroot stored on an unmounted block device.
   *
   * The device will be mounted on demand.
   */
  class chroot_block_device : public chroot_block_device_base
#ifdef SBUILD_FEATURE_UNION
			    , public chroot_union
#endif // SBUILD_FEATURE_UNION
  {
  public:
    /// Exception type.
    typedef chroot::error error;

  protected:
    /// The constructor.
    chroot_block_device ();

    /// The copy constructor.
    chroot_block_device (const chroot_block_device& rhs);

    /// The copy constructor.
    chroot_block_device (const chroot_lvm_snapshot& rhs);

    friend class chroot;
    friend class chroot_lvm_snapshot;

  public:
    /// The destructor.
    virtual ~chroot_block_device ();

    virtual chroot::ptr
    clone () const;

    virtual void
    setup_env (environment& env);

    virtual session_flags
    get_session_flags () const;

#ifdef SBUILD_FEATURE_UNION
    virtual chroot::ptr
    clone_source () const;
#endif // SBUILD_FEATURE_UNION

    virtual void
    get_details (format_detail& detail) const;

    virtual void
    get_keyfile (keyfile& keyfile) const;

    virtual void
    set_keyfile (keyfile const& keyfile,
		 string_list&   used_keys);
  };

}

#endif /* SBUILD_CHROOT_BLOCK_DEVICE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
