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
template <typename A, typename B, typename C, typename D, typename E>
inline std::string
sbuild::custom_error_base<T>::format_error (A const&   context1,
					    B const&   context2,
					    C const&   context3,
					    error_type error,
					    D const&   detail1,
					    E const&   detail2)
{
  std::string format;
  std::string msg(get_error(error));
  unsigned int nargs(0);

  if (msg.find("%1%") != std::string::npos)
    {
      nargs = 1;
    }
  else if(typeid(context1) != typeid(null))
    {
      format += "%1%: ";
      nargs = 1;
    }

  if (msg.find("%2%") != std::string::npos)
    {
      nargs = 2;
    }
  else if (typeid(context2) != typeid(null))
    {
      format += "%2%: ";
      nargs = 2;
    }

  if (msg.find("%3%") != std::string::npos)
    {
      nargs = 3;
    }
  else if (typeid(context3) != typeid(null))
    {
      format += "%3%: ";
      nargs = 3;
    }

  format += msg;

  if (msg.find("%4%") != std::string::npos)
    {
      nargs = 4;
    }
  else if (typeid(detail1) != typeid(null))
    {
      if (msg.empty())
	format += "%4%";
      else
	format += ": %4%";
      nargs = 4;
    }

  if (msg.find("%5%") != std::string::npos)
    {
      nargs = 5;
    }
  else if (typeid(detail2) != typeid(null))
    {
      if (msg.empty() && nargs < 4)
	format += "%5%";
      else
	format += ": %5%";
      nargs = 5;
    }

  boost::format fmt(format);
  if (nargs >= 1)
    fmt % context1;
  if (nargs >= 2)
    fmt % context2;
  if (nargs >= 3)
    fmt % context3;
  if (nargs >= 4)
    fmt % detail1;
  if (nargs >= 5)
    fmt % detail2;

  return fmt.str();
}

template <typename T>
template <typename A, typename B, typename C, typename D, typename E>
inline std::string
sbuild::custom_error_base<T>::format_error (A const&   context1,
					    B const&   context2,
					    C const&   context3,
					    std::runtime_error const& error,
					    D const&   detail1,
					    E const&   detail2)
{
  std::string format;
  std::string msg(error.what());
  unsigned int nargs(0);

  if (typeid(context1) != typeid(null))
    {
      format += "%1%: ";
      nargs = 1;
    }

  if (typeid(context2) != typeid(null))
    {
      format += "%2%: ";
      nargs = 2;
    }

  if (typeid(context3) != typeid(null))
    {
      format += "%3%: ";
      nargs = 3;
    }

  format += msg;

  if (typeid(detail1) != typeid(null))
    {
      if (msg.empty())
	format += "%4%";
      else
	format += ": %4%";
      nargs = 4;

    }

  if (typeid(detail2) != typeid(null))
    {
      if (msg.empty() && nargs < 4)
	format += "%5%";
      else
	format += ": %5%";
      nargs = 5;
    }

  boost::format fmt(format);
  if (nargs >= 1)
    fmt % context1;
  if (nargs >= 2)
    fmt % context2;
  if (nargs >= 3)
    fmt % context3;
  if (nargs >= 4)
    fmt % detail1;
  if (nargs >= 5)
    fmt % detail2;

  return fmt.str();
}

template <typename T>
const char *
sbuild::custom_error_base<T>::get_error (error_type error)
{
  typename map_type::const_iterator pos = error_strings.find(error);

  if (pos != error_strings.end())
    return gettext(pos->second);

  // Untranslated: it's a programming error to get this message.
  return "Unknown error";
}

#endif /* SBUILD_CUSTOM_ERROR_TCC */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
