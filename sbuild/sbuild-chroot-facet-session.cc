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

#include "sbuild-chroot.h"
#include "sbuild-chroot-facet-session.h"
#include "sbuild-chroot-facet-source-clonable.h"
#include "sbuild-chroot-plain.h"
#ifdef SBUILD_FEATURE_LVMSNAP
#include "sbuild-chroot-lvm-snapshot.h"
#endif // SBUILD_FEATURE_LVMSNAP
#ifdef SBUILD_FEATURE_UNION
#include "sbuild-chroot-facet-union.h"
#endif // SBUILD_FEATURE_UNION
#include "sbuild-format-detail.h"

#include <cassert>

#include <boost/format.hpp>

using boost::format;
using std::endl;
using namespace sbuild;

chroot_facet_session::chroot_facet_session ():
  chroot_facet(),
  session_manageable(true),
  session_active(false)
{
}

chroot_facet_session::~chroot_facet_session ()
{
}

chroot_facet_session::ptr
chroot_facet_session::create ()
{
  return ptr(new chroot_facet_session());
}

chroot_facet::ptr
chroot_facet_session::clone () const
{
  return ptr(new chroot_facet_session(*this));
}

std::string const&
chroot_facet_session::get_name () const
{
  static const std::string name("session");

  return name;
}

void
chroot_facet_session::clone_session_setup (chroot::ptr&       clone,
					   std::string const& session_id) const
{
  // Disable session, delete aliases.
  chroot_facet_session::ptr session(clone->get_facet<chroot_facet_session>());
  assert(session);
  if (session)
    {
      clone->set_session_id(session_id);
      assert(clone->get_session_id() == session_id);
      clone->set_description
	(clone->get_description() + ' ' + _("(session chroot)"));
      session->set_session_manageable(false);
      session->set_session_active(true);
      session->get_session_flags(*clone); // For testing.
    }

  log_debug(DEBUG_INFO)
    << format("Cloned session %1%")
    % clone->get_name() << endl;

  // Disable source cloning.
  clone->remove_facet<chroot_facet_source_clonable>();

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
      std::string device(dirname(snapshot->get_device(), '/'));
      device += "/" + clone->get_session_id();
      snapshot->set_snapshot_device(device);
    }
#endif // SBUILD_FEATURE_LVMSNAP

#ifdef SBUILD_FEATURE_UNION
  /* Filesystem unions need the overlay directory specifying. */
  chroot_facet_union::ptr puni(clone->get_facet<chroot_facet_union>());

  if (puni)
    {
      std::string overlay = puni->get_union_overlay_directory();
      overlay += "/" + clone->get_session_id();
      puni->set_union_overlay_directory(overlay);

      std::string underlay = puni->get_union_underlay_directory();
      underlay += "/" + clone->get_session_id();
      puni->set_union_underlay_directory(underlay);
    }
#endif // SBUILD_FEATURE_UNION
}

bool
chroot_facet_session::get_session_manageable () const
{
  return this->session_manageable;
}

void
chroot_facet_session::set_session_manageable (bool manageable)
{
  this->session_manageable = manageable;
}

bool
chroot_facet_session::get_session_active () const
{
  return this->session_active;
}

void
chroot_facet_session::set_session_active (bool active)
{
  this->session_active = active;
}

void
chroot_facet_session::setup_env (chroot const& chroot,
				     environment&  env) const
{
}

chroot::session_flags
chroot_facet_session::get_session_flags (chroot const& chroot) const
{
  chroot::session_flags flags;
  /// If active, support session.
  /// If unionfs support configured, only set if active?
  if (get_session_manageable() && !get_session_active())
    flags = chroot::SESSION_CREATE;
  else
    flags = chroot::SESSION_NOFLAGS;

  return flags;
}

void
chroot_facet_session::get_details (chroot const&  chroot,
				   format_detail& detail) const
{
  /// @todo: Replace base chroot session ID with local ID.
  if (get_session_active() && !chroot.get_session_id().empty())
    detail.add(_("Session ID"), chroot.get_session_id());
}

void
chroot_facet_session::get_keyfile (chroot const& chroot,
				   keyfile&      keyfile) const
{
}

void
chroot_facet_session::set_keyfile (chroot&        chroot,
				   keyfile const& keyfile,
				   string_list&   used_keys)
{
  // Null methods for obsolete keys.
  void (sbuild::chroot_facet_source::* nullmethod)(bool) = 0;
  void (sbuild::chroot_facet_source::* nullvmethod)(string_list const&) = 0;

  // Setting when not clonable is deprecated.  It can't be obsoleted
  // yet because it is required to allow use and ending of existing
  // sessions which have set this parameter (even though it's
  // useless).
  keyfile::get_object_value(*this, nullmethod,
			    keyfile, chroot.get_keyfile_name(),
			    "active",
			    keyfile::PRIORITY_DEPRECATED);
  used_keys.push_back("active");

  keyfile::get_object_list_value(*this, nullvmethod,
				 keyfile, chroot.get_keyfile_name(),
				 "source-users",
				 keyfile::PRIORITY_DEPRECATED);
  used_keys.push_back("source-users");

  keyfile::get_object_list_value(*this, nullvmethod,
				 keyfile, chroot.get_keyfile_name(),
				 "source-groups",
				 keyfile::PRIORITY_DEPRECATED);
  used_keys.push_back("source-groups");

  keyfile::get_object_list_value(*this, nullvmethod,
				 keyfile, chroot.get_keyfile_name(),
				 "source-root-users",
				 keyfile::PRIORITY_DEPRECATED);
  used_keys.push_back("source-root-users");

  keyfile::get_object_list_value(*this, nullvmethod,
				 keyfile, chroot.get_keyfile_name(),
				 "source-root-groups",
				 keyfile::PRIORITY_DEPRECATED);
  used_keys.push_back("source-root-groups");
}
