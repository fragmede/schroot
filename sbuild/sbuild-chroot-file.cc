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

#include "sbuild-chroot-file.h"
#include "sbuild-format-detail.h"
#include "sbuild-lock.h"

#include <cerrno>
#include <cstring>

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

chroot_file::chroot_file ():
  chroot(),
  chroot_source(),
  file(),
  repack(false)
{
  set_run_setup_scripts(true);
  set_run_exec_scripts(true);
}

chroot_file::~chroot_file ()
{
}

sbuild::chroot::ptr
chroot_file::clone () const
{
  return ptr(new chroot_file(*this));
}

sbuild::chroot::ptr
chroot_file::clone_source () const
{
  chroot_file *clone_file = new chroot_file(*this);
  ptr clone(clone_file);

  chroot_source::clone_source_setup(clone);
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

std::string const&
chroot_file::get_chroot_type () const
{
  static const std::string type("file");

  return type;
}

void
chroot_file::setup_env (environment& env)
{
  chroot::setup_env(env);
  chroot_source::setup_env(env);

  env.add("CHROOT_FILE", get_file());
  env.add("CHROOT_FILE_REPACK", this->repack);
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

sbuild::chroot::session_flags
chroot_file::get_session_flags () const
{
  return SESSION_CREATE | chroot_source::get_session_flags();
}

void
chroot_file::get_details (format_detail& detail) const
{
  chroot::get_details(detail);
  chroot_source::get_details(detail);

  if (!this->file.empty())
    detail
      .add(_("File"), get_file())
      .add(_("File Repack"), this->repack);
}

void
chroot_file::get_keyfile (keyfile& keyfile) const
{
  chroot::get_keyfile(keyfile);
  chroot_source::get_keyfile(keyfile);

  keyfile::set_object_value(*this, &chroot_file::get_file,
			    keyfile, get_name(), "file");

  if (get_active())
  keyfile::set_object_value(*this, &chroot_file::get_file_repack,
			    keyfile, get_name(), "file-repack");
}

void
chroot_file::set_keyfile (keyfile const& keyfile,
			  string_list&   used_keys)
{
  chroot::set_keyfile(keyfile, used_keys);
  chroot_source::set_keyfile(keyfile, used_keys);

  keyfile::get_object_value(*this, &chroot_file::set_file,
			    keyfile, get_name(), "file",
			    keyfile::PRIORITY_REQUIRED);
  used_keys.push_back("file");

  keyfile::get_object_value(*this, &chroot_file::set_file_repack,
			    keyfile, get_name(), "file-repack",
			    get_active() ?
			    keyfile::PRIORITY_REQUIRED :
			    keyfile::PRIORITY_DISALLOWED);
  used_keys.push_back("file-repack");
}
