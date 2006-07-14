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

#include <sbuild/sbuild-i18n.h>
#include <sbuild/sbuild-log.h>
#include <sbuild/sbuild-types.h>

#include "schroot-base-main.h"

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <locale>

#include <termios.h>
#include <unistd.h>

#include <syslog.h>

#include <boost/format.hpp>

using std::endl;
using boost::format;
using namespace schroot_base;

main::main (std::string const&  program_name,
	    std::string const&  program_usage,
	    options::ptr const& program_options):
  program_name(program_name),
  program_usage(program_usage),
  program_options(program_options)
{
}

main::~main ()
{
}

void
main::action_version (std::ostream& stream)
{
  format fmt(_("%1% (Debian sbuild) %2% (%3%)\n"));
  fmt % this->program_name % VERSION % sbuild::gmdate(RELEASE_DATE);

  stream << fmt
	 << _("Written by Roger Leigh") << '\n' << '\n'
	 << _("Copyright (C) 2004-2006 Roger Leigh") << '\n'
	 << _("This is free software; see the source for copying conditions.  There is NO\n"
	      "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n")
	 << std::flush;
}

void
main::action_help (std::ostream& stream)
{
  stream
    << _("Usage:") << '\n'
    << "  " << this->program_name << "  "
    << this->program_usage << std::endl;

  stream << this->program_options->get_visible_options() << std::flush;
}

int
main::run (int   argc,
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

      this->program_options->parse(argc, argv);

#ifdef SBUILD_DEBUG
      sbuild::debug_level = sbuild::DEBUG_CRITICAL;
#endif

      openlog("schroot", LOG_PID|LOG_NDELAY, LOG_AUTHPRIV);

      int status = run_impl();

      closelog();

      if (isatty(STDIN_FILENO) && termios_ok)
	{
	  if (tcsetattr(STDIN_FILENO, TCSANOW, &saved_termios) < 0)
	    sbuild::log_warning()
	      << _("Error restoring terminal settings")
	      << endl;
	}

      exit(status);
    }
  catch (std::exception const& e)
    {
      sbuild::log_error() << e.what() << endl;

      try
	{
	  dynamic_cast<boost::program_options::error const&>(e);
	  sbuild::log_info()
	    << format(_("Run \"%1% --help\" to list usage example and all available options"))
	    % argv[0]
	    << endl;
	}
      catch (std::bad_cast const& discard)
	{
	}

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
  catch (...)
    {
      sbuild::log_error() << _("An unknown exception occured") << endl;
      exit(EXIT_FAILURE);
    }
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
