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

#ifndef SBUILD_AUTH_ERROR_H
#define SBUILD_AUTH_ERROR_H

#include <map>
#include <string>

#include <boost/format.hpp>

#include "sbuild-error.h"

namespace sbuild
{

  /**
   * Authentication error.
   */
  class auth_error : public runtime_error
  {
  public:
    enum type
      {
	NONE,            ///< No error occured.  Used for detail only.
	HOSTNAME,        ///< Failed to get hostname.
	USER,            ///< User not found.
	AUTHENTICATION,  ///< Authentication failed.
	AUTHORISATION,   ///< Authorisation failed.
	PAM_DOUBLE_INIT, ///< PAM was already initialised.
	PAM              ///< PAM error.
      };

    /**
     * The constructor.
     *
     * @param error_type the error code.
     */
    auth_error (type error_type):
      runtime_error(format_error(std::string(), error_type))
    {
    }

    /**
     * The constructor.
     *
     * @param detail the details of the error.
     * @param error_type the error code.
     */
    auth_error (std::string const& detail,
		type               error_type):
      runtime_error(format_error(detail, error_type))
    {
    }

    /**
     * The constructor.
     *
     * @param error_type the error code.
     * @param error_number the error number.
     */
    auth_error (type               error_type,
		int                error_number):
      runtime_error(format_error(std::string(), error_type, error_number))
    {
    }

    /**
     * The constructor.
     *
     * @param detail the details of the error.
     * @param error_type the error code.
     * @param error_number the error number.
     */
    auth_error (std::string const& detail,
		type               error_type,
		int                error_number):
      runtime_error(format_error(detail, error_type, error_number))
    {
    }

    /**
     * The constructor.
     *
     * @param error_type the error code.
     * @param error_string the error string.
     */
    auth_error (type               error_type,
		std::string const& error_string):
      runtime_error(format_error(std::string(), error_type, error_string))
    {
    }

    /**
     * The constructor.
     *
     * @param detail the details of the error.
     * @param error_type the error code.
     * @param error_string the error string.
     */
    auth_error (std::string const& detail,
		type               error_type,
		std::string const& error_string):
      runtime_error(format_error(detail, error_type, error_string))
    {
    }

  private:
    /// Mapping between error code and string.
    static std::map<type,const char *> error_strings;

    /**
     * Get a translated error string.
     *
     * @param error_type the error code.
     * @returns a translated error string.
     */
    static const char *
    get_error (type error_type);

    /**
     * Format an error message.
     *
     * @param error_type the error code.
     * @param detail the details of the error.
     * @returns a translated error message.
     */
    static std::string
    format_error (std::string const& detail,
		  type               error_type);

    /**
     * Format an error message.
     *
     * @param detail the details of the error.
     * @param error_type the error code.
     * @param error_number the error number.
     * @returns a translated error message.
     */
    static std::string
    format_error (std::string const& detail,
		  type               error_type,
		  int                error_number);

    /**
     * Format an error message.
     *
     * @param detail the details of the error.
     * @param error_type the error code.
     * @param error_string the error string.
     * @returns a translated error message.
     */
    static std::string
    format_error (std::string const& detail,
		  type               error_type,
		  std::string const& error_string);
  };

}

#endif /* SBUILD_AUTH_ERROR_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
