/* Copyright © 2005-2006  Roger Leigh <rleigh@debian.org>
 *
 * schroot is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * schroot is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA  02111-1307  USA
 *
 *********************************************************************/

#include <config.h>

#include "sbuild-chroot-source.h"
#include "sbuild-format-detail.h"
#include "sbuild-lock.h"

#include <algorithm>

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

chroot_source::chroot_source ():
  chroot()
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
}

string_list const&
chroot_source::get_source_users () const
{
  return this->source_users;
}

void
chroot_source::set_source_users (string_list const& source_users)
{
  this->source_users = source_users;
}

string_list const&
chroot_source::get_source_groups () const
{
  return this->source_groups;
}

void
chroot_source::set_source_groups (string_list const& source_groups)
{
  this->source_groups = source_groups;
}

string_list const&
chroot_source::get_source_root_users () const
{
  return this->source_root_users;
}

void
chroot_source::set_source_root_users (string_list const& users)
{
  this->source_root_users = users;
}

string_list const&
chroot_source::get_source_root_groups () const
{
  return this->source_root_groups;
}

void
chroot_source::set_source_root_groups (string_list const& groups)
{
  this->source_root_groups = groups;
}

void
chroot_source::setup_env (environment& env)
{
}

void
chroot_source::get_details (format_detail& detail) const
{
  detail
    .add(_("Source Users"), get_source_users())
    .add(_("Source Groups"), get_source_groups())
    .add(_("Source Root Users"), get_source_root_users())
    .add(_("Source Root Groups"), get_source_root_groups());
}

void
chroot_source::get_keyfile (keyfile& keyfile) const
{
  keyfile::set_object_list_value(*this, &chroot_source::get_source_users,
				 keyfile, get_name(), "source-users");

  keyfile::set_object_list_value(*this, &chroot_source::get_source_groups,
				 keyfile, get_name(), "source-groups");

  keyfile::set_object_list_value(*this, &chroot_source::get_source_root_users,
				 keyfile, get_name(), "source-root-users");

  keyfile::set_object_list_value(*this, &chroot_source::get_source_root_groups,
				 keyfile, get_name(), "source-root-groups");
}

void
chroot_source::set_keyfile (keyfile const& keyfile)
{
  keyfile::get_object_list_value(*this, &chroot_source::set_source_users,
				 keyfile, get_name(), "source-users",
				 keyfile::PRIORITY_OPTIONAL);

  keyfile::get_object_list_value(*this, &chroot_source::set_source_groups,
				 keyfile, get_name(), "source-groups",
				 keyfile::PRIORITY_OPTIONAL);

  keyfile::get_object_list_value(*this, &chroot_source::set_source_root_users,
				 keyfile, get_name(), "source-root-users",
				 keyfile::PRIORITY_OPTIONAL);

  keyfile::get_object_list_value(*this, &chroot_source::set_source_root_groups,
				 keyfile, get_name(), "source-root-groups",
				 keyfile::PRIORITY_OPTIONAL);
}
