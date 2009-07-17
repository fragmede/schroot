/* Copyright © 2008-2009  Jan-Marek Glogowski <glogow@fbihome.de>
 * Copyright © 2009       Roger Leigh <rleigh@debian.org>
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

#include "sbuild-chroot-union.h"

#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

using namespace sbuild;

namespace
{
  typedef std::pair<chroot_union::error_code,const char *> emap;

  /**
   * This is a list of the supported error codes.  It's used to
   * construct the real error codes map.
   */
  emap init_errors[] =
    {
      // TRANSLATORS: %1% = chroot fs type
      emap(chroot_union::UNION_TYPE_UNKNOWN, N_("Unknown filesystem union type '%1%'"))
    };
}

template<>
error<chroot_union::error_code>::map_type
error<chroot_union::error_code>::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));

chroot_union::chroot_union ():
  chroot_session(),
  chroot_source(),
  union_type("none"),
  union_overlay_directory(SCHROOT_OVERLAY_DIR),
  union_underlay_directory(SCHROOT_UNDERLAY_DIR)
{
}

chroot_union::~chroot_union ()
{
}

void
chroot_union::clone_source_setup (chroot::ptr& clone) const
{
  chroot_source::clone_source_setup(clone);

  std::tr1::shared_ptr<sbuild::chroot_union> fsunion =
    std::tr1::dynamic_pointer_cast<sbuild::chroot_union>(clone);
  if (fsunion)
    fsunion->set_union_type("none");
}

bool
chroot_union::get_source_clonable () const
{
  return chroot_source::get_source_clonable() &&
    get_union_configured();
}

bool
chroot_union::get_union_configured () const
{
  return get_union_type() != "none";
}

std::string const&
chroot_union::get_union_overlay_directory () const
{
  return this->union_overlay_directory;
}

void
chroot_union::set_union_overlay_directory
(std::string const& directory)
{
  if (!is_absname(union_overlay_directory))
    throw chroot::error(union_overlay_directory, chroot::LOCATION_ABS);

  this->union_overlay_directory = directory;
}

std::string const&
chroot_union::get_union_underlay_directory () const
{
  return this->union_underlay_directory;
}

void
chroot_union::set_union_underlay_directory
(std::string const& directory)
{
  if (!is_absname(union_underlay_directory))
    throw chroot::error(union_underlay_directory, chroot::LOCATION_ABS);

  this->union_underlay_directory = directory;
}

std::string const&
chroot_union::get_union_type () const
{
  return this->union_type;
}

void
chroot_union::set_union_type (std::string const& type)
{
  if (type == "aufs" ||
      type == "unionfs" ||
      type == "none")
    this->union_type = type;
  else
    throw error(type, UNION_TYPE_UNKNOWN);

  // If union not enabled, don't implement source interface.
  set_source_clonable(this->union_type != "none");
}

std::string const&
chroot_union::get_union_mount_options () const
{
  return union_mount_options;
}

void
chroot_union::set_union_mount_options
(std::string const& union_mount_options)
{
  this->union_mount_options = union_mount_options;
}

void
chroot_union::setup_env (environment& env)
{
  chroot_session::setup_env(env);
  chroot_source::setup_env(env);

  env.add("CHROOT_UNION_TYPE", get_union_type());
  if (get_union_configured())
    {
      env.add("CHROOT_UNION_MOUNT_OPTIONS",
	      get_union_mount_options());
      env.add("CHROOT_UNION_OVERLAY_DIRECTORY",
	      get_union_overlay_directory());
      env.add("CHROOT_UNION_UNDERLAY_DIRECTORY",
	      get_union_underlay_directory());
    }
}

sbuild::chroot::session_flags
chroot_union::get_session_flags () const
{
  /// @todo: Remove need for this by passing in group name
  const chroot *base = dynamic_cast<const chroot *>(this);
  assert(base != 0);

  sbuild::chroot::session_flags flags = sbuild::chroot::SESSION_NOFLAGS;

  if (get_union_configured())
    {
      flags = chroot_session::get_session_flags() |
	chroot_source::get_session_flags();
      if (base->get_active())
	flags = flags | sbuild::chroot::SESSION_PURGE;
    }

  return flags;
}

void
chroot_union::get_details (format_detail& detail) const
{
  chroot_session::get_details(detail);
  chroot_source::get_details(detail);

  detail.add(_("Filesystem union type"), get_union_type());
  if (get_union_configured())
    {
      if (!this->union_mount_options.empty())
	detail.add(_("Filesystem union mount options"),
		   get_union_mount_options());
      if (!this->union_overlay_directory.empty())
	detail.add(_("Filesystem union overlay directory"),
		   get_union_overlay_directory());
      if (!this->union_underlay_directory.empty())
	detail.add(_("Filesystem union underlay directory"),
		   get_union_underlay_directory());
    }
}

void
chroot_union::get_keyfile (keyfile& keyfile) const
{
  /// @todo: Remove need for this by passing in group name
  const chroot *base = dynamic_cast<const chroot *>(this);
  assert(base != 0);

  chroot_session::get_keyfile(keyfile);
  chroot_source::get_keyfile(keyfile);

  keyfile::set_object_value(*this, &chroot_union::get_union_type,
			    keyfile, base->get_keyfile_name(), "union-type");

  if (get_union_configured())
    {
      keyfile::set_object_value(*this,
				&chroot_union::get_union_mount_options,
				keyfile, base->get_keyfile_name(),
				"union-mount-options");

      keyfile::set_object_value(*this,
				&chroot_union::get_union_overlay_directory,
				keyfile, base->get_keyfile_name(),
				"union-overlay-directory");

      keyfile::set_object_value(*this,
				&chroot_union::get_union_underlay_directory,
				keyfile, base->get_keyfile_name(),
				"union-underlay-directory");
    }
}

void
chroot_union::set_keyfile (keyfile const& keyfile,
			   string_list&   used_keys)
{
  /// @todo: Remove need for this by passing in group name
  const chroot *base = dynamic_cast<const chroot *>(this);
  assert(base != 0);

  chroot_session::set_keyfile(keyfile, used_keys);
  chroot_source::set_keyfile(keyfile, used_keys);

  keyfile::get_object_value(*this, &chroot_union::set_union_type,
			    keyfile, base->get_keyfile_name(), "union-type",
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("union-type");

  keyfile::get_object_value(*this,
			    &chroot_union::set_union_mount_options,
			    keyfile, base->get_keyfile_name(), "union-mount-options",
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("union-mount-options");

  keyfile::get_object_value(*this,
			    &chroot_union::set_union_overlay_directory,
			    keyfile, base->get_keyfile_name(),
			    "union-overlay-directory",
			    (base->get_active() && get_union_configured())?
			    keyfile::PRIORITY_REQUIRED :
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("union-overlay-directory");

  keyfile::get_object_value(*this,
			    &chroot_union::set_union_underlay_directory,
			    keyfile, base->get_keyfile_name(),
			    "union-underlay-directory",
			    (base->get_active() && get_union_configured())?
			    keyfile::PRIORITY_REQUIRED :
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("union-underlay-directory");
}
