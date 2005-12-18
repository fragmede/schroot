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

#include "sbuild-util.h"

using namespace sbuild;

namespace
{

  std::string remove_duplicates(const std::string& str,
				char               dup)
  {
    std::string ret;

    for (std::string::size_type pos = 0;
	 pos < str.length();
	 ++pos)
      {
	ret += str[pos];
	if (str[pos] == dup)
	  {
	    while (pos + 1 < str.length() &&
		   str[pos + 1] == dup)
	      ++pos;
	  }
      }

    return ret;
  }

}

std::string
sbuild::basename(std::string name,
		 char        separator)
{
  // Remove trailing separators
  std::string::size_type cur = name.length();
  while (cur - 1 != 0 && name[cur - 1] == separator)
    --cur;
  name.resize(cur);

  // Find last separator
  std::string::size_type pos = name.rfind(separator);

  std::string ret;
  if (pos == std::string::npos)
    ret = name; // No separators
  else if (pos == 0 && name.length() == 1 && name[0] == separator)
    ret = separator; // Only separators
  else
    ret = name.substr(pos + 1); // Basename only

  return remove_duplicates(ret, separator);
}

std::string
sbuild::dirname(std::string name,
		char        separator)
{
  // Remove trailing separators
  std::string::size_type cur = name.length();
  while (cur - 1 != 0 && name[cur - 1] == separator)
    --cur;
  name.resize(cur);

  // Find last separator
  std::string::size_type pos = name.rfind(separator);

  std::string ret;
  if (pos == std::string::npos)
    ret = "."; // No directory components
  else if (pos == 0)
    ret = separator;
  else
    ret =  name.substr(0, pos); // Dirname part

  return remove_duplicates(ret, separator);
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
