/* Copyright © 2005  Roger Leigh <rleigh@debian.org>
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
#include "sbuild-log.h"
#include "sbuild-types.h"
#include "sbuild-util.h"

namespace sbuild
{

  /**
   * Configuration file parser.  This class loads an INI-style
   * configuration file from disk.  The format is documented in
   * schroot.conf(5).  It is based upon the Glib GKeyFile class, which
   * it is intended to replace.
   *
   * @todo Add support for locale strings.
   * @todo Add support for comments.
   */
  class keyfile
  {
  private:
    /// Key-value-comment tuple.
    typedef std::tr1::tuple<std::string,std::string,std::string> item_type;

    /// Map between key name and key-value-comment tuple.
    typedef std::map<std::string,item_type> item_map_type;

    /// Group-items-comment tuple.
    typedef std::tr1::tuple<std::string,item_map_type,std::string> group_type;

    /// Map between group name and group-items-comment tuple.
    typedef std::map<std::string,group_type> group_map_type;

  public:
    /// Configuration parameter priority.
    enum priority
      {
	PRIORITY_OPTIONAL,   ///< The parameter is optional.
	PRIORITY_REQUIRED,   ///< The parameter is required.
	PRIORITY_DISALLOWED, ///< The parameter is not allowed in this context.
	PRIORITY_DEPRECATED, ///< The parameter is deprecated, but functional.
	PRIORITY_OBSOLETE    ///< The parameter is obsolete, and not functional.
      };

    /// Exception type.
    typedef runtime_error_custom<keyfile> error;

    /**
     * The constructor.
     *
     * @param file the file to load the configuration from.
     */
    keyfile(const std::string& file);

    /**
     * The constructor.
     *
     * @param stream the stream to load the configuration from.
     */
    keyfile(std::istream& stream);

    /// The destructor.
    virtual ~keyfile();

    /**
     * Get a list of groups.
     *
     * @returns a list of groups in the keyfile.  If no groups exist,
     * the list will be empty.
     */
    string_list
    get_groups() const;

    /**
     * Get a list of keys in a group.
     *
     * @param group the group to use.
     * @returns a list of keys in a group.  If no keys exist in the
     * group, or the group does not exist, the list will be empty.
     */
    string_list
    get_keys(const std::string& group) const;

    /**
     * Check if a group exists.
     *
     * @param group the group to check for.
     * @returns true if the group exists, otherwise false.
     */
    bool
    has_group(const std::string& group) const;

    /**
     * Check if a key exists.
     *
     * @param group the group the key is in.
     * @param key the key to check for.
     * @returns true if the key exists, otherwise false.
     */
    bool
    has_key(const std::string& group,
	    const std::string& key) const;


  private:
    /**
     * Parse a key value and set the value of the specified value type.
     *
     * @param string val the key value.
     * @param value the value type to store the parsed value in.
     * @returns true on success, false on failure.
     */
    template <typename T>
    bool
    parse_value (std::string const& stringval,
		 T&                 value) const
    {
      std::istringstream is(stringval);
      is.imbue(std::locale("C"));
      T tmpval;
      is >> tmpval;
      if (!is.bad())
	{
	  value = tmpval;
	  log_debug(DEBUG_NOTICE) << "value=" << value << std::endl;
	  return true;
	}
      log_debug(DEBUG_NOTICE) << "parse error" << std::endl;
      return false;
    }

    bool
    parse_value (std::string const& stringval,
		 bool&              value) const
    {
      if (stringval == "true" || stringval == "yes" || stringval == "1")
	value = true;
      else if (stringval == "false" || stringval == "no" || stringval == "0")
	value = true;
      else
	return false;

      // TODO: throw exception on parse failure.

      log_debug(DEBUG_NOTICE) << "value=" << value << std::endl;
      return true;
    }

    bool
    parse_value (std::string const& stringval,
		 std::string&       value) const
    {
      value = stringval;
      log_debug(DEBUG_NOTICE) << "value=" << value << std::endl;
      return true;
    }

    void
    check_priority (const std::string& group,
		    const std::string& key,
		    priority           priority,
		    bool               valid) const;

  public:
    /**
     * Get a key value.
     *
     * @param group the group the key is in.
     * @param key the key to get.
     * @param value the value to store the key's value in.  This must
     * be settable from an istream and be copyable.
     * @returns true if the key was found, otherwise false (in which
     * case value will be unchanged).
     */
    template <typename T>
    bool
    get_value(const std::string& group,
	      const std::string& key,
	      T&                 value) const
    {
      log_debug(DEBUG_INFO) << "Getting keyfile group=" << group
			    << ", key=" << key << std::endl;
      const item_type *found_item = find_item(group, key);
      if (found_item)
	{
	  const std::string& strval(std::tr1::get<1>(*found_item));
	  return parse_value(strval, value);
	}
      log_debug(DEBUG_NOTICE) << "key not found" << std::endl;
      return false;
    }

    /**
     * Get a key value.  If the value does not exist, is deprecated or
     * obsolete, warn appropriately.
     *
     * @param group the group the key is in.
     * @param key the key to get.
     * @param priority the priority of the option.
     * @param value the value to store the key's value in.  This must
     * be settable from an istream and be copyable.
     * @returns true if the key was found, otherwise false (in which
     * case value will be unchanged).
     */
    template <typename T>
    bool
    get_value(const std::string& group,
	      const std::string& key,
	      priority           priority,
	      T&                 value) const
    {
      bool status = get_value(group, key, value);
      check_priority(group, key, priority, status);
      return status;
    }

    /**
     * Get a key value as a list.
     *
     * @param group the group the key is in.
     * @param key the key to get.
     * @param value the list value to store the key's value in.  The
     * value type must be settable from an istream and be copyable.
     * The list must be a container with a standard insert method.
     * @returns true if the key was found, otherwise false (in which
     * case value will be unchanged).
     */
    template <typename T, template <typename T> class C>
    bool
    get_list_value(const std::string& group,
		   const std::string& key,
		   C<T>&              value) const
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
	      T tmpval;
	      if (parse_value(*pos, tmpval) == false)
		return false;
	      tmplist.push_back(tmpval);
	    }
	  value = tmplist;
	  return true;
	}
      return false;
    }

    /**
     * Get a key value as a list.  If the value does not exist, is
     * deprecated or obsolete, warn appropriately.
     *
     * @param group the group the key is in.
     * @param key the key to get.
     * @param priority the priority of the option.
     * @param value the list value to store the key's value in.  The
     * value type must be settable from an istream and be copyable.
     * The list must be a container with a standard insert method.
     * @returns true if the key was found, otherwise false (in which
     * case value will be unchanged).
     */
    template <typename T, template <typename T> class C>
    bool
    get_list_value(const std::string& group,
		   const std::string& key,
		   priority           priority,
		   C<T>&              value) const
    {
      bool status = get_list_value(group, key, value);
      check_priority(group, key, priority, status);
      return status;
    }

    /**
     * Set a key value.
     *
     * @param group the group the key is in.
     * @param key the key to set.
     * @param value the value to get the key's value from.  This must
     * allow output to an ostream.
     */
    template <typename T>
    void
    set_value(const std::string& group,
	      const std::string& key,
	      const T& value)
    {
      std::ostringstream os;
      os.imbue(std::locale("C"));
      os << std::boolalpha << value;

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

    /**
     * Set a key value from a list.
     *
     * @param group the group the key is in.
     * @param key the key to set.
     * @param value the list value to get the key's value from.  The
     * value type must allow output to an ostream.  The list must be a
     * container with a standard forward iterator.
     */
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
	  os.imbue(std::locale("C"));
	  os << std::boolalpha << *pos;
	  if (os)
	    {
	      strval += os.str();
	      if (pos += 1 != value.end())
		strval += this->separator;
	    }
	}

      set_value (group, key, strval);
    }

    /**
     * Remove a group.
     *
     * @param group the group to remove.
     */
    void
    remove_group(const std::string& group);

    /**
     * Remove a key.
     *
     * @param group the group the key is in.
     * @param key the key to remove.
     */
    void
    remove_key(const std::string& group,
	       const std::string& key);

    /**
     * keyfile initialisation from an istream.
     */
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
	    group = line.substr(1, fpos - 1);

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
	    key = line.substr(0, pos);
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
    /**
     * Print a comment to a stream.  The comment will have hash ('#')
     * marks printed at the start of each line.
     *
     * @param comment the comment to print.
     * @param stream the stream to output to.
     */
    void
    print_comment(const std::string& comment,
		  std::ostream&      stream) const;

  public:
    /**
     * keyfile output to an ostream.
     */
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
    /**
     * Find a group by it's name.
     *
     * @param group the group to find.
     * @returns the group, or 0 if not found.
     */
    const group_type *
    find_group(const std::string& group) const;

    /**
     * Find a group by it's name.
     *
     * @param group the group to find.
     * @returns the group, or 0 if not found.
     */
    group_type *
    find_group(const std::string& group);

    /**
     * Find a key by it's group and name.
     *
     * @param group the group the key is in.
     * @param key the key to find
     * @returns the key, or 0 if not found.
     */
    const item_type *
    find_item(const std::string& group,
	      const std::string& key) const;

    /**
     * Find a key by it's group and name.
     *
     * @param group the group the key is in.
     * @param key the key to find
     * @returns the key, or 0 if not found.
     */
    item_type *
    find_item(const std::string& group,
	      const std::string& key);

    /// The top-level groups.
    group_map_type groups;
    /// The separator used as a list item delimiter.
    char           separator;
  };

}

#endif /* SBUILD_KEYFILE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
