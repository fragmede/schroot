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

#include <config.h>

#include "sbuild-parse-value.h"

using namespace sbuild;

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
      throw parse_error(parse_error::BAD_VALUE, value);
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

/*
 * Local Variables:
 * mode:C++
 * End:
 */
