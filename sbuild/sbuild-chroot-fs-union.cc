/* Copyright © 2008-2009  Jan-Marek Glogowski <glogow@fbihome.de>
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

#include "sbuild-chroot-fs-union.h"

#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

using namespace sbuild;

namespace
{
  typedef std::pair<chroot_fs_union::error_code,const char *> emap;

  /**
   * This is a list of the supported error codes.  It's used to
   * construct the real error codes map.
   */
  emap init_errors[] =
    {
      // TRANSLATORS: %1% = chroot fs type
      emap(chroot_fs_union::FS_TYPE_UNKNOWN, N_("Unknown filesystem type '%1%'"))
    };
}

template<>
error<chroot_fs_union::error_code>::map_type
error<chroot_fs_union::error_code>::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));

chroot_fs_union::chroot_fs_union ():
  chroot_source(),
  fs_union_type("none")
{
}

chroot_fs_union::~chroot_fs_union ()
{
}

bool
chroot_fs_union::get_fs_union_configured () const
{
  std::string type = get_fs_union_type();
  return (type != "none") ? true : false;
}

std::string const&
chroot_fs_union::get_overlay_session_directory () const
{
  return overlay_session_directory;
}

void
chroot_fs_union::set_overlay_session_directory
(std::string const& overlay_session_directory)
{
  if (!is_absname(overlay_session_directory))
    throw chroot::error(overlay_session_directory, chroot::LOCATION_ABS);

  this->overlay_session_directory = overlay_session_directory;
}

std::string const&
chroot_fs_union::get_fs_union_type () const
{
  return fs_union_type;
}

void
chroot_fs_union::set_fs_union_type (std::string const& fs_union_type)
{
  if ((fs_union_type == "aufs") || (fs_union_type == "unionfs"))
    this->fs_union_type = fs_union_type;
  else if (fs_union_type == "none")
    this->fs_union_type = fs_union_type;
  else
    throw error(fs_union_type, FS_TYPE_UNKNOWN);
}

std::string const&
chroot_fs_union::get_fs_union_branch_config () const
{
  return fs_union_branch_config;
}

void
chroot_fs_union::set_fs_union_branch_config
(std::string const& fs_union_branch_config)
{
  this->fs_union_branch_config = fs_union_branch_config;
}

void
chroot_fs_union::setup_env (environment& env)
{
  chroot_source::setup_env(env);

  env.add("CHROOT_FS_UNION_TYPE", get_fs_union_type());
  if (get_fs_union_configured())
    {
      env.add("CHROOT_FS_UNION_OVERLAY_DIRECTORY",
        get_overlay_session_directory());
      env.add("CHROOT_FS_UNION_BRANCH_CONFIG",
        get_fs_union_branch_config());
    }
}

sbuild::chroot::session_flags
chroot_fs_union::get_session_flags () const
{
  const chroot *base = dynamic_cast<const chroot *>(this);
  assert(base != 0);

  std::string type = get_fs_union_type();
  if (base->get_run_setup_scripts() == true)
    {
      if (get_fs_union_configured())
	return chroot::SESSION_CREATE | chroot_source::get_session_flags();
      else
	return chroot::SESSION_CREATE;
    }
  else
    return chroot::SESSION_NOFLAGS;
}

void
chroot_fs_union::get_details (format_detail& detail) const
{
  chroot_source::get_details(detail);

  if (!this->overlay_session_directory.empty())
    detail.add(_("Filesystem union overlay directory"),
      get_overlay_session_directory());
  if (!this->fs_union_branch_config.empty())
    detail.add(_("Filesystem union branch config"),
      get_fs_union_branch_config());
}

void
chroot_fs_union::get_keyfile (keyfile& keyfile) const
{
  /// @todo: Remove need for this by passing in group name
  const chroot *base = dynamic_cast<const chroot *>(this);
  assert(base != 0);

  chroot_source::get_keyfile(keyfile);

  keyfile::set_object_value(*this, &chroot_fs_union::get_fs_union_type,
			    keyfile, base->get_keyfile_name(), "fs-union-type");

  keyfile::set_object_value(*this,
			    &chroot_fs_union::get_fs_union_branch_config,
			    keyfile, base->get_keyfile_name(), "fs-union-branch-config");

  if (base->get_active())
    keyfile::set_object_value(*this,
			      &chroot_fs_union::get_overlay_session_directory,
			      keyfile, base->get_keyfile_name(),
			      "fs-union-overlay-session-directory");
}

void
chroot_fs_union::set_keyfile (keyfile const& keyfile,
			      string_list&   used_keys)
{
  /// @todo: Remove need for this by passing in group name
  const chroot *base = dynamic_cast<const chroot *>(this);
  assert(base != 0);

  chroot_source::set_keyfile(keyfile, used_keys);

  keyfile::get_object_value(*this, &chroot_fs_union::set_fs_union_type,
			    keyfile, base->get_keyfile_name(), "fs-union-type",
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("fs-union-type");

  keyfile::get_object_value(*this,
			    &chroot_fs_union::set_fs_union_branch_config,
			    keyfile, base->get_keyfile_name(), "fs-union-branch-config",
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("fs-union-branch-config");

  if (base->get_active())
    keyfile::get_object_value(*this,
			      &chroot_fs_union::set_overlay_session_directory,
			      keyfile, base->get_keyfile_name(),
			      "fs-union-overlay-session-directory",
			      (get_fs_union_configured() ?
			       keyfile::PRIORITY_REQUIRED :
			       keyfile::PRIORITY_OPTIONAL));

  used_keys.push_back("fs-union-overlay-session-directory");
}

