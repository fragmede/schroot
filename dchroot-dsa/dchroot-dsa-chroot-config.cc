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

#include <sbuild/sbuild-chroot-plain.h>
#include <sbuild/sbuild-log.h>
#include <sbuild/sbuild-parse-error.h>

#include "dchroot-dsa-chroot-config.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <boost/format.hpp>

using std::endl;
using sbuild::keyfile;
using boost::format;
using namespace dchroot_dsa;

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

  sbuild::keyfile kconfig;

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
	  static const char *whitespace = " \t:;,";

	  // Get chroot name
	  std::string::size_type cstart = line.find_first_not_of(whitespace);
	  std::string::size_type cend = line.find_first_of(whitespace, cstart);

	  // Get chroot location
	  std::string::size_type lstart = line.find_first_not_of(whitespace,
								 cend);
	  std::string::size_type lend = line.find_first_of(whitespace, lstart);

	  if (cstart == std::string::npos)
	    throw keyfile::error(linecount, keyfile::INVALID_LINE, line);

	  std::string chroot_name = line.substr(cstart, cend - cstart);

	  std::string location;
	  if (lstart != std::string::npos)
	    location = line.substr(lstart, lend - lstart);

	  // DSA dchroot parses valid users.
	  sbuild::string_list users;
	  if (lend != std::string::npos)
	    users = sbuild::split_string(line.substr(lend), whitespace);

	  // Add details to keyfile.
	  if (kconfig.has_group(chroot_name))
	    {
	      keyfile::error e(linecount, keyfile::DUPLICATE_GROUP,
			       chroot_name);
	      sbuild::log_warning() << e.what() << std::endl;
	      continue;
	    }
	  else
	    kconfig.set_group(chroot_name, "", linecount);

	  kconfig.set_value(chroot_name, "type", "plain", "", linecount);

	  format fmt(_("%1% chroot (dchroot-dsa compatibility)"));
	  fmt % chroot_name;

	  kconfig.set_value(chroot_name, "description", fmt, "", linecount);

	  if (lstart != std::string::npos)
	    kconfig.set_value(chroot_name, "location", location, "", linecount);

	  kconfig.set_list_value(chroot_name, "users",
				 users.begin(), users.end(),
				 "", linecount);
	}
    }

  load_keyfile(kconfig, active);
}
