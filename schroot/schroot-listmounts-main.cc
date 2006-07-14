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

#include "schroot-listmounts-main.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <locale>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <boost/format.hpp>

#include <mntent.h>

#include <lockdev.h>

using std::endl;
using boost::format;
using namespace schroot_listmounts;

main::main (options::ptr& options):
  schroot_base::main("schroot-listmounts",
		     _("[OPTION...] - list mount points"),
		     options),
  options(options)
{
}

main::~main ()
{
}

sbuild::string_list
main::list_mounts (std::string const& mountfile) const
{
  sbuild::string_list ret;

  std::string to_find = sbuild::normalname(this->options->mountpoint);

  std::FILE *mntdb = std::fopen(mountfile.c_str(), "r");
  if (mntdb == 0)
    {
      format fmt(_("Failed to open '%1%': %2%"));
      fmt % mountfile % std::strerror(errno);
      throw std::runtime_error(fmt.str());
    }

  mntent *mount;
  while ((mount = getmntent(mntdb)) != 0)
    {
      std::string mount_dir(mount->mnt_dir);
      if (to_find == "/" ||
	  (mount_dir.find(to_find) == 0 &&
	   (// Names are the same.
	    mount_dir.size() == to_find.size() ||
	    // Must have a following /, or not the same directory.
	    (mount_dir.size() > to_find.size() &&
	     mount_dir[to_find.size()] == '/'))))
	ret.push_back(mount_dir);
    }

  std::cout << std::flush;

  if (std::fclose(mntdb) == EOF)
    {
      format fmt(_("Failed to close '%1%': %2%"));
      fmt % mountfile % std::strerror(errno);
      throw std::runtime_error(fmt.str());
    }

  return ret;
}

void
main::action_listmounts ()
{
  // Check mounts.
  sbuild::string_list mounts =
    list_mounts("/proc/mounts");

  for (sbuild::string_list::const_reverse_iterator pos = mounts.rbegin();
       pos != mounts.rend();
       ++pos)
    std::cout << *pos << '\n';
  std::cout << std::flush;
}

int
main::run_impl ()
{
  if (this->options->action == options::ACTION_HELP)
    action_help(std::cerr);
  else if (this->options->action == options::ACTION_VERSION)
    action_version(std::cerr);
  else if (this->options->action == options::ACTION_LISTMOUNTS)
    action_listmounts();
  else
    assert(0); // Invalid action.

  return EXIT_SUCCESS;
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
