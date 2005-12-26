/* sbuild-keyfile - sbuild GKeyFile wrapper
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

#ifndef SBUILD_KEYFILE_H
#define SBUILD_KEYFILE_H

#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <tr1/tuple>

#include <boost/format.hpp>

#include "sbuild-error.h"
#include "sbuild-i18n.h"
#include "sbuild-types.h"
#include "sbuild-util.h"

namespace sbuild
{

  class keyfile
  {
  private:
    // key, value, comment
    typedef std::tr1::tuple<std::string,std::string,std::string> item_type;

    typedef std::map<std::string,item_type> item_map_type;

    // group, items, comment
    typedef std::tr1::tuple<std::string,item_map_type,std::string> group_type;

    typedef std::map<std::string,group_type> group_map_type;

  public:
    typedef runtime_error_custom<keyfile> error;

    keyfile(const std::string& file);
    keyfile(std::istream& stream);
    virtual ~keyfile();

    string_list
    get_groups() const;

    string_list
    get_keys(const std::string& group) const;

    bool
    has_group(const std::string& group) const;

    bool
    has_key(const std::string& group,
	    const std::string& key) const;

    template <typename T>
    bool
    get_value(const std::string& group,
	      const std::string& key,
	      T& value) const
    {
      const item_type *found_item = find_item(group, key);
      if (found_item)
	{
	  const std::string& strval(std::tr1::get<1>(*found_item));
	  std::istringstream is(strval);
	  T tmpval;
	  is >> tmpval;
	  if (!is.bad())
	    {
	      value = tmpval;
	      return true;
	    }
	}
      return false;
    }

    template <typename T, template <typename T> class C>
    bool
    get_list_value(const std::string& group,
		   const std::string& key,
		   C<T>& value) const
    {
      std::string item_value;
      if (get_value(group, key, item_value))
	{
	  C<T> tmplist;
	  string_list items = split_string(item_value, this->separator);
	  for (string_list::const_iterator pos = items.begin();
	       pos != items.end();
	       ++pos
	       )
	    {
	      std::istringstream is(*pos);
	      T tmpval;
	      is >> tmpval;
	      if (!is)
		return false;
	      tmplist.push_back(tmpval);
	    }
	  value = tmplist;
	}
      return false;
    }

    // Plus locale strings and string lists...
    // Support for comments?

    template <typename T>
    void
    set_value(const std::string& group,
	      const std::string& key,
	      const T& value)
    {
      std::ostringstream os;
      os << value;

      if (!has_group(group))
	this->groups.insert
	  (group_map_type::value_type(group,
				      group_type(group,
						 item_map_type(),
						 std::string())));
      group_type *found_group = find_group(group);
      assert (found_group != 0); // should not fail

      item_map_type& items = std::tr1::get<1>(*found_group);

      item_map_type::iterator pos = items.find(key);
      if (pos != items.end())
	items.erase(pos);

      items.insert
	(item_map_type::value_type(key,
				   item_type(key, value, std::string())));

    }

    template <typename T, template <typename T> class C>
    void
    set_list_value(const std::string& group,
		   const std::string& key,
		   const C<T>& value)
    {
      std::string strval;

      for (typename C<T>::const_iterator pos = value.begin();
	   pos != value.end();
	   ++ pos)
	{
	  std::ostringstream os;
	  os << *pos;
	  if (os)
	    {
	      strval += os.str();
	      if (pos += 1 != value.end())
		strval += this->separator;
	    }
	}

      set_value (group, key, strval);
    }

    void
    remove_group(const std::string& group);

    void
    remove_key(const std::string& group,
	       const std::string& key);

    template <class charT, class traits>
    friend
    std::basic_istream<charT,traits>&
    operator >> (std::basic_istream<charT,traits>& stream, keyfile& kf)
    {
      size_t linecount = 0;
      std::string line;
      std::string group;
      std::string group_comment;
      std::string comment;
      std::string key;
      std::string value;

      while (std::getline(stream, line))
      {
	if (line[0] == '#') // Comment line
	  {
	    if (!comment.empty())
	      comment += '\n';
	    comment += line.substr(1);
	  }
	else if (line[0] == '[') // Group
	  {
	    std::string::size_type fpos = line.find_first_of(']');
	    std::string::size_type lpos = line.find_last_of(']');
	    if (fpos == std::string::npos || fpos != lpos)
	      {
		boost::format fmt(_("Line %1%: invalid group entry: %2%"));
		fmt % linecount % line;
		throw error(fmt);
	      }
	    group = line.substr(1, fpos - 2);

	    if (!comment.empty())
	      {
		if (!group_comment.empty())
		  group_comment += '\n';
		group_comment += comment;
		comment.clear();
	      }
	    // Add group
	    // Add group comment
	    // Check if group already inserted, and append comments if needed.
	  }
	else if (line.length() == 0)
	  {
	    // Do nothing.
	  }
	else
	  {
	    std::string::size_type pos = line.find_first_of('=');
	    if (pos == std::string::npos)
	      {
		boost::format fmt(_("Line %1%: invalid line: %2%"));
		fmt % linecount % line;
		throw error(fmt);
	      }
	    if (pos == 0)
	      {
		boost::format fmt(_("Line %1%: no key specified: %2%"));
		fmt % linecount % line;
		throw error(fmt);
	      }
	    key = line.substr(0, pos - 1);
	    if (pos == line.length() - 1)
	      value = "";
	    else
	      value = line.substr(pos + 1);

	    // Insert item
	    kf.set_value(group, key, value);
	    // Set item comment
	    // Set group comment?
	  }

	linecount++;
      }

      return stream;
    }

  private:
    void
    print_comment(const std::string& comment,
		  std::ostream&      stream) const;

  public:
    template <class charT, class traits>
    friend
    std::basic_ostream<charT,traits>&
    operator << (std::basic_ostream<charT,traits>& stream, const keyfile& kf)
    {
      unsigned int group_count = 0;

      for (group_map_type::const_iterator gp = kf.groups.begin();
	   gp != kf.groups.end();
	   ++gp, ++group_count)
	{
	  if (group_count > 0)
	    stream << '\n';

	  const group_type& group = gp->second;
	  const std::string& groupname = std::tr1::get<0>(group);
	  const std::string& comment = std::tr1::get<2>(group);

	  if (comment.length() > 0)
	    print_comment(comment, stream);

	  stream << '[' << groupname << ']' << '\n';

	  const item_map_type& items(std::tr1::get<1>(group));
	  for (item_map_type::const_iterator it = items.begin();
	       it != items.end();
	       ++it)
	    {
	      const item_type& item = it->second;
	      const std::string& key(std::tr1::get<0>(item));
	      const std::string& value(std::tr1::get<1>(item));
	      const std::string& comment(std::tr1::get<2>(item));

	      if (comment.length() > 0)
		print_comment(comment, stream);

	      stream << key << '=' << value;
	    }
	}

      return stream;
    }

  private:
    const group_type *
    find_group(const std::string& group) const;

    group_type *
    find_group(const std::string& group);

    const item_type *
    find_item(const std::string& group,
	      const std::string& key) const;

    item_type *
    find_item(const std::string& group,
	      const std::string& key);

    group_map_type groups;
    char           separator;
  };

}

#endif /* SBUILD_KEYFILE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
