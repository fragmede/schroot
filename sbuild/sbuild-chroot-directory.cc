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

#include "sbuild-chroot-directory.h"
#include "sbuild-format-detail.h"
#include "sbuild-lock.h"

#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

using namespace sbuild;

chroot_directory::chroot_directory ():
  chroot(),
  directory()
{
}

chroot_directory::~chroot_directory ()
{
}

sbuild::chroot::ptr
chroot_directory::clone () const
{
  return ptr(new chroot_directory(*this));
}

std::string const&
chroot_directory::get_directory () const
{
  return this->directory;
}

void
chroot_directory::set_directory (std::string const& directory)
{
  if (!is_absname(directory))
    throw error(directory, DIRECTORY_ABS);

  this->directory = directory;
}

std::string
chroot_directory::get_path () const
{
  return get_mount_location();
}

std::string const&
chroot_directory::get_chroot_type () const
{
  static const std::string type("directory");

  return type;
}

void
chroot_directory::setup_env (environment& env)
{
  chroot::setup_env(env);

  env.add("CHROOT_DIRECTORY", get_directory());
}

void
chroot_directory::setup_lock (chroot::setup_type type,
			      bool               lock,
			      int                status)
{
  /* Create or unlink session information. */
  if ((type == SETUP_START && lock == true) ||
      (type == SETUP_STOP && lock == false && status == 0))
    {
      bool start = (type == SETUP_START);
      setup_session_info(start);
    }
}

sbuild::chroot::session_flags
chroot_directory::get_session_flags () const
{
  return SESSION_CREATE;
}

void
chroot_directory::get_details (format_detail& detail) const
{
  chroot::get_details(detail);

  detail.add(_("Directory"), get_directory());
}

void
chroot_directory::get_keyfile (keyfile& keyfile) const
{
  chroot::get_keyfile(keyfile);

  keyfile::set_object_value(*this, &chroot_directory::get_directory,
			    keyfile, get_keyfile_name(), "directory");
}

void
chroot_directory::set_keyfile (keyfile const& keyfile,
			       string_list&   used_keys)
{
  chroot::set_keyfile(keyfile, used_keys);

  // "directory" should be required, but we also accept "location" as
  // an alternative (but deprecated) variant.  Therefore, ensure by
  // hand that one of them is defined, but not both.

  bool directory_key = keyfile.has_key(get_keyfile_name(), "directory");
  bool location_key = keyfile.has_key(get_keyfile_name(), "location");

  keyfile::priority directory_priority = keyfile::PRIORITY_OPTIONAL;
  keyfile::priority location_priority = keyfile::PRIORITY_DEPRECATED;

  if (!directory_key && !location_key)
    throw keyfile::error(get_keyfile_name(), keyfile::MISSING_KEY_NL, "directory");

  // Using both keys is not allowed (which one is the correct one?),
  // so force an exception to be thrown when reading the old location
  // key.
  if (directory_key && location_key)
    location_priority = keyfile::PRIORITY_DISALLOWED;

  keyfile::get_object_value(*this, &chroot_directory::set_directory,
			    keyfile, get_keyfile_name(), "directory",
			    directory_priority);
  used_keys.push_back("directory");

  keyfile::get_object_value(*this, &chroot_directory::set_directory,
			    keyfile, get_keyfile_name(), "location",
			    location_priority);
  used_keys.push_back("location");
}
