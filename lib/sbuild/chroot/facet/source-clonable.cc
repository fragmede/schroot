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

#include <sbuild/chroot/chroot.h>
#include <sbuild/chroot/facet/factory.h>
#include <sbuild/chroot/facet/session.h>
#include <sbuild/chroot/facet/source-clonable.h>
#include <sbuild/chroot/facet/source.h>
#ifdef SBUILD_FEATURE_UNION
#include <sbuild/chroot/facet/fsunion.h>
#endif // SBUILD_FEATURE_UNION

#include <cassert>

using boost::format;
using std::endl;
using namespace sbuild;

namespace sbuild
{
  namespace chroot
  {
    namespace facet
    {

      namespace
      {

        factory::facet_info source_clonable_info =
          {
            "source-clonable",
            N_("Support for source chroot cloning"),
            []() -> facet::ptr { return source_clonable::create(); }
          };

        factory source_clonable_register(source_clonable_info);

      }

      source_clonable::source_clonable ():
        facet(),
        source_clone(true),
        source_users(),
        source_groups(),
        source_root_users(),
        source_root_groups()
      {
      }

      source_clonable::~source_clonable ()
      {
      }

      source_clonable::ptr
      source_clonable::create ()
      {
        return ptr(new source_clonable());
      }

      facet::ptr
      source_clonable::clone () const
      {
        return ptr(new source_clonable(*this));
      }

      std::string const&
      source_clonable::get_name () const
      {
        static const std::string name("source-clonable");

        return name;
      }

      void
      source_clonable::clone_source_setup (chroot const& parent,
                                           chroot::ptr&  clone) const
      {
        clone->set_description
          (clone->get_description() + ' ' + _("(source chroot)"));
        clone->set_original(false);
        clone->set_users(this->get_source_users());
        clone->set_groups(this->get_source_groups());
        clone->set_root_users(this->get_source_root_users());
        clone->set_root_groups(this->get_source_root_groups());
        clone->set_aliases(clone->get_aliases());

#ifdef SBUILD_FEATURE_UNION
        clone->remove_facet<fsunion>();
#endif // SBUILD_FEATURE_UNION

        clone->remove_facet<source_clonable>();
        clone->add_facet(source::create());
      }

      bool
      source_clonable::get_source_clone () const
      {
        return this->source_clone;
      }

      void
      source_clonable::set_source_clone (bool source_clone)
      {
        this->source_clone = source_clone;
      }

      string_list const&
      source_clonable::get_source_users () const
      {
        return this->source_users;
      }

      void
      source_clonable::set_source_users (string_list const& source_users)
      {
        this->source_users = source_users;
      }

      string_list const&
      source_clonable::get_source_groups () const
      {
        return this->source_groups;
      }

      void
      source_clonable::set_source_groups (string_list const& source_groups)
      {
        this->source_groups = source_groups;
      }

      string_list const&
      source_clonable::get_source_root_users () const
      {
        return this->source_root_users;
      }

      void
      source_clonable::set_source_root_users (string_list const& users)
      {
        this->source_root_users = users;
      }

      string_list const&
      source_clonable::get_source_root_groups () const
      {
        return this->source_root_groups;
      }

      void
      source_clonable::set_source_root_groups (string_list const& groups)
      {
        this->source_root_groups = groups;
      }

      chroot::session_flags
      source_clonable::get_session_flags (chroot const& chroot) const
      {
        // Cloning is only possible for non-source and inactive chroots.
        if (chroot.get_facet<session>())
          return chroot::SESSION_NOFLAGS;
        else
          return chroot::SESSION_CLONE;
      }

      void
      source_clonable::get_used_keys (string_list& used_keys) const
      {
        used_keys.push_back("source-clone");
        used_keys.push_back("source-users");
        used_keys.push_back("source-groups");
        used_keys.push_back("source-root-users");
        used_keys.push_back("source-root-groups");
      }

      void
      source_clonable::get_details (chroot const&  chroot,
                                    format_detail& detail) const
      {
        detail
          .add(_("Source Users"), get_source_users())
          .add(_("Source Groups"), get_source_groups())
          .add(_("Source Root Users"), get_source_root_users())
          .add(_("Source Root Groups"), get_source_root_groups());
      }

      void
      source_clonable::get_keyfile (chroot const& chroot,
                                    keyfile&      keyfile) const
      {
        keyfile::set_object_value(*this, &source_clonable::get_source_clone,
                                  keyfile, chroot.get_name(),
                                  "source-clone");

        keyfile::set_object_list_value(*this, &source_clonable::get_source_users,
                                       keyfile, chroot.get_name(),
                                       "source-users");

        keyfile::set_object_list_value(*this, &source_clonable::get_source_groups,
                                       keyfile, chroot.get_name(),
                                       "source-groups");

        keyfile::set_object_list_value(*this, &source_clonable::get_source_root_users,
                                       keyfile, chroot.get_name(),
                                       "source-root-users");

        keyfile::set_object_list_value(*this, &source_clonable::get_source_root_groups,
                                       keyfile, chroot.get_name(),
                                       "source-root-groups");
      }

      void
      source_clonable::set_keyfile (chroot&        chroot,
                                    keyfile const& keyfile)
      {
        keyfile::get_object_value(*this, &source_clonable::set_source_clone,
                                  keyfile, chroot.get_name(),
                                  "source-clone",
                                  keyfile::PRIORITY_OPTIONAL);

        keyfile::get_object_list_value(*this, &source_clonable::set_source_users,
                                       keyfile, chroot.get_name(),
                                       "source-users",
                                       keyfile::PRIORITY_OPTIONAL);

        keyfile::get_object_list_value(*this, &source_clonable::set_source_groups,
                                       keyfile, chroot.get_name(),
                                       "source-groups",
                                       keyfile::PRIORITY_OPTIONAL);

        keyfile::get_object_list_value(*this, &source_clonable::set_source_root_users,
                                       keyfile, chroot.get_name(),
                                       "source-root-users",
                                       keyfile::PRIORITY_OPTIONAL);

        keyfile::get_object_list_value(*this, &source_clonable::set_source_root_groups,
                                       keyfile, chroot.get_name(),
                                       "source-root-groups",
                                       keyfile::PRIORITY_OPTIONAL);
      }

    }
  }
}
