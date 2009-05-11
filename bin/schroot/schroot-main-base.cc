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

#include "schroot-main-base.h"

#include <sbuild/sbuild-config.h>
#ifdef SBUILD_FEATURE_PAM
#include <sbuild/sbuild-auth-pam-conv.h>
#include <sbuild/sbuild-auth-pam-conv-tty.h>
#endif // SBUILD_FEATURE_PAM

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <locale>

#include <termios.h>
#include <unistd.h>

#include <boost/format.hpp>

using std::endl;
using boost::format;
using sbuild::N_;
using namespace schroot;

namespace
{

  typedef std::pair<main_base::error_code,const char *> emap;

  /**
   * This is a list of the supported error codes.  It's used to
   * construct the real error codes map.
   */
  emap init_errors[] =
    {
      // TRANSLATORS: %1% = comma-separated list of chroot names
      emap(main_base::CHROOTS_NOTFOUND,  N_("%1%: Chroots not found")),
      // TRANSLATORS: %4% = file
      emap(main_base::CHROOT_FILE,       N_("No chroots are defined in '%4%'")),
      // TRANSLATORS: %4% = file
      // TRANSLATORS: %5% = file
      emap(main_base::CHROOT_FILE2,      N_("No chroots are defined in '%4%' or '%5%'")),
      // TRANSLATORS: %1% = file
      emap(main_base::CHROOT_NOTDEFINED, N_("The specified chroots are not defined in '%1%'")),
      // TRANSLATORS: %1% = chroot name
      emap(main_base::CHROOT_NOTFOUND,   N_("%1%: Chroot not found"))
    };

}

/// Error code to description mapping.
template<>
sbuild::error<main_base::error_code>::map_type
sbuild::error<main_base::error_code>::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));

main_base::main_base (std::string const& program_name,
		      std::string const& program_usage,
		      options_base::ptr& options,
		      bool               use_syslog):
  schroot_base::main(program_name, program_usage,
		     std::tr1::static_pointer_cast<schroot_base::options>(options),
		     use_syslog),
  options(options)
{
}

main_base::~main_base ()
{
}

void
main_base::action_info ()
{
  this->config->print_chroot_info(this->chroots, std::cout);
}

void
main_base::action_location ()
{
  this->config->print_chroot_location(this->chroots, std::cout);
}

void
main_base::compat_check ()
{
}

sbuild::string_list
main_base::get_chroot_options ()
{
  sbuild::string_list ret;

  if (this->options->all_chroots == true ||
      this->options->all_sessions == true)
    {
      sbuild::chroot_config::chroot_list const& list =
	this->config->get_chroots();

      for (sbuild::chroot_config::chroot_list::const_iterator chroot =
	     list.begin();
	   chroot != list.end();
	   ++chroot)
	{
	  if (((*chroot)->get_active() == false &&
	       this->options->all_chroots == false) ||
	      ((*chroot)->get_active() == true &&
	       this->options->all_sessions == false))
	    continue;
	  ret.push_back((*chroot)->get_name());
	}
    }
  else
    {
      sbuild::string_list invalid_chroots =
	this->config->validate_chroots(this->options->chroots);

      if (!invalid_chroots.empty())
	{
	  std::string invalid_list;
	  for (sbuild::string_list::const_iterator chroot =
		 invalid_chroots.begin();
	       chroot != invalid_chroots.end();
	       ++chroot)
	    {
	      invalid_list += *chroot;
	      if (chroot + 1 != invalid_chroots.end())
		invalid_list += ", ";
	    }
	  throw error(invalid_list,
		      (invalid_chroots.size() == 1)
		      ? CHROOT_NOTFOUND : CHROOTS_NOTFOUND);
	}
      ret = this->options->chroots;
    }

  return ret;
}

void
main_base::load_config ()
{
  this->config = sbuild::chroot_config::ptr(new sbuild::chroot_config);
  /* The normal chroot list is used when starting a session or running
     any chroot type or session, or displaying chroot information. */
  if (this->options->load_chroots == true)
    {
      this->config->add(SCHROOT_CONF, false);
      this->config->add(SCHROOT_CONF_CHROOT_D, false);
    }
  /* The session chroot list is used when running or ending an
     existing session, or displaying chroot information. */
  if (this->options->load_sessions == true)
    this->config->add(SCHROOT_SESSION_DIR, true);
}

