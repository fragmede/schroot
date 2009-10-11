/* Copyright © 2005-2007  Roger Leigh <rleigh@debian.org>
 *
 * schroot is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * schroot is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 *********************************************************************/

#include <config.h>

#include "sbuild-chroot.h"
#include "sbuild-error.h"
#include "sbuild-util.h"

#include <cerrno>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#ifdef HAVE_UUID
#include <uuid/uuid.h>
#else
#include <time.h>
#endif

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

  typedef std::pair<sbuild::stat::error_code,const char *> emap;

  /**
   * This is a list of the supported error codes.  It's used to
   * construct the real error codes map.
   */
  emap init_errors[] =
    {
      emap(sbuild::stat::FILE, N_("Failed to stat file")),
      emap(sbuild::stat::FD,   N_("Failed to stat file descriptor"))
    };

  bool chroot_alphasort (sbuild::chroot::ptr const& c1,
			 sbuild::chroot::ptr const& c2)
  {
    return c1->get_name() < c2->get_name();
  }

}

template<>
error<sbuild::stat::error_code>::map_type
error<sbuild::stat::error_code>::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));

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

bool
sbuild::is_valid_filename (std::string const& name,
			   bool               lsb_mode)
{
  bool match = false;

  if (lsb_mode)
    {
      static regex lanana_namespace("^[a-z0-9]+$");
      static regex lsb_namespace("^_?([a-z0-9_.]+-)+[a-z0-9]+$");
      static regex debian_cron_namespace("^[a-z0-9][a-z0-9-]*$");
      static regex debian_dpkg_conffile_cruft("dpkg-(old|dist|new|tmp)$");

      if ((regex_search(name, lanana_namespace) ||
	   regex_search(name, lsb_namespace) ||
	   regex_search(name, debian_cron_namespace)) &&
	  !regex_search(name, debian_dpkg_conffile_cruft))
	match = true;
    }
  else
    {
      static regex traditional_namespace("^[a-zA-Z0-9_-]$");
      if (regex_search(name, traditional_namespace))
	match = true;
    }

  return match;
}

std::string
sbuild::getcwd ()
{
  std::string cwd;

  char *raw_cwd = ::getcwd (0, 0);
  if (raw_cwd)
    cwd = raw_cwd;
  else
    cwd = "/";
  free(raw_cwd);

  return cwd;
}

std::string
sbuild::unique_identifier ()
{
  std::ostringstream id;
  id.imbue(std::locale::classic());

#ifdef HAVE_UUID
  uuid_t uuid;
  char uuid_str[37];
  uuid_generate(uuid);
  uuid_unparse(uuid, uuid_str);
  uuid_clear(uuid);

  id << uuid_str;
#else
  id << isodate(time(0)) << '-' << getpid();
#endif

  return id.str();
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
      if (pos == std::string::npos)
	// Entire string from last_pos
	ret.push_back(value.substr(last_pos, pos));
      else
	// Between pos and last_pos
	ret.push_back(value.substr(last_pos, pos - last_pos));

      // Find next
      last_pos = value.find_first_not_of(separator, pos);
      pos = value.find_first_of(separator, last_pos);
    }

  return ret;
}

