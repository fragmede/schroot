/* Copyright © 2005-2009  Roger Leigh <rleigh@debian.org>
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

#include "sbuild-chroot.h"
#include "sbuild-chroot-facet-mountable.h"
#include "sbuild-chroot-facet-session.h"

#include <cassert>

#include <boost/format.hpp>

using boost::format;
using std::endl;
using namespace sbuild;

chroot_facet_mountable::chroot_facet_mountable ():
  chroot_facet(),
  mount_device(),
  mount_options(),
  location()
{
}

chroot_facet_mountable::~chroot_facet_mountable ()
{
}

chroot_facet_mountable::ptr
chroot_facet_mountable::create ()
{
  return ptr(new chroot_facet_mountable());
}

chroot_facet::ptr
chroot_facet_mountable::clone () const
{
  return ptr(new chroot_facet_mountable(*this));
}

std::string const&
chroot_facet_mountable::get_name () const
{
  static const std::string name("mountable");

  return name;
}

std::string const&
chroot_facet_mountable::get_mount_device () const
{
  return this->mount_device;
}

void
chroot_facet_mountable::set_mount_device (std::string const& mount_device)
{
  this->mount_device = mount_device;
}

std::string const&
chroot_facet_mountable::get_mount_options () const
{
  return this->mount_options;
}

void
chroot_facet_mountable::set_mount_options (std::string const& mount_options)
{
  this->mount_options = mount_options;
}

std::string const&
chroot_facet_mountable::get_location () const
{
  return this->location;
}

void
chroot_facet_mountable::set_location (std::string const& location)
{
  if (!location.empty() && !is_absname(location))
    throw chroot::error(location, chroot::LOCATION_ABS);

  this->location = location;
}

void
chroot_facet_mountable::setup_env (chroot const& chroot,
                                   environment&  env) const
{
  env.add("CHROOT_MOUNT_DEVICE", get_mount_device());
  env.add("CHROOT_MOUNT_OPTIONS", get_mount_options());
  env.add("CHROOT_LOCATION", get_location());
}

sbuild::chroot::session_flags
chroot_facet_mountable::get_session_flags (chroot const& chroot) const
{
  return chroot::SESSION_NOFLAGS;
}

void
chroot_facet_mountable::get_details (chroot const&  chroot,
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
chroot_facet_mountable::get_used_keys (string_list& used_keys) const
{
  used_keys.push_back("mount-device");
  used_keys.push_back("mount-options");
  used_keys.push_back("location");
}

void
chroot_facet_mountable::get_keyfile (chroot const& chroot,
                                     keyfile&      keyfile) const
{
  bool session = static_cast<bool>(chroot.get_facet<chroot_facet_session>());

  if (session)
    keyfile::set_object_value(*this, &chroot_facet_mountable::get_mount_device,
                              keyfile, chroot.get_name(),
                              "mount-device");

  keyfile::set_object_value(*this, &chroot_facet_mountable::get_mount_options,
                            keyfile, chroot.get_name(),
                            "mount-options");

  keyfile::set_object_value(*this, &chroot_facet_mountable::get_location,
                            keyfile, chroot.get_name(),
                            "location");
}

void
chroot_facet_mountable::set_keyfile (chroot&        chroot,
                                     keyfile const& keyfile)
{
  bool session = static_cast<bool>(chroot.get_facet<chroot_facet_session>());

  keyfile::get_object_value(*this, &chroot_facet_mountable::set_mount_device,
                            keyfile, chroot.get_name(),
                            "mount-device",
                            session ?
                            keyfile::PRIORITY_REQUIRED :
                            keyfile::PRIORITY_DISALLOWED);

  keyfile::get_object_value(*this, &chroot_facet_mountable::set_mount_options,
                            keyfile, chroot.get_name(),
                            "mount-options",
                            keyfile::PRIORITY_OPTIONAL);

  keyfile::get_object_value(*this, &chroot_facet_mountable::set_location,
                            keyfile, chroot.get_name(),
                            "location",
                            keyfile::PRIORITY_OPTIONAL);
}
