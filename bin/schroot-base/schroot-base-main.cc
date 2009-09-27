/* Copyright © 2005-2009  Roger Leigh <rleigh@debian.org>
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

#include <sbuild/sbuild-i18n.h>
#include <sbuild/sbuild-log.h>
#include <sbuild/sbuild-types.h>

#include "schroot-base-main.h"

#include <cstdlib>
#include <ctime>
#include <iostream>

#include <syslog.h>

#include <boost/format.hpp>

using std::endl;
using boost::format;
using sbuild::_;
using namespace schroot_base;

main::main (std::string const&  program_name,
	    std::string const&  program_usage,
	    options::ptr const& program_options,
	    bool                use_syslog):
  program_name(program_name),
  program_usage(program_usage),
  program_options(program_options),
  use_syslog(use_syslog)
{
}

main::~main ()
{
}

void
main::action_version (std::ostream& stream)
{
  // TRANSLATORS: %1% = program name
  // TRANSLATORS: %2% = program version
  // TRANSLATORS: %3% = release date
  format fmt(_("%1% (Debian sbuild) %2% (%3%)\n"));
  fmt % this->program_name % VERSION % sbuild::gmdate(RELEASE_DATE);

  format feature("  %1$-12s %2%\n");

  stream << fmt
	 << _("Written by Roger Leigh") << '\n' << '\n'
    // TRANSLATORS: '(C)' is a copyright symbol and '-' is an en-dash.
	 << _("Copyright (C) 2004-2009 Roger Leigh") << '\n'
	 << _("This is free software; see the source for copying conditions.  There is NO\n"
	      "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n")
	 << '\n'
	 << _("Configured features:") << '\n';
#ifdef SBUILD_FEATURE_DEVLOCK
  stream << feature % "DEVLOCK" % _("Device locking");
#endif
#ifdef SBUILD_FEATURE_PAM
  stream << feature % "PAM" % _("Pluggable Authentication Modules");
#endif
#ifdef SBUILD_FEATURE_PERSONALITY
  stream << feature % "PERSONALITY" % _("Linux kernel Application Binary Interface switching");
#endif
#ifdef SBUILD_FEATURE_UNION
  stream << feature % "UNION" % _("Support for filesystem unioning");
#endif
  stream << std::flush;
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
  try
    {
      this->program_options->parse(argc, argv);

#ifdef SBUILD_DEBUG
      sbuild::debug_level = sbuild::DEBUG_CRITICAL;
#endif

      if (this->use_syslog)
	openlog(this->program_name.c_str(), LOG_PID|LOG_NDELAY, LOG_AUTHPRIV);

      int status = run_impl();

      closelog();

      return status;
    }
  catch (std::exception const& e)
    {
      sbuild::log_exception_error(e);

      try
	{
	  dynamic_cast<boost::program_options::error const&>(e);
	  sbuild::log_info()
	    // TRANSLATORS: %1% = program name
	    << format(_("Run \"%1% --help\" to list usage example and all available options"))
	    % argv[0]
	    << endl;
	}
      catch (std::bad_cast const& discard)
	{
	}

      if (this->use_syslog)
	closelog();

      return EXIT_FAILURE;
    }
}
