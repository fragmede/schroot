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

#include <sbuild/chroot/file.h>
#include <sbuild/chroot/facet/session.h>
#include <sbuild/chroot/facet/session-clonable.h>
#include <sbuild/chroot/facet/source-clonable.h>
#include "format-detail.h"
#include "lock.h"

#include <cassert>
#include <cerrno>
#include <cstring>

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

namespace sbuild
{
  namespace chroot
  {

    file::file ():
      chroot(),
      filename(),
      location(),
      repack(false)
    {
      add_facet(facet::source_clonable::create());
    }

    file::file (const file& rhs):
      chroot(rhs),
      filename(rhs.filename),
      location(rhs.location),
      repack(rhs.repack)
    {
    }

    file::~file ()
    {
    }

    chroot::chroot::ptr
    file::clone () const
    {
      return ptr(new file(*this));
    }

    chroot::chroot::ptr
    file::clone_session (std::string const& session_id,
                         std::string const& alias,
                         std::string const& user,
                         bool               root) const
    {
      facet::session_clonable::const_ptr psess
        (get_facet<facet::session_clonable>());
      assert(psess);

      ptr session(new file(*this));
      psess->clone_session_setup(*this, session, session_id, alias, user, root);

      return session;
    }

    chroot::chroot::ptr
    file::clone_source () const
    {
      file *clone_file = new file(*this);
      ptr clone(clone_file);

      facet::source_clonable::const_ptr psrc
        (get_facet<facet::source_clonable>());
      assert(psrc);

      psrc->clone_source_setup(*this, clone);
      clone_file->repack = true;

      return clone;
    }

    std::string const&
    file::get_filename () const
    {
      return this->filename;
    }

    void
    file::set_filename (std::string const& filename)
    {
      if (!is_absname(filename))
        throw error(filename, FILE_ABS);

      this->filename = filename;
    }

    std::string const&
    file::get_location () const
    {
      return this->location;
    }

    void
    file::set_location (std::string const& location)
    {
      if (!location.empty() && !is_absname(location))
        throw chroot::error(location, chroot::LOCATION_ABS);

      this->location = location;
    }

    bool
    file::get_file_repack () const
    {
      return this->repack;
    }

    void
    file::set_file_repack (bool repack)
    {
      this->repack = repack;
    }

    std::string
    file::get_path () const
    {
      std::string path(get_mount_location());

      if (!get_location().empty())
        path += get_location();

      return path;
    }

    std::string const&
    file::get_chroot_type () const
    {
      static const std::string type("file");

      return type;
    }

    void
    file::setup_env (chroot const& chroot,
                     environment&  env) const
    {
      chroot::setup_env(chroot, env);

      env.add("CHROOT_FILE", get_filename());
      env.add("CHROOT_LOCATION", get_location());
      env.add("CHROOT_FILE_REPACK", this->repack);
      env.add("CHROOT_FILE_UNPACK_DIR", SCHROOT_FILE_UNPACK_DIR);
    }

    void
    file::setup_lock (chroot::setup_type type,
                      bool               lock,
                      int                status)
    {
      // Check ownership and permissions.
      if (type == SETUP_START && lock == true)
        {
          stat file_status(this->filename);

          // NOTE: taken from chroot_config::check_security.
          if (file_status.uid() != 0)
            throw error(this->filename, FILE_OWNER);
          if (file_status.check_mode(stat::PERM_OTHER_WRITE))
            throw error(this->filename, FILE_PERMS);
          if (!file_status.is_regular())
            throw error(this->filename, FILE_NOTREG);
        }

      /* By default, file chroots do no locking. */
      /* Create or unlink session information. */
      if ((type == SETUP_START && lock == true) ||
          (type == SETUP_STOP && lock == false && status == 0))
        {

          bool start = (type == SETUP_START);
          get_facet_strict<facet::session>()->setup_session_info(start);
        }
    }

    chroot::chroot::session_flags
    file::get_session_flags (chroot const& chroot) const
    {
      session_flags flags = SESSION_NOFLAGS;

      if (chroot.get_facet<facet::session>())
        flags = SESSION_PURGE;

      return flags;
    }

    void
    file::get_details (chroot const&  chroot,
                       format_detail& detail) const
    {
      chroot::get_details(chroot, detail);

      if (!this->filename.empty())
        detail
          .add(_("File"), get_filename())
          .add(_("File Repack"), this->repack);
      if (!get_location().empty())
        detail.add(_("Location"), get_location());
    }

    void
    file::get_used_keys (string_list& used_keys) const
    {
      chroot::get_used_keys(used_keys);

      used_keys.push_back("file");
      used_keys.push_back("location");
      used_keys.push_back("file-repack");
    }

    void
    file::get_keyfile (chroot const& chroot,
                       keyfile&      keyfile) const
    {
      chroot::get_keyfile(chroot, keyfile);

      bool session = static_cast<bool>(get_facet<facet::session>());

      keyfile::set_object_value(*this, &file::get_filename,
                                keyfile, get_name(), "file");

      keyfile::set_object_value(*this, &file::get_location,
                                keyfile, chroot.get_name(),
                                "location");

      if (session)
        keyfile::set_object_value(*this, &file::get_file_repack,
                                  keyfile, get_name(), "file-repack");
    }

    void
    file::set_keyfile (chroot&        chroot,
                       keyfile const& keyfile)
    {
      chroot::set_keyfile(chroot, keyfile);

      bool session = static_cast<bool>(get_facet<facet::session>());

      keyfile::get_object_value(*this, &file::set_filename,
                                keyfile, get_name(), "file",
                                keyfile::PRIORITY_REQUIRED);

      keyfile::get_object_value(*this, &file::set_location,
                                keyfile, chroot.get_name(),
                                "location",
                                keyfile::PRIORITY_OPTIONAL);

      keyfile::get_object_value(*this, &file::set_file_repack,
                                keyfile, get_name(), "file-repack",
                                session ?
                                keyfile::PRIORITY_REQUIRED :
                                keyfile::PRIORITY_DISALLOWED);
    }

  }
}
