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

#include <iostream>

std::ostream&
sbuild::log_info()
{
  return std::cerr << "I: ";
}

std::ostream&
sbuild::log_warning()
{
  return std::cerr << "W: ";
}

std::ostream&
sbuild::log_error()
{
  return std::cerr << "E: ";
}

std::ostream&
sbuild::log_debug(sbuild::DebugLevel level)
{
  if (debug_level > 0 &&
      level >= debug_level)
    return std::cerr << "D(" << level << "): ";
  else
    return sbuild::cnull;
}

sbuild::DebugLevel sbuild::debug_level = sbuild::DEBUG_NONE;

/*
 * Local Variables:
 * mode:C++
 * End:
 */
