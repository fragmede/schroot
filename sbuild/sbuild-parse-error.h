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

#include <sbuild/sbuild-custom-error.h>

#include <map>
#include <string>

namespace sbuild
{

  /**
   * Parse error.
   */
  template<typename T>
  class parse_error : public custom_error_base<T>
  {
  public:
    typedef typename custom_error_base<T>::error_type error_type;
    typedef typename custom_error_base<T>::null null;

    /**
     * The constructor.
     *
     * @param context the context of the error.
     * @param error the error code.
     */
    template<typename A>
    parse_error (A const&   context,
		 error_type error):
      custom_error_base<T>(format_error(context, null(), null(), error, null(), null()))
    {
    }

    /**
     * The constructor.
     *
     * @param error the error code.
     * @param detail the details of the error.
     */
    template<typename A>
    parse_error (error_type error,
		 A const&   detail):
      custom_error_base<T>(format_error(null(), null(), null(), error, detail, null()))
    {
    }

    /**
     * The constructor.
     *
     * @param line the line the error occured on.
     * @param error the error code.
     * @param detail the details of the error.
     */
    parse_error (size_t             line,
		 error_type         error,
		 std::string const& detail):
      custom_error_base<T>(format_error(line, null(), null(), error, detail, null()))
    {
    }

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
		 error_type         error,
		 std::string const& detail):
      custom_error_base<T>(format_error(line, group, null(), error, detail, null()))
    {
    }

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
		 error_type         error,
		 std::string const& detail):
      custom_error_base<T>(format_error(line, group, key, error, detail, null()))
    {
    }

    /**
     * The constructor.
     *
     * @param group the group the error occured within.
     * @param error the error code.
     * @param detail the details of the error.
     */
    parse_error (std::string const& group,
		 error_type         error,
		 std::string const& detail):
      custom_error_base<T>(format_error(group, null(), null(), error, detail, null()))
    {
    }

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
		 error_type         error,
		 std::string const& detail):
      custom_error_base<T>(format_error(group, key, null(), error, detail, null()))
    {
    }

    /**
     * The constructor.
     *
     * @param context the context of the error.
     * @param error the error code.
     */
    template<typename A>
    parse_error (A const&                  context,
		 std::runtime_error const& error):
      custom_error_base<T>(format_error(context, null(), null(), error, null(), null()))
    {
    }

    /**
     * The constructor.
     *
     * @param line the line the error occured on.
     * @param detail the details of the error.
     */
    parse_error (size_t                    line,
		 std::runtime_error const& error):
      custom_error_base<T>(format_error(line, null(), null(), error, null(), null()))
    {
    }

    /**
     * The constructor.
     *
     * @param line the line the error occured on.
     * @param group the group the error occured within.
     * @param error the error code.
     */
    parse_error (size_t                    line,
		 std::string const&        group,
		 std::runtime_error const& error):
      custom_error_base<T>(format_error(line, group, null(), error, null(), null()))
    {
    }

    /**
     * The constructor.
     *
     * @param line the line the error occured on.
     * @param group the group the error occured within.
     * @param key the key the error occured within.
     * @param error the error code.
     */
    parse_error (size_t                    line,
		 std::string const&        group,
		 std::string const&        key,
		 std::runtime_error const& error):
      custom_error_base<T>(format_error(line, group, key, error, null(), null()))
    {
    }

    /**
     * The constructor.
     *
     * @param group the group the error occured within.
     * @param error the error code.
     */
    parse_error (std::string const&        group,
		 std::runtime_error const& error):
      custom_error_base<T>(format_error(group, null(), null(), error, null(), null()))
    {
    }

    /**
     * The constructor.
     *
     * @param group the group the error occured within.
     * @param key the key the error occured within.
     * @param error the error code.
     */
    parse_error (std::string const&        group,
		 std::string const&        key,
		 std::runtime_error const& error):
      custom_error_base<T>(format_error(group, key, null(), error, null(), null()))
    {
    }

  };

}

#endif /* SBUILD_PARSE_ERROR_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
