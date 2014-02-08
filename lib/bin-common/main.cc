/* Copyright © 2005-2013  Roger Leigh <rleigh@debian.org>
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

#include <schroot/i18n.h>
#include <schroot/log.h>
#include <schroot/types.h>
#include <schroot/feature.h>

#include <bin-common/main.h>

#include <cstdlib>
#include <ctime>
#include <iostream>

#include <syslog.h>

#include <boost/format.hpp>

using std::endl;
using boost::format;
using schroot::_;

namespace bin
{
  namespace common
  {

    main::main (const std::string&  program_name,
                const std::string&  program_usage,
                const options::ptr& program_options,
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
      format fmtr(
#if DISTRIBUTOR_UNSET
                  // TRANSLATORS: %1% = program name
                  // TRANSLATORS: %3% = program version
                  // TRANSLATORS: %4% = release date
                  "%1% %3% (%4%)"
#else
                  // TRANSLATORS: %1% = program name
                  // TRANSLATORS: %2% = distributor
                  // TRANSLATORS: %3% = program version
                  // TRANSLATORS: %4% = release date
                  "%1% (%2%) %3% (%4%)"
#endif
                  );
      fmtr % this->program_name % DISTRIBUTOR % VERSION
        % schroot::gmdate(RELEASE_DATE);

      // TRANSLATORS: %1% = copyright year (start)
      // TRANSLATORS: %2% = copyright year (end)
      format fmtc(_("Copyright © %1%–%2% Roger Leigh"));
      fmtc % "2004" % "2014";

      stream << fmtr << '\n'
             << _("Written by Roger Leigh") << '\n' << '\n'
             << fmtc << '\n'
             << _("This is free software; see the source for copying conditions.  There is NO\n"
                  "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n")
             << '\n'
             << _("Configured features:") << '\n';
      schroot::feature::print_features(stream);
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

#ifdef SCHROOT_DEBUG
          schroot::debug_log_level = schroot::DEBUG_CRITICAL;
#endif

          if (this->use_syslog)
            openlog(this->program_name.c_str(), LOG_PID|LOG_NDELAY, LOG_AUTHPRIV);

          int status = run_impl();

          closelog();

          return status;
        }
      catch (const std::exception& e)
        {
          schroot::log_exception_error(e);

          try
            {
              dynamic_cast<bin::common::options::error const&>(e);
              schroot::log_info()
                // TRANSLATORS: %1% = program name
                << format(_("Run “%1% --help” to list usage example and all available options"))
                % argv[0]
                << endl;
            }
          catch (const std::bad_cast& discard)
            {
            }

          if (this->use_syslog)
            closelog();

          return EXIT_FAILURE;
        }
    }

  }
}
