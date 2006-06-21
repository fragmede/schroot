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

#include <termios.h>
#include <unistd.h>

#include <boost/format.hpp>

#include <syslog.h>

#include "sbuild.h"

#include "schroot-options.h"

#ifdef SBUILD_DCHROOT_COMPAT
#include "dchroot-chroot-config.h"
#include "dchroot-session.h"
#endif

using std::endl;
using boost::format;
using namespace schroot;

std::string
program_name (schroot::options& options)
{
  if (options.compat == options::COMPAT_SCHROOT)
    return "schroot";
  else if (options.compat == options::COMPAT_DCHROOT)
    return "dchroot";
  else if (options.compat == options::COMPAT_DCHROOT_DSA)
    return "dchroot-dsa";
}

/**
 * Print version information.
 *
 * @param stream the stream to output to.
 * @param options the command line options.
 */
void
print_version (std::ostream&     stream,
	       schroot::options& options)
{
  format fmt(_("%1% (Debian sbuild) %2%\n"));
  fmt % program_name(options);
  fmt % VERSION;

  stream << fmt
	 << _("Written by Roger Leigh\n\n")
	 << _("Copyright (C) 2004-2006 Roger Leigh\n")
	 << _("This is free software; see the source for copying conditions.  There is NO\n"
	      "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n")
	 << std::flush;
}

/**
 * Get a list of chroots based on the specified options (--all, --chroot).
 *
 * @param config the chroot configuration
 * @param options the command-line options
 *
 * @returns a list of chroots.
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

/**
 * Main routine.
 *
 * @param argc the number of arguments
 * @param argv argument vector
 *
 * @returns 0 on success, 1 on failure or the exit status of the
 * chroot command.
 */
