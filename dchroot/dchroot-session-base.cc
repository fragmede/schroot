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

#include "dchroot-session-base.h"

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

#include <unistd.h>

#include <syslog.h>

#include <boost/format.hpp>

#include <uuid/uuid.h>

using std::cout;
using std::endl;
using boost::format;
using namespace dchroot;

session_base::session_base (std::string const&  service,
			    config_ptr&         config,
			    operation           operation,
			    sbuild::string_list const& chroots,
			    bool                compat):
  sbuild::session(service, config, operation, chroots),
  compat(compat)
{
}

session_base::~session_base ()
{
}

bool
session_base::get_compat () const
{
  return this->compat;
}

void
session_base::set_compat (bool state)
{
  this->compat = state;
}

void
session_base::run_impl ()
{
  if (get_ruid() != get_uid())
    {
      format fmt(_("(%1%->%2%): dchroot sessions do not support user switching"));
      fmt % get_ruser().c_str() % get_user().c_str();
      throw error(fmt.str(), USER_SWITCH, _("dchroot session restriction"));
    }

  sbuild::session::run_impl();
}

sbuild::string_list
session_base::get_command_directories () const
{
  // dchroot does not treat logins differently from commands with
  // respect to the cwd inside the chroot.
  return get_login_directories();
}
