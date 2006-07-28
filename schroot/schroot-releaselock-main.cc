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

#include "schroot-releaselock-main.h"

#include <cerrno>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <locale>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <boost/format.hpp>

#include <lockdev.h>

using std::endl;
using boost::format;
using namespace schroot_releaselock;

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
  schroot_base::main("schroot-releaselock",
		     // TRANSLATORS: Please use an ellipsis e.g. U+2026
		     N_("[OPTION...] - release a device lock"),
		     options),
  opts(options)
{
}

main::~main ()
{
}

void
main::action_releaselock ()
{
  if (this->opts->pid == 0)
    {
      sbuild::log_warning() << _("No pid specified; forcing release of lock")
			    << endl;
    }

  struct stat statbuf;

  if (stat(this->opts->device.c_str(), &statbuf) == -1)
    throw error(this->opts->device, DEVICE_STAT, strerror(errno));

  if (!S_ISBLK(statbuf.st_mode))
    throw error(this->opts->device, DEVICE_NOTBLOCK);

  pid_t status = dev_unlock(this->opts->device.c_str(), this->opts->pid);
  if (status < 0) // Failure
    throw error(this->opts->device, DEVICE_RELEASE);
  else if (status > 0) // Owned
    throw error(this->opts->device, DEVICE_OWNED, status);
}

int
main::run_impl ()
{
  if (this->opts->action == options::ACTION_HELP)
    action_help(std::cerr);
  else if (this->opts->action == options::ACTION_VERSION)
    action_version(std::cerr);
  else if (this->opts->action == options::ACTION_RELEASELOCK)
    action_releaselock();
  else
    assert(0); // Invalid action.

  return EXIT_SUCCESS;
}
