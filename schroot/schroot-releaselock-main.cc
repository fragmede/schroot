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

main::main (options::ptr& options):
  schroot_base::main("schroot-releaselock",
		     _("[OPTION...] - release a device lock"),
		     options),
  options(options)
{
}

main::~main ()
{
}

void
main::action_releaselock ()
{
  if (this->options->pid == 0)
    {
      sbuild::log_warning() << _("No pid specified; forcing release of lock")
			    << endl;
    }

  struct stat statbuf;

  if (stat(this->options->device.c_str(), &statbuf) == -1)
    {
      sbuild::log_error()
	<< format(_("Failed to stat device '%1%': %2%"))
	% this->options->device % strerror(errno)
	<< endl;
      exit (EXIT_FAILURE);
    }
  if (!S_ISBLK(statbuf.st_mode))
    {
      sbuild::log_error()
	<< format(_("'%1%' is not a block device")) % this->options->device
	<< endl;
      exit (EXIT_FAILURE);
    }

  pid_t status = dev_unlock(this->options->device.c_str(), this->options->pid);
  if (status < 0) // Failure
    {
      sbuild::log_error()
	<< format(_("%1%: failed to release device lock"))
	% this->options->device
	<< endl;
      exit (EXIT_FAILURE);
    }
  else if (status > 0) // Owned
    {
      sbuild::log_error()
	<< format(_("%1%: failed to release device lock owned by pid %2%"))
	% this->options->device % status
	<< endl;
      exit (EXIT_FAILURE);
    }
}

int
main::run_impl ()
{
  if (this->options->action == options::ACTION_HELP)
    action_help(std::cerr);
  else if (this->options->action == options::ACTION_VERSION)
    action_version(std::cerr);
  else if (this->options->action == options::ACTION_RELEASELOCK)
    action_releaselock();
  else
    assert(0); // Invalid action.

  return EXIT_SUCCESS;
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
