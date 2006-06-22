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

#include "sbuild.h"

#include "dchroot-session.h"

#include <cassert>
#include <iostream>
#include <memory>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <syslog.h>

#include <boost/format.hpp>

#include <uuid/uuid.h>

using std::cout;
using std::endl;
using boost::format;
using sbuild::string_list;
using namespace dchroot;

session::session (std::string const& service,
		  config_ptr&        config,
		  operation          operation,
		  string_list const& chroots):
  sbuild::session(service, config, operation, chroots)
{
}

session::~session ()
{
}

sbuild::auth::status
session::get_chroot_auth_status (sbuild::auth::status status,
				 sbuild::chroot::ptr const& chroot) const
{
#ifndef SBUILD_DCHROOT_DSA_COMPAT
  status = change_auth(status, auth::STATUS_NONE);
#else
  /* DSA dchroot checks for a valid user in the groups list, unless
     the groups lists is empty in which case there are no
     restrictions.  This only applies if not switching users (dchroot
     does not support user switching) */

  string_list const& users = chroot->get_users();
  string_list const& groups = chroot->get_groups();

  if (this->get_ruid() == this->get_uid() &&
      users.empty() && groups.empty())
    status = change_auth(status, auth::STATUS_NONE);
  else
    status = change_auth(status,
			 sbuild::session::get_chroot_auth_status(status,
								 chroot));
#endif

  return status;
}

void
session::run_impl ()
{
  if (get_ruid() != get_uid())
    {
      format fmt(_("(%1%->%2%): dchroot sessions do not support user switching"));
      fmt % get_ruser().c_str() % get_user().c_str();
      throw error(fmt.str(), USER_SWITCH, _("dchroot session restriction"));
    }

  sbuild::session::run_impl();
}

string_list
session::get_login_directories () const
{
  string_list ret;

#ifndef SBUILD_DCHROOT_DSA_COMPAT
  // Set current working directory only if preserving environment.
  // Only change to home if not preserving the environment.
  if (!get_environment().empty())
    ret.push_back(this->sbuild::session::cwd);
  else
    ret.push_back(get_home());
#else
  ret.push_back(get_home());
#endif

  // Final fallback to root.
  if (std::find(ret.begin(), ret.end(), "/") == ret.end())
    ret.push_back("/");

  return ret;
}

string_list
session::get_command_directories () const
{
  // dchroot does not treat logins differently from commands with
  // respect to the cwd inside the chroot.
  return get_login_directories();
}

void
session::get_command (sbuild::chroot::ptr& session_chroot,
		      std::string&         file,
		      string_list&         command) const
{
  /* Run login shell */
  if (command.empty() ||
      command[0].empty()) // No command
    {
      command.clear();

      std::string shell = get_shell();
      file = shell;

      if (get_environment().empty() &&
	  session_chroot->get_command_prefix().empty())
	// Not keeping environment and can setup argv correctly; login shell
	{
	  std::string shellbase = sbuild::basename(shell, '/');
	  std::string loginshell = "-" + shellbase;
	  command.push_back(loginshell);

	  sbuild::log_debug(sbuild::DEBUG_NOTICE)
	    << format("Running login shell: %1%") % shell << endl;
	  syslog(LOG_USER|LOG_NOTICE,
		 "[%s chroot] (%s->%s) Running login shell: \"%s\"",
		 session_chroot->get_name().c_str(),
		 get_ruser().c_str(), get_user().c_str(),
		 shell.c_str());
	}
      else
	{
	  command.push_back(shell);
	  sbuild::log_debug(sbuild::DEBUG_NOTICE)
	    << format("Running shell: %1%") % shell << endl;
	  syslog(LOG_USER|LOG_NOTICE,
		 "[%s chroot] (%s->%s) Running shell: \"%s\"",
		 session_chroot->get_name().c_str(),
		 get_ruser().c_str(), get_user().c_str(),
		 shell.c_str());
	}

      if (get_verbosity() != auth::VERBOSITY_QUIET)
	{
	  std::string format_string;
	  if (get_ruid() == get_uid())
	    {
	      if (get_environment().empty() &&
		  session_chroot->get_command_prefix().empty())
		format_string = _("[%1% chroot] Running login shell: \"%4%\"");
	      else
		format_string = _("[%1% chroot] Running shell: \"%4%\"");
	    }
	  else
	    {
	      if (get_environment().empty() &&
		  session_chroot->get_command_prefix().empty())
		format_string = _("[%1% chroot] (%2%->%3%) Running login shell: \"%4%\"");
	      else
		format_string = _("[%1% chroot] (%2%->%3%) Running shell: \"%4%\"");
	    }

	  format fmt(format_string);
	  fmt % session_chroot->get_name()
	      % get_ruser() % get_user()
	      % shell;
	  sbuild::log_info() << fmt << endl;
	}
    }
  else
    {
      std::string programstring = command[0];
#ifdef SBUILD_DCHROOT_DSA_COMPAT
      file = programstring;
#else
      command.clear();
      command.push_back(get_shell());
      command.push_back("-c");
      command.push_back(programstring);
#endif

      std::string commandstring = sbuild::string_list_to_string(command, " ");
      sbuild::log_debug(sbuild::DEBUG_NOTICE)
	<< format("Running command: %1%") % commandstring << endl;
      syslog(LOG_USER|LOG_NOTICE, "[%s chroot] (%s->%s) Running command: \"%s\"",
	     session_chroot->get_name().c_str(), get_ruser().c_str(), get_user().c_str(), commandstring.c_str());

      if (get_verbosity() != auth::VERBOSITY_QUIET)
	{
	  std::string format_string;
	  format_string = (_("[%1% chroot] Running command: \"%2%\""));

	  format fmt(format_string);
	  fmt % session_chroot->get_name()
	      % programstring;
	  sbuild::log_info() << fmt << endl;
	}
    }
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
