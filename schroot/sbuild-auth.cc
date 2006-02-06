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

#include <cassert>
#include <iostream>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <syslog.h>

#include <boost/format.hpp>

using std::cerr;
using std::endl;
using boost::format;
using namespace sbuild;

namespace
{

  /* This is the glue to link PAM user interaction with SbuildAuthConv. */
  static int
  auth_conv (int                        num_msg,
	     const struct pam_message **msgm,
	     struct pam_response      **response,
	     void                      *appdata_ptr)
  {
    if (appdata_ptr == 0)
      return PAM_CONV_ERR;

    AuthConv *conv = static_cast<AuthConv *>(appdata_ptr);
    assert (conv != 0);

    /* Construct a message vector */
    AuthConv::message_list messages;
    for (int i = 0; i < num_msg; ++i)
      {
	const struct pam_message *source = msgm[i];

	auth_message
	  message(static_cast<auth_message::type>(source->msg_style),
		  source->msg);
	messages.push_back(message);
      }

    /* Do the conversation */
    bool status = conv->conversation(messages);

    if (status == true)
      {
	/* Copy response into **reponse */
	struct pam_response *reply =
	  static_cast<struct pam_response *>
	  (malloc(sizeof(struct pam_response) * num_msg));

	for (int i = 0; i < num_msg; ++i)
	  {
	    reply[i].resp_retcode = 0;
	    reply[i].resp = strdup(messages[i].response.c_str());
	  }

	*response = reply;
	reply = 0;

	return PAM_SUCCESS;
      }
    else
      return PAM_CONV_ERR;
  }

}


auth::auth(std::string const& service_name):
  pam(),
  service(service_name),
  uid(0),
  gid(0),
  user(),
  command(),
  home(),
  shell(),
  user_environment(),
  ruid(),
  ruser(),
  conv(dynamic_cast<AuthConv *>(new AuthConvTty)),
  message_verbosity(VERBOSITY_NORMAL)
{
  this->ruid = getuid();
  struct passwd *pwent = getpwuid(this->ruid);
  if (pwent == 0)
    {
      format fmt(_("%1%: user not found: %2%"));
      fmt % this->ruid% strerror(errno);
      throw error(fmt);
    }
  this->ruser = pwent->pw_name;

  /* By default, the auth user is the same as the remote user. */
  set_user(this->ruser);
}

