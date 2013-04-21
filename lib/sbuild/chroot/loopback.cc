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

#include <sbuild/chroot/loopback.h>
#include <sbuild/chroot/facet/session.h>
#include <sbuild/chroot/facet/session-clonable.h>
#include <sbuild/chroot/facet/source-clonable.h>
#include <sbuild/chroot/facet/mountable.h>
#ifdef SBUILD_FEATURE_UNION
#include <sbuild/chroot/facet/fsunion.h>
#endif // SBUILD_FEATURE_UNION
#include "format-detail.h"
#include "lock.h"
#include "util.h"

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

    loopback::loopback ():
      chroot(),
      filename()
    {
      add_facet(facet::mountable::create());
#ifdef SBUILD_FEATURE_UNION
      add_facet(facet::fsunion::create());
#endif // SBUILD_FEATURE_UNION
    }

    loopback::~loopback ()
    {
    }

    loopback::loopback (const loopback& rhs):
      chroot(rhs),
      filename(rhs.filename)
    {
    }

    chroot::chroot::ptr
    loopback::clone () const
    {
      return ptr(new loopback(*this));
    }

    chroot::chroot::ptr
    loopback::clone_session (std::string const& session_id,
                             std::string const& alias,
                             std::string const& user,
                             bool               root) const
    {
      facet::session_clonable::const_ptr psess
        (get_facet<facet::session_clonable>());
      assert(psess);

      ptr session(new loopback(*this));
      psess->clone_session_setup(*this, session, session_id, alias, user, root);

      return session;
    }

    chroot::chroot::ptr
    loopback::clone_source () const
    {
      ptr clone(new loopback(*this));

      facet::source_clonable::const_ptr psrc
        (get_facet<facet::source_clonable>());
      assert(psrc);

      psrc->clone_source_setup(*this, clone);

      return clone;
    }

    std::string const&
    loopback::get_filename () const
    {
      return this->filename;
    }

    void
    loopback::set_filename (std::string const& filename)
    {
      if (!is_absname(filename))
        throw error(filename, FILE_ABS);

      this->filename = filename;
    }

    std::string
    loopback::get_path () const
    {
      facet::mountable::const_ptr pmnt
        (get_facet<facet::mountable>());

      std::string path(get_mount_location());

      if (pmnt)
        path += pmnt->get_location();

      return path;
    }

    std::string const&
    loopback::get_chroot_type () const
    {
      static const std::string type("loopback");

      return type;
    }

    void
    loopback::setup_env (chroot const& chroot,
                         environment&  env) const
    {
      chroot::setup_env(chroot, env);

      env.add("CHROOT_FILE", get_filename());
    }

    void
    loopback::setup_lock (chroot::setup_type type,
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

      /* Create or unlink session information. */
      if ((type == SETUP_START && lock == true) ||
          (type == SETUP_STOP && lock == false && status == 0))
        {
          bool start = (type == SETUP_START);
          get_facet_strict<facet::session>()->setup_session_info(start);
        }
    }

    chroot::chroot::session_flags
    loopback::get_session_flags (chroot const& chroot) const
    {
      return SESSION_NOFLAGS;
    }

    void
    loopback::get_details (chroot const&  chroot,
                           format_detail& detail) const
    {
      chroot::get_details(chroot, detail);

      if (!this->filename.empty())
        detail.add(_("File"), get_filename());
    }

    void
    loopback::get_used_keys (string_list& used_keys) const
    {
      chroot::get_used_keys(used_keys);

      used_keys.push_back("file");
    }

    void
    loopback::get_keyfile (chroot const& chroot,
                           keyfile&      keyfile) const
    {
      chroot::get_keyfile(chroot, keyfile);

      keyfile::set_object_value(*this, &loopback::get_filename,
                                keyfile, get_name(), "file");
    }

    void
    loopback::set_keyfile (chroot&        chroot,
                           keyfile const& keyfile)
    {
      chroot::set_keyfile(chroot, keyfile);

      keyfile::get_object_value(*this, &loopback::set_filename,
                                keyfile, get_name(), "file",
                                keyfile::PRIORITY_REQUIRED);
    }

  }
}
