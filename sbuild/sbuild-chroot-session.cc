/* Copyright © 2005-2009  Roger Leigh <rleigh@debian.org>
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

#include "sbuild-chroot-session.h"
#include "sbuild-format-detail.h"

#include <algorithm>

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

chroot_session::chroot_session ():
  session_manageable(true),
  session_active(false)
{
}

chroot_session::~chroot_session ()
{
}

void
chroot_session::clone_session_setup (chroot::ptr&       clone,
				     std::string const& session_id) const
{
  // Disable session, delete aliases.
  std::tr1::shared_ptr<chroot_session> session(std::tr1::dynamic_pointer_cast<chroot_session>(clone));
  if (session)
    {
      session->set_session_manageable(false);
      session->set_session_active(true);
    }
}

bool
chroot_session::get_session_manageable () const
{
  return this->session_manageable;
}

void
chroot_session::set_session_manageable (bool manageable)
{
  this->session_manageable = manageable;
}

bool
chroot_session::get_session_active () const
{
  return this->session_active;
}

void
chroot_session::set_session_active (bool active)
{
  this->session_active = active;
}

void
chroot_session::setup_env (environment& env)
{
}

sbuild::chroot::session_flags
chroot_session::get_session_flags () const
{
  /// @todo: Remove need for this.
  const chroot *base = dynamic_cast<const chroot *>(this);
  assert(base != 0);

  /// If active, support session.
  if (get_session_manageable() && !get_session_active())
    return chroot::SESSION_CREATE;
  else
    return chroot::SESSION_NOFLAGS;
}

void
chroot_session::get_details (format_detail& detail) const
{
}

void
chroot_session::get_keyfile (keyfile& keyfile) const
{
  /// @todo: Remove need for this by passing in group name
  const chroot *base = dynamic_cast<const chroot *>(this);
  assert(base != 0);
}

void
chroot_session::set_keyfile (keyfile const& keyfile,
			    string_list&   used_keys)
{
  /// @todo: Remove need for this by passing in group name
  const chroot *base = dynamic_cast<const chroot *>(this);
  assert(base != 0);
}
