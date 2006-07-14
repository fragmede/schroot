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

#include "schroot-main-base.h"

#include <sbuild/sbuild-auth-conv.h>
#include <sbuild/sbuild-auth-conv-tty.h>

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <locale>

#include <termios.h>
#include <unistd.h>

#include <boost/format.hpp>

using std::endl;
using boost::format;
using namespace schroot;

main_base::main_base (std::string const& program_name,
		      std::string const& program_usage,
		      options_base::ptr& options):
  schroot_base::main(program_name, program_usage,
		     std::tr1::static_pointer_cast<schroot_base::options>(options)),
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
	  for (sbuild::string_list::const_iterator chroot =
		 invalid_chroots.begin();
	       chroot != invalid_chroots.end();
	       ++chroot)
	    sbuild::log_error() << format(_("%1%: No such chroot")) % *chroot
				<< endl;
	  exit(EXIT_FAILURE);
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
    this->config->add(SCHROOT_CONF, false);
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
      exit(EXIT_SUCCESS);
    }

  if (this->options->action == options_base::ACTION_VERSION)
    {
      action_version(std::cout);
      exit(EXIT_SUCCESS);
    }

  /* Initialise chroot configuration. */
  load_config();

  if (this->config->get_chroots().empty() && this->options->quiet == false)
    {
      if (this->options->load_chroots == true &&
	  this->options->load_sessions == true)
	sbuild::log_warning()
	  << format(_("No chroots are defined in '%1%' or '%2%'"))
	  % SCHROOT_CONF % SCHROOT_SESSION_DIR
	  << endl;
      else
	{
	  const char *cfile = (this->options->load_sessions)
	    ? SCHROOT_CONF : SCHROOT_SESSION_DIR;
	  sbuild::log_warning()
	    << format(_("No chroots are defined in '%1%'")) % cfile
	    << endl;
	}
    }

  /* Print chroot list (including aliases). */
  if (this->options->action == options_base::ACTION_LIST)
    {
      action_list();
      exit(EXIT_SUCCESS);
    }

  /* Get list of chroots to use */
  chroots = get_chroot_options();
  if (this->chroots.empty())
    {
      sbuild::log_error()
	<< format(_("The specified chroots are not defined in '%1%'"))
	% SCHROOT_CONF
	<< endl;
      exit (EXIT_FAILURE);
    }

  /* Print chroot information for specified chroots. */
  if (this->options->action == options_base::ACTION_INFO)
    {
      action_info();
      exit (EXIT_SUCCESS);
    }
  if (this->options->action == options_base::ACTION_LOCATION)
    {
      action_location();
      exit (EXIT_SUCCESS);
    }
  if (this->options->action == options_base::ACTION_CONFIG)
    {
      action_config();
      exit (EXIT_SUCCESS);
    }

  if (this->options->action == options_base::ACTION_SESSION_BEGIN &&
      this->chroots.size() != 1)
    {
      sbuild::log_error()
	<< _("Only one chroot may be specified when beginning a session")
	<< endl;
      exit (EXIT_FAILURE);
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
	this->session->set_command(this->options->command);
      if (this->options->preserve)
	this->session->set_environment(environ);
      this->session->set_force(this->options->session_force);
      sbuild::auth::verbosity verbosity = sbuild::auth::VERBOSITY_NORMAL;
      if (this->options->quiet)
	verbosity = sbuild::auth::VERBOSITY_QUIET;
      else if (this->options->verbose)
	verbosity = sbuild::auth::VERBOSITY_VERBOSE;
      this->session->set_verbosity(verbosity);

      /* Set up authentication timeouts. */
      std::tr1::shared_ptr<sbuild::auth_conv>
	conv(new sbuild::auth_conv_tty);
      time_t curtime = 0;
      time(&curtime);
      conv->set_warning_timeout(curtime + 15);
      conv->set_fatal_timeout(curtime + 20);
      this->session->set_conv(conv);

      /* Run session. */
      this->session->run();
    }
  catch (std::runtime_error& e)
    {
      if (!this->options->quiet)
	sbuild::log_error() << e.what() << endl;
    }

  if (this->session)
    exit(this->session->get_child_status());
  else
    exit(EXIT_FAILURE);
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
