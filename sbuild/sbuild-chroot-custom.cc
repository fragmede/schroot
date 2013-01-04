/* Copyright © 2005-2012  Roger Leigh <rleigh@debian.org>
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

#include "sbuild-chroot-custom.h"
#include "sbuild-chroot-facet-session.h"
#include "sbuild-chroot-facet-session-clonable.h"
#include "sbuild-chroot-facet-source-clonable.h"
#include "sbuild-format-detail.h"
#include "sbuild-lock.h"

#include <cassert>
#include <cerrno>
#include <cstring>

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

chroot_custom::chroot_custom ():
  chroot(),
  purgeable(false)
{
}

chroot_custom::chroot_custom (const chroot_custom& rhs):
  chroot(rhs),
  purgeable(false)
{
}

chroot_custom::~chroot_custom ()
{
}

sbuild::chroot::ptr
chroot_custom::clone () const
{
  return ptr(new chroot_custom(*this));
}

sbuild::chroot::ptr
chroot_custom::clone_session (std::string const& session_id,
                              std::string const& alias,
                              std::string const& user,
                              bool               root) const
{
  chroot_facet_session_clonable::const_ptr psess
    (get_facet<chroot_facet_session_clonable>());
  assert(psess);

  ptr session(new chroot_custom(*this));
  psess->clone_session_setup(*this, session, session_id, alias, user, root);

  return session;
}

sbuild::chroot::ptr
chroot_custom::clone_source () const
{
  chroot_custom *clone_custom = new chroot_custom(*this);
  ptr clone(clone_custom);

  chroot_facet_source_clonable::const_ptr psrc
    (get_facet<chroot_facet_source_clonable>());
  assert(psrc);

  psrc->clone_source_setup(*this, clone);

  return clone;
}

void
chroot_custom::set_session_cloneable (bool cloneable)
{
  if (cloneable)
    add_facet(sbuild::chroot_facet_session_clonable::create());
  else
    remove_facet<chroot_facet_session_clonable>();
}

void
chroot_custom::set_session_purgeable (bool purgeable)
{
  this->purgeable = purgeable;
}

bool
chroot_custom::get_session_purgeable () const
{
  return this->purgeable;
}

void
chroot_custom::set_source_cloneable (bool cloneable)
{
  if (cloneable)
    add_facet(chroot_facet_source_clonable::create());
  else
    remove_facet<chroot_facet_source_clonable>();
}

std::string
chroot_custom::get_path () const
{
  // TODO: Allow customisation?  Or require use of mount location?
  return get_mount_location();
}

void
chroot_custom::setup_env (chroot const& chroot,
                          environment& env) const
{
  chroot::setup_env(chroot, env);
}

std::string const&
chroot_custom::get_chroot_type () const
{
  static const std::string type("custom");

  return type;
}

void
chroot_custom::setup_lock (chroot::setup_type type,
                           bool               lock,
                           int                status)
{
  /* By default, custom chroots do no locking. */
  /* Create or unlink session information. */
  if ((type == SETUP_START && lock == true) ||
      (type == SETUP_STOP && lock == false && status == 0))
    {

      bool start = (type == SETUP_START);
      setup_session_info(start);
    }
}

sbuild::chroot::session_flags
chroot_custom::get_session_flags (chroot const& chroot) const
{
  session_flags flags = SESSION_NOFLAGS;

  // TODO: Only set if purge is set.

  if (chroot.get_facet<chroot_facet_session>() &&
      get_session_purgeable())
    flags = SESSION_PURGE;

  return flags;
}

void
chroot_custom::get_details (chroot const&  chroot,
                            format_detail& detail) const
{
  chroot::get_details(chroot, detail);
}

void
chroot_custom::get_used_keys (string_list& used_keys) const
{
  chroot::get_used_keys(used_keys);

  used_keys.push_back("custom-cloneable");
  used_keys.push_back("custom-purgeable");
  used_keys.push_back("custom-source-cloneable");
}

void
chroot_custom::get_keyfile (chroot const& chroot,
                            keyfile&      keyfile) const
{
  chroot::get_keyfile(chroot, keyfile);

  keyfile::set_object_value(*this,
                            &chroot_custom::get_session_purgeable,
                            keyfile, get_name(),
                            "custom-session-purgeable");
}

void
chroot_custom::set_keyfile (chroot& chroot,
                            keyfile const& keyfile)
{
  chroot::set_keyfile(chroot, keyfile);

  bool session = static_cast<bool>(get_facet<chroot_facet_session>());

  keyfile::get_object_value(*this, &chroot_custom::set_session_cloneable,
                            keyfile, get_name(), "custom-session-cloneable",
                            session ?
                            keyfile::PRIORITY_DISALLOWED :
                            keyfile::PRIORITY_OPTIONAL);

  keyfile::get_object_value(*this, &chroot_custom::set_session_purgeable,
                            keyfile, get_name(), "custom-session-purgeable",
                            keyfile::PRIORITY_OPTIONAL);

  keyfile::get_object_value(*this, &chroot_custom::set_source_cloneable,
                            keyfile, get_name(), "custom-source-cloneable",
                            session ?
                            keyfile::PRIORITY_DISALLOWED :
                            keyfile::PRIORITY_OPTIONAL);
}
