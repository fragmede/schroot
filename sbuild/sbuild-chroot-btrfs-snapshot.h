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

#ifndef SBUILD_CHROOT_BTRFS_SNAPSHOT_H
#define SBUILD_CHROOT_BTRFS_SNAPSHOT_H

#include <sbuild/sbuild-chroot.h>

namespace sbuild
{

  /**
   * A chroot stored on an BTRFS logical volume (LV).
   *
   * A snapshot LV will be created and mounted on demand.
   */
  class chroot_btrfs_snapshot : public chroot
  {
  protected:
    /// The constructor.
    chroot_btrfs_snapshot ();

    /// The copy constructor.
    chroot_btrfs_snapshot (const chroot_btrfs_snapshot& rhs);

    friend class chroot;

  public:
    /// The destructor.
    virtual ~chroot_btrfs_snapshot ();

    virtual chroot::ptr
    clone () const;

    virtual chroot::ptr
    clone_session (std::string const& session_id,
		   std::string const& user,
		   bool               root) const;

    virtual chroot::ptr
    clone_source () const;

    /**
     * Get the source subvolume path.  This is used by "btrfs
     * subvolume snapshot".
     *
     * @returns the source subvolume.
     */
    std::string const&
    get_source_subvolume () const;

    /**
     * Set the source subvolume path.  This is used by "btrfs
     * subvolume snapshot".
     *
     * @param source_subvolume the source subvolume.
     */
    void
    set_source_subvolume (std::string const& source_subvolume);

    /**
     * Get the snapshot path.  This is used by "btrfs subvolume
     * snapshot".
     *
     * @returns the path.
     */
    std::string const&
    get_snapshot_path () const;

    /**
     * Set the snapshot path.  This is used by "btrfs subvolume
     * snapshot".
     *
     * @param snapshot_path the snapshot path.
     */
    void
    set_snapshot_path (std::string const& snapshot_path);

    virtual std::string const&
    get_chroot_type () const;

    virtual std::string
    get_path () const;

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
    /// Btrfs source subvolume
    std::string source_subvolume;
    /// Btrfs snapshot path
    std::string snapshot_path;
  };

}

#endif /* SBUILD_CHROOT_BTRFS_SNAPSHOT_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
