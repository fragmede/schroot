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

#include "sbuild-chroot.h"
#include "sbuild-chroot-config.h"
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
  original_chroot_name(),
  selected_chroot_name()
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

std::string const&
chroot_facet_session::get_original_name () const
{
  return this->original_chroot_name;
}

void
chroot_facet_session::set_original_name (std::string const& name)
{
  this->original_chroot_name = name;
  this->selected_chroot_name = name;
}

std::string const&
chroot_facet_session::get_selected_name () const
{
  return this->selected_chroot_name;
}

void
chroot_facet_session::set_selected_name (std::string const& name)
{
  std::string ns, shortname;
  chroot_config::get_namespace(name, ns, shortname);
  this->selected_chroot_name = shortname;
}

void
chroot_facet_session::setup_env (chroot const& chroot,
                                 environment&  env) const
{
  // Add original name to environment, but only if set (otherwise
  // defaults to session ID).
  if (!get_original_name().empty())
    env.add("CHROOT_NAME", get_original_name());

  if (!get_selected_name().empty())
    env.add("CHROOT_ALIAS", get_selected_name());
}

sbuild::chroot::session_flags
chroot_facet_session::get_session_flags (chroot const& chroot) const
{
  return chroot::SESSION_NOFLAGS;
}

void
chroot_facet_session::get_details (chroot const&  chroot,
                                   format_detail& detail) const
{
  if (!get_original_name().empty())
    detail.add(_("Original Chroot Name"), get_original_name());
  if (!get_original_name().empty())
    detail.add(_("Selected Chroot Name"), get_selected_name());
  if (!chroot.get_name().empty())
    detail.add(_("Session ID"), chroot.get_name());
}

void
chroot_facet_session::get_used_keys (string_list& used_keys) const
{
  used_keys.push_back("active");
  used_keys.push_back("source-users");
  used_keys.push_back("source-groups");
  used_keys.push_back("source-root-users");
  used_keys.push_back("source-root-groups");
  used_keys.push_back("original-name");
  used_keys.push_back("selected-name");
}

void
chroot_facet_session::get_keyfile (chroot const& chroot,
                                   keyfile&      keyfile) const
{
  keyfile::set_object_value(*this, &chroot_facet_session::get_original_name,
                            keyfile, chroot.get_name(),
                            "original-name");

  keyfile::set_object_value(*this, &chroot_facet_session::get_selected_name,
                            keyfile, chroot.get_name(),
                            "selected-name");
}

void
chroot_facet_session::set_keyfile (chroot&        chroot,
                                   keyfile const& keyfile)
{
  // Null methods for obsolete keys.
  void (chroot_facet_session::* nullmethod)(bool) = 0;
  void (chroot_facet_session::* nullvmethod)(string_list const&) = 0;

  // Setting when not clonable is deprecated.  It can't be obsoleted
  // yet because it is required to allow use and ending of existing
  // sessions which have set this parameter (even though it's
  // useless).
  keyfile::get_object_value(*this, nullmethod,
                            keyfile, chroot.get_name(),
                            "active",
                            keyfile::PRIORITY_OBSOLETE);

  keyfile::get_object_list_value(*this, nullvmethod,
                                 keyfile, chroot.get_name(),
                                 "source-users",
                                 keyfile::PRIORITY_OBSOLETE);

  keyfile::get_object_list_value(*this, nullvmethod,
                                 keyfile, chroot.get_name(),
                                 "source-groups",
                                 keyfile::PRIORITY_OBSOLETE);

  keyfile::get_object_list_value(*this, nullvmethod,
                                 keyfile, chroot.get_name(),
                                 "source-root-users",
                                 keyfile::PRIORITY_OBSOLETE);

  keyfile::get_object_list_value(*this, nullvmethod,
                                 keyfile, chroot.get_name(),
                                 "source-root-groups",
                                 keyfile::PRIORITY_OBSOLETE);

  keyfile::get_object_value(*this, &chroot_facet_session::set_original_name,
                            keyfile, chroot.get_name(),
                            "original-name",
                            keyfile::PRIORITY_OPTIONAL);

  keyfile::get_object_value(*this, &chroot_facet_session::set_selected_name,
                            keyfile, chroot.get_name(),
                            "selected-name",
                            keyfile::PRIORITY_OPTIONAL);
}
