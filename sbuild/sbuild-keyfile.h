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

#ifndef SBUILD_KEYFILE_H
#define SBUILD_KEYFILE_H

#include <sbuild/sbuild-i18n.h>
#include <sbuild/sbuild-log.h>
#include <sbuild/sbuild-parse-error.h>
#include <sbuild/sbuild-parse-value.h>
#include <sbuild/sbuild-types.h>
#include <sbuild/sbuild-tr1types.h>
#include <sbuild/sbuild-util.h>

#include <cassert>
#include <map>
#include <string>
#include <sstream>

#include <boost/format.hpp>

namespace sbuild
{

  /**
   * Configuration file parser.  This class loads an INI-style
   * configuration file from a file or stream.  The format is
   * documented in schroot.conf(5).  It is based upon the Glib
   * GKeyFile class, which it is intended to replace.
   */
  class keyfile
  {
  private:
    /// Key-value-comment-line tuple.
    typedef std::tr1::tuple<std::string,std::string,std::string,unsigned int>
    item_type;

    /// Map between key name and key-value-comment tuple.
    typedef std::map<std::string,item_type> item_map_type;

    /// Group-items-comment-line tuple.
    typedef std::tr1::tuple<std::string,item_map_type,std::string,unsigned int> group_type;

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
    typedef parse_error error;

    /// The constructor.
    keyfile ();

    /**
     * The constructor.
     *
     * @param file the file to load the configuration from.
     */
    keyfile (std::string const& file);

    /**
     * The constructor.
     *
     * @param stream the stream to load the configuration from.
     */
    keyfile (std::istream& stream);

    /// The destructor.
    virtual ~keyfile ();

    /**
     * Get a list of groups.
     *
     * @returns a list of groups in the keyfile.  If no groups exist,
     * the list will be empty.
     */
    string_list
    get_groups () const;

    /**
     * Get a list of keys in a group.
     *
     * @param group the group to use.
     * @returns a list of keys in a group.  If no keys exist in the
     * group, or the group does not exist, the list will be empty.
     */
    string_list
    get_keys (std::string const& group) const;

    /**
     * Check if a group exists.
     *
     * @param group the group to check for.
     * @returns true if the group exists, otherwise false.
     */
    bool
    has_group (std::string const& group) const;

    /**
     * Check if a key exists.
     *
     * @param group the group the key is in.
     * @param key the key to check for.
     * @returns true if the key exists, otherwise false.
     */
    bool
    has_key (std::string const& group,
	     std::string const& key) const;

    /**
     * Set a group.  The group will be created (and the comment set)
     * only if the group does not already exist.
     *
     * @param group the group to set.
     * @param comment the comment to set.
     */
    void
    set_group (std::string const& group,
	       std::string const& comment);

  private:
    /**
     * Set a group.  The group will be created (and the comment set)
     * only if the group does not already exist.
     *
     * @param group the group to set.
     * @param comment the comment to set.
     * @param line the line number in the input file, or 0 otherwise.
     */
    void
    set_group (std::string const& group,
	       std::string const& comment,
	       unsigned int       line);

  public:
    /**
     * Get a group comment.
     *
     * @param group the group to find.
     * @returns the comment.
     */
    std::string
    get_comment (std::string const& group) const;

    /**
     * Get a key comment.
     *
     * @param group the group to find.
     * @param key the key to find.
     * @returns the comment.
     */
    std::string
    get_comment (std::string const& group,
		 std::string const& key) const;

    /**
     * Get a group line number.
     *
     * @param group the group to find.
     * @returns the line number, or 0 if not available.
     */
    unsigned int
    get_line (std::string const& group) const;

