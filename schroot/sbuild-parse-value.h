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

#ifndef SBUILD_PARSE_VALUE_H
#define SBUILD_PARSE_VALUE_H

#include <string>

#include <boost/format.hpp>

#include "sbuild-error.h"
#include "sbuild-log.h"

namespace sbuild
{

  /**
   * Parse a text string value.  This is a wrapper around a string
   * value, to convert it into any desired type.
   */
  class parse_value
  {
  public:
    /// Exception type.
    typedef runtime_error_custom<parse_value> error;

    /**
     * The constructor.
     * @param value the value to parse.
     */
    parse_value (std::string const& value);

    /// The destructor.
    virtual ~parse_value ();

    /**
     * Convert object into any type T.
     * @returns an object of type T; an exception will be thrown on
     * parse failure.
     */
    template <typename T>
    operator T (void)
    {
      T tmp;

      if (parse(tmp) == false)
	{
	  boost::format fmt("Could not parse value \"%1%\"");
	  fmt % this->value;
	  throw error(fmt);
	}

      return tmp;
    }

  private:
    /**
     * Parse a boolean value.
     * @param parsed_value the variable to store the parsed value.
     * @returns true on success, false on failure.
     */
    bool
    parse (bool& parsed_value) const;

    /**
     * Parse a string value.
     * @param parsed_value the variable to store the parsed value.
     * @returns true on success, false on failure.
     */
    bool
    parse (std::string& parsed_value) const;

    /**
     * Parse a value of type T.
     * @param parsed_value the variable to store the parsed value.
     * @returns true on success, false on failure.
     */
    template <typename T>
    bool
    parse (T& parsed_value) const
    {
      std::istringstream is(this->value);
      is.imbue(std::locale("C"));
      T tmpval;
      if (is >> tmpval)
	{
	  parsed_value = tmpval;
	  log_debug(DEBUG_NOTICE) << "value=" << parsed_value << std::endl;
	  return true;
	}
      log_debug(DEBUG_NOTICE) << "parse error" << std::endl;
      return false;
    }

    std::string value;
  };

}

#endif /* SBUILD_PARSE_VALUE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
