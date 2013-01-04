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

#ifndef CSBUILD_DEBIAN_CHANGES_H
#define CSBUILD_DEBIAN_CHANGES_H

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

namespace csbuild
{

  /**
   * Debian changes file parser.  This class is a generic parser for
   * the file format used in Debian changes files for binary and
   * source uploads (.changes and .dsc files).
   */
  class debian_changes
  {
  private:
    typedef std::string key_type;
    typedef std::vector<std::string> value_type;
    typedef unsigned int size_type;

    /// Key-value-line tuple.
    typedef std::tuple<key_type,value_type,size_type>
    item_type;

    /// Map between key name and key-value-comment tuple.
    typedef std::map<key_type,item_type> item_map_type;

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

    /// Error codes.
    enum error_code
      {
        BAD_FILE,          ///< The file to parse couldn't be opened.
        DEPRECATED_KEY,    ///< The key is deprecated.
        DEPRECATED_KEY_NL, ///< The key is deprecated (no line specified).
        DISALLOWED_KEY,    ///< The key is not allowed.
        DISALLOWED_KEY_NL, ///< The key is not allowed (no line specified).
        DUPLICATE_KEY,     ///< The key is a duplicate.
        INVALID_LINE,      ///< The line is invalid.
        MISSING_KEY,       ///< The key is missing.
        MISSING_KEY_NL,    ///< The key is missing (no line specified).
        NO_KEY,            ///< No key was specified.
        OBSOLETE_KEY,      ///< The key is obsolete.
        OBSOLETE_KEY_NL,   ///< The key is obsolete (no line specified).
        PASSTHROUGH_K,    ///< Pass through exception with key.
        PASSTHROUGH_LK    ///< Pass through exception with line and key.
      };

    /// Exception type.
    typedef sbuild::parse_error<error_code> error;

    /// The constructor.
    debian_changes ();

    /**
     * The constructor.
     *
     * @param file the file to load the configuration from.
     */
    debian_changes (std::string const& file);

    /**
     * The constructor.
     *
     * @param stream the stream to load the configuration from.
     */
    debian_changes (std::istream& stream);

    /// The destructor.
    virtual ~debian_changes ();

    /**
     * Get a list of keys.
     * @returns a list of keys.
     */
    sbuild::string_list
    get_keys () const;

    /**
     * Check if a key exists.
     *
     * @param key the key to check for.
     * @returns true if the key exists, otherwise false.
     */
    bool
    has_key (key_type const& key) const;

    /**
     * Get a key line number.
     *
     * @param key the key to find.
     * @returns the line number, or 0 if not available.
     */
    size_type
    get_line (key_type const& key) const;

    /**
     * Get a key value.
     *
     * @param key the key to get.
     * @param value the value to store the key's value in.  This must
     * be settable from an istream and be copyable.
     * @returns true if the key was found, otherwise false (in which
     * case value will be unchanged).
     */
    template <typename T>
    bool
    get_value (key_type const& key,
               T&              value) const
    {
      sbuild::log_debug(sbuild::DEBUG_INFO)
        << "Getting debian_changes key=" << key << std::endl;
      const item_type *found_item = find_item(key);
      if (found_item)
        {
          value_type const& strval(std::get<1>(*found_item));
          try
            {
              sbuild::parse_value(strval, value);
              return true;
            }
          catch (sbuild::parse_value_error const& e)
            {
              size_type line = get_line(key);
              if (line)
                {
                  error ep(line, key, PASSTHROUGH_LK, e);
                  log_exception_warning(ep);
                }
              else
                {
                  error ep(key, PASSTHROUGH_K, e);
                  log_exception_warning(ep);
                }
              return false;
            }
        }
      sbuild::log_debug(sbuild::DEBUG_NOTICE)
        << "key not found" << std::endl;
      return false;
    }

