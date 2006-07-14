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

#include "sbuild-keyfile.h"

#include <fstream>

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

namespace
{

  typedef std::pair<keyfile::error_code,const char *> emap;

  /**
   * This is a list of the supported error codes.  It's used to
   * construct the real error codes map.
   */
  emap init_errors[] =
    {
      emap(keyfile::BAD_FILE,          N_("Can't open file '%4%'")),
      emap(keyfile::DISALLOWED_KEY,    N_("line %1% [%2%]: Disallowed key '%4%' used")),
      emap(keyfile::DISALLOWED_KEY_NL, N_("[%2%]: Disallowed key '%4%' used")),
      emap(keyfile::DUPLICATE_GROUP,   N_("line %1%: Duplicate group '%4%'")),
      emap(keyfile::DUPLICATE_KEY,     N_("line %1% [%2%]: Duplicate key '%4%'")),
      emap(keyfile::INVALID_GROUP,     N_("line %1%: Invalid group: \"%4%\"")),
      emap(keyfile::INVALID_LINE,      N_("line %1%: Invalid line: \"%4%\"")),
      emap(keyfile::MISSING_KEY,       N_("[%1%]: Required key '%4%' is missing")),
      emap(keyfile::NO_GROUP,          N_("line %1%: No group specified: \"%4%\"")),
      emap(keyfile::NO_KEY,            N_("line %1%: No key specified: \"%4%\"")),
      emap(keyfile::PASSTHROUGH_GK,    N_("[%1%] %2%: %4%")),
      emap(keyfile::PASSTHROUGH_LGK,   N_("line %1% [%2%] %3%: %4%"))
    };

}

template<>
error<keyfile::error_code>::map_type
error<keyfile::error_code>::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));

keyfile::keyfile ():
  groups(),
  separator(',')
{
}

keyfile::keyfile (std::string const& file):
  groups(),
  separator(',')
{
  std::ifstream fs(file.c_str());
  if (fs)
    {
      fs.imbue(std::locale("C"));
      fs >> *this;
    }
  else
    {
      throw error(BAD_FILE, file);
    }
}

keyfile::keyfile (std::istream& stream):
  groups(),
  separator(',')
{
  stream >> *this;
}

keyfile::~keyfile()
{
}

string_list
keyfile::get_groups () const
{
  string_list ret;

  for (group_map_type::const_iterator pos = this->groups.begin();
       pos != this->groups.end();
       ++pos)
    ret.push_back(pos->first);

  return ret;
}

string_list
keyfile::get_keys (std::string const& group) const
{
  string_list ret;

  const group_type *found_group = find_group(group);
  if (found_group)
    {
      item_map_type const& items(std::tr1::get<1>(*found_group));
      for (item_map_type::const_iterator pos = items.begin();
	   pos != items.end();
	   ++pos)
	ret.push_back(pos->first);
    }

  return ret;
}

bool
keyfile::has_group (std::string const& group) const
{
  return (find_group(group) != 0);
}

bool
keyfile::has_key (std::string const& group,
		  std::string const& key) const
{
  return (find_item(group, key) != 0);
}

void
keyfile::set_group (std::string const& group,
		    std::string const& comment)
{
  set_group(group, comment, 0);
}

void
keyfile::set_group (std::string const& group,
		    std::string const& comment,
		    unsigned int       line)
{
  if (!has_group(group))
    this->groups.insert
      (group_map_type::value_type(group,
				  group_type(group,
					     item_map_type(),
					     comment,
					     line)));
}

std::string
keyfile::get_comment (std::string const& group) const
{
  const keyfile::group_type *found_group = find_group(group);
  if (found_group)
    return std::tr1::get<2>(*found_group);
  else
    return std::string();
}

std::string
keyfile::get_comment (std::string const& group,
		      std::string const& key) const
{
  const item_type *found_item = find_item(group, key);
  if (found_item)
      return std::tr1::get<2>(*found_item);
  else
    return std::string();
}

unsigned int
keyfile::get_line (std::string const& group) const
{
  const keyfile::group_type *found_group = find_group(group);
  if (found_group)
    return std::tr1::get<3>(*found_group);
  else
    return 0;
}

unsigned int
keyfile::get_line (std::string const& group,
		   std::string const& key) const
{
  const item_type *found_item = find_item(group, key);
  if (found_item)
      return std::tr1::get<3>(*found_item);
  else
    return 0;
}

bool
keyfile::get_locale_string (std::string const& group,
			    std::string const& key,
			    std::string&       value) const
{
  std::string localename = std::locale("").name();
  std::string::size_type pos;
  bool status = false;

  // Strip off any charset.
  if ((pos = localename.find_first_of('.')) != std::string::npos)
    localename = localename.substr(0, pos);
  status = get_locale_string(group, key, localename, value);

  // Strip off territory.
  if (status == false &&
      (pos = localename.find_first_of('_')) != std::string::npos)
    {
      localename = localename.substr(0, pos);
      status = get_locale_string(group, key, localename, value);
    }

  // Fall back to non-localised version.
  if (status == false)
    status = get_value(group, key, value);

  return status;
}

bool
keyfile::get_locale_string (std::string const& group,
			    std::string const& key,
			    priority           priority,
			    std::string&       value) const
{
  bool status = get_locale_string(group, key, value);
  check_priority(group, key, priority, status);
  return status;
}

