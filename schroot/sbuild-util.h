/* sbuild-util - sbuild utility functions
 *
 * Copyright © 2005  Roger Leigh <rleigh@debian.org>
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

#ifndef SBUILD_UTIL_H
#define SBUILD_UTIL_H

#include <string>

#include "sbuild-types.h"

namespace sbuild
{

  std::string
  basename(std::string name,
	   char        separator = '/');

  std::string
  dirname(std::string name,
	  char        separator = '/');

  std::string
  format_string(const char *format, ...);

  std::string
  string_list_to_string(string_list const& list,
			std::string const& separator);

  string_list
  split_string(const std::string& value,
	       char               separator);

  std::string
  find_program_in_path(const std::string& program);

}

#endif /* SBUILD_UTIL_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
