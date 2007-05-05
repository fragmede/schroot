/* Copyright © 2005-2007  Roger Leigh <rleigh@debian.org>
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
      fs.imbue(std::locale::classic());
      fs >> *this;
    }
  else
    {
      throw error(file, BAD_FILE);
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
keyfile::get_keys (group_name_type const& group) const
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
keyfile::has_group (group_name_type const& group) const
{
  return (find_group(group) != 0);
}

bool
keyfile::has_key (group_name_type const& group,
		  key_type const& key) const
{
  return (find_item(group, key) != 0);
}

void
keyfile::set_group (group_name_type const& group,
		    comment_type const&    comment)
{
  set_group(group, comment, 0);
}

void
keyfile::set_group (group_name_type const& group,
		    comment_type const& comment,
		    size_type       line)
{
  if (!has_group(group))
    this->groups.insert
      (group_map_type::value_type(group,
				  group_type(group,
					     item_map_type(),
					     comment,
					     line)));
}

keyfile::comment_type
keyfile::get_comment (group_name_type const& group) const
{
  const keyfile::group_type *found_group = find_group(group);
  if (found_group)
    return std::tr1::get<2>(*found_group);
  else
    return comment_type();
}

keyfile::comment_type
keyfile::get_comment (group_name_type const& group,
		      key_type const& key) const
{
  const item_type *found_item = find_item(group, key);
  if (found_item)
      return std::tr1::get<2>(*found_item);
  else
    return comment_type();
}

keyfile::size_type
keyfile::get_line (group_name_type const& group) const
{
  const keyfile::group_type *found_group = find_group(group);
  if (found_group)
    return std::tr1::get<3>(*found_group);
  else
    return 0;
}

keyfile::size_type
keyfile::get_line (group_name_type const& group,
		   key_type const& key) const
{
  const item_type *found_item = find_item(group, key);
  if (found_item)
      return std::tr1::get<3>(*found_item);
  else
    return 0;
}

bool
keyfile::get_locale_string (group_name_type const& group,
			    key_type const& key,
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
keyfile::get_locale_string (group_name_type const& group,
			    key_type const& key,
			    priority           priority,
			    std::string&       value) const
{
  bool status = get_locale_string(group, key, value);
  check_priority(group, key, priority, status);
  return status;
}

bool
keyfile::get_locale_string (group_name_type const& group,
			    key_type const& key,
			    std::string const& locale,
			    std::string&       value) const
{
  std::string lkey = key + '[' + locale + ']';
  return get_value(group, lkey, value);
}

bool
keyfile::get_locale_string (group_name_type const& group,
			    key_type const& key,
			    std::string const& locale,
			    priority           priority,
			    std::string&       value) const
{
  bool status = get_locale_string(group, key, locale, value);
  check_priority(group, key, priority, status);
  return status;
}

void
keyfile::remove_group (group_name_type const& group)
{
  group_map_type::iterator pos = this->groups.find(group);
  if (pos != this->groups.end())
    this->groups.erase(pos);
}

void
keyfile::remove_key (group_name_type const& group,
		     key_type const& key)
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
      group_name_type const& groupname = std::tr1::get<0>(group);
      comment_type const& comment = std::tr1::get<2>(group);
      size_type const& line = std::tr1::get<3>(group);
      set_group(groupname, comment, line);

      item_map_type const& items(std::tr1::get<1>(group));
      for (item_map_type::const_iterator it = items.begin();
	   it != items.end();
	   ++it)
	{
	  item_type const& item = it->second;
	  key_type const& key(std::tr1::get<0>(item));
	  value_type const& value(std::tr1::get<1>(item));
	  comment_type const& comment(std::tr1::get<2>(item));
	  size_type const& line(std::tr1::get<3>(item));
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
keyfile::find_group (group_name_type const& group) const
{
  group_map_type::const_iterator pos = this->groups.find(group);
  if (pos != this->groups.end())
    return &pos->second;

  return 0;
}

keyfile::group_type *
keyfile::find_group (group_name_type const& group)
{
  group_map_type::iterator pos = this->groups.find(group);
  if (pos != this->groups.end())
    return &pos->second;

  return 0;
}

const keyfile::item_type *
keyfile::find_item (group_name_type const& group,
		    key_type const& key) const
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
keyfile::find_item (group_name_type const& group,
		    key_type const& key)
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
keyfile::print_comment (comment_type const& comment,
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
keyfile::check_priority (group_name_type const& group,
			 key_type const& key,
			 priority priority,
			 bool     valid) const
{
  if (valid == false)
    {
      size_type gline = get_line(group);

      switch (priority)
	{
	case PRIORITY_REQUIRED:
	  {
	    if (gline)
	      throw error(gline, group, MISSING_KEY, key);
	    else
	      throw error(group, MISSING_KEY_NL, key);
	  }
	  break;
	default:
	  break;
	}
    }
  else
    {
      size_type line = get_line(group, key);

      switch (priority)
	{
	case PRIORITY_DEPRECATED:
	  {
	    if (line)
	      {
		error e(line, group, DEPRECATED_KEY, key);
		e.set_reason(_("This option will be removed in the future"));
		log_exception_warning(e);
	      }
	    else
	      {
		error e(group, DEPRECATED_KEY_NL, key);
		e.set_reason(_("This option will be removed in the future"));
		log_exception_warning(e);
	      }
	  }
	  break;
	case PRIORITY_OBSOLETE:
	  {
	    if (line)
	      {
		error e(line, group, OBSOLETE_KEY, key);
		e.set_reason(_("This option has been removed, and no longer has any effect"));
		log_exception_warning(e);
	      }
	    else
	      {
		error e(group, OBSOLETE_KEY_NL, key);
		e.set_reason(_("This option has been removed, and no longer has any effect"));
		log_exception_warning(e);
	      }
	  }
	  break;
	case PRIORITY_DISALLOWED:
	  {
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
