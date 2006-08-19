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

#include "dchroot-dsa-session.h"

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
using namespace dchroot_dsa;

session::session (std::string const&         service,
		  config_ptr&                config,
		  operation                  operation,
		  sbuild::string_list const& chroots,
		  bool                       compat):
  dchroot::session_base(service, config, operation, chroots, compat)
{
}

session::~session ()
{
}

sbuild::auth::status
session::get_chroot_auth_status (sbuild::auth::status status,
				 sbuild::chroot::ptr const& chroot) const
{
  /* DSA dchroot checks for a valid user in the groups list, unless
     the groups lists is empty in which case there are no
     restrictions.  This only applies if not switching users (dchroot
     does not support user switching) */

  if (get_compat() == true)
    {
      sbuild::string_list const& users = chroot->get_users();
      sbuild::string_list const& groups = chroot->get_groups();

      if (this->get_ruid() == this->get_uid() &&
	  users.empty() && groups.empty())
	status = change_auth(status, auth::STATUS_NONE);
      else
	status = change_auth(status,
			     sbuild::session::get_chroot_auth_status(status,
								     chroot));
    }
  else // schroot compatibility
    {
      status = change_auth(status,
			   sbuild::session::get_chroot_auth_status(status,
								   chroot));
    }

  return status;
}

sbuild::string_list
session::get_login_directories () const
{
  sbuild::string_list ret;

  std::string const& wd(get_wd());
  if (!wd.empty())
    {
      // Set specified working directory.
      ret.push_back(wd);
    }
  else
    {
      ret.push_back(get_home());

      // Final fallback to root.
      if (std::find(ret.begin(), ret.end(), "/") == ret.end())
	ret.push_back("/");
    }

  return ret;
}

void
session::get_user_command (sbuild::chroot::ptr& session_chroot,
			   std::string&         file,
			   sbuild::string_list& command) const
{
  std::string programstring = command[0];
  file = programstring;

  if (!sbuild::is_absname(file))
    throw error(file, COMMAND_ABS);

  std::string commandstring = sbuild::string_list_to_string(command, " ");
  sbuild::log_debug(sbuild::DEBUG_NOTICE)
    << format("Running command: %1%") % commandstring << endl;
  syslog(LOG_USER|LOG_NOTICE, "[%s chroot] (%s->%s) Running command: \"%s\"",
	 session_chroot->get_name().c_str(), get_ruser().c_str(), get_user().c_str(), commandstring.c_str());

  if (get_verbosity() != auth::VERBOSITY_QUIET)
    {
      std::string format_string;
      // TRANSLATORS: %1% = chroot name
      // TRANSLATORS: %2% = command
      format_string = (_("[%1% chroot] Running command: \"%2%\""));

      format fmt(format_string);
      fmt % session_chroot->get_name()
	% programstring;
      sbuild::log_info() << fmt << endl;
    }
}