bool
keyfile::get_locale_string (std::string const& group,
			    std::string const& key,
			    std::string const& locale,
			    std::string&       value) const
{
  std::string lkey = key + '[' + locale + ']';
  return get_value(group, lkey, value);
}

bool
keyfile::get_locale_string (std::string const& group,
			    std::string const& key,
			    std::string const& locale,
			    priority           priority,
			    std::string&       value) const
{
  bool status = get_locale_string(group, key, locale, value);
  check_priority(group, key, priority, status);
  return status;
}

void
keyfile::remove_group (std::string const& group)
{
  group_map_type::iterator pos = this->groups.find(group);
  if (pos != this->groups.end())
    this->groups.erase(pos);
}

void
keyfile::remove_key (std::string const& group,
		     std::string const& key)
{
  group_type *found_group = find_group(group);
  if (found_group)
    {
      item_map_type& items = std::tr1::get<1>(*found_group);
      item_map_type::iterator pos = items.find(key);
      if (pos != items.end())
	items.erase(pos);
    }
}

keyfile&
keyfile::operator += (keyfile const& rhs)
{
  for (group_map_type::const_iterator gp = rhs.groups.begin();
       gp != rhs.groups.end();
       ++gp)
    {
      group_type const& group = gp->second;
      std::string const& groupname = std::tr1::get<0>(group);
      std::string const& comment = std::tr1::get<2>(group);
      unsigned int const& line = std::tr1::get<3>(group);
      set_group(groupname, comment, line);

      item_map_type const& items(std::tr1::get<1>(group));
      for (item_map_type::const_iterator it = items.begin();
	   it != items.end();
	   ++it)
	{
	  item_type const& item = it->second;
	  std::string const& key(std::tr1::get<0>(item));
	  std::string const& value(std::tr1::get<1>(item));
	  std::string const& comment(std::tr1::get<2>(item));
	  unsigned int const& line(std::tr1::get<3>(item));
	  set_value(groupname, key, value, comment, line);
	}
    }
  return *this;
}

keyfile
operator + (keyfile const& lhs,
	    keyfile const& rhs)
{
  keyfile ret(lhs);
  ret += rhs;
  return ret;
}

const keyfile::group_type *
keyfile::find_group (std::string const& group) const
{
  group_map_type::const_iterator pos = this->groups.find(group);
  if (pos != this->groups.end())
    return &pos->second;

  return 0;
}

keyfile::group_type *
keyfile::find_group (std::string const& group)
{
  group_map_type::iterator pos = this->groups.find(group);
  if (pos != this->groups.end())
    return &pos->second;

  return 0;
}

const keyfile::item_type *
keyfile::find_item (std::string const& group,
		    std::string const& key) const
{
  const group_type *found_group = find_group(group);
  if (found_group)
    {
      item_map_type const& items = std::tr1::get<1>(*found_group);
      item_map_type::const_iterator pos = items.find(key);
      if (pos != items.end())
	return &pos->second;
    }

  return 0;
}

keyfile::item_type *
keyfile::find_item (std::string const& group,
		    std::string const& key)
{
  group_type *found_group = find_group(group);
  if (found_group)
    {
      item_map_type& items = std::tr1::get<1>(*found_group);
      item_map_type::iterator pos = items.find(key);
      if (pos != items.end())
	return &pos->second;
    }

  return 0;
}

void
keyfile::print_comment (std::string const& comment,
			std::ostream&      stream)
{
  std::string::size_type last_pos = 0;
  std::string::size_type pos = comment.find_first_of('\n', last_pos);

  while (1)
    {
      if (last_pos == pos)
	stream << "#\n";
      else
	stream << '#' << comment.substr(last_pos, pos - last_pos) << '\n';

      // Find next
      if (pos < comment.length() - 1)
	{
	  last_pos = pos + 1;
	  pos = comment.find_first_of('\n', last_pos);
	}
      else
	break;
    }
}

void
keyfile::check_priority (std::string const& group,
			 std::string const& key,
			 priority priority,
			 bool     valid) const
{
  if (valid == false)
    {
      switch (priority)
	{
	case PRIORITY_REQUIRED:
	  {
	    throw error(group, MISSING_KEY, key);
	  }
	  break;
	default:
	  break;
	}
    }
  else
    {
      switch (priority)
	{
	case PRIORITY_DEPRECATED:
	  log_warning()
	    << format(_("%1% chroot: A deprecated parameter '%2%' has been specified."))
	    % group % key
	    << std::endl;
	  log_info()
	    << _("This option will be removed in the future.") << std::endl;
	  break;
	case PRIORITY_OBSOLETE:
	  log_warning()
	    << format(_("%1% chroot: An obsolete parameter '%2%' has been specified."))
	    % group % key
	    << std::endl;
	  log_info()
	    << _("This option has been removed, and no longer has any effect.") << std::endl;
	  break;
	case PRIORITY_DISALLOWED:
	  {
	    unsigned int line = get_line(group, key);
	    if (line)
	      throw error(line, group, DISALLOWED_KEY, key);
	    else
	      throw error(group, DISALLOWED_KEY_NL, key);
	  }
	  break;
	    default:
	      break;
	}
    }
}
