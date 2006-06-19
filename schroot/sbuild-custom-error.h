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

#ifndef SBUILD_CUSTOM_ERROR_H
#define SBUILD_CUSTOM_ERROR_H

#include <map>
#include <string>

#include <boost/format.hpp>

#include "sbuild-error.h"

namespace sbuild
{

  /**
   * Custom error.
   */
  template <typename T>
  class custom_error : public runtime_error
  {
  public:
    typedef T error_type;
    typedef std::map<error_type,const char *> map_type;

    /**
     * The constructor.
     *
     * @param error the error code.
     */
    custom_error (error_type error):
      runtime_error(format_error(std::string(), error))
    {
    }

    /**
     * The constructor.
     *
     * @param detail the details of the error.
     * @param error the error code.
     */
    custom_error (std::string const& detail,
		   error_type error):
      runtime_error(format_error(detail, error))
    {
    }

    /**
     * The constructor.
     *
     * @param detail the details of the error.
     */
    custom_error (std::string const& detail):
      runtime_error(detail)
    {
    }

    /**
     * The constructor.
     *
     * @param error the error code.
     * @param error_number the error number.
     */
    custom_error (error_type error,
		  int        error_number):
      runtime_error(format_error(std::string(), error, error_number))
    {
    }

    /**
     * The constructor.
     *
     * @param detail the details of the error.
     * @param error the error code.
     * @param error_number the error number.
     */
    custom_error (std::string const& detail,
		  error_type         error,
		  int                error_number):
      runtime_error(format_error(detail, error, error_number))
    {
    }

    /**
     * The constructor.
     *
     * @param detail the details of the error.
     * @param error_number the error number.
     */
    custom_error (std::string const& detail,
		  int                error_number):
      runtime_error(format_error(detail, error_number))
    {
    }

    /**
     * The constructor.
     *
     * @param error the error code.
     * @param error_string the error string.
     */
    custom_error (error_type         error,
		  std::string const& error_string):
      runtime_error(format_error(std::string(), error, error_string))
    {
    }

    /**
     * The constructor.
     *
     * @param detail the details of the error.
     * @param error the error code.
     * @param error_string the error string.
     */
    custom_error (std::string const& detail,
		  error_type         error,
		  std::string const& error_string):
      runtime_error(format_error(detail, error, error_string))
    {
    }

    /**
     * The constructor.
     *
     * @param detail the details of the error.
     * @param error_string the error string.
     */
    custom_error (std::string const& detail,
		  std::string const& error_string):
      runtime_error(format_error(detail, error_string))
    {
    }

  private:
    /// Mapping between error code and string.
    static map_type error_strings;

    /**
     * Get a translated error string.
     *
     * @param error the error code.
     * @returns a translated error string.
     */
    static const char *
    get_error (error_type error);

    /**
     * Format an error message.
     *
     * @param error the error code.
     * @param detail the details of the error.
     * @returns a translated error message.
     */
    static std::string
    format_error (std::string const& detail,
		  error_type         error);

    /**
     * Format an error message.
     *
     * @param detail the details of the error.
     * @param error the error code.
     * @param error_number the error number.
     * @returns a translated error message.
     */
    static std::string
    format_error (std::string const& detail,
		  error_type         error,
		  int                error_number);

    /**
     * Format an error message.
     *
     * @param detail the details of the error.
     * @param error_number the error number.
     * @returns a translated error message.
     */
    static std::string
    format_error (std::string const& detail,
		  int                error_number);

    /**
     * Format an error message.
     *
     * @param detail the details of the error.
     * @param error the error code.
     * @param error_string the error string.
     * @returns a translated error message.
     */
    static std::string
    format_error (std::string const& detail,
		  error_type         error,
		  std::string const& error_string);

    /**
     * Format an error message.
     *
     * @param detail the details of the error.
     * @param error_string the error string.
     * @returns a translated error message.
     */
    static std::string
    format_error (std::string const& detail,
		  std::string const& error_string);
  };


}

#include "sbuild-custom-error.tcc"

#endif /* SBUILD_CUSTOM_ERROR_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
