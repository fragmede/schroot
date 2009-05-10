/* Copyright © 2005-2007  Roger Leigh <rleigh@debian.org>
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

#include "sbuild-chroot-plain.h"
#include "sbuild-format-detail.h"
#include "sbuild-lock.h"

#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

using namespace sbuild;

chroot_plain::chroot_plain ():
  chroot()
{
  set_run_setup_scripts(false);
}

chroot_plain::~chroot_plain ()
{
}

sbuild::chroot::ptr
chroot_plain::clone () const
{
  return ptr(new chroot_plain(*this));
}

std::string const&
chroot_plain::get_directory () const
{
  return this->directory;
}

void
chroot_plain::set_directory (std::string const& directory)
{
  if (!is_absname(directory))
    throw chroot::error(directory, DIRECTORY_ABS);

  this->directory = directory;
}

std::string const&
chroot_plain::get_chroot_type () const
{
  static const std::string type("plain");

  return type;
}

std::string
chroot_plain::get_path () const
{
  return get_directory();
}

void
chroot_plain::setup_env (environment& env)
{
  chroot::setup_env(env);

  env.add("CHROOT_DIRECTORY", get_directory());
}

void
chroot_plain::setup_lock (chroot::setup_type type,
			  bool               lock,
			  int                status)
{
  /* By default, plain chroots do no locking. */
}

sbuild::chroot::session_flags
chroot_plain::get_session_flags () const
{
  return SESSION_NOFLAGS;
}

void
chroot_plain::get_details (format_detail& detail) const
{
  chroot::get_details(detail);

  detail.add(_("Directory"), get_directory());
}

void
chroot_plain::get_keyfile (keyfile& keyfile) const
{
  chroot::get_keyfile(keyfile);

  keyfile::set_object_value(*this, &chroot_plain::get_directory,
			    keyfile, get_name(), "directory");
}

void
chroot_plain::set_keyfile (keyfile const& keyfile,
			   string_list&   used_keys)
{
  chroot::set_keyfile(keyfile, used_keys);

  // "directory" should be required, but we also accept "location" as
  // an alternative (but deprecated) variant.  Therefore, ensure by
  // hand that one of them is defined, but not both.

  bool directory_key = keyfile.has_key(get_name(), "directory");
  bool location_key = keyfile.has_key(get_name(), "location");

  keyfile::priority directory_priority = keyfile::PRIORITY_OPTIONAL;
  keyfile::priority location_priority = keyfile::PRIORITY_DEPRECATED;

  if (!directory_key && !location_key)
    throw keyfile::error(get_name(), keyfile::MISSING_KEY_NL, "directory");

  // Using both keys is not allowed (which one is the correct one?),
  // so force an exception to be thrown when reading the old location
  // key.
  if (directory_key && location_key)
    location_priority = keyfile::PRIORITY_DISALLOWED;

  keyfile::get_object_value(*this, &chroot_plain::set_directory,
			    keyfile, get_name(), "directory",
			    directory_priority);
  used_keys.push_back("directory");

  keyfile::get_object_value(*this, &chroot_plain::set_directory,
			    keyfile, get_name(), "location",
			    location_priority);
  used_keys.push_back("location");
}
