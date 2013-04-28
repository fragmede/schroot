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

#include <sbuild/chroot/custom.h>
#include <sbuild/chroot/facet/factory.h>
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

      namespace
      {

        factory::facet_info custom_info =
          {
            "custom",
            N_("Support for ‘custom’ chroots"),
            []() -> facet::ptr { return custom::create(); }
          };

        factory custom_register(custom_info);

      }

    custom::custom ():
      chroot(),
      purgeable(false)
    {
    }

    custom::custom (const custom& rhs):
      chroot(rhs),
      purgeable(false)
    {
    }

    custom::~custom ()
    {
    }

    chroot::chroot::ptr
    custom::clone () const
    {
      return ptr(new custom(*this));
    }

    chroot::chroot::ptr
    custom::clone_session (std::string const& session_id,
                           std::string const& alias,
                           std::string const& user,
                           bool               root) const
    {
      facet::session_clonable::const_ptr psess
        (get_facet<facet::session_clonable>());
      assert(psess);

      ptr session(new custom(*this));
      psess->clone_session_setup(*this, session, session_id, alias, user, root);

      return session;
    }

    chroot::chroot::ptr
    custom::clone_source () const
    {
      custom *clone_custom = new custom(*this);
      ptr clone(clone_custom);

      facet::source_clonable::const_ptr psrc
        (get_facet<facet::source_clonable>());
      assert(psrc);

      psrc->clone_source_setup(*this, clone);

      return clone;
    }

    void
    custom::set_session_cloneable (bool cloneable)
    {
      if (cloneable)
        add_facet(facet::session_clonable::create());
      else
        remove_facet<facet::session_clonable>();
    }

    void
    custom::set_session_purgeable (bool purgeable)
    {
      this->purgeable = purgeable;
    }

    bool
    custom::get_session_purgeable () const
    {
      return this->purgeable;
    }

    void
    custom::set_source_cloneable (bool cloneable)
    {
      if (cloneable)
        add_facet(facet::source_clonable::create());
      else
        remove_facet<facet::source_clonable>();
    }

    std::string
    custom::get_path () const
    {
      // TODO: Allow customisation?  Or require use of mount location?
      return get_mount_location();
    }

    void
    custom::setup_env (chroot const& chroot,
                       environment& env) const
    {
      chroot::setup_env(chroot, env);
    }

    std::string const&
    custom::get_chroot_type () const
    {
      static const std::string type("custom");

      return type;
    }

    void
    custom::setup_lock (chroot::setup_type type,
                        bool               lock,
                        int                status)
    {
      /* By default, custom chroots do no locking. */
      /* Create or unlink session information. */
      if ((type == SETUP_START && lock == true) ||
          (type == SETUP_STOP && lock == false && status == 0))
        {

          bool start = (type == SETUP_START);
          setup_session_info(start);
        }
    }

    chroot::chroot::session_flags
    custom::get_session_flags (chroot const& chroot) const
    {
      session_flags flags = SESSION_NOFLAGS;

      // TODO: Only set if purge is set.

      if (chroot.get_facet<facet::session>() &&
          get_session_purgeable())
        flags = SESSION_PURGE;

      return flags;
    }

    void
    custom::get_details (chroot const&  chroot,
                         format_detail& detail) const
    {
      chroot::get_details(chroot, detail);
    }

    void
    custom::get_used_keys (string_list& used_keys) const
    {
      chroot::get_used_keys(used_keys);

      used_keys.push_back("custom-cloneable");
      used_keys.push_back("custom-purgeable");
      used_keys.push_back("custom-source-cloneable");
    }

    void
    custom::get_keyfile (chroot const& chroot,
                         keyfile&      keyfile) const
    {
      chroot::get_keyfile(chroot, keyfile);

      keyfile::set_object_value(*this,
                                &custom::get_session_purgeable,
                                keyfile, get_name(),
                                "custom-session-purgeable");
    }

    void
    custom::set_keyfile (chroot& chroot,
                         keyfile const& keyfile)
    {
      chroot::set_keyfile(chroot, keyfile);

      bool session = static_cast<bool>(get_facet<facet::session>());

      keyfile::get_object_value(*this, &custom::set_session_cloneable,
                                keyfile, get_name(), "custom-session-cloneable",
                                session ?
                                keyfile::PRIORITY_DISALLOWED :
                                keyfile::PRIORITY_OPTIONAL);

      keyfile::get_object_value(*this, &custom::set_session_purgeable,
                                keyfile, get_name(), "custom-session-purgeable",
                                keyfile::PRIORITY_OPTIONAL);

      keyfile::get_object_value(*this, &custom::set_source_cloneable,
                                keyfile, get_name(), "custom-source-cloneable",
                                session ?
                                keyfile::PRIORITY_DISALLOWED :
                                keyfile::PRIORITY_OPTIONAL);
    }

  }
}