auth::~auth()
{
  // Shutdown PAM.
  try
    {
      stop();
    }
  catch (...)
    {
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
  this->uid = 0;
  this->gid = 0;
  this->home = "/";
  this->shell = "/bin/false";

  this->user = user;

  struct passwd *pwent = getpwnam(this->user.c_str());
  if (pwent == 0)
    {
      format fmt(_("%1%: user not found: %2%"));
      fmt % this->user.c_str() % strerror(errno);
      throw error(fmt);
    }
  this->uid = pwent->pw_uid;
  this->gid = pwent->pw_gid;
  this->home = pwent->pw_dir;
  this->shell = pwent->pw_shell;
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
auth::get_pam_environment () const
{
  return environment(pam_getenvlist(this->pam));
}

uid_t
auth::get_ruid () const
{
  return this->ruid;
}

std::string const&
auth::get_ruser () const
{
  return this->ruser;
}

auth::verbosity
auth::get_verbosity () const
{
  return this->message_verbosity;
}

void
auth::set_verbosity (auth::verbosity verbosity)
{
  this->message_verbosity = verbosity;
}

auth::conv_ptr&
auth::get_conv ()
{
  return this->conv;
}

void
auth::set_conv (conv_ptr& conv)
{
  this->conv = conv;
}

void
auth::run ()
{
  try
    {
      start();
      authenticate();
      setupenv();
      account();
      try
	{
	  cred_establish();

	  const char *authuser = 0;
	  const void *tmpcast = static_cast<const void *>(authuser);
	  pam_get_item(this->pam, PAM_USER, &tmpcast);
	  log_debug(DEBUG_INFO)
	    << format("PAM authentication succeeded for user %1%") % authuser
	    << endl;

	  run_impl();

	  /* The session is now finished, either
	     successfully or not.  All PAM operations are
	     now for cleanup and shutdown, and we must
	     clean up whether or not errors were raised at
	     any previous point.  This means only the
	     first error is reported back to the user. */

	  /* Don't cope with failure, since we are now
	     already bailing out, and an error may already
	     have been raised */
	}
      catch (error const& e)
	{
	  try
	    {
	      cred_delete();
	    }
	  catch (error const& discard)
	    {
	    }
	  throw;
	}
    }
  catch (error const& e)
    {
      try
	{
	  /* Don't cope with failure, since we are now already bailing out,
	     and an error may already have been raised */
	  stop();
	}
      catch (error const& discard)
	{
	}
      throw;
    }
}

void
auth::start ()
{
  assert(!this->user.empty());

  if (this->pam != 0)
    {
      log_debug(DEBUG_CRITICAL)
	<< "pam_start FAIL (already initialised)" << endl;
      throw error(_("PAM error: PAM is already initialised"));
    }

  struct pam_conv conv_hook =
    {
      auth_conv,
      static_cast<void *>(this->conv.get())
    };

  int pam_status;

  if ((pam_status =
       pam_start(this->service.c_str(), this->user.c_str(),
		 &conv_hook, &this->pam)) != PAM_SUCCESS)
    {
      log_debug(DEBUG_WARNING) << "pam_start FAIL" << endl;
      format fmt(_("PAM error: %1%"));
      fmt % pam_strerror(this->pam, pam_status);
      throw error(fmt);
    }

  log_debug(DEBUG_NOTICE) << "pam_start OK" << endl;
}

void
auth::stop ()
{
  if (this->pam); // PAM must be initialised
  {
    int pam_status;

    if ((pam_status =
	 pam_end(this->pam, PAM_SUCCESS)) != PAM_SUCCESS)
      {
	log_debug(DEBUG_WARNING) << "pam_end FAIL" << endl;
	format fmt(_("PAM error: %1%"));
	fmt % pam_strerror(this->pam, pam_status);
	throw error(fmt);
      }

    this->pam = 0;
    log_debug(DEBUG_NOTICE) << "pam_end OK" << endl;
  }
}

void
auth::authenticate ()
{
  assert(!this->user.empty());
  assert(this->pam != 0); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_set_item(this->pam, PAM_RUSER, this->ruser.c_str())) != PAM_SUCCESS)
    {
      log_debug(DEBUG_WARNING) << "pam_set_item (PAM_RUSER) FAIL" << endl;
      format fmt(_("PAM set RUSER error: %1%"));
      fmt % pam_strerror(this->pam, pam_status);
      throw error(fmt);
    }

  long hl = 256; /* sysconf(_SC_HOST_NAME_MAX); BROKEN with Debian libc6 2.3.2.ds1-22 */

  char *hostname = new char[hl];
  if (gethostname(hostname, hl) != 0)
    {
      log_debug(DEBUG_CRITICAL) << "gethostname FAIL" << endl;
      format fmt(_("Failed to get hostname: %1%"));
      fmt % pam_strerror(this->pam, pam_status);
      throw error(fmt);
    }

  if ((pam_status =
       pam_set_item(this->pam, PAM_RHOST, hostname)) != PAM_SUCCESS)
    {
      log_debug(DEBUG_WARNING) << "pam_set_item (PAM_RHOST) FAIL" << endl;
      format fmt(_("PAM set RHOST error: %1%"));
      fmt % pam_strerror(this->pam, pam_status);
      throw error(fmt);
    }

  delete[] hostname;
  hostname = 0;

  const char *tty = ttyname(STDIN_FILENO);
  if (tty)
    {
      if ((pam_status =
	   pam_set_item(this->pam, PAM_TTY, tty)) != PAM_SUCCESS)
	{
	  log_debug(DEBUG_WARNING) << "pam_set_item (PAM_TTY) FAIL" << endl;
	  format fmt(_("PAM set TTY error: %1%"));
	  fmt % pam_strerror(this->pam, pam_status);
	  throw error(fmt);
	}
    }

  /* Authenticate as required. */
  switch (get_auth_status())
    {
    case STATUS_NONE:
      if ((pam_status =
	   pam_set_item(this->pam, PAM_USER, this->user.c_str())) != PAM_SUCCESS)
	{
	  log_debug(DEBUG_WARNING) << "pam_set_item (PAM_USER) FAIL" << endl;
	  format fmt(_("PAM set USER error: %1%"));
	  fmt % pam_strerror(this->pam, pam_status);
	  throw error(fmt);
	}
      break;

    case STATUS_USER:
      if ((pam_status =
	   pam_authenticate(this->pam, 0)) != PAM_SUCCESS)
	{
	  log_debug(DEBUG_INFO) << "pam_authenticate FAIL" << endl;
	  syslog(LOG_AUTH|LOG_WARNING, "%s->%s Authentication failure",
		 this->ruser.c_str(), this->user.c_str());
	  format fmt(_("PAM authentication failed: %1%"));
	  fmt % pam_strerror(this->pam, pam_status);
	  throw error(fmt);
	}
      log_debug(DEBUG_NOTICE) << "pam_authenticate OK" << endl;
      break;

    case STATUS_FAIL:
	{
	  log_debug(DEBUG_INFO) << "PAM auth premature FAIL" << endl;
	  cerr << format(_("You do not have permission to access the %1% service."))
	    % this->service
	       << '\n'
	       << _("This failure will be reported.")
	       << endl;
	  syslog(LOG_AUTH|LOG_WARNING,
		 "%s->%s Unauthorised",
		 this->ruser.c_str(), this->user.c_str());
	  throw error(_("access not authorised"));
	}
    default:
      break;
    }
}

