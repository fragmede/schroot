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
  chroot_facet()
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
chroot_facet_session::setup_env (chroot const& chroot,
				     environment&  env) const
{
}

chroot::session_flags
chroot_facet_session::get_session_flags (chroot const& chroot) const
{
  return chroot::SESSION_NOFLAGS;
}

void
chroot_facet_session::get_details (chroot const&  chroot,
				   format_detail& detail) const
{
  /// @todo: Replace base chroot session ID with local ID.
  if (!chroot.get_session_id().empty())
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
  void (sbuild::chroot_facet_session::* nullmethod)(bool) = 0;
  void (sbuild::chroot_facet_session::* nullvmethod)(string_list const&) = 0;

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