int
main (int   argc,
      char *argv[])
{
  struct termios saved_termios;
  bool termios_ok = false;

  try
    {
      // Set up locale.
      std::locale::global(std::locale(""));
      std::cout.imbue(std::locale());
      std::cerr.imbue(std::locale());

      // Save terminal state.
      if (isatty(STDIN_FILENO))
	{
	  if (tcgetattr(STDIN_FILENO, &saved_termios) < 0)
	    {
	      termios_ok = false;
	      sbuild::log_warning()
		<< _("Error saving terminal settings")
		<< endl;
	    }
	  else
	    termios_ok = true;
	}

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

      if (options.compat != options::COMPAT_SCHROOT && options.verbose)
	{
	  sbuild::log_warning()
	    << _("Running schroot in dchroot compatibility mode")
	    << endl;
	  sbuild::log_info()
	    << _("Run 'schroot' for full capabilities")
	    << endl;
	}

      if (options.action == options::ACTION_VERSION)
	{
	  print_version(std::cout, options);
	  exit(EXIT_SUCCESS);
	}

      /* Initialise chroot configuration. */
#ifdef SBUILD_DCHROOT_COMPAT
      bool use_dchroot_conf = false;
      if (options.compat != options::COMPAT_SCHROOT)
	{
	  struct stat statbuf;
	  if (stat(DCHROOT_CONF, &statbuf) == 0 && !S_ISDIR(statbuf.st_mode))
	    {
	      use_dchroot_conf = true;

	      if (options.verbose)
		{
		  sbuild::log_warning()
		    << _("Using dchroot configuration file: ") << DCHROOT_CONF
		    << endl;
		  sbuild::log_info()
		    << format(_("Run '%1%'"))
		    % "dchroot --config >> " SCHROOT_CONF
		    << endl;
		  sbuild::log_info()
		    << _("to migrate to a schroot configuration.")
		    << endl;
		  sbuild::log_info()
		    << format(_("Edit '%1%' to add appropriate group access."))
		    % SCHROOT_CONF
		    << endl;
		  sbuild::log_info()
		    << format(_("Remove '%1%' to use the new configuration."))
		    % DCHROOT_CONF
		    << endl;
		}
	    }
	}
#endif

      sbuild::chroot_config::ptr config;
#ifdef SBUILD_DCHROOT_COMPAT
      if (options.compat != options::COMPAT_SCHROOT && use_dchroot_conf)
	{
	  config = sbuild::chroot_config::ptr(new dchroot::chroot_config);
	  if (options.load_chroots == true)
	    config->add(DCHROOT_CONF, false);
	}
      else
#endif
	{
	  config = sbuild::chroot_config::ptr(new sbuild::chroot_config);
	  /* The normal chroot list is used when starting a session or running
	     any chroot type or session, or displaying chroot information. */
	  if (options.load_chroots == true)
	    config->add(SCHROOT_CONF, false);
	  /* The session chroot list is used when running or ending an
	     existing session, or displaying chroot information. */
	  if (options.load_sessions == true)
	    config->add(SCHROOT_SESSION_DIR, true);
	}

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
	  if (options.compat == options::COMPAT_SCHROOT)
	    config->print_chroot_list(std::cout);
	  else
	    config->print_chroot_list_simple(std::cout);
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
      if (options.action == options::ACTION_LOCATION)
	{
	  if (options.compat == options::COMPAT_SCHROOT)
	    {
	      config->print_chroot_location(chroots, std::cout);
	    }
	  else if (options.compat == options::COMPAT_DCHROOT)
	    {
	      sbuild::string_list chroot;
	      chroot.push_back(options.chroot_path);
	      config->print_chroot_location(chroot, std::cout);
	    }
	  else if (options.compat == options::COMPAT_DCHROOT_DSA)
	    {
	      config->print_chroot_location(chroots, std::cout);
	    }
	  exit (EXIT_SUCCESS);
	}
      if (options.action == options::ACTION_CONFIG)
	{
	  std::cout << "# "
		    << format(_("schroot configuration generated by %1% %2%"))
	    % program_name(options) % VERSION
		    << endl;
#ifdef SBUILD_DCHROOT_COMPAT
	  // Help text at head of new config.
	  std::cout << "# " << endl
		    << "# "
		    << _("To allow users access to the chroots, use the users or groups keys.") << endl;
	  std::cout << "# "
		    << _("To allow passwordless root access, use the root-users or root-groups keys.") << endl;
	  std::cout << "# "
		    << format(_("Remove '%1%' to use the new configuration."))
	    % DCHROOT_CONF
		    << endl;
#endif
	    std::cout << endl;
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

      // Using dchroot.conf implies using dchroot_session, which does
      // not require group access.  If loading schroot.conf, we always
      // want normal session management.
      sbuild::session::ptr session;
#ifdef SBUILD_DCHROOT_COMPAT
      if (options.compat != options::COMPAT_SCHROOT && use_dchroot_conf)
	session = sbuild::session::ptr
	  (new dchroot::session("schroot", config, sess_op, chroots));
      else
#endif
	session = sbuild::session::ptr
	  (new sbuild::session("schroot", config, sess_op, chroots));

      try
	{
	  if (!options.user.empty() && options.compat == options::COMPAT_SCHROOT)
	    session->set_user(options.user);
	  if (!options.command.empty())
	    session->set_command(options.command);
	  if (options.preserve)
	    session->set_environment(environ);
	  session->set_force(options.session_force);
	  sbuild::auth::verbosity verbosity = sbuild::auth::VERBOSITY_NORMAL;
	  if (options.quiet)
	    verbosity = sbuild::auth::VERBOSITY_QUIET;
	  else if (options.verbose)
	    verbosity = sbuild::auth::VERBOSITY_VERBOSE;
	  session->set_verbosity(verbosity);

	  /* Set up authentication timeouts. */
	  std::tr1::shared_ptr<sbuild::auth_conv>
	    conv(new sbuild::auth_conv_tty);
	  time_t curtime = 0;
	  time(&curtime);
	  conv->set_warning_timeout(curtime + 15);
	  conv->set_fatal_timeout(curtime + 20);
	  session->set_conv(conv);

	  /* Run session. */
	  session->run();
	}
      catch (std::runtime_error& e)
	{
	  if (!options.quiet)
	    sbuild::log_error()
	      << format(_("Session failure: %1%")) % e.what() << endl;
	}

      closelog();

      if (isatty(STDIN_FILENO) && termios_ok)
	{
	  if (tcsetattr(STDIN_FILENO, TCSANOW, &saved_termios) < 0)
	    sbuild::log_warning()
	      << _("Error restoring terminal settings")
	      << endl;
	}

      exit(session->get_child_status());
    }
  catch (std::exception const& e)
    {
      sbuild::log_error() << e.what() << endl;

      closelog();

      if (isatty(STDIN_FILENO) && termios_ok)
	{
	  if (tcsetattr(STDIN_FILENO, TCSANOW, &saved_termios) < 0)
	    sbuild::log_warning()
	      << _("Error restoring terminal settings")
	      << endl;
	}

      exit(EXIT_FAILURE);
    }
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
