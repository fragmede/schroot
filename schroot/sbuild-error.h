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

#ifndef SBUILD_ERROR_H
#define SBUILD_ERROR_H

#include <stdexcept>

#include <boost/format.hpp>

namespace sbuild
{

  /**
   * Generic runtime error.
   */
  class runtime_error : public std::runtime_error
  {
  public:
    /**
     * The constructor.
     *
     * @param error the error message.
     */
    runtime_error (std::string const& error):
      std::runtime_error(error)
    {}

    /// The destructor.
    virtual ~runtime_error () throw ()
    {}
  };

  /**
   * Runtime error specific to a class.
   */
  template <typename T>
  class runtime_error_custom : public runtime_error
  {
  public:
    /**
     * The constructor.
     *
     * @param error the error message.
     */
    runtime_error_custom (std::string const& error):
      runtime_error(error)
    {}

    /**
     * The constructor.
     *
     * @param error the error message (formatted).
     */
    runtime_error_custom (boost::format const& error):
      runtime_error(error.str())
    {}

    /// The destructor.
    virtual ~runtime_error_custom () throw ()
    {}
  };

}

#endif /* SBUILD_ERROR_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
