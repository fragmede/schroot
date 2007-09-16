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

#include "csbuild-main.h"

#include <cerrno>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <locale>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <boost/format.hpp>

using std::endl;
using boost::format;
using sbuild::_;
using sbuild::N_;
using namespace csbuild;

namespace
{

  typedef std::pair<main::error_code,const char *> emap;

  /**
   * This is a list of the supported error codes.  It's used to
   * construct the real error codes map.
   */
  emap init_errors[] =
    {
      emap(main::DEVICE_NOTBLOCK, N_("File is not a block device")),
      // TRANSLATORS: %4% = integer process ID
      emap(main::DEVICE_OWNED,    N_("Failed to release device lock (lock held by PID %4%)")),
      emap(main::DEVICE_RELEASE,  N_("Failed to release device lock")),
      emap(main::DEVICE_STAT,     N_("Failed to stat device"))
};

}

template<>
sbuild::error<main::error_code>::map_type
sbuild::error<main::error_code>::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));

main::main (options::ptr& options):
  schroot_base::main("csbuild",
		     // TRANSLATORS: '...' is an ellipsis e.g. U+2026,
		     // and '-' is an em-dash.
		     _("[OPTION...] - build Debian packages from source"),
		     options,
		     false),
  opts(options)
{
}

main::~main ()
{
}

void
main::action_build ()
{
}

int
main::run_impl ()
{
  if (this->opts->action == options::ACTION_HELP)
    action_help(std::cerr);
  else if (this->opts->action == options::ACTION_VERSION)
    action_version(std::cerr);
  else if (this->opts->action == options::ACTION_BUILD)
    action_build();
  else
    assert(0); // Invalid action.

  return EXIT_SUCCESS;
}
