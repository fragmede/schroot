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

#ifndef SBUILD_CUSTOM_ERROR_TCC
#define SBUILD_CUSTOM_ERROR_TCC

#include "sbuild-i18n.h"
#include "sbuild-custom-error.h"

#include <boost/format.hpp>

template <typename T>
const char *
sbuild::custom_error<T>::get_error (error_type error)
{
  typename map_type::const_iterator pos = error_strings.find(error);

  if (pos != error_strings.end())
    return gettext(pos->second);

  // Untranslated: it's a programming error to get this message.
  return "Unknown error";
}

template <typename T>
std::string
sbuild::custom_error<T>::format_error (std::string const& detail,
				       error_type         error)
{
  if (detail.length() > 0)
    {
      boost::format fmt("%1%: %2%");
      fmt % detail % get_error(error);
      return fmt.str();
    }
  else
    return get_error(error);
}

template <typename T>
std::string
sbuild::custom_error<T>::format_error (std::string const& detail,
				       error_type         error,
				       int                error_number)
{
  return format_error(detail, error, std::string(strerror(error_number)));
}

template <typename T>
std::string
sbuild::custom_error<T>::format_error (std::string const& detail,
				       int                error_number)
{
  return format_error(detail, std::string(strerror(error_number)));
}

template <typename T>
std::string
sbuild::custom_error<T>::format_error (std::string const& detail,
				       std::string const& error_string)
{
  if (detail.length() > 0)
    {
      boost::format fmt("%1%: %2%");
      fmt % detail % error_string;
      return fmt.str();
    }
  else
    {
      boost::format fmt("%1%");
      fmt % error_string;
      return fmt.str();
    }
}

template <typename T>
std::string
sbuild::custom_error<T>::format_error (std::string const& detail,
				       error_type         error,
				       std::string const& error_string)
{
  if (detail.length() > 0)
    {
      boost::format fmt("%1%: %2%: %3%");
      fmt % detail
	% get_error(error)
	% error_string;
      return fmt.str();
    }
  else
    {
      boost::format fmt("%1%: %2%");
      fmt % get_error(error)
	% error_string;
      return fmt.str();
    }
}

#endif /* SBUILD_CUSTOM_ERROR_TCC */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
