/* Copyright © 2005-2007  Roger Leigh <rleigh@debian.org>
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

#include <config.h>

#include "sbuild-parse-value.h"

using namespace sbuild;

template<>
error<parse_value_error_code>::map_type
error<parse_value_error_code>::error_strings =
  {
    // TRANSLATORS: %1% = value (arbitrary text)
    {BAD_VALUE, N_("Could not parse value ‘%1%’")}
  };

void
sbuild::parse_value (std::string const& value,
                     bool&              parsed_value)
{
  if (value == "true" || value == "yes" || value == "1")
    parsed_value = true;
  else if (value == "false" || value == "no" || value == "0")
    parsed_value = false;
  else
    {
      log_debug(DEBUG_NOTICE) << "parse error" << std::endl;
      throw parse_value_error(value, BAD_VALUE);
    }

  log_debug(DEBUG_NOTICE) << "value=" << parsed_value << std::endl;
}

void
sbuild::parse_value (std::string const& value,
                     std::string&       parsed_value)
{
  parsed_value = value;
  log_debug(DEBUG_NOTICE) << "value=" << parsed_value << std::endl;
}
