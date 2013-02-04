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

#ifndef SBUILD_KEYFILE_H
#define SBUILD_KEYFILE_H

#include <sbuild/sbuild-basic-keyfile.h>

namespace sbuild
{

  /**
   * Keyfile parser template
   */
  class keyfile_parser
  {
  public:
    /// Exception type.
    typedef basic_keyfile::error error;

    /// The constructor.
    keyfile_parser():
      group(),
      group_set(false),
      key(),
      key_set(false),
      value(),
      value_set(false),
      comment(),
      comment_set(false),
      line_number(0)
    {}

    /// The destructor.
    virtual ~keyfile_parser()
    {}

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
      if (comment_set == true)
        {
          comment.clear();
          comment_set = false;
        }
      if (group_set == true)
        {
          // The group isn't cleared
          group_set = false;
        }
      if (key_set == true)
        {
          key.clear();
          key_set = false;
        }
      if (value_set == true)
        {
          value.clear();
          value_set = false;
        }

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
            throw error(line_number, basic_keyfile::INVALID_GROUP, line);
          group = line.substr(1, fpos - 1);

          if (group.length() == 0)
            throw error(line_number, basic_keyfile::INVALID_GROUP, line);

          comment_set = true;
          group_set = true;
        }
      else // Item
        {
          std::string::size_type pos = line.find_first_of('=');
          if (pos == std::string::npos)
            throw error(line_number, basic_keyfile::INVALID_LINE, line);
          if (pos == 0)
            throw error(line_number, basic_keyfile::NO_KEY, line);
          key = line.substr(0, pos);
          if (pos == line.length() - 1)
            value = "";
          else
            value = line.substr(pos + 1);

          // No group specified
          if (group.empty())
            throw error(line_number, basic_keyfile::NO_GROUP, line);

          comment_set = true;
          key_set = true;
          value_set = true;
        }

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

    /// Group name.
    std::string  group;

    /// Group name is set.
    bool         group_set;

    /// Key name.
    std::string  key;

    /// Key name is set.
    bool         key_set;

    /// Value.
    std::string  value;

    /// Value is set.
    bool         value_set;

    /// Comment.
    std::string  comment;

    /// Comment is set.
    bool         comment_set;

    /// Line number.
    unsigned int line_number;

  };

  /**
   * Configuration file parser.  This class loads an INI-style
   * configuration file from a file or stream.  The format is
   * documented in schroot.conf(5).
   */
  class keyfile : public basic_keyfile
  {
  public:
    /// The constructor.
    keyfile();

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
    virtual ~keyfile();

    /**
     * basic_keyfile initialisation from an istream.
     *
     * @param stream the stream to input from.
     * @param kf the keyfile to set.
     * @returns the stream.
     */
    friend
    std::istream&
    operator >> (std::istream& stream,
                 keyfile&      kf)
    {
      keyfile tmp;
      keyfile_parser state;
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
     * @param kf the keyfile to output.
     * @returns the stream.
     */
    friend
    std::ostream&
    operator << (std::ostream& stream,
                 keyfile const& kf)
    {
      size_type group_count = 0;

      for (typename group_map_type::const_iterator gp = kf.groups.begin();
           gp != kf.groups.end();
           ++gp, ++group_count)
        {
          if (group_count > 0)
            stream << '\n';

          group_type const& group = gp->second;
          group_name_type const& groupname = std::get<0>(group);
          comment_type const& comment = std::get<2>(group);

          if (comment.length() > 0)
            print_comment(comment, stream);

          stream << '[' << groupname << ']' << '\n';

          item_map_type const& items(std::get<1>(group));
          for (const auto& it : items)
            {
              item_type const& item = it.second;
              key_type const& key(std::get<0>(item));
              value_type const& value(std::get<1>(item));
              comment_type const& comment(std::get<2>(item));

              if (comment.length() > 0)
                print_comment(comment, stream);

              stream << key << '=' << value << '\n';
            }
        }

      return stream;
    }
  };

}

#endif /* SBUILD_KEYFILE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