    /**
     * Get a key value.  If the value does not exist, is deprecated or
     * obsolete, warn appropriately.
     *
     * @param key the key to get.
     * @param priority the priority of the option.
     * @param value the value to store the key's value in.  This must
     * be settable from an istream and be copyable.
     * @returns true if the key was found, otherwise false (in which
     * case value will be unchanged).
     */
    template <typename T>
    bool
    get_value (key_type const& key,
               priority        priority,
               T&              value) const
    {
      bool status = get_value(key, value);
      check_priority(key, priority, status);
      return status;
    }

    /**
     * Get a key value.
     *
     * @param key the key to get.
     * @param value the value to store the key's value in.
     * @returns true if the key was found, otherwise false (in which
     * case value will be unchanged).
     */
    bool
    get_value (key_type const& key,
               value_type&     value) const;

    /**
     * Get a key value.  If the value does not exist, is deprecated or
     * obsolete, warn appropriately.
     *
     * @param key the key to get.
     * @param priority the priority of the option.
     * @param value the value to store the key's value in.
     * @returns true if the key was found, otherwise false (in which
     * case value will be unchanged).
     */
    bool
    get_value (key_type const& key,
               priority        priority,
               value_type&     value) const;

    /**
     * Get a key value as a list.
     *
     * @param key the key to get.  Note that if the key contains
     * multiple lines, each line will be split using the separator.
     * @param container the container to store the key's value in.
     * The value type must be settable from an istream and be
     * copyable.  The list must be a container with a standard insert
     * method.
     * @param separator the characters to separate the values.
     * @returns true if the key was found, otherwise false (in which
     * case value will be undefined).
     */
    template <typename C>
    bool
    get_list_value (key_type const&    key,
                    C&                 container,
                    std::string const& separator) const
    {
      value_type item_value;
      if (get_value(key, item_value))
        {
          for (value_type::const_iterator vpos = item_value.begin();
               vpos != item_value.end();
               ++vpos)
            {
              sbuild::string_list items =
                sbuild::split_string(*vpos, std::string(1, separator));
              for (sbuild::string_list::const_iterator pos = items.begin();
                   pos != items.end();
                   ++pos
                   )
                {
                  typename C::value_type tmp;

                  try
                    {
                      sbuild::parse_value(*pos, tmp);
                    }
                  catch (sbuild::parse_value_error const& e)
                    {
                      size_type line = get_line(key);
                      if (line)
                        {
                          error ep(line, key, PASSTHROUGH_LK, e);
                          log_exception_warning(ep);
                        }
                      else
                        {
                          error ep(key, PASSTHROUGH_K, e);
                          log_exception_warning(ep);
                        }
                      return false;
                    }

                  container.push_back(tmp);
                }
            }
          return true;
        }
      return false;
    }

    /**
     * Get a key value as a list.  If the value does not exist, is
     * deprecated or obsolete, warn appropriately.
     *
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
    get_list_value (key_type const& key,
                    priority        priority,
                    C&              container) const
    {
      bool status = get_list_value(key, container);
      check_priority(key, priority, status);
      return status;
    }

    /**
     * Set a key value.
     *
     * @param key the key to set.
     * @param value the value to get the key's value from.  This must
     * allow output to an ostream.
     */
    template <typename T>
    void
    set_value (key_type const& key,
               T const&        value)
    {
      set_value(key, value, 0);
    }

    /**
     * Set a key value.
     *
     * @param key the key to set.
     * @param value the value to get the key's value from.  This must
     * allow output to an ostream.
     * @param line the line number in the input file, or 0 otherwise.
     */
    template <typename T>
    void
    set_value (key_type const& key,
               T const&        value,
               size_type       line)
    {
      std::ostringstream os;
      os.imbue(std::locale::classic());
      os << std::boolalpha << value;
      value_type val;
      val.push_back(os.str());

      item_map_type::iterator pos = items.find(key);
      if (pos != items.end())
        items.erase(pos);
      items.insert
        (item_map_type::value_type(key,
                                   item_type(key, val, line)));
    }

