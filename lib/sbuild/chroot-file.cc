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

#include "chroot-file.h"
#include "chroot-facet-session.h"
#include "chroot-facet-session-clonable.h"
#include "chroot-facet-source-clonable.h"
#include "format-detail.h"
#include "lock.h"

#include <cassert>
#include <cerrno>
#include <cstring>

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

chroot_file::chroot_file ():
  chroot(),
  file(),
  location(),
  repack(false)
{
  add_facet(chroot_facet_source_clonable::create());
}

chroot_file::chroot_file (const chroot_file& rhs):
  chroot(rhs),
  file(rhs.file),
  location(rhs.location),
  repack(rhs.repack)
{
}

chroot_file::~chroot_file ()
{
}

chroot::chroot::ptr
chroot_file::clone () const
{
  return ptr(new chroot_file(*this));
}

chroot::chroot::ptr
chroot_file::clone_session (std::string const& session_id,
                            std::string const& alias,
                            std::string const& user,
                            bool               root) const
{
  chroot_facet_session_clonable::const_ptr psess
    (get_facet<chroot_facet_session_clonable>());
  assert(psess);

  ptr session(new chroot_file(*this));
  psess->clone_session_setup(*this, session, session_id, alias, user, root);

  return session;
}

chroot::chroot::ptr
chroot_file::clone_source () const
{
  chroot_file *clone_file = new chroot_file(*this);
  ptr clone(clone_file);

  chroot_facet_source_clonable::const_ptr psrc
    (get_facet<chroot_facet_source_clonable>());
  assert(psrc);

  psrc->clone_source_setup(*this, clone);
  clone_file->repack = true;

  return clone;
}

std::string const&
chroot_file::get_file () const
{
  return this->file;
}

void
chroot_file::set_file (std::string const& file)
{
  if (!is_absname(file))
    throw error(file, FILE_ABS);

  this->file = file;
}

std::string const&
chroot_file::get_location () const
{
  return this->location;
}

void
chroot_file::set_location (std::string const& location)
{
  if (!location.empty() && !is_absname(location))
    throw chroot::error(location, chroot::LOCATION_ABS);

  this->location = location;
}

bool
chroot_file::get_file_repack () const
{
  return this->repack;
}

void
chroot_file::set_file_repack (bool repack)
{
  this->repack = repack;
}

std::string
chroot_file::get_path () const
{
  std::string path(get_mount_location());

  if (!get_location().empty())
    path += get_location();

  return path;
}

std::string const&
chroot_file::get_chroot_type () const
{
  static const std::string type("file");

  return type;
}

void
chroot_file::setup_env (chroot const& chroot,
                        environment&  env) const
{
  chroot::setup_env(chroot, env);

  env.add("CHROOT_FILE", get_file());
  env.add("CHROOT_LOCATION", get_location());
  env.add("CHROOT_FILE_REPACK", this->repack);
  env.add("CHROOT_FILE_UNPACK_DIR", SCHROOT_FILE_UNPACK_DIR);
}

void
chroot_file::setup_lock (chroot::setup_type type,
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

  /* By default, file chroots do no locking. */
  /* Create or unlink session information. */
  if ((type == SETUP_START && lock == true) ||
      (type == SETUP_STOP && lock == false && status == 0))
    {

      bool start = (type == SETUP_START);
      setup_session_info(start);
    }
}

chroot::chroot::session_flags
chroot_file::get_session_flags (chroot const& chroot) const
{
  session_flags flags = SESSION_NOFLAGS;

  if (chroot.get_facet<chroot_facet_session>())
    flags = SESSION_PURGE;

  return flags;
}

void
chroot_file::get_details (chroot const&  chroot,
                          format_detail& detail) const
{
  chroot::get_details(chroot, detail);

  if (!this->file.empty())
    detail
      .add(_("File"), get_file())
      .add(_("File Repack"), this->repack);
  if (!get_location().empty())
    detail.add(_("Location"), get_location());
}

void
chroot_file::get_used_keys (string_list& used_keys) const
{
  chroot::get_used_keys(used_keys);

  used_keys.push_back("file");
  used_keys.push_back("location");
  used_keys.push_back("file-repack");
}

void
chroot_file::get_keyfile (chroot const& chroot,
                          keyfile&      keyfile) const
{
  chroot::get_keyfile(chroot, keyfile);

  bool session = static_cast<bool>(get_facet<chroot_facet_session>());

  keyfile::set_object_value(*this, &chroot_file::get_file,
                            keyfile, get_name(), "file");

  keyfile::set_object_value(*this, &chroot_file::get_location,
                            keyfile, chroot.get_name(),
                            "location");

  if (session)
    keyfile::set_object_value(*this, &chroot_file::get_file_repack,
                              keyfile, get_name(), "file-repack");
}

void
chroot_file::set_keyfile (chroot&        chroot,
                          keyfile const& keyfile)
{
  chroot::set_keyfile(chroot, keyfile);

  bool session = static_cast<bool>(get_facet<chroot_facet_session>());

  keyfile::get_object_value(*this, &chroot_file::set_file,
                            keyfile, get_name(), "file",
                            keyfile::PRIORITY_REQUIRED);

  keyfile::get_object_value(*this, &chroot_file::set_location,
                            keyfile, chroot.get_name(),
                            "location",
                            keyfile::PRIORITY_OPTIONAL);

  keyfile::get_object_value(*this, &chroot_file::set_file_repack,
                            keyfile, get_name(), "file-repack",
                            session ?
                            keyfile::PRIORITY_REQUIRED :
                            keyfile::PRIORITY_DISALLOWED);
}
