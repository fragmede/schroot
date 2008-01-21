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
  chroot()
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
chroot_directory::get_location () const
{
  return chroot::get_location();
}

void
chroot_directory::set_location (std::string const& location)
{
  if (!is_absname(location))
    throw error(location, LOCATION_ABS);

  chroot::set_location(location);
}

std::string
chroot_directory::get_path () const
{
  // When running setup scripts, we are session-capable, so the path
  // is the bind-mounted location, rather than the original location.
  if (get_run_setup_scripts() == true)
    return get_mount_location();
  else
    return get_location();
}

std::string const&
chroot_directory::get_chroot_type () const
{
  static const std::string type("directory");

  return type;
}

void
chroot_directory::setup_lock (chroot::setup_type type,
			      bool               lock,
			      int                status)
{
  /* By default, directory chroots do no locking. */
  /* Create or unlink session information. */
  if (get_run_setup_scripts() == true)
    {
      if ((type == SETUP_START && lock == true) ||
	  (type == SETUP_STOP && lock == false && status == 0))
	{
	  bool start = (type == SETUP_START);
	  setup_session_info(start);
	}
    }
}

sbuild::chroot::session_flags
chroot_directory::get_session_flags () const
{
  if (get_run_setup_scripts() == true)
    return SESSION_CREATE;
  else
    return SESSION_NOFLAGS;
}

void
chroot_directory::get_details (format_detail& detail) const
{
  chroot::get_details(detail);
}

void
chroot_directory::get_keyfile (keyfile& keyfile) const
{
  chroot::get_keyfile(keyfile);

  keyfile::set_object_value(*this, &chroot_directory::get_location,
			    keyfile, get_name(), "location");
}

void
chroot_directory::set_keyfile (keyfile const& keyfile,
			       string_list&   used_keys)
{
  chroot::set_keyfile(keyfile, used_keys);

  keyfile::get_object_value(*this, &chroot_directory::set_location,
			    keyfile, get_name(), "location",
			    keyfile::PRIORITY_REQUIRED);
  used_keys.push_back("location");
}
