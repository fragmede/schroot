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

#include <sbuild/chroot/facet/factory.h>
#include <sbuild/chroot/facet/mountable.h>
#include <sbuild/chroot/facet/session-clonable.h>
#include <sbuild/chroot/facet/session.h>
#include <sbuild/chroot/facet/source-clonable.h>
#include "format-detail.h"

#include <cassert>
#include <cerrno>

#include <boost/format.hpp>

using std::endl;
using boost::format;
using namespace sbuild;

namespace sbuild
{
  namespace chroot
  {
    namespace facet
    {

      custom::custom ():
        storage(),
        purgeable(false)
      {
      }

      custom::custom (const custom& rhs):
        storage(rhs),
        purgeable(rhs.purgeable)
      {
      }

      custom::~custom ()
      {
      }

      std::string const&
      custom::get_name () const
      {
        static const std::string name("custom");

        return name;
      }

      custom::ptr
      custom::create ()
      {
        return ptr(new custom());
      }

      facet::ptr
      custom::clone () const
      {
        return ptr(new custom(*this));
      }

      void
      custom::set_session_cloneable (bool cloneable)
      {
        if (cloneable)
          owner->add_facet(session_clonable::create());
        else
          owner->remove_facet<session_clonable>();
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
          owner->add_facet(source_clonable::create());
        else
          owner->remove_facet<source_clonable>();
      }

      std::string
      custom::get_path () const
      {
        // TODO: Allow customisation?  Or require use of mount location?
        return owner->get_mount_location();
      }

      void
      custom::setup_env (chroot const& chroot,
                         environment& env) const
      {
        storage::setup_env(chroot, env);
      }

      void
      custom::setup_lock (chroot::setup_type type,
                          bool               lock,
                          int                status)
      {
        /* By default, custom chroots do no locking. */
        /* Create or unlink session information. */
        if ((type == chroot::SETUP_START && lock == true) ||
            (type == chroot::SETUP_STOP && lock == false && status == 0))
          {

            bool start = (type == chroot::SETUP_START);
            owner->get_facet_strict<session>()->setup_session_info(start);
          }
      }

      chroot::session_flags
      custom::get_session_flags (chroot const& chroot) const
      {
        chroot::session_flags flags = chroot::SESSION_NOFLAGS;

        // TODO: Only set if purge is set.

        if (chroot.get_facet<session>() &&
            get_session_purgeable())
          flags = chroot::SESSION_PURGE;

        return flags;
      }

      void
      custom::get_details (chroot const&  chroot,
                           format_detail& detail) const
      {
        storage::get_details(chroot, detail);
      }

      void
      custom::get_used_keys (string_list& used_keys) const
      {
        storage::get_used_keys(used_keys);

        used_keys.push_back("custom-cloneable");
        used_keys.push_back("custom-purgeable");
        used_keys.push_back("custom-source-cloneable");
      }

      void
      custom::get_keyfile (chroot const& chroot,
                           keyfile&      keyfile) const
      {
        storage::get_keyfile(chroot, keyfile);

        keyfile::set_object_value(*this,
                                  &custom::get_session_purgeable,
                                  keyfile, chroot.get_name(),
                                  "custom-session-purgeable");
      }

      void
      custom::set_keyfile (chroot& chroot,
                           keyfile const& keyfile)
      {
        storage::set_keyfile(chroot, keyfile);

        bool is_session = static_cast<bool>(chroot.get_facet<session>());

        keyfile::get_object_value(*this, &custom::set_session_cloneable,
                                  keyfile, chroot.get_name(), "custom-session-cloneable",
                                  is_session ?
                                  keyfile::PRIORITY_DISALLOWED :
                                  keyfile::PRIORITY_OPTIONAL);

        keyfile::get_object_value(*this, &custom::set_session_purgeable,
                                  keyfile, chroot.get_name(), "custom-session-purgeable",
                                  keyfile::PRIORITY_OPTIONAL);

        keyfile::get_object_value(*this, &custom::set_source_cloneable,
                                  keyfile, chroot.get_name(), "custom-source-cloneable",
                                  is_session ?
                                  keyfile::PRIORITY_DISALLOWED :
                                  keyfile::PRIORITY_OPTIONAL);
      }
    }
  }
}
