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

parse_value::parse_value (std::string const& value):
  value(value)
{
}

parse_value::~parse_value ()
{
}

bool
parse_value::parse (bool& parsed_value) const
{
  if (this->value == "true" || this->value == "yes" || this->value == "1")
    parsed_value = true;
  else if (this->value == "false" || this->value == "no" || this->value == "0")
    parsed_value = false;
  else
    return false;

  log_debug(DEBUG_NOTICE) << "value=" << parsed_value << std::endl;
  return true;
}

bool
parse_value::parse (std::string& parsed_value) const
{
  parsed_value = this->value;
  log_debug(DEBUG_NOTICE) << "value=" << parsed_value << std::endl;
  return true;
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
