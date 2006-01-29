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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace sbuild;

namespace
{

  std::string remove_duplicates(std::string const& str,
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
    ret = name.substr(0, pos); // Dirname part

  return remove_duplicates(ret, separator);
}

std::string
sbuild::string_list_to_string(sbuild::string_list const& list,
			      std::string const&         separator)
{
  std::string ret;

  for (string_list::const_iterator cur = list.begin();
       cur != list.end();
       ++cur)
    {
      ret += *cur;
      if (cur + 1 != list.end())
	ret += separator;
    }

  return ret;
}

string_list
sbuild::split_string(std::string const& value,
		     char               separator)
{
  string_list ret;

  // Skip any separators at the start
  std::string::size_type last_pos =
    value.find_first_not_of(separator, 0);
  // Find first separator.
  std::string::size_type pos = value.find_first_of(separator, last_pos);

  while (pos !=std::string::npos || last_pos != std::string::npos)
    {
      // Add to list
      ret.push_back(value.substr(last_pos, pos - last_pos));
      // Find next
      last_pos = value.find_first_not_of(separator, pos);
      pos = value.find_first_of(separator, last_pos);
    }

  return ret;
}

std::string
sbuild::find_program_in_path(std::string const& program,
			     std::string const& path,
			     std::string const& prefix)
{
  if (program.find_first_of('/') != std::string::npos)
    return program;

  string_list dirs = split_string(path, ':');

  for (string_list::const_iterator dir = dirs.begin();
       dir != dirs.end();
       ++dir)
    {
      std::string realname = *dir + '/' + program;
      std::string absname;
      if (prefix.length() > 0)
	{
	  absname = prefix;
	  if (dir->length() > 0 && (*dir)[0] != '/')
	    absname += '/';
	}
      absname += realname;

      struct stat statbuf;
      if (stat(absname.c_str(), &statbuf) == 0)
	{
	  if (S_ISREG(statbuf.st_mode) &&
	      access (absname.c_str(), X_OK) == 0)
	    return realname;
	}
    }

  return "";
}

void
sbuild::strv_delete(char **strv)
{
  for (char **pos = strv; pos != 0 && *pos != 0; ++pos)
    delete *pos;
  delete[] strv;
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
