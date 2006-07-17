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

#include "sbuild-error.h"
#include "sbuild-log.h"
#include "sbuild-nostream.h"
#include "sbuild-util.h"

#include <iostream>

std::ostream&
sbuild::log_info ()
{
  return std::cerr << "I: ";
}

std::ostream&
sbuild::log_warning ()
{
  return std::cerr << "W: ";
}

std::ostream&
sbuild::log_error ()
{
  return std::cerr << "E: ";
}

std::ostream&
sbuild::log_debug (sbuild::DebugLevel level)
{
  if (debug_level > 0 &&
      level >= debug_level)
    return std::cerr << "D(" << level << "): ";
  else
    return sbuild::cnull;
}

void
sbuild::log_exception_warning (std::exception const& e)
{
  log_warning() << e.what() << std::endl;

  try
    {
      sbuild::error_base const& eb(dynamic_cast<sbuild::error_base const&>(e));
      string_list lines = split_string(eb.why(), "\n");
      for (string_list::const_iterator line = lines.begin();
	   line != lines.end();
	   ++line)
	log_info() << *line << std::endl;
    }
  catch (std::bad_cast const& discard)
    {
    }
}

void
sbuild::log_exception_error (std::exception const& e)
{
  log_error() << e.what() << std::endl;

  try
    {
      sbuild::error_base const& eb(dynamic_cast<sbuild::error_base const&>(e));
      string_list lines = split_string(eb.why(), "\n");
      for (string_list::const_iterator line = lines.begin();
	   line != lines.end();
	   ++line)
	log_info() << *line << std::endl;
    }
  catch (std::bad_cast const& discard)
    {
    }
}

sbuild::DebugLevel sbuild::debug_level = sbuild::DEBUG_NONE;
