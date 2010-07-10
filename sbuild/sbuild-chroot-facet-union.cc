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

#include "sbuild-chroot.h"
#include "sbuild-chroot-facet-session.h"
#include "sbuild-chroot-facet-union.h"
#include "sbuild-chroot-facet-source-clonable.h"

#include <cassert>

using boost::format;
using std::endl;
using namespace sbuild;

namespace
{
  typedef std::pair<chroot_facet_union::error_code,const char *> emap;

  /**
   * This is a list of the supported error codes.  It's used to
   * construct the real error codes map.
   */
  emap init_errors[] =
    {
      // TRANSLATORS: %1% = chroot fs type
      emap(chroot_facet_union::UNION_TYPE_UNKNOWN, N_("Unknown filesystem union type '%1%'")),
      emap(chroot_facet_union::UNION_OVERLAY_ABS,  N_("Union overlay must have an absolute path")),
      emap(chroot_facet_union::UNION_UNDERLAY_ABS, N_("Union underlay must have an absolute path"))
    };
}

template<>
error<chroot_facet_union::error_code>::map_type
error<chroot_facet_union::error_code>::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));

chroot_facet_union::chroot_facet_union ():
  chroot_facet(),
  union_type("none"),
  union_overlay_directory(SCHROOT_OVERLAY_DIR),
  union_underlay_directory(SCHROOT_UNDERLAY_DIR)
{
}

chroot_facet_union::~chroot_facet_union ()
{
}

chroot_facet_union::ptr
chroot_facet_union::create ()
{
  return ptr(new chroot_facet_union());
}

chroot_facet::ptr
chroot_facet_union::clone () const
{
  return ptr(new chroot_facet_union(*this));
}

std::string const&
chroot_facet_union::get_name () const
{
  static const std::string name("union");

  return name;
}

void
chroot_facet_union::clone_source_setup (chroot::ptr& clone) const
{
  const chroot *base = dynamic_cast<const chroot *>(this->owner);
  assert(base);

  chroot_facet_source_clonable::const_ptr psrc
    (base->get_facet<chroot_facet_source_clonable>());
  if (psrc)
    psrc->clone_source_setup(clone);

  chroot_facet_union::ptr puni
    (clone->get_facet<chroot_facet_union>());
  if (puni)
    puni->set_union_type("none");
}

bool
chroot_facet_union::get_union_configured () const
{
  return get_union_type() != "none";
}

std::string const&
chroot_facet_union::get_union_overlay_directory () const
{
  return this->union_overlay_directory;
}

void
chroot_facet_union::set_union_overlay_directory
(std::string const& directory)
{
  if (!is_absname(directory))
    throw error(directory, UNION_OVERLAY_ABS);

  this->union_overlay_directory = directory;
}

std::string const&
chroot_facet_union::get_union_underlay_directory () const
{
  return this->union_underlay_directory;
}

void
chroot_facet_union::set_union_underlay_directory
(std::string const& directory)
{
  if (!is_absname(directory))
    throw error(directory, UNION_UNDERLAY_ABS);

  this->union_underlay_directory = directory;
}

std::string const&
chroot_facet_union::get_union_type () const
{
  return this->union_type;
}

void
chroot_facet_union::set_union_type (std::string const& type)
{
  if (type == "aufs" ||
      type == "unionfs" ||
      type == "none")
    this->union_type = type;
  else
    throw error(type, UNION_TYPE_UNKNOWN);

  chroot *base = dynamic_cast<chroot *>(this->owner);
  assert(base);

  if (this->union_type != "none")
    {
      if (!base->get_facet<chroot_facet_source_clonable>())
	base->add_facet(chroot_facet_source_clonable::create());
    }
  else
    base->remove_facet<chroot_facet_source_clonable>();
}

std::string const&
chroot_facet_union::get_union_mount_options () const
{
  return union_mount_options;
}

void
chroot_facet_union::set_union_mount_options
(std::string const& union_mount_options)
{
  this->union_mount_options = union_mount_options;
}

void
chroot_facet_union::setup_env (chroot const& chroot,
			       environment&  env) const
{
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
chroot_facet_union::get_session_flags (chroot const& chroot) const
{
  sbuild::chroot::session_flags flags = sbuild::chroot::SESSION_NOFLAGS;

  if (get_union_configured() && chroot.get_facet<chroot_facet_session>())
    flags = sbuild::chroot::SESSION_PURGE;

  return flags;
}

void
chroot_facet_union::get_details (chroot const& chroot,
				 format_detail& detail) const
{
  detail.add(_("Filesystem Union Type"), get_union_type());
  if (get_union_configured())
    {
      if (!this->union_mount_options.empty())
	detail.add(_("Filesystem Union Mount Options"),
		   get_union_mount_options());
      if (!this->union_overlay_directory.empty())
	detail.add(_("Filesystem Union Overlay Directory"),
		   get_union_overlay_directory());
      if (!this->union_underlay_directory.empty())
	detail.add(_("Filesystem Union Underlay Directory"),
		   get_union_underlay_directory());
    }
}

void
chroot_facet_union::get_keyfile (chroot const& chroot,
				 keyfile&      keyfile) const
{
  keyfile::set_object_value(*this, &chroot_facet_union::get_union_type,
			    keyfile, chroot.get_keyfile_name(), "union-type");

  if (get_union_configured())
    {
      keyfile::set_object_value(*this,
				&chroot_facet_union::get_union_mount_options,
				keyfile, chroot.get_keyfile_name(),
				"union-mount-options");

      keyfile::set_object_value(*this,
				&chroot_facet_union::get_union_overlay_directory,
				keyfile, chroot.get_keyfile_name(),
				"union-overlay-directory");

      keyfile::set_object_value(*this,
				&chroot_facet_union::get_union_underlay_directory,
				keyfile, chroot.get_keyfile_name(),
				"union-underlay-directory");
    }
}

void
chroot_facet_union::set_keyfile (chroot&        chroot,
				 keyfile const& keyfile,
				 string_list&   used_keys)
{
  bool session = static_cast<bool>(chroot.get_facet<chroot_facet_session>());

  keyfile::get_object_value(*this, &chroot_facet_union::set_union_type,
			    keyfile, chroot.get_keyfile_name(), "union-type",
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("union-type");

  // If we are a union, add specific source options here.
  chroot_facet_source_clonable::ptr psrc
    (chroot.get_facet<chroot_facet_source_clonable>());
  if (psrc)
    psrc->set_keyfile(chroot, keyfile, used_keys);

  keyfile::get_object_value(*this,
			    &chroot_facet_union::set_union_mount_options,
			    keyfile, chroot.get_keyfile_name(), "union-mount-options",
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("union-mount-options");

  keyfile::get_object_value(*this,
			    &chroot_facet_union::set_union_overlay_directory,
			    keyfile, chroot.get_keyfile_name(),
			    "union-overlay-directory",
			    (session && get_union_configured())?
			    keyfile::PRIORITY_REQUIRED :
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("union-overlay-directory");

  keyfile::get_object_value(*this,
			    &chroot_facet_union::set_union_underlay_directory,
			    keyfile, chroot.get_keyfile_name(),
			    "union-underlay-directory",
			    (session && get_union_configured())?
			    keyfile::PRIORITY_REQUIRED :
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("union-underlay-directory");
}
