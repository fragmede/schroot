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

#include <config.h>

#include <sbuild/chroot/chroot.h>
#include <sbuild/chroot/facet/mountable.h>
#include <sbuild/chroot/facet/session.h>
#include <sbuild/chroot/facet/session-clonable.h>
#include <sbuild/chroot/facet/source-clonable.h>
#include <sbuild/chroot/plain.h>
#ifdef SBUILD_FEATURE_BLOCKDEV
#include <sbuild/chroot/facet/block-device-base.h>
#endif
#ifdef SBUILD_FEATURE_LVMSNAP
#include <sbuild/chroot/facet/lvm-snapshot.h>
#endif // SBUILD_FEATURE_LVMSNAP
#ifdef SBUILD_FEATURE_LOOPBACK
#include <sbuild/chroot/facet/loopback.h>
#endif // SBUILD_FEATURE_LOOPBACK
#ifdef SBUILD_FEATURE_BTRFSSNAP
#include <sbuild/chroot/facet/btrfs-snapshot.h>
#endif // SBUILD_FEATURE_BTRFSSNAP
#ifdef SBUILD_FEATURE_UNION
#include <sbuild/chroot/facet/fsunion.h>
#endif // SBUILD_FEATURE_UNION
#include "format-detail.h"

#include <cassert>

#include <boost/format.hpp>

using boost::format;
using std::endl;
using namespace sbuild;

namespace sbuild
{
  namespace chroot
  {
    namespace facet
    {

      session_clonable::session_clonable ():
        facet()
      {
      }

      session_clonable::~session_clonable ()
      {
      }

      session_clonable::ptr
      session_clonable::create ()
      {
        return ptr(new session_clonable());
      }

      facet::ptr
      session_clonable::clone () const
      {
        return ptr(new session_clonable(*this));
      }

      std::string const&
      session_clonable::get_name () const
      {
        static const std::string name("session");

        return name;
      }

      void
      session_clonable::clone_session_setup (chroot const&      parent,
                                             chroot::ptr&       clone,
                                             std::string const& session_id,
                                             std::string const& alias,
                                             std::string const& user,
                                             bool               root) const
      {
        // Disable session cloning.
        clone->remove_facet<session_clonable>();
        // Disable source cloning.
        clone->remove_facet<source_clonable>();
        clone->add_facet(session::create());

        // Disable session, delete aliases.
        session::ptr psess(clone->get_facet<session>());
        assert(psess);

        psess->set_original_name(clone->get_name());
        psess->set_selected_name(alias);
        clone->set_name(session_id);
        assert(clone->get_name() == session_id);
        clone->set_description
          (clone->get_description() + ' ' + _("(session chroot)"));

        string_list empty_list;
        string_list allowed_users;
        if (!user.empty())
          allowed_users.push_back(user);

        if (root)
          {
            clone->set_users(empty_list);
            clone->set_root_users(allowed_users);
          }
        else
          {
            clone->set_users(allowed_users);
            clone->set_root_users(empty_list);
          }
        clone->set_groups(empty_list);
        clone->set_root_groups(empty_list);
        clone->set_aliases(empty_list);

        log_debug(DEBUG_INFO)
          << format("Cloned session %1%")
          % clone->get_name() << endl;

        /* If a chroot mount location has not yet been set, and the
           chroot is not a plain chroot, set a mount location with the
           session id.  Only set for non-plain chroots which run
           setup scripts. */
        {
          std::shared_ptr<plain> plain(std::dynamic_pointer_cast<plain>(clone));

          if (clone->get_mount_location().empty() && !plain)
            {
              log_debug(DEBUG_NOTICE) << "Setting mount location" << endl;
              std::string location(std::string(SCHROOT_MOUNT_DIR) + "/" +
                                   session_id);
              clone->set_mount_location(location);
            }
        }

        log_debug(DEBUG_NOTICE)
          << format("Mount Location: %1%") % clone->get_mount_location()
          << endl;

#ifdef SBUILD_FEATURE_BLOCKDEV
        /* Block devices need the mount device name specifying. */
        /* Note that this will be overridden by LVM snapshot, below, so the
           order here is important. */
        std::shared_ptr<block_device_base> blockdevbase(clone->get_facet<block_device_base>());
        if (blockdevbase)
          {
            mountable::ptr pmnt
              (clone->get_facet<mountable>());
            if (pmnt)
              pmnt->set_mount_device(blockdevbase->get_device());
          }
#endif // SBUILD_FEATURE_BLOCKDEV

#ifdef SBUILD_FEATURE_LOOPBACK
        /* Loopback chroots need the mount device name specifying. */
        loopback::ptr loop(clone->get_facet<loopback>());
        if (loop)
          {
            mountable::ptr pmnt
              (clone->get_facet<mountable>());
            if (pmnt)
              pmnt->set_mount_device(loop->get_filename());
          }
#endif // SBUILD_FEATURE_LOOPBACK

#ifdef SBUILD_FEATURE_LVMSNAP
        /* LVM devices need the snapshot device name specifying. */
        lvm_snapshot::ptr snapshot(clone->get_facet<lvm_snapshot>());
        if (snapshot && !snapshot->get_device().empty())
          {
            std::string device(dirname(snapshot->get_device()));
            device += "/" + clone->get_name();
            snapshot->set_snapshot_device(device);
          }
#endif // SBUILD_FEATURE_LVMSNAP

#ifdef SBUILD_FEATURE_BTRFSSNAP
        /* Btrfs snapshots need the snapshot name specifying. */
        btrfs_snapshot::ptr btrfs_snap(clone->get_facet<btrfs_snapshot>());
        if (btrfs_snap && !btrfs_snap->get_snapshot_directory().empty())
          {
            std::string snapname(btrfs_snap->get_snapshot_directory());
            snapname += "/" + clone->get_name();
            btrfs_snap->set_snapshot_name(snapname);
          }
#endif // SBUILD_FEATURE_BTRFSSNAP

#ifdef SBUILD_FEATURE_UNION
        // If the parent did not have a union facet, then neither should we.
        fsunion::const_ptr pparentuni(parent.get_facet<fsunion>());
        if (!pparentuni)
          clone->remove_facet<fsunion>();

        /* Filesystem unions need the overlay directory specifying. */
        fsunion::ptr puni(clone->get_facet<fsunion>());

        if (puni)
          {
            std::string overlay = puni->get_union_overlay_directory();
            overlay += "/" + clone->get_name();
            puni->set_union_overlay_directory(overlay);

            std::string underlay = puni->get_union_underlay_directory();
            underlay += "/" + clone->get_name();
            puni->set_union_underlay_directory(underlay);
          }
#endif // SBUILD_FEATURE_UNION
      }

      chroot::session_flags
      session_clonable::get_session_flags (chroot const& chroot) const
      {
        return chroot::SESSION_CREATE;
      }

    }
  }
}