string_list
sbuild::split_string_strict (std::string const& value,
			     std::string const& separator)
{
  string_list ret;

  std::string::size_type last_pos = 0;
  // Find first separator.
  std::string::size_type pos = value.find_first_of(separator, last_pos);

  while (pos !=std::string::npos)
    {
      // Add to list
      if (pos == std::string::npos)
	// Entire string from last_pos
	ret.push_back(value.substr(last_pos, pos));
      else
	// Between pos and last_pos
	ret.push_back(value.substr(last_pos, pos - last_pos));

      // Find next
      last_pos = pos + separator.length();
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

      try
	{
	  if (stat(absname).is_regular() &&
	      access (absname.c_str(), X_OK) == 0)
	    return realname;
	}
      catch (std::runtime_error const& e)
	{
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

sbuild::stat::stat (const char *file):
  file(file),
  fd(0),
  errorno(0),
  status()
{
  if (::stat(file, &this->status) < 0)
    this->errorno = errno;
}

sbuild::stat::stat (std::string const& file):
  file(file),
  fd(0),
  errorno(0),
  status()
{
  if (::stat(file.c_str(), &this->status) < 0)
    this->errorno = errno;
}

sbuild::stat::stat (std::string const& file,
		    int                fd):
  file(file),
  fd(fd),
  errorno(0),
  status()
{
  if (::fstat(fd, &this->status) < 0)
    this->errorno = errno;
}

sbuild::stat::stat (int fd):
  file(),
  fd(fd),
  errorno(0),
  status()
{
  if (::fstat(fd, &this->status) < 0)
    this->errorno = errno;
}

sbuild::stat::~stat ()
{
}

sbuild::passwd::passwd ():
  ::passwd(),
  buffer(),
  valid(false)
{
  clear();
}

sbuild::passwd::passwd (uid_t uid):
  ::passwd(),
  buffer(),
  valid(false)
{
  clear();

  query_uid(uid);
}

sbuild::passwd::passwd (const char *name):
  ::passwd(),
  buffer(),
  valid(false)
{
  clear();

  query_name(name);
}

sbuild::passwd::passwd (std::string const& name):
  ::passwd(),
  buffer(),
  valid(false)
{
  clear();

  query_name(name);
}

void
sbuild::passwd::clear ()
{
  valid = false;

  buffer.clear();

  ::passwd::pw_name = 0;
  ::passwd::pw_passwd = 0;
  ::passwd::pw_uid = 0;
  ::passwd::pw_gid = 0;
  ::passwd::pw_gecos = 0;
  ::passwd::pw_dir = 0;
  ::passwd::pw_shell = 0;
}

void
sbuild::passwd::query_uid (uid_t uid)
{
  buffer_type::size_type size = 1 << 7;
  buffer.reserve(size);
  int error;

  ::passwd *pwd_result;

  while ((error = getpwuid_r(uid, this,
			     &buffer[0], buffer.capacity(),
			     &pwd_result)))
    {
      size <<= 1;
      buffer.reserve(size);
    }

  if (pwd_result)
    valid = true;
  else
    errno = error;
}

void
sbuild::passwd::query_name (const char *name)
{
  buffer_type::size_type size = 1 << 8;
  buffer.reserve(size);
  int error;

  ::passwd *pwd_result;

  while ((error = getpwnam_r(name, this,
			     &buffer[0], buffer.capacity(),
			     &pwd_result)))
    {
      size <<= 1;
      buffer.reserve(size);
    }

  if (pwd_result)
    valid = true;
  else
    errno = error;
}

void
sbuild::passwd::query_name (std::string const& name)
{
  query_name(name.c_str());
}

bool
sbuild::passwd::operator ! () const
{
  return !valid;
}

sbuild::group::group ():
  ::group(),
  buffer(),
  valid(false)
{
  clear();
}

sbuild::group::group (gid_t gid):
  ::group(),
  buffer(),
  valid(false)
{
  clear();

  query_gid(gid);
}

sbuild::group::group (const char *name):
  ::group(),
  buffer(),
  valid(false)
{
  clear();

  query_name(name);
}

sbuild::group::group (std::string const& name):
  ::group(),
  buffer(),
  valid(false)
{
  clear();

  query_name(name);
}

void
sbuild::group::clear ()
{
  valid = false;

  buffer.clear();

  ::group::gr_name = 0;
  ::group::gr_passwd = 0;
  ::group::gr_gid = 0;
  ::group::gr_mem = 0;
}

void
sbuild::group::query_gid (gid_t gid)
{
  buffer_type::size_type size = 1 << 7;
  buffer.reserve(size);
  int error;

  ::group *grp_result;

  while ((error = getgrgid_r(gid, this,
			     &buffer[0], buffer.capacity(),
			     &grp_result)))
    {
      size <<= 1;
      buffer.reserve(size);
    }

  if (grp_result)
    valid = true;
  else
    errno = error;
}

void
sbuild::group::query_name (const char *name)
{
  buffer_type::size_type size = 1 << 8;
  buffer.reserve(size);
  int error;

  ::group *grp_result;

  while ((error = getgrnam_r(name, this,
			     &buffer[0], buffer.capacity(),
			     &grp_result)))
    {
      size <<= 1;
      buffer.reserve(size);
    }

  if (grp_result)
    valid = true;
  else
    errno = error;
}

void
sbuild::group::query_name (std::string const& name)
{
  query_name(name.c_str());
}

bool
sbuild::group::operator ! () const
{
  return !valid;
}
