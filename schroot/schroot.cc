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

#include <cstdlib>
#include <iostream>
#include <locale>

#include <boost/format.hpp>

#include <syslog.h>

#include "sbuild.h"

#include "schroot-options.h"

using std::endl;
using boost::format;
using namespace schroot;

/*
 * print_version:
 * @file: the file to print to
 *
 * Print version information.
 */
void
print_version (std::ostream& stream)
{
  stream << format(_("schroot (Debian sbuild) %1%\n")) % VERSION
	 << _("Written by Roger Leigh\n\n")
	 << _("Copyright (C) 2004-2006 Roger Leigh\n")
	 << _("This is free software; see the source for copying conditions.  There is NO\n"
	      "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n")
	 << std::flush;
}

/*
 * get_chroot_options:
 * @config: an #sbuild::chroot_config
 * @options: an #schroot::options
 *
 * Get a list of chroots based on the specified options (--all, --chroot).
 *
 * Returns a NULL-terminated string vector (GStrv).
 */
sbuild::string_list
get_chroot_options (std::tr1::shared_ptr<sbuild::chroot_config>& config,
		    schroot::options&                     options)
{
  sbuild::string_list ret;

  if (options.all_chroots == true || options.all_sessions == true)
    {
      sbuild::chroot_config::chroot_list const& list = config->get_chroots();

      for (sbuild::chroot_config::chroot_list::const_iterator chroot = list.begin();
	   chroot != list.end();
	   ++chroot)
	{
	  if (((*chroot)->get_active() == false && options.all_chroots == false) ||
	      ((*chroot)->get_active() == true && options.all_sessions == false))
	    continue;
	  ret.push_back((*chroot)->get_name());
	}
    }
  else
    {
      sbuild::string_list invalid_chroots =
	config->validate_chroots(options.chroots);

      if (!invalid_chroots.empty())
	{
	  for (sbuild::string_list::const_iterator chroot = invalid_chroots.begin();
	       chroot != invalid_chroots.end();
	       ++chroot)
	    sbuild::log_error() << format(_("%1%: No such chroot")) % *chroot
				<< endl;
	  exit(EXIT_FAILURE);
	}
      ret = options.chroots;
    }

  return ret;
}

/*
 * main:
 * @argc: the number of arguments
 * @argv: argument vector
 *
 * Main routine.
 *
 * Returns 0 on success, 1 on failure.
 */
int
main (int   argc,
      char *argv[])
{
  try
    {
      // Set up locale.
      std::locale::global(std::locale(""));
      std::cout.imbue(std::locale());
      std::cerr.imbue(std::locale());

      bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
      textdomain (GETTEXT_PACKAGE);

#ifdef SBUILD_DEBUG
      sbuild::debug_level = sbuild::DEBUG_NOTICE;
#else
      sbuild::debug_level = sbuild::DEBUG_NONE;
#endif

      openlog("schroot", LOG_PID|LOG_NDELAY, LOG_AUTHPRIV);

      /* Parse command-line options into opt structure. */
      options options(argc, argv);

      if (options.action == options::ACTION_VERSION)
	{
	  print_version(std::cout);
	  exit(EXIT_SUCCESS);
	}

      /* Initialise chroot configuration. */
      std::tr1::shared_ptr<sbuild::chroot_config>
	config(new sbuild::chroot_config);

      /* The normal chroot list is used when starting a session or running
	 any chroot type or session, or displaying chroot information. */
      if (options.load_chroots == true)
	config->add_config_file(SCHROOT_CONF, false);
      /* The session chroot list is used when running or ending an
	 existing session, or displaying chroot information. */
      if (options.load_sessions == true)
	config->add_config_directory(SCHROOT_SESSION_DIR, true);

      if (config->get_chroots().empty() && options.quiet == false)
	{
	  if (options.load_chroots == true && options.load_sessions == true)
	    sbuild::log_warning()
	      << format(_("No chroots are defined in %1% or %2%"))
	      % SCHROOT_CONF % SCHROOT_SESSION_DIR
	      << endl;
	  else
	    {
	      const char *cfile = (options.load_sessions) ? SCHROOT_CONF : SCHROOT_SESSION_DIR;
	    sbuild::log_warning()
	      << format(_("No chroots are defined in %1%")) % cfile
	      << endl;
	    }
	}

      /* Print chroot list (including aliases). */
      if (options.action == options::ACTION_LIST)
	{
	  config->print_chroot_list(std::cout);
	  exit(EXIT_SUCCESS);
	}

      /* Get list of chroots to use */
      sbuild::string_list const& chroots = get_chroot_options(config, options);
      if (chroots.empty())
	{
	  sbuild::log_error()
	    << format(_("The specified chroots are not defined in %1%"))
	    % SCHROOT_CONF
	    << endl;
	  exit (EXIT_FAILURE);
	}

      /* Print chroot information for specified chroots. */
      if (options.action == options::ACTION_INFO)
	{
	  config->print_chroot_info(chroots, std::cout);
	  exit (EXIT_SUCCESS);
	}
      if (options.action == options::ACTION_CONFIG)
	{
	  config->print_chroot_config(chroots, std::cout);
	  exit (EXIT_SUCCESS);
	}

      if (options.action == options::ACTION_SESSION_BEGIN &&
	  chroots.size() != 1)
	{
	  sbuild::log_error()
	    << _("Only one chroot may be specified when beginning a session")
	    << endl;
	  exit (EXIT_FAILURE);
	}

      /* Create a session. */
      sbuild::session::operation sess_op(sbuild::session::OPERATION_AUTOMATIC);
      if (options.action == options::ACTION_SESSION_BEGIN)
	sess_op = sbuild::session::OPERATION_BEGIN;
      else if (options.action == options::ACTION_SESSION_RECOVER)
	sess_op = sbuild::session::OPERATION_RECOVER;
      else if (options.action == options::ACTION_SESSION_RUN)
	sess_op = sbuild::session::OPERATION_RUN;
      else if (options.action == options::ACTION_SESSION_END)
	sess_op = sbuild::session::OPERATION_END;

      sbuild::session session("schroot", config, sess_op, chroots);
      try
	{
	  if (!options.user.empty())
	    session.set_user(options.user);
	  if (!options.command.empty())
	    session.set_command(options.command);
	  /* The environment can be preserved only if not switching users. */
	  if (options.preserve && session.get_ruid() == session.get_uid())
	    session.set_environment(environ);
	  session.set_force(options.session_force);
	  sbuild::auth::verbosity verbosity = sbuild::auth::VERBOSITY_NORMAL;
	  if (options.quiet)
	    verbosity = sbuild::auth::VERBOSITY_QUIET;
	  else if (options.verbose)
	    verbosity = sbuild::auth::VERBOSITY_VERBOSE;
	  session.set_verbosity(verbosity);

	  /* Set up authentication timeouts. */
	  std::tr1::shared_ptr<sbuild::auth_conv>
	    conv(new sbuild::auth_conv_tty);
	  time_t curtime = 0;
	  time(&curtime);
	  conv->set_warning_timeout(curtime + 15);
	  conv->set_fatal_timeout(curtime + 20);
	  session.set_conv(conv);

	  /* Run session. */
	  session.run();
	}
      catch (std::runtime_error& e)
	{
	  if (!options.quiet)
	    sbuild::log_error()
	      << format(_("Session failure: %1%")) % e.what() << endl;
	}

      closelog();
      exit(session.get_child_status());
    }
  catch (std::exception const& e)
    {
      sbuild::log_error() << e.what() << endl;

      closelog();
      exit(EXIT_FAILURE);
    }
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
