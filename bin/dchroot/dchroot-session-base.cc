/* Copyright © 2005-2007  Roger Leigh <rleigh@debian.org>
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

using std::cout;
using std::endl;
using sbuild::_;
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
  if (get_auth()->get_ruid() != get_auth()->get_uid())
    throw error(get_auth()->get_ruser(), get_auth()->get_user(), USER_SWITCH,
		_("dchroot session restriction"));

  sbuild::session::run_impl();
}

sbuild::string_list
session_base::get_command_directories () const
{
  // dchroot does not treat logins differently from commands with
  // respect to the cwd inside the chroot.
  return get_login_directories();
}