    /**
     * Set a key value.
     *
     * @param key the key to set.
     * @param value the value to get the key's value from.
     */
    void
    set_value (key_type const&   key,
               value_type const& value)
    {
      set_value(key, value, 0);
    }

    /**
     * Set a key value.
     *
     * @param key the key to set.
     * @param value the value to get the key's value from.
     * @param line the line number in the input file, or 0 otherwise.
     */
    void
    set_value (key_type const&   key,
               value_type const& value,
               size_type         line);

    /**
     * Set a key value from a list.
     *
     * @param key the key to set.
     * @param begin an iterator referring to the start of the
     * list. The value type must allow output to an ostream.
     * @param end an iterator referring to the end of the list.
     */
    template <typename I>
    void
    set_list_value (key_type const& key,
                    I               begin,
                    I               end)
    {
      set_list_value (key, begin, end, 0);
    }

    /**
     * Set a key value from a list.
     *
     * @param key the key to set.
     * @param begin an iterator referring to the start of the
     * list. The value type must allow output to an ostream.
     * @param end an iterator referring to the end of the list.
     * @param separator the characters to separate the values.
     * @param line the line number in the input file, or 0 otherwise.
     */
    template <typename I>
    void
    set_list_value (key_type const&    key,
                    I                  begin,
                    I                  end,
                    std::string const& separator,
                    size_type          line)
    {
      std::string strval;

      for (I pos = begin; pos != end; ++ pos)
        {
          std::ostringstream os;
          os.imbue(std::locale::classic());
          os << std::boolalpha << *pos;
          if (os)
            {
              strval += os.str();
              if (pos + 1 != end)
                strval += separator;
            }
        }

      set_value (key, strval, line);
    }

    /**
     * Remove a key.
     *
     * @param key the key to remove.
     */
    void
    remove_key (key_type const& key);

    /**
     * Add a debian_changes to the debian_changes.
     *
     * @param rhs the debian_changes to add.
     * @returns the modified debian_changes.
     */
    debian_changes&
    operator += (debian_changes const& rhs);

    /**
     * Add a debian_changes to the debian_changes.
     *
     * @param lhs the debian_changes to add to.
     * @param rhs the values to add.
     * @returns the new debian_changes.
     */
    friend debian_changes
    operator + (debian_changes const& lhs,
                debian_changes const& rhs);

    /**
     * debian_changes initialisation from an istream.
     *
     * @param stream the stream to input from.
     * @param dc the debian_changes to set.
     * @returns the stream.
     */
    template <class charT, class traits>
    friend
    std::basic_istream<charT,traits>&
    operator >> (std::basic_istream<charT,traits>& stream,
                 debian_changes&                   dc)
    {
      debian_changes tmp;
      size_t linecount = 0;
      std::string line;
      std::string key;
      std::string value;

      while (std::getline(stream, line))
      {
        linecount++;

        if (line.length() == 0)
          {
            // Empty line; do nothing.
          }
        else // Item
          {
            std::string::size_type pos = line.find_first_of('=');
            if (pos == std::string::npos)
              throw error(linecount, INVALID_LINE, line);
            if (pos == 0)
              throw error(linecount, NO_KEY, line);
            key = line.substr(0, pos);
            if (pos == line.length() - 1)
              value = "";
            else
              value = line.substr(pos + 1);

            // Insert item
            if (tmp.has_key(key))
              throw error(linecount, DUPLICATE_KEY, key);
            else
              tmp.set_value(key, value, linecount);
          }
      }

      dc += tmp;

      return stream;
    }

