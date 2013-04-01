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

#include "chroot-lvm-snapshot.h"
#include "chroot-block-device.h"
#include "chroot-facet-session.h"
#include "chroot-facet-session-clonable.h"
#include "chroot-facet-source-clonable.h"
#include "chroot-facet-mountable.h"
#include "format-detail.h"

#include <cassert>
#include <cerrno>

#include <boost/format.hpp>

using std::endl;
using boost::format;
using namespace sbuild;

chroot_lvm_snapshot::chroot_lvm_snapshot ():
  chroot_block_device_base(),
  snapshot_device(),
  snapshot_options()
{
  add_facet(chroot_facet_source_clonable::create());
}

chroot_lvm_snapshot::chroot_lvm_snapshot (const chroot_lvm_snapshot& rhs):
  chroot_block_device_base(rhs),
  snapshot_device(rhs.snapshot_device),
  snapshot_options(rhs.snapshot_options)
{
}

chroot_lvm_snapshot::~chroot_lvm_snapshot ()
{
}

sbuild::chroot::ptr
chroot_lvm_snapshot::clone () const
{
  return ptr(new chroot_lvm_snapshot(*this));
}

sbuild::chroot::ptr
chroot_lvm_snapshot::clone_session (std::string const& session_id,
                                    std::string const& alias,
                                    std::string const& user,
                                    bool               root) const
{
  chroot_facet_session_clonable::const_ptr psess
    (get_facet<chroot_facet_session_clonable>());
  assert(psess);

  ptr session(new chroot_lvm_snapshot(*this));
  psess->clone_session_setup(*this, session, session_id, alias, user, root);

  return session;
}

sbuild::chroot::ptr
chroot_lvm_snapshot::clone_source () const
{
  ptr clone(new chroot_block_device(*this));

  chroot_facet_source_clonable::const_ptr psrc
    (get_facet<chroot_facet_source_clonable>());
  assert(psrc);

  psrc->clone_source_setup(*this, clone);

  return clone;
}

std::string const&
chroot_lvm_snapshot::get_snapshot_device () const
{
  return this->snapshot_device;
}

void
chroot_lvm_snapshot::set_snapshot_device (std::string const& snapshot_device)
{
  if (!is_absname(snapshot_device))
    throw error(snapshot_device, DEVICE_ABS);

  this->snapshot_device = snapshot_device;

  chroot_facet_mountable::ptr pmnt
    (get_facet<chroot_facet_mountable>());
  if (pmnt)
    pmnt->set_mount_device(this->snapshot_device);
}

std::string const&
chroot_lvm_snapshot::get_snapshot_options () const
{
  return this->snapshot_options;
}

void
chroot_lvm_snapshot::set_snapshot_options (std::string const& snapshot_options)
{
  this->snapshot_options = snapshot_options;
}

std::string const&
chroot_lvm_snapshot::get_chroot_type () const
{
  static const std::string type("lvm-snapshot");

  return type;
}

void
chroot_lvm_snapshot::setup_env (chroot const& chroot,
                                environment&  env) const
{
  chroot_block_device_base::setup_env(chroot, env);

  env.add("CHROOT_LVM_SNAPSHOT_NAME", sbuild::basename(get_snapshot_device()));
  env.add("CHROOT_LVM_SNAPSHOT_DEVICE", get_snapshot_device());
  env.add("CHROOT_LVM_SNAPSHOT_OPTIONS", get_snapshot_options());
}

void
chroot_lvm_snapshot::setup_lock (chroot::setup_type type,
                                 bool               lock,
                                 int                status)
{
  std::string device;

  /* Lock is removed by setup script on setup stop.  Unlocking here
     would fail: the LVM snapshot device no longer exists. */
  if (!(type == SETUP_STOP && lock == false))
    {
      if (type == SETUP_START)
        device = get_device();
      else
        device = get_snapshot_device();

      if (device.empty())
        throw error(CHROOT_DEVICE);

      try
        {
          stat file_status(device);
          if (!file_status.is_block())
            {
              throw error(get_device(), DEVICE_NOTBLOCK);
            }
        }
      catch (sbuild::stat::error const& e) // Failed to stat
        {
          // Don't throw if stopping a session and the device stat
          // failed.  This is because the setup scripts shouldn't fail
          // to be run if the LVM snapshot no longer exists, which
          // would prevent the session from being ended.
          if (type != SETUP_STOP)
            throw;
        }
    }

  /* Create or unlink session information. */
  if ((type == SETUP_START && lock == true) ||
      (type == SETUP_STOP && lock == false && status == 0))
    {
      bool start = (type == SETUP_START);
      setup_session_info(start);
    }
}

sbuild::chroot::session_flags
chroot_lvm_snapshot::get_session_flags (chroot const& chroot) const
{
  session_flags flags = SESSION_NOFLAGS;

  if (get_facet<chroot_facet_session>())
    flags = flags | SESSION_PURGE;

  return flags;
}

void
chroot_lvm_snapshot::get_details (chroot const& chroot,
                                  format_detail& detail) const
{
  chroot_block_device_base::get_details(chroot, detail);

  if (!this->snapshot_device.empty())
    detail.add(_("LVM Snapshot Device"), get_snapshot_device());
  if (!this->snapshot_options.empty())
    detail.add(_("LVM Snapshot Options"), get_snapshot_options());
}

void
chroot_lvm_snapshot::get_used_keys (string_list& used_keys) const
{
  chroot_block_device_base::get_used_keys(used_keys);

  used_keys.push_back("lvm-snapshot-device");
  used_keys.push_back("lvm-snapshot-options");
}

void
chroot_lvm_snapshot::get_keyfile (chroot const& chroot,
                                  keyfile& keyfile) const
{
  chroot_block_device_base::get_keyfile(chroot, keyfile);

  bool session = static_cast<bool>(get_facet<chroot_facet_session>());

  if (session)
    keyfile::set_object_value(*this,
                              &chroot_lvm_snapshot::get_snapshot_device,
                              keyfile, get_name(),
                              "lvm-snapshot-device");

  if (!session)
    keyfile::set_object_value(*this,
                              &chroot_lvm_snapshot::get_snapshot_options,
                              keyfile, get_name(),
                              "lvm-snapshot-options");
}

void
chroot_lvm_snapshot::set_keyfile (chroot&        chroot,
                                  keyfile const& keyfile)
{
  chroot_block_device_base::set_keyfile(chroot, keyfile);

  bool session = static_cast<bool>(get_facet<chroot_facet_session>());

  keyfile::get_object_value(*this, &chroot_lvm_snapshot::set_snapshot_device,
                            keyfile, get_name(), "lvm-snapshot-device",
                            session ?
                            keyfile::PRIORITY_REQUIRED :
                            keyfile::PRIORITY_DISALLOWED);

  keyfile::get_object_value(*this, &chroot_lvm_snapshot::set_snapshot_options,
                            keyfile, get_name(), "lvm-snapshot-options",
                            session ?
                            keyfile::PRIORITY_DEPRECATED :
                            keyfile::PRIORITY_REQUIRED); // Only needed for creating snapshot, not using snapshot
}
