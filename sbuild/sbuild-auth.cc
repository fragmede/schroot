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

#include "sbuild-auth.h"
#include "sbuild-util.h"

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

#include <syslog.h>

#include <boost/format.hpp>

using std::cerr;
using std::endl;
using boost::format;
using namespace sbuild;

namespace
{

  typedef std::pair<sbuild::auth::error_code,const char *> emap;

  /**
   * This is a list of the supported error codes.  It's used to
   * construct the real error codes map.
   */
  emap init_errors[] =
    {
      emap(auth::HOSTNAME,        N_("Failed to get hostname")),
      // TRANSLATORS: %1% = user name or user ID
      emap(auth::USER,            N_("User '%1%' not found")),
      // TRANSLATORS: %1% = group name or group ID
      emap(auth::GROUP,           N_("Group '%1%' not found")),
      emap(auth::AUTHENTICATION,  N_("Authentication failed")),
      emap(auth::AUTHORISATION,   N_("Access not authorised")),
      emap(auth::PAM_DOUBLE_INIT, N_("PAM is already initialised")),
      emap(auth::PAM,             N_("PAM error")),
      emap(auth::PAM_END,         N_("PAM failed to shut down cleanly"))
    };

}

template<>
error<auth::error_code>::map_type
error<auth::error_code>::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));

auth::auth (std::string const& service_name):
  service(service_name),
  uid(0),
  gid(0),
  user(),
  command(),
  home(),
  wd(),
  shell(),
  user_environment(),
  ruid(),
  rgid(),
  ruser(),
  rgroup()
{
  this->ruid = getuid();
  this->rgid = getgid();
  passwd pwent(this->ruid);
  if (!pwent)
    {
      if (errno)
	throw error(this->ruid, USER, strerror(errno));
      else
	throw error(this->ruid, USER);
    }
  this->ruser = pwent.pw_name;

  group grent(this->rgid);
  if (!grent)
    {
      if (errno)
	throw error(this->ruid, GROUP, strerror(errno));
      else
	throw error(this->ruid, GROUP);
    }
  this->rgroup = grent.gr_name;

  /* By default, the auth user is the same as the remote user. */
  set_user(this->ruser);
}

auth::~auth ()
{
  // Shutdown PAM.
  try
    {
      stop();
    }
  catch (error const& e)
    {
      sbuild::log_exception_error(e);
    }
}

std::string const&
auth::get_service () const
{
  return this->service;
}

uid_t
auth::get_uid () const
{
  return this->uid;
}

gid_t
auth::get_gid () const
{
  return this->gid;
}

std::string const&
auth::get_user () const
{
  return this->user;
}

void
auth::set_user (std::string const& user)
{
  this->uid = getuid();
  this->gid = getgid();
  this->home = "/";
  this->shell = "/bin/false";

  this->user = user;

  passwd pwent(this->user);
  if (!pwent)
    {
      if (errno)
	throw error(user, USER, strerror(errno));
      else
	throw error(user, USER);
    }
  this->uid = pwent.pw_uid;
  this->gid = pwent.pw_gid;
  this->home = pwent.pw_dir;
  this->shell = pwent.pw_shell;
  log_debug(DEBUG_INFO)
    << format("auth uid = %1%, gid = %2%") % this->uid % this->gid
    << endl;
}

string_list const&
auth::get_command () const
{
  return this->command;
}

void
auth::set_command (string_list const& command)
{
  this->command = command;
}

std::string const&
auth::get_home () const
{
  return this->home;
}

std::string const&
auth::get_wd () const
{
  return this->wd;
}

void
auth::set_wd (std::string const& wd)
{
  this->wd = wd;
}

std::string const&
auth::get_shell () const
{
  return this->shell;
}

environment const&
auth::get_environment () const
{
  return this->user_environment;
}

void
auth::set_environment (char **environment)
{
  set_environment(sbuild::environment(environment));
}

void
auth::set_environment (environment const& environment)
{
  this->user_environment = environment;
}

environment
auth::get_minimal_environment () const
{
  environment minimal;
  if (!this->user_environment.empty())
    minimal = this->user_environment;

  // For security, PATH is always set to a sane state for root, but
  // only set in other cases if not preserving the environment.
  if (this->uid == 0)
    minimal.add(std::make_pair("PATH", "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/bin/X11"));
  else if (this->user_environment.empty())
    minimal.add(std::make_pair("PATH", "/usr/local/bin:/usr/bin:/bin:/usr/bin/X11:/usr/games"));

  if (this->user_environment.empty())
    {
      if (!this->home.empty() )
	minimal.add(std::make_pair("HOME", this->home));
      else
	minimal.add(std::make_pair("HOME", "/"));
      if (!this->user.empty())
	{
	  minimal.add(std::make_pair("LOGNAME", this->user));
	  minimal.add(std::make_pair("USER", this->user));
	}
      {
	const char *term = getenv("TERM");
	if (term)
	  minimal.add(std::make_pair("TERM", term));
      }
      if (!this->shell.empty())
	minimal.add(std::make_pair("SHELL", this->shell));
    }

  return minimal;
}

uid_t
auth::get_ruid () const
{
  return this->ruid;
}

gid_t
auth::get_rgid () const
{
  return this->rgid;
}

std::string const&
auth::get_ruser () const
{
  return this->ruser;
}

std::string const&
auth::get_rgroup () const
{
  return this->rgroup;
}

void
auth::start ()
{
}

void
auth::stop ()
{
}

void
auth::authenticate (status auth_status)
{
}

void
auth::setupenv ()
{
}

void
auth::account ()
{
}

void
auth::cred_establish ()
{
}

void
auth::cred_delete ()
{
}

void
auth::open_session ()
{
}

void
auth::close_session ()
{
}