void
auth::setupenv ()
{
  assert(this->pam != 0); // PAM must be initialised

  int pam_status;

  /* Initial environment to set, used if the environment was not
     specified. */
  environment newenv;

  if (this->uid == 0)
    newenv.add(std::make_pair("PATH", "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/bin/X11"));
  else
    newenv.add(std::make_pair("PATH", "/usr/local/bin:/usr/bin:/bin:/usr/bin/X11:/usr/games"));

  if (!this->home.empty() )
    newenv.add(std::make_pair("HOME", this->home));
  else
    newenv.add(std::make_pair("HOME", "/"));
  if (!this->user.empty())
    {
      newenv.add(std::make_pair("LOGNAME", this->user));
      newenv.add(std::make_pair("USER", this->user));
    }
  {
    const char *term = getenv("TERM");
    if (term)
      newenv.add(std::make_pair("TERM", term));
  }

  // The environment can only be preserved if not switching to root.
  environment environment;
  if (!this->user_environment.empty())
    environment = this->user_environment;
  else
    environment = newenv;

  for (environment::const_iterator cur = environment.begin();
       cur != environment.end();
       ++cur)
    {
      std::string env_string = cur->first + "=" + cur->second;
      if ((pam_status =
	   pam_putenv(this->pam, env_string.c_str())) != PAM_SUCCESS)
	{
	  log_debug(DEBUG_WARNING) << "pam_putenv FAIL" << endl;
	  format fmt(_("PAM error: %1%"));
	  fmt % pam_strerror(this->pam, pam_status);
	  throw error(fmt);
	}
      log_debug(DEBUG_INFO)
	<< format("pam_putenv: set %1%=%2%") % cur->first % cur->second
	<< endl;
    }

  log_debug(DEBUG_NOTICE) << "pam_putenv OK" << endl;
}

void
auth::account ()
{
  assert(this->pam != 0); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_acct_mgmt(this->pam, 0)) != PAM_SUCCESS)
    {
      /* We don't handle changing expired passwords here, since we are
	 not login or ssh. */
      log_debug(DEBUG_WARNING) << "pam_acct_mgmt FAIL" << endl;
      format fmt(_("PAM error: %1%"));
      fmt % pam_strerror(this->pam, pam_status);
      throw error(fmt);
    }

  log_debug(DEBUG_NOTICE) << "pam_acct_mgmt OK" << endl;
}

void
auth::cred_establish ()
{
  assert(this->pam != 0); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_setcred(this->pam, PAM_ESTABLISH_CRED)) != PAM_SUCCESS)
    {
      log_debug(DEBUG_WARNING) << "pam_setcred FAIL" << endl;
      format fmt(_("PAM error: %1%"));
      fmt % pam_strerror(this->pam, pam_status);
      throw error(fmt);
    }

  log_debug(DEBUG_NOTICE) << "pam_setcred OK" << endl;
}

void
auth::cred_delete ()
{
  assert(this->pam != 0); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_setcred(this->pam, PAM_DELETE_CRED)) != PAM_SUCCESS)
    {
      log_debug(DEBUG_WARNING) << "pam_setcred (delete) FAIL" << endl;
      format fmt(_("PAM error: %1%"));
      fmt % pam_strerror(this->pam, pam_status);
      throw error(fmt);
    }

  log_debug(DEBUG_NOTICE) << "pam_setcred (delete) OK" << endl;
}

void
auth::open_session ()
{
  assert(this->pam != 0); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_open_session(this->pam, 0)) != PAM_SUCCESS)
    {
      log_debug(DEBUG_WARNING) << "pam_open_session FAIL" << endl;
      format fmt(_("PAM error: %1%"));
      fmt % pam_strerror(this->pam, pam_status);
      throw error(fmt);
    }

  log_debug(DEBUG_NOTICE) << "pam_open_session OK" << endl;
}

void
auth::close_session ()
{
  assert(this->pam != 0); // PAM must be initialised

  int pam_status;

  if ((pam_status =
       pam_close_session(this->pam, 0)) != PAM_SUCCESS)
    {
      log_debug(DEBUG_WARNING) << "pam_close_session FAIL" << endl;
      format fmt(_("PAM error: %1%"));
      fmt % pam_strerror(this->pam, pam_status);
      throw error(fmt);
    }

  log_debug(DEBUG_NOTICE) << "pam_close_session OK" << endl;
}

auth::status
auth::get_auth_status () const
{
  status authtype = STATUS_NONE;

  authtype = change_auth(authtype, STATUS_USER);

  return authtype;
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
