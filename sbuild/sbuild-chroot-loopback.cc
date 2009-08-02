/* Copyright © 2005-2008  Roger Leigh <rleigh@debian.org>
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

#include "sbuild-chroot-loopback.h"
#include "sbuild-chroot-facet-session.h"
#include "sbuild-chroot-facet-mountable.h"
#ifdef SBUILD_FEATURE_UNION
#include "sbuild-chroot-facet-union.h"
#endif // SBUILD_FEATURE_UNION
#include "sbuild-format-detail.h"
#include "sbuild-lock.h"
#include "sbuild-util.h"

#include <cassert>
#include <cerrno>
#include <cstring>

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

chroot_loopback::chroot_loopback ():
  chroot(),
  file()
{
  add_facet(sbuild::chroot_facet_mountable::create());
#ifdef SBUILD_FEATURE_UNION
  add_facet(sbuild::chroot_facet_union::create());
#endif // SBUILD_FEATURE_UNION
}

chroot_loopback::~chroot_loopback ()
{
}

chroot_loopback::chroot_loopback (const chroot_loopback& rhs):
  chroot(rhs),
  file(rhs.file)
{
}

sbuild::chroot::ptr
chroot_loopback::clone () const
{
  return ptr(new chroot_loopback(*this));
}

sbuild::chroot::ptr
chroot_loopback::clone_session (std::string const& session_id) const
{
  chroot_facet_session::const_ptr psess(get_facet<chroot_facet_session>());
  assert(psess);

  ptr session(new chroot_loopback(*this));
  psess->clone_session_setup(session, session_id);

  return session;
}

sbuild::chroot::ptr
chroot_loopback::clone_source () const
{
  ptr clone;

#ifdef SBUILD_FEATURE_UNION
  chroot_facet_union::const_ptr puni(get_facet<chroot_facet_union>());
  assert(puni);

  if (puni->get_union_configured()) {
    clone = ptr(new chroot_loopback(*this));
    puni->clone_source_setup(clone);
  }
#endif // SBUILD_FEATURE_UNION

  return clone;
}

std::string const&
chroot_loopback::get_file () const
{
  return this->file;
}

void
chroot_loopback::set_file (std::string const& file)
{
  if (!is_absname(file))
    throw error(file, FILE_ABS);

  this->file = file;

  chroot_facet_mountable::ptr pmnt
    (get_facet<chroot_facet_mountable>());
  pmnt->set_mount_device(this->file);
}

std::string
chroot_loopback::get_path () const
{
  chroot_facet_mountable::const_ptr pmnt
    (get_facet<chroot_facet_mountable>());

  std::string path(get_mount_location());

  if (pmnt)
    path += pmnt->get_location();

  return path;
}

std::string const&
chroot_loopback::get_chroot_type () const
{
  static const std::string type("loopback");

  return type;
}

void
chroot_loopback::setup_env (chroot const& chroot,
			    environment&  env) const
{
  chroot::setup_env(chroot, env);

  env.add("CHROOT_FILE", get_file());
}

void
chroot_loopback::setup_lock (chroot::setup_type type,
			     bool               lock,
			     int                status)
{
  // Check ownership and permissions.
  if (type == SETUP_START && lock == true)
    {
      stat file_status(this->file);

      // NOTE: taken from chroot_config::check_security.
      if (file_status.uid() != 0)
	throw error(this->file, FILE_OWNER);
      if (file_status.check_mode(stat::PERM_OTHER_WRITE))
	throw error(this->file, FILE_PERMS);
      if (!file_status.is_regular())
	throw error(this->file, FILE_NOTREG);
    }

  /* By default, loopback chroots do no locking. */
#ifdef SBUILD_FEATURE_UNION
  /**
   * By default, loopback chroots do no locking, but can create sessions
   * using filesystem unions.
   */
  chroot_facet_union::const_ptr puni(get_facet<chroot_facet_union>());
  assert(puni);

  if (puni->get_union_configured() &&
      ((type == SETUP_START && lock == true) ||
       (type == SETUP_STOP && lock == false && status == 0)))
    {
      bool start = (type == SETUP_START);
      setup_session_info(start);
    }
#endif
}

sbuild::chroot::session_flags
chroot_loopback::get_session_flags (chroot const& chroot) const
{
  return chroot::SESSION_NOFLAGS;
}

void
chroot_loopback::get_details (chroot const&  chroot,
			      format_detail& detail) const
{
  chroot::get_details(chroot, detail);

  if (!this->file.empty())
    detail.add(_("File"), get_file());
}

void
chroot_loopback::get_keyfile (chroot const& chroot,
			      keyfile&      keyfile) const
{
  chroot::get_keyfile(chroot, keyfile);

  keyfile::set_object_value(*this, &chroot_loopback::get_file,
			    keyfile, get_keyfile_name(), "file");
}

void
chroot_loopback::set_keyfile (chroot&        chroot,
			      keyfile const& keyfile,
			      string_list&   used_keys)
{
  chroot::set_keyfile(chroot, keyfile, used_keys);

  keyfile::get_object_value(*this, &chroot_loopback::set_file,
			    keyfile, get_keyfile_name(), "file",
			    keyfile::PRIORITY_REQUIRED);
  used_keys.push_back("file");
}
