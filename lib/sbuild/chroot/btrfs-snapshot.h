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

#ifndef SBUILD_CHROOT_BTRFS_SNAPSHOT_H
#define SBUILD_CHROOT_BTRFS_SNAPSHOT_H

#include <sbuild/chroot/chroot.h>

namespace sbuild
{
  namespace chroot
  {
    /**
     * A chroot stored on an BTRFS logical volume (LV).
     *
     * A snapshot LV will be created and mounted on demand.
     */
    class btrfs_snapshot : public chroot
    {
    protected:
      /// The constructor.
      btrfs_snapshot ();

      /// The copy constructor.
      btrfs_snapshot (const btrfs_snapshot& rhs);

      friend class chroot;

    public:
      /// The destructor.
      virtual ~btrfs_snapshot ();

      virtual chroot::ptr
      clone () const;

      virtual chroot::ptr
      clone_session (std::string const& session_id,
                     std::string const& alias,
                     std::string const& user,
                     bool               root) const;

      virtual chroot::ptr
      clone_source () const;
    };

  }
}

#endif /* SBUILD_CHROOT_BTRFS_SNAPSHOT_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
