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

#ifdef SBUILD_FEATURE_UNION
    // Implementation of the chroot_source interface
    virtual string_list const&
    get_source_users () const;

    virtual void
    set_source_users (string_list const& users);

    virtual string_list const&
    get_source_groups () const;

    virtual void
    set_source_groups (string_list const& groups);

    virtual string_list const&
    get_source_root_users () const;

    virtual void
    set_source_root_users (string_list const& users);

    virtual string_list const&
    get_source_root_groups () const;

    virtual void
    set_source_root_groups (string_list const& groups);

    virtual bool
    get_source () const;

    virtual void
    set_source (bool source);
#endif // SBUILD_FEATURE_UNION

#ifdef SBUILD_FEATURE_UNION
  private:
    /// Is the chroot source or clone?
    bool          is_source;
    /// Users allowed to access the source chroot.
    string_list   source_users;
    /// Groups allowed to access the source chroot.
    string_list   source_groups;
    /// Users allowed to access the source chroot as root.
    string_list   source_root_users;
    /// Groups allowed to access the source chroot as root.
    string_list   source_root_groups;
#endif // SBUILD_FEATURE_UNION
  };

}

#endif /* SBUILD_CHROOT_BLOCK_DEVICE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
