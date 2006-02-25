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

#include "sbuild-log.h"

namespace sbuild
{
  /**
   * Parse a boolean value.
   * @param stringval the string to parse.
   * @param value the variable to store the parsed value.
   * @returns true on success, false on failure.
   *
   * @todo Throw exception on parse failure.
   */
  bool
  parse_value (std::string const& stringval,
	       bool&              value);

  /**
   * Parse a string value.
   * @param stringval the string to parse.
   * @param value the variable to store the parsed value.
   * @returns true on success, false on failure.
   */
  bool
  parse_value (std::string const& stringval,
	       std::string&       value);

  /**
   * Parse a value.
   * @param stringval the string to parse.
   * @param value the variable to store the parsed value.
   * @returns true on success, false on failure.
   */
  template <typename T>
  bool
  parse_value (std::string const& stringval,
	       T&                 value)
  {
    std::istringstream is(stringval);
    is.imbue(std::locale("C"));
    T tmpval;
    if (is >> tmpval)
      {
	value = tmpval;
	log_debug(DEBUG_NOTICE) << "value=" << value << std::endl;
	return true;
      }
    log_debug(DEBUG_NOTICE) << "parse error" << std::endl;
    return false;
  }

}

#endif /* SBUILD_PARSE_VALUE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
