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
#include "sbuild-format-detail.h"
#include "sbuild-lock.h"
#include "sbuild-util.h"

#include <cerrno>
#include <cstring>

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

chroot_loopback::chroot_loopback ():
  chroot(),
  chroot_mountable(),
  file()
{
}

chroot_loopback::~chroot_loopback ()
{
}

sbuild::chroot::ptr
chroot_loopback::clone () const
{
  return ptr(new chroot_loopback(*this));
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
}

std::string
chroot_loopback::get_path () const
{
  return get_mount_location() + get_location();
}

void
chroot_loopback::set_mount_device (std::string const& mount_device)
{
  // Setting the mount device is not permitted for loopback chroots.
}

std::string const&
chroot_loopback::get_mount_device () const
{
  return this->file;
}

std::string const&
chroot_loopback::get_chroot_type () const
{
  static const std::string type("loopback");

  return type;
}

void
chroot_loopback::setup_env (environment& env)
{
  chroot::setup_env(env);
  chroot_mountable::setup_env(env);

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
}

sbuild::chroot::session_flags
chroot_loopback::get_session_flags () const
{
  return SESSION_NOFLAGS | chroot_mountable::get_session_flags();
}

void
chroot_loopback::get_details (format_detail& detail) const
{
  chroot::get_details(detail);
  chroot_mountable::get_details(detail);

  if (!this->file.empty())
    detail.add(_("File"), get_file());
}

void
chroot_loopback::get_keyfile (keyfile& keyfile) const
{
  chroot::get_keyfile(keyfile);
  chroot_mountable::get_keyfile(keyfile);

  keyfile::set_object_value(*this, &chroot_loopback::get_file,
			    keyfile, get_keyfile_name(), "file");
}

void
chroot_loopback::set_keyfile (keyfile const& keyfile,
			      string_list&   used_keys)
{
  chroot::set_keyfile(keyfile, used_keys);
  chroot_mountable::set_keyfile(keyfile, used_keys);

  keyfile::get_object_value(*this, &chroot_loopback::set_file,
			    keyfile, get_keyfile_name(), "file",
			    keyfile::PRIORITY_REQUIRED);
  used_keys.push_back("file");
}
