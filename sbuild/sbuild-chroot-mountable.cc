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

#include "sbuild-chroot-block-device.h"
#include "sbuild-format-detail.h"
#include "sbuild-lock.h"
#include "sbuild-util.h"

#include <cassert>
#include <cerrno>
#include <cstring>

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

chroot_mountable::chroot_mountable ():
  mount_options(),
  location()
{
}

chroot_mountable::~chroot_mountable ()
{
}

std::string const&
chroot_mountable::get_mount_options () const
{
  return this->mount_options;
}

void
chroot_mountable::set_mount_options (std::string const& mount_options)
{
  this->mount_options = mount_options;
}

std::string const&
chroot_mountable::get_location () const
{
  return this->location;
}

void
chroot_mountable::set_location (std::string const& location)
{
  if (!location.empty() && !is_absname(location))
    throw chroot::error(location, chroot::LOCATION_ABS);

  this->location = location;
}

void
chroot_mountable::setup_env (chroot const& chroot,
			     environment&  env) const
{
  env.add("CHROOT_MOUNT_DEVICE", get_mount_device());
  env.add("CHROOT_MOUNT_OPTIONS", get_mount_options());
  env.add("CHROOT_LOCATION", get_location());
}

sbuild::chroot::session_flags
chroot_mountable::get_session_flags (chroot const& chroot) const
{
  return chroot::SESSION_NOFLAGS;
}

void
chroot_mountable::get_details (chroot const& chroot,
			       format_detail& detail) const
{
  if (!get_mount_device().empty())
    detail.add(_("Mount Device"), get_mount_device());
  if (!get_mount_options().empty())
    detail.add(_("Mount Options"), get_mount_options());
  if (!get_location().empty())
    detail.add(_("Location"), get_location());
}

void
chroot_mountable::get_keyfile (chroot const& chroot,
			       keyfile& keyfile) const
{
  keyfile::set_object_value(*this, &chroot_mountable::get_mount_options,
			    keyfile, chroot.get_keyfile_name(), "mount-options");

  keyfile::set_object_value(*this, &chroot_mountable::get_location,
			    keyfile, chroot.get_keyfile_name(), "location");
}

void
chroot_mountable::set_keyfile (chroot&        chroot,
			       keyfile const& keyfile,
			       string_list&   used_keys)
{
  keyfile::get_object_value(*this, &chroot_mountable::set_mount_options,
			    keyfile, chroot.get_keyfile_name(), "mount-options",
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("mount-options");

  keyfile::get_object_value(*this, &chroot_mountable::set_location,
			    keyfile, chroot.get_keyfile_name(), "location",
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("location");
}
