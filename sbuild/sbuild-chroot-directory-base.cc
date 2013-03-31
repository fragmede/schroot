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

#include "sbuild-chroot-directory-base.h"
#include "sbuild-format-detail.h"
#include "sbuild-lock.h"

#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

using namespace sbuild;

chroot_directory_base::chroot_directory_base ():
  chroot(),
  directory()
{
}

chroot_directory_base::chroot_directory_base (const chroot_directory_base& rhs):
  chroot(rhs),
  directory(rhs.directory)
{
}

chroot_directory_base::chroot_directory_base (const chroot& rhs):
  chroot(rhs),
  directory()
{
}

chroot_directory_base::~chroot_directory_base ()
{
}

std::string const&
chroot_directory_base::get_directory () const
{
  return this->directory;
}

void
chroot_directory_base::set_directory (std::string const& directory)
{
  if (!is_absname(directory))
    throw chroot::error(directory, DIRECTORY_ABS);

  this->directory = directory;
}

void
chroot_directory_base::setup_env (chroot const& chroot,
                                  environment& env) const
{
  chroot::setup_env(chroot, env);

  env.add("CHROOT_DIRECTORY", get_directory());
}

void
chroot_directory_base::get_details (chroot const& chroot,
                                    format_detail& detail) const
{
  chroot::get_details(chroot, detail);

  detail.add(_("Directory"), get_directory());
}

void
chroot_directory_base::get_used_keys (string_list& used_keys) const
{
  chroot::get_used_keys(used_keys);

  used_keys.push_back("directory");
  used_keys.push_back("location");
}

void
chroot_directory_base::get_keyfile (chroot const& chroot,
                                    keyfile& keyfile) const
{
  chroot::get_keyfile(chroot, keyfile);

  keyfile::set_object_value(*this, &chroot_directory_base::get_directory,
                            keyfile, get_name(), "directory");
}

void
chroot_directory_base::set_keyfile (chroot&        chroot,
                                    keyfile const& keyfile)
{
  chroot::set_keyfile(chroot, keyfile);

  // "directory" should be required, but we also accept "location" as
  // an alternative (but deprecated) variant.  Therefore, ensure by
  // hand that one of them is defined, but not both.

  bool directory_key = keyfile.has_key(get_name(), "directory");
  bool location_key = keyfile.has_key(get_name(), "location");

  keyfile::priority directory_priority = keyfile::PRIORITY_OPTIONAL;
  keyfile::priority location_priority = keyfile::PRIORITY_OBSOLETE;

  if (!directory_key && !location_key)
    throw keyfile::error(get_name(), keyfile::MISSING_KEY_NL, "directory");

  // Using both keys is not allowed (which one is the correct one?),
  // so force an exception to be thrown when reading the old location
  // key.
  if (directory_key && location_key)
    location_priority = keyfile::PRIORITY_DISALLOWED;

  keyfile::get_object_value(*this, &chroot_directory_base::set_directory,
                            keyfile, get_name(), "directory",
                            directory_priority);

  keyfile::get_object_value(*this, &chroot_directory_base::set_directory,
                            keyfile, get_name(), "location",
                            location_priority);
}