int
main_base::run_impl ()
{
  compat_check();

  if (this->options->action == options_base::ACTION_HELP)
    {
      action_help(std::cout);
      return EXIT_SUCCESS;
    }

  if (this->options->action == options_base::ACTION_VERSION)
    {
      action_version(std::cout);
      return EXIT_SUCCESS;
    }

  /* Initialise chroot configuration. */
  load_config();

  if (this->config->get_chroots().empty() && this->options->quiet == false)
    {
      if (this->options->load_chroots == true &&
	  this->options->load_sessions == true)
	log_exception_warning
	  (error(CHROOT_FILE2, SCHROOT_CONF, SCHROOT_SESSION_DIR));
      else
	{
	  const char *cfile = (this->options->load_sessions)
	    ? SCHROOT_SESSION_DIR : SCHROOT_CONF;
	  log_exception_warning(error(CHROOT_FILE, cfile));
	}
    }

  /* Print chroot list (including aliases). */
  if (this->options->action == options_base::ACTION_LIST)
    {
      action_list();
      return EXIT_SUCCESS;
    }

  /* Get list of chroots to use */
  chroots = get_chroot_options();
  if (this->chroots.empty())
    {
      if (!(this->options->all_chroots == true ||
	    this->options->all_sessions == true))
	throw error(SCHROOT_CONF, CHROOT_NOTDEFINED);
      else
	{
	  // If one of the --all options was used, then don't treat
	  // the lack of chroots as an error.  TODO: Also check if any
	  // additional chroots were specified with -c; this needs
	  // changes in get_chroot_options.
	  log_exception_warning(error((this->options->all_chroots == true) ?
				      SCHROOT_CONF : SCHROOT_SESSION_DIR,
				      CHROOT_NOTDEFINED));
	  return EXIT_SUCCESS;
	}
    }

  /* Print chroot information for specified chroots. */
  if (this->options->action == options_base::ACTION_INFO)
    {
      action_info();
      return EXIT_SUCCESS;
    }
  if (this->options->action == options_base::ACTION_LOCATION)
    {
      action_location();
      return EXIT_SUCCESS;
    }
  if (this->options->action == options_base::ACTION_CONFIG)
    {
      action_config();
      return EXIT_SUCCESS;
    }

  /* Create a session. */
  sbuild::session::operation sess_op(sbuild::session::OPERATION_AUTOMATIC);
  if (this->options->action == options_base::ACTION_SESSION_BEGIN)
    sess_op = sbuild::session::OPERATION_BEGIN;
  else if (this->options->action == options_base::ACTION_SESSION_RECOVER)
    sess_op = sbuild::session::OPERATION_RECOVER;
  else if (this->options->action == options_base::ACTION_SESSION_RUN)
    sess_op = sbuild::session::OPERATION_RUN;
  else if (this->options->action == options_base::ACTION_SESSION_END)
    sess_op = sbuild::session::OPERATION_END;

  try
    {
      create_session(sess_op);

      if (!this->options->command.empty())
	this->session->get_auth()->set_command(this->options->command);
      if (!this->options->directory.empty())
	this->session->get_auth()->set_wd(this->options->directory);
      if (this->options->preserve)
	this->session->get_auth()->set_environment(environ);
      this->session->set_session_id(this->options->session_name);
      this->session->set_force(this->options->session_force);
      sbuild::auth::verbosity verbosity = sbuild::auth::VERBOSITY_NORMAL;
      if (this->options->quiet)
	verbosity = sbuild::auth::VERBOSITY_QUIET;
      else if (this->options->verbose)
	verbosity = sbuild::auth::VERBOSITY_VERBOSE;
      this->session->get_auth()->set_verbosity(verbosity);

      /* Run session. */
      this->session->run();
    }
  catch (std::runtime_error const& e)
    {
      if (!this->options->quiet)
	sbuild::log_exception_error(e);
    }

  if (this->session)
    return this->session->get_child_status();
  else
    return EXIT_FAILURE;
}