    /**
     * debian_changes output to an ostream.
     *
     * @param stream the stream to output to.
     * @param dc the debian_changes to output.
     * @returns the stream.
     */
    template <class charT, class traits>
    friend
    std::basic_ostream<charT,traits>&
    operator << (std::basic_ostream<charT,traits>& stream,
                 debian_changes const&             dc)
    {
      size_type group_count = 0;

      for (item_map_type::const_iterator it = dc.items.begin();
           it != dc.items.end();
           ++it)
        {
          item_type const& item = it->second;
          key_type const& key(std::get<0>(item));
          value_type const& value(std::get<1>(item));

          stream << key << '=' << value << '\n';
        }

      return stream;
    }

  private:
    /**
     * Find a key by its name.
     *
     * @param key the key to find
     * @returns the key, or 0 if not found.
     */
    const item_type *
    find_item (key_type const& key) const;

    /**
     * Find a key by its name.
     *
     * @param key the key to find
     * @returns the key, or 0 if not found.
     */
    item_type *
    find_item (key_type const& key);

    /**
     * Check if a key is missing or present when not permitted.
     *
     * @param key the key to get.
     * @param priority the key priority.
     * @param valid true if key exists, false if not existing.
     */
    void
    check_priority (key_type const& key,
                    priority        priority,
                    bool            valid) const;

    /// The top-level items.
    item_map_type items;

