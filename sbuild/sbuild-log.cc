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

#include "sbuild-ctty.h"
#include "sbuild-error.h"
#include "sbuild-i18n.h"
#include "sbuild-log.h"
#include "sbuild-nostream.h"
#include "sbuild-util.h"

#include <iostream>

#include <boost/format.hpp>

using boost::format;

std::ostream&
sbuild::log_info ()
{
  // TRANSLATORS: "I" is an abbreviation of "Information"
  return std::cerr << _("I: ");
}

std::ostream&
sbuild::log_warning ()
{
  // TRANSLATORS: "W" is an abbreviation of "Warning"
  return std::cerr << _("W: ");
}

std::ostream&
sbuild::log_error ()
{
  // TRANSLATORS: "E" is an abbreviation of "Error"
  return std::cerr << _("E: ");
}

std::ostream&
sbuild::log_debug (sbuild::DebugLevel level)
{
  if (debug_level > 0 &&
      level >= debug_level)
    // TRANSLATORS: %1% = integer debug level
    // TRANSLATORS: "D" is an abbreviation of "Debug"
    return std::cerr << format(_("D(%1%): ")) % level;
  else
    return sbuild::cnull;
}

std::ostream&
sbuild::log_ctty_info ()
{
  // TRANSLATORS: "I" is an abbreviation of "Information"
  return cctty << _("I: ");
}

std::ostream&
sbuild::log_ctty_warning ()
{
  // TRANSLATORS: "W" is an abbreviation of "Warning"
  return cctty << _("W: ");
}

std::ostream&
sbuild::log_ctty_error ()
{
  // TRANSLATORS: "E" is an abbreviation of "Error"
  return cctty << _("E: ");
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

void
sbuild::log_ctty_exception_warning (std::exception const& e)
{
  log_ctty_warning() << e.what() << std::endl;

  try
    {
      sbuild::error_base const& eb(dynamic_cast<sbuild::error_base const&>(e));
      string_list lines = split_string(eb.why(), "\n");
      for (string_list::const_iterator line = lines.begin();
	   line != lines.end();
	   ++line)
	log_ctty_info() << *line << std::endl;
    }
  catch (std::bad_cast const& discard)
    {
    }
}

void
sbuild::log_ctty_exception_error (std::exception const& e)
{
  log_ctty_error() << e.what() << std::endl;

  try
    {
      sbuild::error_base const& eb(dynamic_cast<sbuild::error_base const&>(e));
      string_list lines = split_string(eb.why(), "\n");
      for (string_list::const_iterator line = lines.begin();
	   line != lines.end();
	   ++line)
	log_ctty_info() << *line << std::endl;
    }
  catch (std::bad_cast const& discard)
    {
    }
}

void
sbuild::log_unknown_exception_error ()
{
  log_error() << _("An unknown exception occurred") << std::endl;
}

sbuild::DebugLevel sbuild::debug_level = sbuild::DEBUG_NONE;
