/* Copyright © 2005-2009  Roger Leigh <rleigh@debian.org>
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

#ifndef SBUILD_BASIC_KEYFILE_H
#define SBUILD_BASIC_KEYFILE_H

#include <sbuild/sbuild-i18n.h>
#include <sbuild/sbuild-log.h>
#include <sbuild/sbuild-keyfile-base.h>
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
   * Basic keyfile parser template
   */
  template <typename K>
  class basic_keyfile_parser
  {
  public:
    /// Exception type.
    typedef keyfile_base::error error;

    /// The constructor.
    basic_keyfile_parser ():
      group(),
      group_set(false),
      key(),
      key_set(false),
      value(),
      value_set(false),
      comment(),
      comment_set(false),
      line_number(0)
    {
    }

    /// The destructor.
    virtual ~basic_keyfile_parser ()
    {
    }

    /// Group name.
    typename K::group_name_type group;

    /// Group name is set.
    bool                        group_set;

    /// Key name.
    typename K::key_type        key;

    /// Key name is set.
    bool                        key_set;

    /// Value.
    typename K::value_type      value;

    /// Value is set.
    bool                        value_set;

    /// Comment.
    typename K::comment_type    comment;

    /// Comment is set.
    bool                        comment_set;

    /// Line number.
    typename K::size_type       line_number;

    /**
     * Start processing input.
     * Any setup may be done here.
     */
    virtual void
    begin ()
    {
      line_number = 0;
    }

    /**
     * Parse a line of input.  This function will be called for every
     * line of input in the source file.  The input line, line, is
     * parsed appropriately.  Any of the group, key, value, and
     * comment members are set as required.  If any of these members
     * are ready for insertion into the keyfile, then the
     * corresponding _set member must be set to true to signal the
     * fact to the caller.
     * @param line the line to parse.
     */
    virtual void
    parse_line (std::string const& line)
    {
      ++line_number;
    }

    /**
     * Stop processing input.  Any cleanup may be done here.  For
     * example, any cached group or item may be set here.
     */
    virtual void
    end()
    {
    }
  };

  /**
   * Configuration file parser.  This class loads an INI-style
   * configuration file from a file or stream.  The format is
   * documented in schroot.conf(5).
   */
  template <typename K, typename P = basic_keyfile_parser<K> >
  class basic_keyfile : public keyfile_base
  {
  public:
    /// Group name.
    typedef typename K::group_name_type group_name_type;

    /// Key name.
    typedef typename K::key_type key_type;

    /// Value.
    typedef typename K::value_type value_type;

    /// Comment.
    typedef typename K::comment_type comment_type;

    /// Line number.
    typedef typename K::size_type size_type;

    /// Vector of groups
    typedef std::vector<group_name_type> group_list;

    /// Vector of values
    typedef std::vector<value_type> value_list;

  private:
    /// Parse type.
    typedef P parse_type;

    /// Key-value-comment-line tuple.
    typedef std::tr1::tuple<key_type,value_type,comment_type,size_type>
    item_type;

    /// Map between key name and key-value-comment tuple.
    typedef std::map<key_type,item_type> item_map_type;

    /// Group-items-comment-line tuple.
    typedef std::tr1::tuple<group_name_type,item_map_type,comment_type,size_type> group_type;

    /// Map between group name and group-items-comment tuple.
    typedef std::map<group_name_type,group_type> group_map_type;

    /// Vector of keys
    typedef std::vector<key_type> key_list;

  public:
    /// The constructor.
    basic_keyfile ();

    /**
     * The constructor.
     *
     * @param file the file to load the configuration from.
     */
    basic_keyfile (std::string const& file);

    /**
     * The constructor.
     *
     * @param stream the stream to load the configuration from.
     */
    basic_keyfile (std::istream& stream);

    /// The destructor.
    virtual ~basic_keyfile ();

    /**
     * Get a list of groups.
     *
     * @returns a list of groups in the basic_keyfile.  If no groups exist,
     * the list will be empty.
     */
    group_list
    get_groups () const;

    /**
     * Get a list of keys in a group.
     *
     * @param group the group to use.
     * @returns a list of keys in a group.  If no keys exist in the
     * group, or the group does not exist, the list will be empty.
     */
    key_list
    get_keys (group_name_type const& group) const;

    /**
     * Check for unused keys in a group.  If keys other than the
     * specified keys exist in the specified group, print a warning
     * about unknown keys having been used.
     *
     * @param group the group to use.
     * @param keys the keys which have been used.
     */
    void
    check_keys (group_name_type const& group,
		key_list const&        keys) const;

    /**
     * Check if a group exists.
     *
     * @param group the group to check for.
     * @returns true if the group exists, otherwise false.
     */
    bool
    has_group (group_name_type const& group) const;

    /**
     * Check if a key exists.
     *
     * @param group the group the key is in.
     * @param key the key to check for.
     * @returns true if the key exists, otherwise false.
     */
    bool
    has_key (group_name_type const& group,
	     key_type const&        key) const;

    /**
     * Set a group.  The group will be created (and the comment set)
     * only if the group does not already exist.
     *
     * @param group the group to set.
     * @param comment the comment to set.
     */
    void
    set_group (group_name_type const& group,
	       comment_type const&    comment);

    /**
     * Set a group.  The group will be created (and the comment set)
     * only if the group does not already exist.
     *
     * @param group the group to set.
     * @param comment the comment to set.
     * @param line the line number in the input file, or 0 otherwise.
     */
    void
    set_group (group_name_type const& group,
	       comment_type const&    comment,
	       size_type              line);

    /**
     * Get a group comment.
     *
     * @param group the group to find.
     * @returns the comment.
     */
    comment_type
    get_comment (group_name_type const& group) const;

    /**
     * Get a key comment.
     *
     * @param group the group to find.
     * @param key the key to find.
     * @returns the comment.
     */
    comment_type
    get_comment (group_name_type const& group,
		 key_type const&        key) const;

    /**
     * Get a group line number.
     *
     * @param group the group to find.
     * @returns the line number, or 0 if not available.
     */
    size_type
    get_line (group_name_type const& group) const;

    /**
     * Get a key line number.
     *
     * @param group the group to find.
     * @param key the key to find.
     * @returns the line number, or 0 if not available.
     */
    size_type
    get_line (group_name_type const& group,
	      key_type const&        key) const;

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
    get_value (group_name_type const& group,
	       key_type const&        key,
	       T&                     value) const
    {
      log_debug(DEBUG_INFO) << "Getting keyfile group=" << group
			    << ", key=" << key << std::endl;
      const item_type *found_item = find_item(group, key);
      if (found_item)
	{
	  value_type const& strval(std::tr1::get<1>(*found_item));
	  try
	    {
	      parse_value(strval, value);
	      return true;
	    }
	  catch (parse_value_error const& e)
	    {
	      size_type line = get_line(group, key);
	      if (line)
		{
		  error ep(line, group, key, PASSTHROUGH_LGK, e);
		  log_exception_warning(ep);
		}
	      else
		{
		  error ep(group, key, PASSTHROUGH_GK, e);
		  log_exception_warning(ep);
		}
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
    get_value (group_name_type const& group,
	       key_type const&        key,
	       priority               priority,
	       T&                     value) const
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
    get_locale_string (group_name_type const& group,
		       key_type const&        key,
		       value_type&            value) const;

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
    get_locale_string (group_name_type const& group,
		       key_type const&        key,
		       priority               priority,
		       value_type&            value) const;

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
    get_locale_string (group_name_type const& group,
		       key_type const&        key,
		       std::string const&     locale,
		       value_type&            value) const;

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
    get_locale_string (group_name_type const& group,
		       key_type const&        key,
		       std::string const&     locale,
		       priority               priority,
		       value_type&            value) const;

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
    get_list_value (group_name_type const& group,
		    key_type const&        key,
		    C&                     container) const
    {
      value_type item_value;
      if (get_value(group, key, item_value))
	{
	  value_list items = split_string(item_value,
					  this->separator);
	  for (typename value_list::const_iterator pos = items.begin();
	       pos != items.end();
	       ++pos
	       )
	    {
	      typename C::value_type tmp;

	      try
		{
		  parse_value(*pos, tmp);
		}
	      catch (parse_value_error const& e)
		{
		  size_type line = get_line(group, key);
		  if (line)
		    {
		      error ep(line, group, key, PASSTHROUGH_LGK, e);
		      log_exception_warning(ep);
		    }
		  else
		    {
		      error ep(group, key, PASSTHROUGH_GK, e);
		      log_exception_warning(ep);
		    }
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
    get_list_value (group_name_type const& group,
		    key_type const&        key,
		    priority               priority,
		    C&                     container) const
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
    set_value (group_name_type const& group,
	       key_type const&        key,
	       T const&               value)
    {
      set_value(group, key, value, comment_type());
    }

    /**
     * Set a key value.
     *
     * @param group the group the key is in.
     * @param key the key to set.
     * @param value the value to get the key's value from.  This must
     * allow output to an ostream.
     * @param comment the comment for this key.
     */
    template <typename T>
    void
    set_value (group_name_type const& group,
	       key_type const&        key,
	       T const&               value,
	       comment_type const&    comment)
    {
      set_value(group, key, value, comment, 0);
    }

    /**
     * Set a key value.
     *
     * @param group the group the key is in.
     * @param key the key to set.
     * @param value the value to get the key's value from.  This must
     * allow output to an ostream.
     * @param comment the comment for this key.
     * @param line the line number in the input file, or 0 otherwise.
     */
    template <typename T>
    void
    set_value (group_name_type const& group,
	       key_type const&        key,
	       T const&               value,
	       comment_type const&    comment,
	       size_type              line)
    {
      std::ostringstream os;
      os.imbue(std::locale::classic());
      os << std::boolalpha << value;

      set_group(group, "");
      group_type *found_group = find_group(group);
      assert (found_group != 0); // should not fail

      item_map_type& items = std::tr1::get<1>(*found_group);

      typename item_map_type::iterator pos = items.find(key);
      if (pos != items.end())
	items.erase(pos);
      items.insert
	(typename item_map_type::value_type(key,
					    item_type(key, os.str(),
						      comment, line)));
    }

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
    set_list_value (group_name_type const& group,
		    key_type const&        key,
		    I                      begin,
		    I                      end)
    {
      set_list_value(group, key, begin, end, comment_type());
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
    set_list_value (group_name_type const& group,
		    key_type const&        key,
		    I                      begin,
		    I                      end,
		    comment_type const&    comment)
    {
      set_list_value (group, key, begin, end, comment, 0);
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
     * @param line the line number in the input file, or 0 otherwise.
     */
    template <typename I>
    void
    set_list_value (group_name_type const& group,
		    key_type const&        key,
		    I                      begin,
		    I                      end,
		    comment_type const&    comment,
		    size_type              line)
    {
      value_type strval;

      for (I pos = begin; pos != end; ++ pos)
	{
	  std::ostringstream os;
	  os.imbue(std::locale::classic());
	  os << std::boolalpha << *pos;
	  if (os)
	    {
	      strval += os.str();
	      if (pos + 1 != end)
		strval += this->separator;
	    }
	}

      set_value (group, key, strval, comment, line);
    }

    /**
     * Remove a group.
     *
     * @param group the group to remove.
     */
    void
    remove_group (group_name_type const& group);

    /**
     * Remove a key.
     *
     * @param group the group the key is in.
     * @param key the key to remove.
     */
    void
    remove_key (group_name_type const& group,
		key_type const&        key);

    /**
     * Add a basic_keyfile to the basic_keyfile.
     *
     * @param rhs the basic_keyfile to add.
     * @returns the modified basic_keyfile.
     */
    basic_keyfile&
    operator += (basic_keyfile const& rhs);

    /**
     * Add a basic_keyfile to the basic_keyfile.
     *
     * @param lhs the basic_keyfile to add to.
     * @param rhs the values to add.
     * @returns the new basic_keyfile.
     */
    template <typename _K, typename _P>
    friend basic_keyfile<_K, _P>
    operator + (basic_keyfile<_K, _P> const& lhs,
		basic_keyfile<_K, _P> const& rhs);

    /**
     * basic_keyfile initialisation from an istream.
     *
     * @param stream the stream to input from.
     * @param kf the basic_keyfile to set.
     * @returns the stream.
     */
    template <class charT, class traits>
    friend
    std::basic_istream<charT,traits>&
    operator >> (std::basic_istream<charT,traits>& stream,
		 basic_keyfile&                    kf)
    {
      basic_keyfile tmp;
      parse_type state;
      std::string line;

      state.begin();

      while (std::getline(stream, line))
      {
	state.parse_line(line);

	// Insert group
	if (state.group_set)
	  {
	    if (tmp.has_group(state.group))
	      throw error(state.line_number, DUPLICATE_GROUP, state.group);
	    else
	      tmp.set_group(state.group, state.comment, state.line_number);
	  }

	// Insert item
	if (state.key_set && state.value_set)
	  {
	    if (tmp.has_key(state.group, state.key))
	      throw error(state.line_number, state.group, DUPLICATE_KEY, state.key);
	    else
	      tmp.set_value(state.group, state.key, state.value, state.comment, state.line_number);
	  }
      }

      state.end();
      // TODO: do inserts here as well.

      kf += tmp;

      return stream;
    }

    /**
     * basic_keyfile output to an ostream.
     *
     * @param stream the stream to output to.
     * @param kf the basic_keyfile to output.
     * @returns the stream.
     */
    template <class charT, class traits>
    friend
    std::basic_ostream<charT,traits>&
    operator << (std::basic_ostream<charT,traits>& stream,
		 basic_keyfile const&              kf)
    {
      size_type group_count = 0;

      for (typename group_map_type::const_iterator gp = kf.groups.begin();
	   gp != kf.groups.end();
	   ++gp, ++group_count)
	{
	  if (group_count > 0)
	    stream << '\n';

	  group_type const& group = gp->second;
	  group_name_type const& groupname = std::tr1::get<0>(group);
	  comment_type const& comment = std::tr1::get<2>(group);

	  if (comment.length() > 0)
	    print_comment(comment, stream);

	  stream << '[' << groupname << ']' << '\n';

	  item_map_type const& items(std::tr1::get<1>(group));
	  for (typename item_map_type::const_iterator it = items.begin();
	       it != items.end();
	       ++it)
	    {
	      item_type const& item = it->second;
	      key_type const& key(std::tr1::get<0>(item));
	      value_type const& value(std::tr1::get<1>(item));
	      comment_type const& comment(std::tr1::get<2>(item));

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
    find_group (group_name_type const& group) const;

    /**
     * Find a group by it's name.
     *
     * @param group the group to find.
     * @returns the group, or 0 if not found.
     */
    group_type *
    find_group (group_name_type const& group);

    /**
     * Find a key by it's group and name.
     *
     * @param group the group the key is in.
     * @param key the key to find
     * @returns the key, or 0 if not found.
     */
    const item_type *
    find_item (group_name_type const& group,
	       key_type const&        key) const;

    /**
     * Find a key by it's group and name.
     *
     * @param group the group the key is in.
     * @param key the key to find
     * @returns the key, or 0 if not found.
     */
    item_type *
    find_item (group_name_type const& group,
	       key_type const&        key);

    /**
     * Check if a key is missing or present when not permitted.
     *
     * @param group the group the key is in.
     * @param key the key to get.
     * @param priority the key priority.
     * @param valid true if key exists, false if not existing.
     */
    void
    check_priority (group_name_type const& group,
		    key_type const&        key,
		    priority               priority,
		    bool                   valid) const;

    /**
     * Print a comment to a stream.  The comment will have hash ('#')
     * marks printed at the start of each line.
     *
     * @param comment the comment to print.
     * @param stream the stream to output to.
     *
     * @todo Use split string or some general iterator/algorithm
     * instead of custom string manipulation.  This could be reused by
     * log_exception_* functions and split_string.
     */
    static void
    print_comment (comment_type const& comment,
		   std::ostream&       stream);

    /// The top-level groups.
    group_map_type groups;
    /// The separator used as a list item delimiter.
    value_type     separator;

  public:
    /**
     * Set a key value from an object method return value.  This is
     * the same as calling set_value directly, but handles exceptions
     * being thrown by the object method, which are turned into error
     * exceptions.
     *
     * @param object the object to use.
     * @param method the object method to call.
     * @param basic_keyfile the basic_keyfile to use.
     * @param group the group the key is in.
     * @param key the key to set.
     */
    template<class C, typename T>
    static void
    set_object_value (C const&               object,
		      T                (C::* method)() const,
		      basic_keyfile&         basic_keyfile,
		      group_name_type const& group,
		      key_type const&        key)
    {
      try
	{
	  if (method)
	    basic_keyfile.set_value(group, key, (object.*method)());
	}
      catch (std::runtime_error const& e)
	{
	  throw error(group, key, PASSTHROUGH_GK, e);
	}
    }

    /**
     * Set a key value from an object method return value reference.
     * This is the same as calling set_value directly, but handles
     * exceptions being thrown by the object method, which are turned
     * into error exceptions.
     *
     * @param object the object to use.
     * @param method the object method to call.
     * @param basic_keyfile the basic_keyfile to use.
     * @param group the group the key is in.
     * @param key the key to set.
     */
    template<class C, typename T>
    static void
    set_object_value (C const&               object,
		      T const&         (C::* method)() const,
		      basic_keyfile&         basic_keyfile,
		      group_name_type const& group,
		      key_type const&        key)
    {
      try
	{
	  if (method)
	    basic_keyfile.set_value(group, key, (object.*method)());
	}
      catch (std::runtime_error const& e)
	{
	  throw error(group, key, PASSTHROUGH_GK, e);
	}
    }

    /**
     * Set a key list value from an object method return value.  The
     * method must return a container with begin() and end() methods
     * which return forward iterators.  This is the same as calling
     * set_list_value directly, but handles exceptions being thrown by
     * the object method, which are turned into error exceptions.
     *
     * @param object the object to use.
     * @param method the object method to call.
     * @param basic_keyfile the basic_keyfile to use.
     * @param group the group the key is in.
     * @param key the key to set.
     */
    template<class C, typename T>
    static void
    set_object_list_value (C const&               object,
			   T                (C::* method)() const,
			   basic_keyfile&         basic_keyfile,
			   group_name_type const& group,
			   key_type const&        key)
    {
      try
	{
	  if (method)
	    basic_keyfile.set_list_value(group, key,
					 (object.*method)().begin(),
					 (object.*method)().end());
	}
      catch (std::runtime_error const& e)
	{
	  throw error(group, key, PASSTHROUGH_GK, e);
	}
    }

    /**
     * Set a key list value from an object method return value.  The
     * method must return a container reference with begin() and end()
     * methods which return forward iterators.  This is the same as
     * calling set_list_value directly, but handles exceptions being
     * thrown by the object method, which are turned into error
     * exceptions.
     *
     * @param object the object to use.
     * @param method the object method to call.
     * @param basic_keyfile the basic_keyfile to use.
     * @param group the group the key is in.
     * @param key the key to set.
     */
    template<class C, typename T>
    static void
    set_object_list_value (C const&               object,
			   T const&         (C::* method)() const,
			   basic_keyfile&         basic_keyfile,
			   group_name_type const& group,
			   key_type const&        key)
    {
      try
	{
	  if (method)
	    basic_keyfile.set_list_value(group, key,
					 (object.*method)().begin(),
					 (object.*method)().end());
	}
      catch (std::runtime_error const& e)
	{
	  throw error(group, key, PASSTHROUGH_GK, e);
	}
    }

    /**
     * Get a key value and set it in an object using an object method.
     * This is the same as calling get_value directly, but handles
     * exceptions being thrown by the object method, and
     * deserialisation errors, which are turned into error exceptions
     * pointing to the group, key and line number in the input file.
     *
     * @param object the object to use.
     * @param method the object method to call.
     * @param basic_keyfile the basic_keyfile to use.
     * @param group the group the key is in.
     * @param key the key to set.
     * @param priority the key priority.
     */
    template<class C, typename T>
    static void
    get_object_value (C&                      object,
		      void              (C::* method)(T param),
		      basic_keyfile const&    basic_keyfile,
		      group_name_type const&  group,
		      key_type const&         key,
		      basic_keyfile::priority priority)
    {
      try
	{
	  T value;
	  if (basic_keyfile.get_value(group, key, priority, value)
	      && method)
	    (object.*method)(value);
	}
      catch (std::runtime_error const& e)
	{
	  size_type line = basic_keyfile.get_line(group, key);
	  if (line)
	    throw error(line, group, key, PASSTHROUGH_LGK, e);
	  else
	    throw error(group, key, PASSTHROUGH_GK, e);
	}
    }

    /**
     * Get a key value and set it by reference in an object using an
     * object method.  This is the same as calling get_value directly,
     * but handles exceptions being thrown by the object method, and
     * deserialisation errors, which are turned into error exceptions
     * pointing to the group, key and line number in the input file.
     *
     * @param object the object to use.
     * @param method the object method to call.
     * @param basic_keyfile the basic_keyfile to use.
     * @param group the group the key is in.
     * @param key the key to set.
     * @param priority the key priority.
     */
    template<class C, typename T>
    static void
    get_object_value (C&                      object,
		      void              (C::* method)(T const& param),
		      basic_keyfile const&    basic_keyfile,
		      group_name_type const&  group,
		      key_type const&         key,
		      basic_keyfile::priority priority)
    {
      try
	{
	  T value;
	  if (basic_keyfile.get_value(group, key, priority, value)
	      && method)
	    (object.*method)(value);
	}
      catch (std::runtime_error const& e)
	{
	  size_type line = basic_keyfile.get_line(group, key);
	  if (line)
	    throw error(line, group, key, PASSTHROUGH_LGK, e);
	  else
	    throw error(group, key, PASSTHROUGH_GK, e);
	}
    }

    /**
     * Get a key list value and set it in an object using an object
     * method.  This is the same as calling get_list_value directly,
     * but handles exceptions being thrown by the object method, and
     * deserialisation errors, which are turned into error exceptions
     * pointing to the group, key and line number in the input file.
     *
     * @param object the object to use.
     * @param method the object method to call.
     * @param basic_keyfile the basic_keyfile to use.
     * @param group the group the key is in.
     * @param key the key to set.
     * @param priority the key priority.
     */
    template<class C, typename T>
    static void
    get_object_list_value (C&                      object,
			   void              (C::* method)(T param),
			   basic_keyfile const&    basic_keyfile,
			   group_name_type const&  group,
			   key_type const&         key,
			   basic_keyfile::priority priority)
    {
      try
	{
	  T value;
	  if (basic_keyfile.get_list_value(group, key, priority, value)
	      && method)
	    (object.*method)(value);
	}
      catch (std::runtime_error const& e)
	{
	  size_type line = basic_keyfile.get_line(group, key);
	  if (line)
	    throw error(line, group, key, PASSTHROUGH_LGK, e);
	  else
	    throw error(group, key, PASSTHROUGH_GK, e);
	  throw error(basic_keyfile.get_line(group, key),
		      group, key, e);
	}
    }

    /**
     * Get a key list value and set it by reference in an object using
     * an object method.  This is the same as calling get_list_value
     * directly, but handles exceptions being thrown by the object
     * method, and deserialisation errors, which are turned into error
     * exceptions pointing to the group, key and line number in the
     * input file.
     *
     * @param object the object to use.
     * @param method the object method to call.
     * @param basic_keyfile the basic_keyfile to use.
     * @param group the group the key is in.
     * @param key the key to set.
     * @param priority the key priority.
     */
    template<class C, typename T>
    static void
    get_object_list_value (C&                      object,
			   void              (C::* method)(T const& param),
			   basic_keyfile const&    basic_keyfile,
			   group_name_type const&  group,
			   key_type const&         key,
			   basic_keyfile::priority priority)
    {
      try
	{
	  T value;
	  if (basic_keyfile.get_list_value(group, key, priority, value)
	      && method)
	    (object.*method)(value);
	}
      catch (std::runtime_error const& e)
	{
	  size_type line = basic_keyfile.get_line(group, key);
	  if (line)
	    throw error(line, group, key, PASSTHROUGH_LGK, e);
	  else
	    throw error(group, key, PASSTHROUGH_GK, e);
	  throw error(basic_keyfile.get_line(group, key),
		      group, key, e);
	}
    }
  };

}

#include <sbuild/sbuild-basic-keyfile.tcc>

#endif /* SBUILD_BASIC_KEYFILE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
