/* Copyright © 2005-2007  Roger Leigh <rleigh@codelibre.net>
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
   * Traits class for an INI-style configuration file.  The format is
   * documented in schroot.conf(5).
   */
  struct keyfile_traits
  {
    /// Group name.
    typedef std::string group_name_type;

    /// Key name.
    typedef std::string key_type;

    /// Value.
    typedef std::string value_type;

    /// Comment.
    typedef std::string comment_type;

    /// Line number.
    typedef unsigned int size_type;
  };

  /**
   * Keyfile parser template
   */
  template <typename K>
  class keyfile_parser : public basic_keyfile_parser<K>
  {
  public:
    // Workaround for GCC bug.
    typedef keyfile_base::error error;
    // This is the correct form, but is not currently supported by
    // GCC.  http://gcc.gnu.org/bugzilla/show_bug.cgi?id=14258
    // using typename basic_keyfile_parser<K>::error;

    using basic_keyfile_parser<K>::group;
    using basic_keyfile_parser<K>::group_set;
    using basic_keyfile_parser<K>::key;
    using basic_keyfile_parser<K>::key_set;
    using basic_keyfile_parser<K>::value;
    using basic_keyfile_parser<K>::value_set;
    using basic_keyfile_parser<K>::comment;
    using basic_keyfile_parser<K>::comment_set;
    using basic_keyfile_parser<K>::line_number;

    keyfile_parser():
      basic_keyfile_parser<K>()
    {}

    virtual ~keyfile_parser()
    {}

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
            throw error(line_number, keyfile_base::INVALID_GROUP, line);
          group = line.substr(1, fpos - 1);

          if (group.length() == 0)
            throw error(line_number, keyfile_base::INVALID_GROUP, line);

          comment_set = true;
          group_set = true;
        }
      else // Item
        {
          std::string::size_type pos = line.find_first_of('=');
          if (pos == std::string::npos)
            throw error(line_number, keyfile_base::INVALID_LINE, line);
          if (pos == 0)
            throw error(line_number, keyfile_base::NO_KEY, line);
          key = line.substr(0, pos);
          if (pos == line.length() - 1)
            value = "";
          else
            value = line.substr(pos + 1);

          // No group specified
          if (group.empty())
            throw error(line_number, keyfile_base::NO_GROUP, line);

          comment_set = true;
          key_set = true;
          value_set = true;
        }

      basic_keyfile_parser<K>::parse_line(line);
    }
  };

  /**
   * Configuration file parser.  This class loads an INI-style
   * configuration file from a file or stream.  The format is
   * documented in schroot.conf(5).
   */
  typedef basic_keyfile<keyfile_traits, keyfile_parser<keyfile_traits> > keyfile;

}

#endif /* SBUILD_KEYFILE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
