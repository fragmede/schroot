/* Copyright © 2005-2006  Roger Leigh <rleigh@debian.org>
 *
 * schroot is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * schroot is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA  02111-1307  USA
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
chroot_plain::get_location () const
{
  return chroot::get_location();
}

void
chroot_plain::set_location (std::string const& location)
{
  if (!is_absname(location))
    throw error(location, LOCATION_ABS);

  chroot::set_location(location);
}

std::string
chroot_plain::get_path () const
{
  // When running setup scripts, we are session-capable, so the path
  // is the bind-mounted location, rather than the original location.
  if (get_run_setup_scripts() == true)
    return get_mount_location();
  else
    return get_location();
}

std::string const&
chroot_plain::get_chroot_type () const
{
  static const std::string type("plain");

  return type;
}

void
chroot_plain::setup_env (environment& env)
{
  this->chroot::setup_env(env);

  env.add("CHROOT_LOCATION", get_location());
}

void
chroot_plain::setup_lock (setup_type type,
			  bool       lock,
			  int        status)
{
  /* By default, plain chroots do no locking. */
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
chroot_plain::get_session_flags () const
{
  if (get_run_setup_scripts() == true)
    return SESSION_CREATE;
  else
    return static_cast<session_flags>(0);
}

void
chroot_plain::get_details (format_detail& detail) const
{
  chroot::get_details(detail);
}

void
chroot_plain::get_keyfile (keyfile& keyfile) const
{
  chroot::get_keyfile(keyfile);

  keyfile::set_object_value(*this, &chroot_plain::get_location,
			    keyfile, get_name(), "location");
}

void
chroot_plain::set_keyfile (keyfile const& keyfile)
{
  chroot::set_keyfile(keyfile);

  keyfile::get_object_value(*this, &chroot_plain::set_location,
			    keyfile, get_name(), "location",
			    keyfile::PRIORITY_REQUIRED);
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
