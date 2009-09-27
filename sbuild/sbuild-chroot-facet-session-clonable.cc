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
#include "sbuild-chroot-facet-session-clonable.h"
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

chroot_facet_session_clonable::chroot_facet_session_clonable ():
  chroot_facet()
{
}

chroot_facet_session_clonable::~chroot_facet_session_clonable ()
{
}

chroot_facet_session_clonable::ptr
chroot_facet_session_clonable::create ()
{
  return ptr(new chroot_facet_session_clonable());
}

chroot_facet::ptr
chroot_facet_session_clonable::clone () const
{
  return ptr(new chroot_facet_session_clonable(*this));
}

std::string const&
chroot_facet_session_clonable::get_name () const
{
  static const std::string name("session");

  return name;
}

void
chroot_facet_session_clonable::clone_session_setup (chroot::ptr&       clone,
						    std::string const& session_id,
						    std::string const& user,
						    bool               root) const
{
  clone->remove_facet<chroot_facet_session_clonable>();
  clone->add_facet(chroot_facet_session::create());

  // Disable session, delete aliases.
  chroot_facet_session::ptr session(clone->get_facet<chroot_facet_session>());
  assert(session);
  if (session)
    {
      clone->set_session_id(session_id);
      assert(clone->get_session_id() == session_id);
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
  if (snapshot && !snapshot->get_device().empty())
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

void
chroot_facet_session_clonable::setup_env (chroot const& chroot,
					  environment&  env) const
{
}

chroot::session_flags
chroot_facet_session_clonable::get_session_flags (chroot const& chroot) const
{
  return chroot::SESSION_CREATE;
}

void
chroot_facet_session_clonable::get_details (chroot const&  chroot,
					    format_detail& detail) const
{
}

void
chroot_facet_session_clonable::get_keyfile (chroot const& chroot,
					    keyfile&      keyfile) const
{
}

void
chroot_facet_session_clonable::set_keyfile (chroot&        chroot,
					    keyfile const& keyfile,
					    string_list&   used_keys)
{
}
