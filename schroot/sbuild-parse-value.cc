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

#include "sbuild.h"

bool
sbuild::parse_value (std::string const& stringval,
		     bool&              value)
{
  if (stringval == "true" || stringval == "yes" || stringval == "1")
    value = true;
  else if (stringval == "false" || stringval == "no" || stringval == "0")
    value = false;
  else /// @todo Throw exception on parse failure.
    return false;

  log_debug(DEBUG_NOTICE) << "value=" << value << std::endl;
  return true;
}

bool
sbuild::parse_value (std::string const& stringval,
		     std::string&       value)
{
  value = stringval;
  log_debug(DEBUG_NOTICE) << "value=" << value << std::endl;
  return true;
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
