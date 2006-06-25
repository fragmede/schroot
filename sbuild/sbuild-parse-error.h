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

#ifndef SBUILD_PARSE_ERROR_H
#define SBUILD_PARSE_ERROR_H

#include <sbuild/sbuild-error.h>

#include <map>
#include <string>

namespace sbuild
{

  /**
   * Parse error.
   */
  class parse_error : public runtime_error
  {
  public:
    enum type
      {
	NONE,            ///< No error occured.  Used for detail only.
	BAD_FILE,        ///< The file to parse couldn't be opened.
	BAD_VALUE,       ///< The value could not be parsed.
	INVALID_LINE,    ///< The line is invalid.
	NO_GROUP,        ///< No group was specified.
	INVALID_GROUP,   ///< The group is invalid.
	DUPLICATE_GROUP, ///< The group is a duplicate.
	NO_KEY,          ///< No key was specified.
	DUPLICATE_KEY,   ///< The key is a duplicate.
	MISSING_KEY,     ///< The key is missing.
	DISALLOWED_KEY   ///< The key is not allowed.
      };

    /**
     * The constructor.
     *
     * @param error the error code.
     * @param detail the details of the error.
     */
    parse_error (type               error,
		 std::string const& detail);

    /**
     * The constructor.
     *
     * @param line the line the error occured on.
     * @param error the error code.
     * @param detail the details of the error.
     */
    parse_error (size_t             line,
		 type               error,
		 std::string const& detail);

    /**
     * The constructor.
     *
     * @param line the line the error occured on.
     * @param group the group the error occured within.
     * @param error the error code.
     * @param detail the details of the error.
     */
    parse_error (size_t             line,
		 std::string const& group,
		 type               error,
		 std::string const& detail);

    /**
     * The constructor.
     *
     * @param line the line the error occured on.
     * @param group the group the error occured within.
     * @param key the key the error occured within.
     * @param error the error code.
     * @param detail the details of the error.
     */
    parse_error (size_t             line,
		 std::string const& group,
		 std::string const& key,
		 type               error,
		 std::string const& detail);

    /**
     * The constructor.
     *
     * @param group the group the error occured within.
     * @param error the error code.
     * @param detail the details of the error.
     */
    parse_error (std::string const& group,
		 type               error,
		 std::string const& detail);

    /**
     * The constructor.
     *
     * @param group the group the error occured within.
     * @param key the key the error occured within.
     * @param error the error code.
     * @param detail the details of the error.
     */
    parse_error (std::string const& group,
		 std::string const& key,
		 type               error,
		 std::string const& detail);

  private:
    /// Mapping between error code and string.
    static std::map<type,const char *> error_strings;

    /**
     * Get a translated error string.
     *
     * @param error the error code.
     * @returns a translated error string.
     */
    static const char *
    get_error (type error);

    /**
     * Format an error message.
     *
     * @param error the error code.
     * @param detail the details of the error.
     */
    static std::string
    format_error (type               error,
		  std::string const& detail);

    /**
     * Format an error message.
     *
     * @param line the line the error occured on.
     * @param error the error code.
     * @param detail the details of the error.
     */
    static std::string
    format_error (size_t             line,
		  type               error,
		  std::string const& detail);

    /**
     * Format an error message.
     *
     * @param line the line the error occured on.
     * @param group the group the error occured within.
     * @param error the error code.
     * @param detail the details of the error.
     */
    static std::string
    format_error (size_t             line,
		  std::string const& group,
		  type               error,
		  std::string const& detail);

    /**
     * Format an error message.
     *
     * @param line the line the error occured on.
     * @param group the group the error occured within.
     * @param key the key the error occured within.
     * @param error the error code.
     * @param detail the details of the error.
     */
    static std::string
    format_error (size_t             line,
		  std::string const& group,
		  std::string const& key,
		  type               error,
		  std::string const& detail);

    /**
     * Format an error message.
     *
     * @param group the group the error occured within.
     * @param error the error code.
     * @param detail the details of the error.
     */
    static std::string
    format_error (std::string const& group,
		  type               error,
		  std::string const& detail);

    /**
     * Format an error message.
     *
     * @param group the group the error occured within.
     * @param key the key the error occured within.
     * @param error the error code.
     * @param detail the details of the error.
     */
    static std::string
    format_error (std::string const& group,
		  std::string const& key,
		  type               error,
		  std::string const& detail);
  };

}

#endif /* SBUILD_PARSE_ERROR_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
