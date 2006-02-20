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

#include "sbuild.h"

#include "dchroot-chroot-config.h"

#include <iostream>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <boost/format.hpp>

using std::endl;
using boost::format;
using namespace dchroot;

chroot_config::chroot_config ():
  sbuild::chroot_config()
{
}

chroot_config::chroot_config (std::string const& file,
			      bool               active):
  sbuild::chroot_config(file, active)
{
}

chroot_config::~chroot_config ()
{
}

void
chroot_config::parse_data (std::istream& stream,
			    bool          active)
{
  active = false; // dchroot does not support sessions.

  size_t linecount = 0;
  std::string line;
  std::string chroot_name;
  std::string chroot_location;
  bool default_set = false;

  while (std::getline(stream, line))
    {
      linecount++;

      if (line[0] == '#')
	{
	  // Comment line; do nothing.
	}
      else if (line.length() == 0)
	{
	  // Empty line; do nothing.
	}
      else // Item
	{
	  static const char *whitespace = " \t";

	  // Get chroot name
	  std::string::size_type cstart = line.find_first_not_of(whitespace);
	  std::string::size_type cend = line.find_first_of(whitespace, cstart);

	  // Get chroot location
	  std::string::size_type lstart = line.find_first_not_of(whitespace,
								 cend);
	  std::string::size_type lend = line.find_first_of(whitespace, lstart);

	  // Check for trailing non-whitespace.
	  std::string::size_type tstart = line.find_first_not_of(whitespace,
								 lend);

	  if (cstart == std::string::npos ||
	      cend == std::string::npos ||
	      lstart == std::string::npos ||
	      tstart != std::string::npos)
	    {
	      boost::format fmt(_("line %1%: invalid line: %2%"));
	      fmt % linecount % line;
	      throw error(fmt);
	    }

	  std::string chroot_name = line.substr(cstart, cend - cstart);
	  std::string location = line.substr(lstart, lend - lstart);

	  /* Create chroot object. */
	  sbuild::chroot::ptr chroot = sbuild::chroot::create("plain");
	  chroot->set_active(active);
	  chroot->set_name(chroot_name);
	  chroot->set_description(chroot_name + " chroot (dchroot compatibility)");

	  sbuild::chroot_plain *plain =
	    dynamic_cast<sbuild::chroot_plain *>(chroot.get());
	  plain->set_location(location);

	  if (!default_set)
	    {
	      sbuild::string_list aliases;
	      aliases.push_back("default");
	      chroot->set_aliases(aliases);
	      default_set = true;
	    }

	  add(chroot);
	}
    }
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
