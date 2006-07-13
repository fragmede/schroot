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

namespace sbuild
{

  /**
   * Custom error.
   */
  template <typename T>
  class custom_error : public error<T>
  {
  public:
    typedef typename error<T>::error_type error_type;
    typedef typename error<T>::null null;

    /**
     * The constructor.
     *
     * @param error the error code.
     */
    custom_error (error_type error):
      sbuild::error<T>(format_error(null(), null(), null(), error, null(), null()))
    {
    }

    /**
     * The constructor.
     *
     * @param context the context of the error.
     * @param error the error code.
     */
    template<typename C>
    custom_error (C const&   context,
		  error_type error):
      sbuild::error<T>(format_error(context, null(), null(), error, null(), null()))
    {
    }

    /**
     * The constructor.
     *
     * @param error the error code.
     * @param detail the details of the error.
     */
    template<typename D>
    custom_error (error_type error,
		  D const&   detail):
      sbuild::error<T>(format_error(null(), null(), null(), error, detail, null()))
    {
    }

    /**
     * The constructor.
     *
     * @param error the error code.
     * @param detail the details of the error.
     * @param detail2 additional details of the error.
     */
    template<typename D, typename E>
    custom_error (error_type error,
		  D const&   detail,
		  E const&   detail2):
      sbuild::error<T>(format_error(null(), null(), null(), error, detail, detail2))
    {
    }

    /**
     * The constructor.
     *
     * @param context the context of the error.
     * @param error the error code.
     * @param detail the details of the error.
     */
    template<typename C, typename D>
    custom_error (C const&   context,
		  error_type error,
		  D const&   detail):
      sbuild::error<T>(format_error(context, null(), null(), error, detail, null()))
    {
    }

    /**
     * The constructor.
     *
     * @param context the context of the error.
     * @param error the error code.
     * @param detail the details of the error.
     * @param detail2 additional details of the error.
     */
    template<typename C, typename D, typename E>
    custom_error (C const&   context,
		  error_type error,
		  D const&   detail,
		  E const&   detail2):
      sbuild::error<T>(format_error(context, null(), null(), error, detail, detail2))
    {
    }

    /**
     * The constructor.
     *
     * @param error the error code.
     */
    custom_error (std::runtime_error const& error):
      sbuild::error<T>(format_error(null(), null(), null(), error, null(), null()))
    {
    }

    /**
     * The constructor.
     *
     * @param context the context of the error.
     * @param error the error code.
     */
    template<typename C>
    custom_error (C const&                  context,
		  std::runtime_error const& error):
      sbuild::error<T>(format_error(context, null(), null(), error, null(), null()))
    {
    }

    /// The destructor.
    virtual ~custom_error () throw ()
    {}
  };

}

#endif /* SBUILD_CUSTOM_ERROR_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
