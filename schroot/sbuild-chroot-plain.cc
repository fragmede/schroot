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

#include "sbuild.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

using namespace sbuild;

chroot_plain::chroot_plain ():
  chroot(),
  location()
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
  return this->location;
}

void
chroot_plain::set_location (std::string const& location)
{
  this->location = location;
}

std::string const&
chroot_plain::get_mount_location () const
{
  return this->location;
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
			  bool       lock)
{
  /* By default, plain chroots do no locking. */
}

sbuild::chroot::session_flags
chroot_plain::get_session_flags () const
{
  return static_cast<session_flags>(0);
}

void
chroot_plain::print_details (std::ostream& stream) const
{
  this->chroot::print_details(stream);

  if (!this->location.empty())
    stream << format_details(_("Location"), get_location());
  stream << std::flush;
}

void
chroot_plain::get_keyfile (keyfile& keyfile) const
{
  chroot::get_keyfile(keyfile);

  keyfile.set_value(get_name(), "location",
		    get_location());
}

void
chroot_plain::set_keyfile (keyfile const& keyfile)
{
  chroot::set_keyfile(keyfile);

  std::string location;
  if (keyfile.get_value(get_name(), "location",
			keyfile::PRIORITY_REQUIRED, location))
    set_location(location);
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
