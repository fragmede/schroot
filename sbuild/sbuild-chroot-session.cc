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

#include "sbuild-chroot-session.h"
#include "sbuild-chroot-source.h"
#include "sbuild-chroot-plain.h"
#ifdef SBUILD_FEATURE_LVMSNAP
#include "sbuild-chroot-lvm-snapshot.h"
#endif // SBUILD_FEATURE_LVMSNAP
#ifdef SBUILD_FEATURE_UNION
#include "sbuild-chroot-union.h"
#endif // SBUILD_FEATURE_UNION
#include "sbuild-format-detail.h"

#include <algorithm>

#include <boost/format.hpp>

using boost::format;
using std::endl;
using namespace sbuild;

chroot_session::chroot_session ():
  session_manageable(true),
  session_active(false)
{
}

chroot_session::~chroot_session ()
{
}

void
chroot_session::clone_session_setup (chroot::ptr&       clone,
				     std::string const& session_id) const
{
  // Disable session, delete aliases.
  std::tr1::shared_ptr<chroot_session> session(std::tr1::dynamic_pointer_cast<chroot_session>(clone));
  if (session)
    {
      clone->set_session_id(session_id);
      clone->set_description
	(clone->get_description() + ' ' + _("(session chroot)"));
      session->set_session_manageable(false);
      session->set_session_active(true);
    }

  log_debug(DEBUG_INFO)
    << format("Cloned session %1%")
    % clone->get_name() << endl;

  // Disable source cloning.
  std::tr1::shared_ptr<chroot_source> source(std::tr1::dynamic_pointer_cast<chroot_source>(clone));
  if (source)
    source->set_source_clonable(false);

  /* If a chroot mount location has not yet been set, and the
     chroot is not a plain chroot, set a mount location with the
     session id.  Only set for non-plain chroots which run
     setup scripts. */
  {
    std::tr1::shared_ptr<chroot_plain> plain(std::tr1::dynamic_pointer_cast<chroot_plain>(clone));

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

#ifdef SBUILD_FEATURE_LVMSNAP
  /* LVM devices need the snapshot device name specifying. */
  std::tr1::shared_ptr<chroot_lvm_snapshot> snapshot(std::tr1::dynamic_pointer_cast<chroot_lvm_snapshot>(clone));
  if (snapshot)
    {
      std::string dir(dirname(snapshot->get_device(), '/'));
      std::string device(dir + "/" + clone->get_session_id());
      snapshot->set_snapshot_device(device);
    }
#endif // SBUILD_FEATURE_LVMSNAP

#ifdef SBUILD_FEATURE_UNION
  /* Filesystem unions need the overlay directory specifying. */
  std::tr1::shared_ptr<chroot_union> fsunion(std::tr1::dynamic_pointer_cast<chroot_union>(clone));
  if (fsunion)
    {
      std::string overlay = fsunion->get_union_overlay_directory();
      overlay += "/" + clone->get_session_id();
      fsunion->set_union_overlay_directory(overlay);

      std::string underlay = fsunion->get_union_underlay_directory();
      underlay += "/" + clone->get_session_id();
      fsunion->set_union_underlay_directory(underlay);
    }
#endif // SBUILD_FEATURE_UNION
}

bool
chroot_session::get_session_manageable () const
{
  return this->session_manageable;
}

void
chroot_session::set_session_manageable (bool manageable)
{
  this->session_manageable = manageable;
}

bool
chroot_session::get_session_active () const
{
  return this->session_active;
}

void
chroot_session::set_session_active (bool active)
{
  this->session_active = active;

  /// @todo: Remove need for this.
  chroot *base = dynamic_cast<chroot *>(this);
  base->set_active(active);
}

void
chroot_session::setup_env (environment& env)
{
}

sbuild::chroot::session_flags
chroot_session::get_session_flags () const
{
  /// @todo: Remove need for this.
  const chroot *base = dynamic_cast<const chroot *>(this);
  assert(base != 0);

  /// If active, support session.
  if (get_session_manageable() && !get_session_active())
    return chroot::SESSION_CREATE;
  else
    return chroot::SESSION_NOFLAGS;
}

void
chroot_session::get_details (format_detail& detail) const
{
}

void
chroot_session::get_keyfile (keyfile& keyfile) const
{
  /// @todo: Remove need for this by passing in group name
  const chroot *base = dynamic_cast<const chroot *>(this);
  assert(base != 0);
}

void
chroot_session::set_keyfile (keyfile const& keyfile,
			    string_list&   used_keys)
{
  /// @todo: Remove need for this by passing in group name
  const chroot *base = dynamic_cast<const chroot *>(this);
  assert(base != 0);
}
