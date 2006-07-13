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

#include "sbuild-error.h"
#include "sbuild-util.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace sbuild;

namespace
{

  /**
   * Remove duplicate adjacent characters from a string.
   *
   * @param str the string to check.
   * @param dup the duplicate character to check for.
   * @returns a string with any duplicates removed.
   */
  std::string remove_duplicates (std::string const& str,
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
sbuild::basename (std::string name,
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
sbuild::dirname (std::string name,
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
sbuild::normalname (std::string name,
		    char        separator)
{
  // Remove trailing separators
  std::string::size_type cur = name.length();
  while (cur - 1 != 0 && name[cur - 1] == separator)
    --cur;
  name.resize(cur);

  return remove_duplicates(name, separator);
}

bool
sbuild::is_absname (std::string const& name)
{
  if (name.empty() || name[0] != '/')
    return false;
  else
    return true;
}

std::string
sbuild::string_list_to_string (sbuild::string_list const& list,
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
sbuild::split_string (std::string const& value,
		      std::string const& separator)
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

std::wstring
sbuild::widen_string (std::string const& str,
		      std::locale        locale)
{
  typedef std::codecvt<wchar_t, char, mbstate_t> codecvt_type;
  codecvt_type const& cvt = std::use_facet<codecvt_type>(locale);
  mbstate_t state;
  const char *cbegin = str.data(), *cend = str.data() + str.size(), *cnext;
  wchar_t *wcnext;
  wchar_t wcbuf[80];
  std::wstring ret;

  std::memset(&state, 0, sizeof(mbstate_t));

  while (1)
    {
      std::codecvt_base::result res =
	cvt.in(state,
	       cbegin, cend, cnext,
	       wcbuf, wcbuf + (sizeof(wcbuf) / sizeof(wcbuf[0])), wcnext);

      if (res == std::codecvt_base::ok || res == std::codecvt_base::partial)
	{
	  ret += std::wstring(wcbuf, wcnext);
	  if (cend == cnext)
	    break;
	}
      else if (res == std::codecvt_base::noconv)
	{
	  ret += std::wstring(cbegin, cend);
	  break;
	}
      else if (res == std::codecvt_base::error)
	{
	  throw std::runtime_error
	    ("A character set conversion failed.  Please report this bug.");
	  break;
	}
      else
	break;

      cbegin = cnext;
    }

  return ret;
}

std::string
sbuild::narrow_string (std::wstring const& str,
		       std::locale         locale)
{
  typedef std::codecvt<wchar_t, char, mbstate_t> codecvt_type;
  codecvt_type const& cvt = std::use_facet<codecvt_type>(locale);
  mbstate_t state;
  const wchar_t *wcbegin = str.data(), *wcend = str.data() + str.size(), *wcnext;
  char *cnext;
  char cbuf[80];
  std::string ret;

  std::memset(&state, 0, sizeof(mbstate_t));

  while (1)
    {
      std::codecvt_base::result res =
	cvt.out(state,
		wcbegin, wcend, wcnext,
		cbuf, cbuf + (sizeof(cbuf) / sizeof(cbuf[0])), cnext);

      if (res == std::codecvt_base::ok || res == std::codecvt_base::partial)
	{
	  ret += std::string(cbuf, cnext);
	  if (wcend == wcnext)
	    break;
	}
      else if (res == std::codecvt_base::noconv)
	{
	  ret += std::string(wcbegin, wcend);
	  break;
	}
      else if (res == std::codecvt_base::error)
	{
	  throw std::runtime_error
	    ("A character set conversion failed.  Please report this bug.");
	  break;
	}
      else
	break;

      wcbegin = wcnext;
    }

  return ret;
}

std::string
sbuild::find_program_in_path (std::string const& program,
			      std::string const& path,
			      std::string const& prefix)
{
  if (program.find_first_of('/') != std::string::npos)
    return program;

  string_list dirs = split_string(path, std::string(1, ':'));

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

char **
sbuild::string_list_to_strv (string_list const& str)
{
  char **ret = new char *[str.size() + 1];

  for (string_list::size_type i = 0;
       i < str.size();
       ++i)
    {
      ret[i] = new char[str[i].length() + 1];
      std::strcpy(ret[i], str[i].c_str());
    }
  ret[str.size()] = 0;

  return ret;
}


void
sbuild::strv_delete (char **strv)
{
  for (char **pos = strv; pos != 0 && *pos != 0; ++pos)
    delete *pos;
  delete[] strv;
}

int
sbuild::exec (std::string const& file,
	      string_list const& command,
	      environment const& env)
{
  char **argv = string_list_to_strv(command);
  char **envp = env.get_strv();
  int status;

  if ((status = execve(file.c_str(), argv, envp)) != 0)
    {
      strv_delete(argv);
      strv_delete(envp);
    }

  return status;
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
