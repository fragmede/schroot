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

#include <sbuild/sbuild-error.h>

#include <map>
#include <string>

namespace sbuild
{

  /**
   * Custom error base.
   */
  template <typename T>
  class custom_error_base : public runtime_error
  {
  public:
    typedef T error_type;
    typedef std::map<error_type,const char *> map_type;

    /**
     * The constructor.
     *
     * @param error the error message.
     */
    custom_error_base(std::string const& error):
      runtime_error(error)
    {
    }

    /// The destructor.
    virtual ~custom_error_base () throw ()
    {}

    /**
     * Null class to represent an absence of context or detail in an
     * error.
     */
    class null
    {
      /**
       * null output to an ostream.
       * @todo Output placeholder text.
       */
      template <class charT, class traits>
      friend
      std::basic_ostream<charT,traits>&
      operator << (std::basic_ostream<charT,traits>& stream,
		   null const&                       n)
      {
	return stream;
      }
    };

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

  protected:
    /**
     * Format an error message.
     *
     * @param context1 context of the error.
     * @param context2 additional context of the error.
     * @param context3 additional context of the error.
     * @param error the error code.
     * @param detail1 details of the error.
     * @param detail2 additional details of the error.
     * @returns a translated error message.
     */
    template <typename A, typename B, typename C, typename D, typename E>
    static std::string
    format_error (A const&   context1,
		  B const&   context2,
		  C const&   context3,
		  error_type error,
		  D const&   detail1,
		  E const&   detail2);

    /**
     * Format an error message.
     *
     * @param context1 context of the error.
     * @param context2 additional context of the error.
     * @param context3 additional context of the error.
     * @param error the error code.
     * @param detail1 details of the error.
     * @param detail2 additional details of the error.
     * @returns a translated error message.
     */
    template <typename A, typename B, typename C, typename D, typename E>
    static std::string
    format_error (A const&                  context1,
		  B const&                  context2,
		  C const&                  context3,
		  std::runtime_error const& error,
		  D const&                  detail1,
		  E const&                  detail2);

  };

  /**
   * Custom error.
   */
  template <typename T>
  class custom_error : public custom_error_base<T>
  {
  public:
    typedef typename custom_error_base<T>::error_type error_type;
    typedef typename custom_error_base<T>::null null;

    /**
     * The constructor.
     *
     * @param error the error code.
     */
    custom_error (error_type error):
      custom_error_base<T>(format_error(null(), null(), null(), error, null(), null()))
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
      custom_error_base<T>(format_error(detail, null(), null(), error, null(), null()))
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
      custom_error_base<T>(format_error(null(), null(), null(), error, error_string, null()))
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
      custom_error_base<T>(format_error(detail, null(), null(), error, error_string, null()))
    {
    }

    /**
     * The constructor.
     *
     * @param detail the details of the error.
     * @param error the error code.
     */
    custom_error (std::runtime_error const& error):
      custom_error_base<T>(format_error(null(), null(), null(), error, null(), null()))
    {
    }

    /**
     * The constructor.
     *
     * @param detail the details of the error.
     * @param error the error code.
     */
    custom_error (std::string const&        detail,
		  std::runtime_error const& error):
      custom_error_base<T>(format_error(detail, null(), null(), error, null(), null()))
    {
    }

    /// The destructor.
    virtual ~custom_error () throw ()
    {}
  };

}

#include "sbuild-custom-error.tcc"

#endif /* SBUILD_CUSTOM_ERROR_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