    /**
     * Get a key line number.
     *
     * @param group the group to find.
     * @param key the key to find.
     * @returns the line number, or 0 if not available.
     */
    unsigned int
    get_line (std::string const& group,
	      std::string const& key) const;

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
    get_value (std::string const& group,
	       std::string const& key,
	       T&                 value) const
    {
      log_debug(DEBUG_INFO) << "Getting keyfile group=" << group
			    << ", key=" << key << std::endl;
      const item_type *found_item = find_item(group, key);
      if (found_item)
	{
	  std::string const& strval(std::tr1::get<1>(*found_item));
	  try
	    {
	      parse_value(strval, value);
	      return true;
	    }
	  catch (parse_error const& e)
	    {
	      error ep(group, key, parse_error::NONE, e.what());
	      log_warning() << ep.what() << std::endl;
	      return false;
	    }
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
    get_value (std::string const& group,
	       std::string const& key,
	       priority           priority,
	       T&                 value) const
    {
      bool status = get_value(group, key, value);
      check_priority(group, key, priority, status);
      return status;
    }

    /**
     * Get a localised key string value.
     *
     * @param group the group the key is in.
     * @param key the key to get.
     * @param value the string to store the key's localised value in.
     * @returns true if the key was found, otherwise false (in which
     * case value will be unchanged).
     */
    bool
    get_locale_string (std::string const& group,
		       std::string const& key,
		       std::string&       value) const;

    /**
     * Get a localised key string value.  If the value does not exist,
     * is deprecated or obsolete, warn appropriately.
     *
     * @param group the group the key is in.
     * @param key the key to get.
     * @param priority the priority of the option.
     * @param value the string to store the key's localised value in.
     * @returns true if the key was found, otherwise false (in which
     * case value will be unchanged).
     */
    bool
    get_locale_string (std::string const& group,
		       std::string const& key,
		       priority           priority,
		       std::string&       value) const;

    /**
     * Get a localised key string value for a specific locale.
     *
     * @param group the group the key is in.
     * @param key the key to get.
     * @param locale the locale to use.
     * @param value the string to store the key's localised value in.
     * @returns true if the key was found, otherwise false (in which
     * case value will be unchanged).
     */
    bool
    get_locale_string (std::string const& group,
		       std::string const& key,
		       std::string const& locale,
		       std::string&       value) const;

    /**
     * Get a localised key string value for a specific locale.  If the
     * value does not exist, is deprecated or obsolete, warn
     * appropriately.
     *
     * @param group the group the key is in.
     * @param key the key to get.
     * @param locale the locale to use.
     * @param priority the priority of the option.
     * @param value the string to store the key's localised value in.
     * @returns true if the key was found, otherwise false (in which
     * case value will be unchanged).
     */
    bool
    get_locale_string (std::string const& group,
		       std::string const& key,
		       std::string const& locale,
		       priority           priority,
		       std::string&       value) const;

    /**
     * Get a key value as a list.
     *
     * @param group the group the key is in.
     * @param key the key to get.
     * @param container the container to store the key's value in.
     * The value type must be settable from an istream and be
     * copyable.  The list must be a container with a standard insert
     * method.
     * @returns true if the key was found, otherwise false (in which
     * case value will be undefined).
     */
    template <typename C>
    bool
    get_list_value (std::string const& group,
		    std::string const& key,
		    C&                 container) const
    {
      std::string item_value;
      if (get_value(group, key, item_value))
	{
	  string_list items = split_string(item_value,
					   std::string(1, this->separator));
	  for (string_list::const_iterator pos = items.begin();
	       pos != items.end();
	       ++pos
	       )
	    {
	      typename C::value_type tmp;

	      try
		{
		  parse_value(*pos, tmp);
		}
	      catch (parse_error const& e)
		{
		  error ep(group, key, parse_error::NONE, e.what());
		  log_warning() << ep.what() << std::endl;
		  return false;
		}

	      container.push_back(tmp);
	    }
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
     * @param container the container to store the key's value in.
     * The value type must be settable from an istream and be
     * copyable.  The list must be a container with a standard insert
     * method.
     * @returns true if the key was found, otherwise false (in which
     * case value will be undefined).
     */
    template <typename C>
    bool
    get_list_value (std::string const& group,
		    std::string const& key,
		    priority           priority,
		    C&                 container) const
    {
      bool status = get_list_value(group, key, container);
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
    set_value (std::string const& group,
	       std::string const& key,
	       T const&           value)
    {
      set_value(group, key, value, std::string());
    }

    /**
     * Set a key value.
     *
     * @param group the group the key is in.
     * @param key the key to set.
     * @param value the value to get the key's value from.  This must
     * @param comment the comment for this key.
     * allow output to an ostream.
     */
    template <typename T>
    void
    set_value (std::string const& group,
	       std::string const& key,
	       T const&           value,
	       std::string const& comment)
    {
      std::ostringstream os;
      os.imbue(std::locale("C"));
      os << std::boolalpha << value;

      set_value(group, key, os.str(), comment, 0);
    }

  private:
    /**
     * Set a key value.
     *
     * @param group the group the key is in.
     * @param key the key to set.
     * @param value the value to get the key's value from.  This must
     * @param comment the comment for this key.
     * @param line the line number in the input file, or 0 otherwise.
     * allow output to an ostream.
     */
    void
    set_value (std::string const& group,
	       std::string const& key,
	       std::string const& value,
	       std::string const& comment,
	       unsigned int       line)
    {
      set_group(group, "");
      group_type *found_group = find_group(group);
      assert (found_group != 0); // should not fail

      item_map_type& items = std::tr1::get<1>(*found_group);

      item_map_type::iterator pos = items.find(key);
      if (pos != items.end())
	items.erase(pos);
      items.insert
	(item_map_type::value_type(key,
				   item_type(key, value, comment, line)));
    }

  public:
    /**
     * Set a key value from a list.
     *
     * @param group the group the key is in.
     * @param key the key to set.
     * @param begin an iterator referring to the start of the
     * list. The value type must allow output to an ostream.
     * @param end an iterator referring to the end of the list.
     */
    template <typename I>
    void
    set_list_value (std::string const& group,
		    std::string const& key,
		    I                  begin,
		    I                  end)
    {
      set_list_value(group, key, begin, end, std::string());
    }

    /**
     * Set a key value from a list.
     *
     * @param group the group the key is in.
     * @param key the key to set.
     * @param begin an iterator referring to the start of the
     * list. The value type must allow output to an ostream.
     * @param end an iterator referring to the end of the list.
     * @param comment the comment for this key.
     */
    template <typename I>
    void
    set_list_value (std::string const& group,
		    std::string const& key,
		    I                  begin,
		    I                  end,
		    std::string const& comment)
    {
      std::string strval;

      for (I pos = begin; pos != end; ++ pos)
	{
	  std::ostringstream os;
	  os.imbue(std::locale("C"));
	  os << std::boolalpha << *pos;
	  if (os)
	    {
	      strval += os.str();
	      if (pos + 1 != end)
		strval += this->separator;
	    }
	}

      set_value (group, key, strval, comment);
    }

    /**
     * Remove a group.
     *
     * @param group the group to remove.
     */
    void
    remove_group (std::string const& group);

    /**
     * Remove a key.
     *
     * @param group the group the key is in.
     * @param key the key to remove.
     */
    void
    remove_key (std::string const& group,
		std::string const& key);

    /**
     * Add a keyfile to the keyfile.
     *
     * @param rhs the keyfile to add.
     * @returns the modified keyfile.
     */
    keyfile&
    operator += (keyfile const& rhs);

    /**
     * Add a keyfile to the keyfile.
     *
     * @param lhs the keyfile to add to.
     * @param rhs the values to add.
     * @returns the new keyfile.
     */
    friend keyfile
    operator + (keyfile const& lhs,
		keyfile const& rhs);

    /**
     * keyfile initialisation from an istream.
     */
    template <class charT, class traits>
    friend
    std::basic_istream<charT,traits>&
    operator >> (std::basic_istream<charT,traits>& stream,
		 keyfile&                          kf)
    {
      keyfile tmp;
      size_t linecount = 0;
      std::string line;
      std::string group;
      std::string comment;
      std::string key;
      std::string value;

      while (std::getline(stream, line))
      {
	linecount++;

	if (line.length() == 0)
	  {
	    // Empty line; do nothing.
	  }
	else if (line[0] == '#') // Comment line
	  {
	    if (!comment.empty())
	      comment += '\n';
	    comment += line.substr(1);
	  }
	else if (line[0] == '[') // Group
	  {
	    std::string::size_type fpos = line.find_first_of(']');
	    std::string::size_type lpos = line.find_last_of(']');
	    if (fpos == std::string::npos || lpos == std::string::npos ||
		fpos != lpos)
	      {
		throw error(linecount, parse_error::INVALID_GROUP, line);
	      }
	    group = line.substr(1, fpos - 1);

	    if (group.length() == 0)
	      {
		throw error(linecount, parse_error::INVALID_GROUP, line);
	      }

	    // Insert group
	    if (tmp.has_group(group))
	      {
		error e(linecount, parse_error::DUPLICATE_GROUP, group);
		log_warning() << e.what() << std::endl;
	      }
	    else
	      tmp.set_group(group, comment, linecount);
	    comment.clear();
	  }
	else // Item
	  {
	    std::string::size_type pos = line.find_first_of('=');
	    if (pos == std::string::npos)
	      {
		throw error(linecount, parse_error::INVALID_LINE, line);
	      }
	    if (pos == 0)
	      {
		throw error(linecount, parse_error::NO_KEY, line);
	      }
	    key = line.substr(0, pos);
	    if (pos == line.length() - 1)
	      value = "";
	    else
	      value = line.substr(pos + 1);

	    // No group specified
	    if (group.empty())
	      {
		throw error(linecount, parse_error::NO_GROUP, line);
	      }

	    // Insert item
	    if (tmp.has_key(group, key))
	      {
		error e(linecount, group, parse_error::DUPLICATE_KEY, key);
		log_warning() << e.what() << std::endl;
	      }
	    else
	      tmp.set_value(group, key, value, comment, linecount);
	    comment.clear();
	  }
      }

      kf += tmp;

      return stream;
    }

    /**
     * keyfile output to an ostream.
     */
    template <class charT, class traits>
    friend
    std::basic_ostream<charT,traits>&
    operator << (std::basic_ostream<charT,traits>& stream,
		 keyfile const&                    kf)
    {
      unsigned int group_count = 0;

      for (group_map_type::const_iterator gp = kf.groups.begin();
	   gp != kf.groups.end();
	   ++gp, ++group_count)
	{
	  if (group_count > 0)
	    stream << '\n';

	  group_type const& group = gp->second;
	  std::string const& groupname = std::tr1::get<0>(group);
	  std::string const& comment = std::tr1::get<2>(group);

	  if (comment.length() > 0)
	    print_comment(comment, stream);

	  stream << '[' << groupname << ']' << '\n';

	  item_map_type const& items(std::tr1::get<1>(group));
	  for (item_map_type::const_iterator it = items.begin();
	       it != items.end();
	       ++it)
	    {
	      item_type const& item = it->second;
	      std::string const& key(std::tr1::get<0>(item));
	      std::string const& value(std::tr1::get<1>(item));
	      std::string const& comment(std::tr1::get<2>(item));

	      if (comment.length() > 0)
		print_comment(comment, stream);

	      stream << key << '=' << value << '\n';
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
    find_group (std::string const& group) const;

    /**
     * Find a group by it's name.
     *
     * @param group the group to find.
     * @returns the group, or 0 if not found.
     */
    group_type *
    find_group (std::string const& group);

    /**
     * Find a key by it's group and name.
     *
     * @param group the group the key is in.
     * @param key the key to find
     * @returns the key, or 0 if not found.
     */
    const item_type *
    find_item (std::string const& group,
	       std::string const& key) const;

    /**
     * Find a key by it's group and name.
     *
     * @param group the group the key is in.
     * @param key the key to find
     * @returns the key, or 0 if not found.
     */
    item_type *
    find_item (std::string const& group,
	       std::string const& key);

    /**
     * Check if a key is missing or present when not permitted.
     *
     * @param group the group the key is in.
     * @param key the key to get.
     * @param priority the key priority.
     * @param valid true if key exists, false if not existing.
     */
    void
    check_priority (std::string const& group,
		    std::string const& key,
		    priority           priority,
		    bool               valid) const;

    /**
     * Print a comment to a stream.  The comment will have hash ('#')
     * marks printed at the start of each line.
     *
     * @param comment the comment to print.
     * @param stream the stream to output to.
     */
    static void
    print_comment (std::string const& comment,
		   std::ostream&      stream);

    /// The top-level groups.
    group_map_type groups;
    /// The separator used as a list item delimiter.
    char           separator;

  public:
    /**
     * Set a key value from an object method return value.  This is
     * the same as calling set_value directly, but handles exceptions
     * being thrown by the object method, which are turned into
     * parse_error exceptions.
     *
     * @param object the object to use.
     * @param method the object method to call.
     * @param keyfile the keyfile to use.
     * @param group the group the key is in.
     * @param key the key to set.
     */
    template<class C, typename T>
    static void
    set_object_value (C const&            object,
		      T             (C::* method)() const,
		      keyfile&            keyfile,
		      std::string const&  group,
		      std::string const&  key)
    {
      try
	{
	  keyfile.set_value(group, key, (object.*method)());
	}
      catch (runtime_error const& e)
	{
	  throw parse_error(group, key, parse_error::NONE, e.what());
	}
    }

    /**
     * Set a key value from an object method return value reference.
     * This is the same as calling set_value directly, but handles
     * exceptions being thrown by the object method, which are turned
     * into parse_error exceptions.
     *
     * @param object the object to use.
     * @param method the object method to call.
     * @param keyfile the keyfile to use.
     * @param group the group the key is in.
     * @param key the key to set.
     */
    template<class C, typename T>
    static void
    set_object_value (C const&            object,
		      T const&      (C::* method)() const,
		      keyfile&            keyfile,
		      std::string const&  group,
		      std::string const&  key)
    {
      try
	{
	  keyfile.set_value(group, key, (object.*method)());
	}
      catch (runtime_error const& e)
	{
	  throw parse_error(group, key, parse_error::NONE, e.what());
	}
    }

    /**
     * Set a key list value from an object method return value.  The
     * method must return a container with begin() and end() methods
     * which return forward iterators.  This is the same as calling
     * set_list_value directly, but handles exceptions being thrown by
     * the object method, which are turned into parse_error
     * exceptions.
     *
     * @param object the object to use.
     * @param method the object method to call.
     * @param keyfile the keyfile to use.
     * @param group the group the key is in.
     * @param key the key to set.
     */
    template<class C, typename T>
    static void
    set_object_list_value (C const&            object,
			   T             (C::* method)() const,
			   keyfile&            keyfile,
			   std::string const&  group,
			   std::string const&  key)
    {
      try
	{
	  keyfile.set_list_value(group, key,
				 (object.*method)().begin(),
				 (object.*method)().end());
	}
      catch (runtime_error const& e)
	{
	  throw parse_error(group, key, parse_error::NONE, e.what());
	}
    }

    /**
     * Set a key list value from an object method return value.  The
     * method must return a container reference with begin() and end()
     * methods which return forward iterators.  This is the same as
     * calling set_list_value directly, but handles exceptions being
     * thrown by the object method, which are turned into parse_error
     * exceptions.
     *
     * @param object the object to use.
     * @param method the object method to call.
     * @param keyfile the keyfile to use.
     * @param group the group the key is in.
     * @param key the key to set.
     */
    template<class C, typename T>
    static void
    set_object_list_value (C const&            object,
			   T const&      (C::* method)() const,
			   keyfile&            keyfile,
			   std::string const&  group,
			   std::string const&  key)
    {
      try
	{
	  keyfile.set_list_value(group, key,
				 (object.*method)().begin(),
				 (object.*method)().end());
	}
      catch (runtime_error const& e)
	{
	  throw parse_error(group, key, parse_error::NONE, e.what());
	}
    }

    /**
     * Get a key value and set it in an object using an object method.
     * This is the same as calling get_value directly, but handles
     * exceptions being thrown by the object method, and
     * deserialisation errors, which are turned into parse_error
     * exceptions pointing to the group, key and line number in the
     * input file.
     *
     * @param object the object to use.
     * @param method the object method to call.
     * @param keyfile the keyfile to use.
     * @param group the group the key is in.
     * @param key the key to set.
     * @param priority the key priority.
     */
    template<class C, typename T>
    static void
    get_object_value (C&                  object,
		      void          (C::* method)(T param),
		      keyfile const&      keyfile,
		      std::string const&  group,
		      std::string const&  key,
		      keyfile::priority   priority)
    {
      T value;
      if (keyfile.get_value(group, key, priority, value))
	{
	  try
	    {
	      (object.*method)(value);
	    }
	  catch (runtime_error const& e)
	    {
	      throw parse_error(keyfile.get_line(group, key),
				group, key, parse_error::NONE, e.what());
	    }
	}
    }

    /**
     * Get a key value and set it by reference in an object using an
     * object method.  This is the same as calling get_value directly,
     * but handles exceptions being thrown by the object method, and
     * deserialisation errors, which are turned into parse_error
     * exceptions pointing to the group, key and line number in the
     * input file.
     *
     * @param object the object to use.
     * @param method the object method to call.
     * @param keyfile the keyfile to use.
     * @param group the group the key is in.
     * @param key the key to set.
     * @param priority the key priority.
     */
    template<class C, typename T>
    static void
    get_object_value (C&                  object,
		      void          (C::* method)(T const& param),
		      keyfile const&      keyfile,
		      std::string const&  group,
		      std::string const&  key,
		      keyfile::priority   priority)
    {
      T value;
      if (keyfile.get_value(group, key, priority, value))
	{
	  try
	    {
	      (object.*method)(value);
	    }
	  catch (runtime_error const& e)
	    {
	      throw parse_error(keyfile.get_line(group, key),
				group, key, parse_error::NONE, e.what());
	    }
	}
    }

    /**
     * Get a key list value and set it in an object using an object
     * method.  This is the same as calling get_list_value directly,
     * but handles exceptions being thrown by the object method, and
     * deserialisation errors, which are turned into parse_error
     * exceptions pointing to the group, key and line number in the
     * input file.
     *
     * @param object the object to use.
     * @param method the object method to call.
     * @param keyfile the keyfile to use.
     * @param group the group the key is in.
     * @param key the key to set.
     * @param priority the key priority.
     */
    template<class C, typename T>
    static void
    get_object_list_value (C&                  object,
			   void          (C::* method)(T param),
			   keyfile const&      keyfile,
			   std::string const&  group,
			   std::string const&  key,
			   keyfile::priority   priority)
    {
      T value;
      if (keyfile.get_list_value(group, key, priority, value))
	{
	  try
	    {
	      (object.*method)(value);
	    }
	  catch (runtime_error const& e)
	    {
	      throw parse_error(keyfile.get_line(group, key),
				group, key, parse_error::NONE, e.what());
	    }
	}
    }

    /**
     * Get a key list value and set it by reference in an object using
     * an object method.  This is the same as calling get_list_value
     * directly, but handles exceptions being thrown by the object
     * method, and deserialisation errors, which are turned into
     * parse_error exceptions pointing to the group, key and line
     * number in the input file.
     *
     * @param object the object to use.
     * @param method the object method to call.
     * @param keyfile the keyfile to use.
     * @param group the group the key is in.
     * @param key the key to set.
     * @param priority the key priority.
     */
    template<class C, typename T>
    static void
    get_object_list_value (C&                  object,
			   void          (C::* method)(T const& param),
			   keyfile const&      keyfile,
			   std::string const&  group,
			   std::string const&  key,
			   keyfile::priority   priority)
    {
      T value;
      if (keyfile.get_list_value(group, key, priority, value))
	{
	  try
	    {
	      (object.*method)(value);
	    }
	  catch (runtime_error const& e)
	    {
	      throw parse_error(keyfile.get_line(group, key),
				group, key, parse_error::NONE, e.what());
	    }
	}
    }
  };

}

#endif /* SBUILD_KEYFILE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
