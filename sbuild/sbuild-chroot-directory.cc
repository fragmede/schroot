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
  chroot_directory_base()
#ifdef SBUILD_FEATURE_UNION
  , chroot_union()
#endif // SBUILD_FEATURE_UNION
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

#ifdef SBUILD_FEATURE_UNION
sbuild::chroot::ptr
chroot_directory::clone_source () const
{
  ptr clone;

#ifdef SBUILD_FEATURE_UNION
  if (get_union_configured()) {
    clone = ptr(new chroot_directory(*this));
    chroot_source::clone_source_setup(clone);
  }
#endif // SBUILD_FEATURE_UNION
  return ptr(clone);
}
#endif // SBUILD_FEATURE_UNION

std::string
chroot_directory::get_path () const
{
  return get_mount_location();
}

void
chroot_directory::setup_env (environment& env)
{
  chroot_directory_base::setup_env(env);
#ifdef SBUILD_FEATURE_UNION
  chroot_union::setup_env(env);
#endif // SBUILD_FEATURE_UNION
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
#ifdef SBUILD_FEATURE_UNION
  return SESSION_CREATE | chroot_union::get_session_flags();
#else
  return SESSION_CREATE;
#endif // SBUILD_FEATURE_UNION
}

void
chroot_directory::get_details (format_detail& detail) const
{
  chroot_directory_base::get_details(detail);
#ifdef SBUILD_FEATURE_UNION
  chroot_union::get_details(detail);
#endif // SBUILD_FEATURE_UNION
}

void
chroot_directory::get_keyfile (keyfile& keyfile) const
{
  chroot_directory_base::get_keyfile(keyfile);
#ifdef SBUILD_FEATURE_UNION
  chroot_union::get_keyfile(keyfile);
#endif // SBUILD_FEATURE_UNION
}

void
chroot_directory::set_keyfile (keyfile const& keyfile,
			      string_list&   used_keys)
{
  chroot_directory_base::set_keyfile(keyfile, used_keys);
#ifdef SBUILD_FEATURE_UNION
  chroot_union::set_keyfile(keyfile, used_keys);
#endif // SBUILD_FEATURE_UNION
}
