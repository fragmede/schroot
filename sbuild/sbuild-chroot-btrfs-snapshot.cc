/* Copyright © 2005-2009  Roger Leigh <rleigh@debian.org>
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

#include "sbuild-chroot-btrfs-snapshot.h"
#include "sbuild-chroot-directory.h"
#include "sbuild-chroot-facet-session-clonable.h"
#include "sbuild-chroot-facet-source-clonable.h"
#include "sbuild-format-detail.h"
#include "sbuild-lock.h"

#include <cassert>
#include <cerrno>

#include <boost/format.hpp>

using std::endl;
using boost::format;
using namespace sbuild;

chroot_btrfs_snapshot::chroot_btrfs_snapshot ():
  chroot(),
  source_subvolume(),
  snapshot_path()
{
  add_facet(sbuild::chroot_facet_source_clonable::create());
}

chroot_btrfs_snapshot::chroot_btrfs_snapshot (const chroot_btrfs_snapshot& rhs):
  chroot(rhs),
  source_subvolume(rhs.source_subvolume),
  snapshot_path(rhs.snapshot_path)
{
}

chroot_btrfs_snapshot::~chroot_btrfs_snapshot ()
{
}

sbuild::chroot::ptr
chroot_btrfs_snapshot::clone () const
{
  return ptr(new chroot_btrfs_snapshot(*this));
}

sbuild::chroot::ptr
chroot_btrfs_snapshot::clone_session (std::string const& session_id,
				      std::string const& user,
				      bool               root) const
{
  chroot_facet_session_clonable::const_ptr psess
    (get_facet<chroot_facet_session_clonable>());
  assert(psess);

  ptr session(new chroot_btrfs_snapshot(*this));
  psess->clone_session_setup(session, session_id, user, root);

  return session;
}

sbuild::chroot::ptr
chroot_btrfs_snapshot::clone_source () const
{
  ptr clone(new chroot_directory(*this));

  chroot_facet_source_clonable::const_ptr psrc
    (get_facet<chroot_facet_source_clonable>());
  assert(psrc);

  psrc->clone_source_setup(clone);

  return clone;
}

std::string const&
chroot_btrfs_snapshot::get_source_subvolume () const
{
  return this->source_subvolume;
}

void
chroot_btrfs_snapshot::set_source_subvolume (std::string const& source_subvolume)
{
  if (!is_absname(source_subvolume))
    throw error(source_subvolume, DIRECTORY_ABS);

  this->source_subvolume = source_subvolume;
}

std::string const&
chroot_btrfs_snapshot::get_snapshot_path () const
{
  return this->snapshot_path;
}

void
chroot_btrfs_snapshot::set_snapshot_path (std::string const& snapshot_path)
{
  if (!is_absname(snapshot_path))
    throw error(source_subvolume, DIRECTORY_ABS);

  this->snapshot_path = snapshot_path;
}

std::string const&
chroot_btrfs_snapshot::get_chroot_type () const
{
  static const std::string type("btrfs-snapshot");

  return type;
}

std::string
chroot_btrfs_snapshot::get_path () const
{
  return get_mount_location();
}

void
chroot_btrfs_snapshot::setup_env (chroot const& chroot,
				  environment&  env) const
{
  chroot::setup_env(chroot, env);

  env.add("CHROOT_BTRFS_SOURCE_SUBVOLUME", get_source_subvolume());
  env.add("CHROOT_BTRFS_SNAPSHOT_PATH", get_snapshot_path());
}

void
chroot_btrfs_snapshot::setup_lock (chroot::setup_type type,
				   bool               lock,
				   int                status)
{
  /* Create or unlink session information. */
  if ((type == SETUP_START && lock == true) ||
      (type == SETUP_STOP && lock == false && status == 0))
    {
      bool start = (type == SETUP_START);
      setup_session_info(start);
    }
}

sbuild::chroot::session_flags
chroot_btrfs_snapshot::get_session_flags (chroot const& chroot) const
{
  session_flags flags = SESSION_NOFLAGS;

  if (get_active())
    flags = flags | SESSION_PURGE;

  return flags;
}

void
chroot_btrfs_snapshot::get_details (chroot const& chroot,
				    format_detail& detail) const
{
  chroot::get_details(chroot, detail);

  if (!this->source_subvolume.empty())
    detail.add(_("Btrfs Source Subvolume"), get_source_subvolume());
  if (!this->snapshot_path.empty())
    detail.add(_("Btrfs Snapshot Path"), get_snapshot_path());
}

void
chroot_btrfs_snapshot::get_keyfile (chroot const& chroot,
				    keyfile& keyfile) const
{
  chroot::get_keyfile(chroot, keyfile);

  if (!get_active())
    keyfile::set_object_value(*this,
			      &chroot_btrfs_snapshot::get_source_subvolume,
			      keyfile, get_keyfile_name(),
			      "btrfs-source-subvolume");

  keyfile::set_object_value(*this,
			    &chroot_btrfs_snapshot::get_snapshot_path,
			    keyfile, get_keyfile_name(),
			    "btrfs-snapshot-path");
}

void
chroot_btrfs_snapshot::set_keyfile (chroot&        chroot,
				    keyfile const& keyfile,
				    string_list&   used_keys)
{
  chroot::set_keyfile(chroot, keyfile, used_keys);

  keyfile::get_object_value(*this, &chroot_btrfs_snapshot::set_source_subvolume,
			    keyfile, get_keyfile_name(), "btrfs-source-subvolume",
			    get_active() ?
			    keyfile::PRIORITY_DISALLOWED :
			    keyfile::PRIORITY_REQUIRED
			    ); // Only needed for creating snapshot, not using snapshot
  used_keys.push_back("btrfs-source-subvolume");

  keyfile::get_object_value(*this, &chroot_btrfs_snapshot::set_snapshot_path,
			    keyfile, get_keyfile_name(), "btrfs-snapshot-path",
			    keyfile::PRIORITY_REQUIRED);
  used_keys.push_back("btrfs-snapshot-path");
}