  public:
    /**
     * Set a key value from an object method return value.  This is
     * the same as calling set_value directly, but handles exceptions
     * being thrown by the object method, which are turned into error
     * exceptions.
     *
     * @param object the object to use.
     * @param method the object method to call.
     * @param debian_changes the debian_changes to use.
     * @param key the key to set.
     */
    template<class C, typename T>
    static void
    set_object_value (C const&                        object,
                      T                         (C::* method)() const,
                      debian_changes&                 debian_changes,
                      debian_changes::key_type const& key)
    {
      try
        {
          debian_changes.set_value(key, (object.*method)());
        }
      catch (std::runtime_error const& e)
        {
          throw error(key, PASSTHROUGH_K, e);
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
     * @param debian_changes the debian_changes to use.
     * @param key the key to set.
     */
    template<class C, typename T>
    static void
    set_object_value (C const&                        object,
                      T const&                  (C::* method)() const,
                      debian_changes&                 debian_changes,
                      debian_changes::key_type const& key)
    {
      try
        {
          debian_changes.set_value(key, (object.*method)());
        }
      catch (std::runtime_error const& e)
        {
          throw error(key, PASSTHROUGH_K, e);
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
     * @param debian_changes the debian_changes to use.
     * @param key the key to set.
     */
    template<class C, typename T>
    static void
    set_object_list_value (C const&                        object,
                           T                         (C::* method)() const,
                           debian_changes&                 debian_changes,
                           debian_changes::key_type const& key)
    {
      try
        {
          debian_changes.set_list_value(key,
                                 (object.*method)().begin(),
                                 (object.*method)().end());
        }
      catch (std::runtime_error const& e)
        {
          throw error(key, PASSTHROUGH_K, e);
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
     * @param debian_changes the debian_changes to use.
     * @param key the key to set.
     */
    template<class C, typename T>
    static void
    set_object_list_value (C const&                        object,
                           T const&                  (C::* method)() const,
                           debian_changes&                 debian_changes,
                           debian_changes::key_type const& key)
    {
      try
        {
          debian_changes.set_list_value(key,
                                 (object.*method)().begin(),
                                 (object.*method)().end());
        }
      catch (std::runtime_error const& e)
        {
          throw error(key, PASSTHROUGH_K, e);
        }
    }

    /**
     * Get a key value and set it in an object using an object method.
     * This is the same as calling get_value directly, but handles
     * exceptions being thrown by the object method, and
     * deserialisation errors, which are turned into error exceptions
     * pointing to the key and line number in the input file.
     *
     * @param object the object to use.
     * @param method the object method to call.
     * @param debian_changes the debian_changes to use.
     * @param key the key to set.
     * @param priority the key priority.
     */
    template<class C, typename T>
    static void
    get_object_value (C&                              object,
                      void                      (C::* method)(T param),
                      debian_changes const&           debian_changes,
                      debian_changes::key_type const& key,
                      debian_changes::priority        priority)
    {
      try
        {
          T value;
          if (debian_changes.get_value(key, priority, value))
            (object.*method)(value);
        }
      catch (std::runtime_error const& e)
        {
          size_type line = debian_changes.get_line(key);
          if (line)
            throw error(line, key, PASSTHROUGH_LK, e);
          else
            throw error(key, PASSTHROUGH_K, e);
        }
    }

    /**
     * Get a key value and set it by reference in an object using an
     * object method.  This is the same as calling get_value directly,
     * but handles exceptions being thrown by the object method, and
     * deserialisation errors, which are turned into error exceptions
     * pointing to the key and line number in the input file.
     *
     * @param object the object to use.
     * @param method the object method to call.
     * @param debian_changes the debian_changes to use.
     * @param key the key to set.
     * @param priority the key priority.
     */
    template<class C, typename T>
    static void
    get_object_value (C&                              object,
                      void                      (C::* method)(T const& param),
                      debian_changes const&           debian_changes,
                      debian_changes::key_type const& key,
                      debian_changes::priority        priority)
    {
      try
        {
          T value;
          if (debian_changes.get_value(key, priority, value))
            (object.*method)(value);
        }
      catch (std::runtime_error const& e)
        {
          size_type line = debian_changes.get_line(key);
          if (line)
            throw error(line, key, PASSTHROUGH_LK, e);
          else
            throw error(key, PASSTHROUGH_K, e);
        }
    }

    /**
     * Get a key list value and set it in an object using an object
     * method.  This is the same as calling get_list_value directly,
     * but handles exceptions being thrown by the object method, and
     * deserialisation errors, which are turned into error exceptions
     * pointing to the key and line number in the input file.
     *
     * @param object the object to use.
     * @param method the object method to call.
     * @param debian_changes the debian_changes to use.
     * @param key the key to set.
     * @param priority the key priority.
     */
    template<class C, typename T>
    static void
    get_object_list_value (C&                              object,
                           void                      (C::* method)(T param),
                           debian_changes const&           debian_changes,
                           debian_changes::key_type const& key,
                           debian_changes::priority        priority)
    {
      try
        {
          T value;
          if (debian_changes.get_list_value(key, priority, value))
            (object.*method)(value);
        }
      catch (std::runtime_error const& e)
        {
          size_type line = debian_changes.get_line(key);
          if (line)
            throw error(line, key, PASSTHROUGH_LK, e);
          else
            throw error(key, PASSTHROUGH_K, e);
          throw error(debian_changes.get_line(key),
                      key, e);
        }
    }

    /**
     * Get a key list value and set it by reference in an object using
     * an object method.  This is the same as calling get_list_value
     * directly, but handles exceptions being thrown by the object
     * method, and deserialisation errors, which are turned into error
     * exceptions pointing to the key and line number in the
     * input file.
     *
     * @param object the object to use.
     * @param method the object method to call.
     * @param debian_changes the debian_changes to use.
     * @param key the key to set.
     * @param priority the key priority.
     */
    template<class C, typename T>
    static void
    get_object_list_value (C&                              object,
                           void                      (C::* method)(T const& param),
                           debian_changes const&           debian_changes,
                           debian_changes::key_type const& key,
                           debian_changes::priority        priority)
    {
      try
        {
          T value;
          if (debian_changes.get_list_value(key, priority, value))
            (object.*method)(value);
        }
      catch (std::runtime_error const& e)
        {
          size_type line = debian_changes.get_line(key);
          if (line)
            throw error(line, key, PASSTHROUGH_LK, e);
          else
            throw error(key, PASSTHROUGH_K, e);
          throw error(debian_changes.get_line(key),
                      key, e);
        }
    }
  };

}

#endif /* CSBUILD_DEBIAN_CHANGES_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
