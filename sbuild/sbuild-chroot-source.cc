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

#include "sbuild-chroot-source.h"
#include "sbuild-format-detail.h"

#include <algorithm>

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

chroot_source::chroot_source ()
{
}

chroot_source::~chroot_source ()
{
}

void
chroot_source::clone_source_setup (chroot::ptr& clone) const
{
  clone->set_name(clone->get_name() + "-source");
  clone->set_description
    (clone->get_description() + ' ' + _("(source chroot)"));
  clone->set_original(false);
  clone->set_users(this->get_source_users());
  clone->set_groups(this->get_source_groups());
  clone->set_root_users(this->get_source_root_users());
  clone->set_root_groups(this->get_source_root_groups());
  string_list const& aliases = clone->get_aliases();
  string_list source_aliases;
  for (string_list::const_iterator alias = aliases.begin();
       alias != aliases.end();
       ++alias)
    source_aliases.push_back(*alias + "-source");
  clone->set_aliases(source_aliases);

  std::tr1::shared_ptr<chroot_source> source(std::tr1::dynamic_pointer_cast<chroot_source>(clone));
  if (source)
    source->set_source(true);
}

void
chroot_source::setup_env (environment& env)
{
}

sbuild::chroot::session_flags
chroot_source::get_session_flags () const
{
  /// @todo: Remove need for this.
  const chroot *base = dynamic_cast<const chroot *>(this);
  assert(base != 0);

  // Cloning is only possible for non-source and inactive chroots.
  if (this->get_source() || base->get_active())
    return chroot::SESSION_NOFLAGS;
  else
    return chroot::SESSION_CLONE;
}

void
chroot_source::get_details (format_detail& detail) const
{
  if (!this->get_source())
    detail
      .add(_("Source Users"), get_source_users())
      .add(_("Source Groups"), get_source_groups())
      .add(_("Source Root Users"), get_source_root_users())
      .add(_("Source Root Groups"), get_source_root_groups());
}

void
chroot_source::get_keyfile (keyfile& keyfile) const
{
  /// @todo: Remove need for this by passing in group name
  const chroot *base = dynamic_cast<const chroot *>(this);
  assert(base != 0);

  if (!this->get_source())
    {
      keyfile::set_object_list_value(*this, &chroot_source::get_source_users,
				     keyfile, base->get_keyfile_name(), "source-users");

      keyfile::set_object_list_value(*this, &chroot_source::get_source_groups,
				     keyfile, base->get_keyfile_name(), "source-groups");

      keyfile::set_object_list_value(*this, &chroot_source::get_source_root_users,
				     keyfile, base->get_keyfile_name(), "source-root-users");

      keyfile::set_object_list_value(*this, &chroot_source::get_source_root_groups,
				     keyfile, base->get_keyfile_name(), "source-root-groups");
    }
}

void
chroot_source::set_keyfile (keyfile const& keyfile,
			    string_list&   used_keys)
{
  /// @todo: Remove need for this by passing in group name
  const chroot *base = dynamic_cast<const chroot *>(this);
  assert(base != 0);

  if (!this->get_source())
    {
      keyfile::get_object_list_value(*this, &chroot_source::set_source_users,
				     keyfile, base->get_keyfile_name(), "source-users",
				     keyfile::PRIORITY_OPTIONAL);
      used_keys.push_back("source-users");

      keyfile::get_object_list_value(*this, &chroot_source::set_source_groups,
				     keyfile, base->get_keyfile_name(), "source-groups",
				     keyfile::PRIORITY_OPTIONAL);
      used_keys.push_back("source-groups");

      keyfile::get_object_list_value(*this, &chroot_source::set_source_root_users,
				     keyfile, base->get_keyfile_name(), "source-root-users",
				     keyfile::PRIORITY_OPTIONAL);
      used_keys.push_back("source-root-users");

      keyfile::get_object_list_value(*this, &chroot_source::set_source_root_groups,
				     keyfile, base->get_keyfile_name(), "source-root-groups",
				     keyfile::PRIORITY_OPTIONAL);
      used_keys.push_back("source-root-groups");
    }
}
