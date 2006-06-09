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

#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <locale>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <mntent.h>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <lockdev.h>

#include "sbuild.h"

#include "schroot-listmounts-options.h"

using std::endl;
using boost::format;
namespace opt = boost::program_options;

using namespace schroot_listmounts;

/**
 * Print version information.
 *
 * @param stream the stream to output to.
 */
void
print_version (std::ostream& stream)
{
  stream << format(_("schroot-listmounts (Debian sbuild) %1%\n")) % VERSION
	 << _("Written by Roger Leigh\n\n")
	 << _("Copyright (C) 2004-2006 Roger Leigh\n")
	 << _("This is free software; see the source for copying conditions.  There is NO\n"
	      "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n")
	 << std::flush;
}

/**
 * List mounts.
 *
 * @param mountfile the file containing the database of mounted filesystems.
 * @param mountpoint the mount point to check for.
 */
sbuild::string_list
list_mounts (std::string const& mountfile,
	     std::string const& mountpoint)
{
  sbuild::string_list ret;

  std::string to_find = sbuild::normalname(mountpoint);

  std::FILE *mntdb = std::fopen(mountfile.c_str(), "r");
  if (mntdb == 0)
    {
      format fmt(_("%1%: Failed to open: %2%"));
      fmt % mountfile % std::strerror(errno);
      throw sbuild::runtime_error(fmt.str());
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
      format fmt(_("%1%: Failed to close: %2%"));
      fmt % mountfile % std::strerror(errno);
      throw sbuild::runtime_error(fmt.str());
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
  try
    {
      // Set up locale.
      std::locale::global(std::locale(""));
      std::cout.imbue(std::locale());
      std::cerr.imbue(std::locale());

      bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
      textdomain (GETTEXT_PACKAGE);

#ifndef SBUILD_DEBUG
      sbuild::debug_level = sbuild::DEBUG_CRITICAL;
#else
      sbuild::debug_level = sbuild::DEBUG_NONE;
#endif

      // Parse command-line options.
      options opts(argc, argv);

      if (opts.version)
	{
	  print_version(std::cerr);
	  exit(EXIT_SUCCESS);
	}

      if (opts.mountpoint.empty())
	{
	  sbuild::log_error() << _("No mountpoint specified") << endl;
	  exit (EXIT_FAILURE);
	}

      // Check mounts.
      sbuild::string_list mounts =
	list_mounts("/proc/mounts", opts.mountpoint);

      for (sbuild::string_list::const_reverse_iterator pos = mounts.rbegin();
	   pos != mounts.rend();
	   ++pos)
	std::cout << *pos << '\n';
      std::cout << std::flush;

      exit (EXIT_SUCCESS);
    }
  catch (std::exception const& e)
    {
      sbuild::log_error() << e.what() << endl;

      exit(EXIT_FAILURE);
    }
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
