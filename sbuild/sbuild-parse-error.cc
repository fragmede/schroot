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

#include "sbuild-i18n.h"
#include "sbuild-parse-error.h"

#include <boost/format.hpp>

using namespace sbuild;
using boost::format;

namespace
{

  typedef std::pair<parse_error::type,const char *> emap;

  /**
   * This is a list of the supported error codes.  It's used to
   * construct the real error codes map.
   */
  emap init_errors[] =
    {
      emap(parse_error::NONE,            N_("No error")),
      emap(parse_error::BAD_FILE,        N_("Can't open file")),
      emap(parse_error::BAD_VALUE,       N_("Could not parse value")),
      emap(parse_error::INVALID_LINE,    N_("Invalid line")),
      emap(parse_error::NO_GROUP,        N_("No group specified")),
      emap(parse_error::INVALID_GROUP,   N_("Invalid group")),
      emap(parse_error::DUPLICATE_GROUP, N_("Duplicate group")),
      emap(parse_error::NO_KEY,          N_("No key specified")),
      emap(parse_error::DUPLICATE_KEY,   N_("Duplicate key")),
      emap(parse_error::MISSING_KEY,     N_("Required key is missing")),
      emap(parse_error::DISALLOWED_KEY,  N_("Disallowed key used"))
    };

}

std::map<parse_error::type,const char *>
parse_error::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));


parse_error::parse_error (type               error,
			  std::string const& detail):
  runtime_error(format_error(error, detail))
{
}

parse_error::parse_error (size_t             line,
			  type               error,
			  std::string const& detail):
  runtime_error(format_error(line, error, detail))
{
}

parse_error::parse_error (size_t             line,
			  std::string const& group,
			  type               error,
			  std::string const& detail):
  runtime_error(format_error(line, group, error, detail))
{
}

parse_error::parse_error (size_t             line,
			  std::string const& group,
			  std::string const& key,
			  type               error,
			  std::string const& detail):
  runtime_error(format_error(line, group, key, error, detail))
{
}

parse_error::parse_error (std::string const& group,
			  type               error,
			  std::string const& detail):
  runtime_error(format_error(group, error, detail))
{
}

parse_error::parse_error (std::string const& group,
			  std::string const& key,
			  type               error,
			  std::string const& detail):
  runtime_error(format_error(group, key, error, detail))
{
}

const char *
parse_error::get_error (type error)
{
  std::map<type,const char *>::const_iterator pos =
    error_strings.find(error);

  if (pos != error_strings.end())
    return gettext(pos->second);

  return _("Unknown error");
}

std::string
parse_error::format_error (type               error,
			   std::string const& detail)
{
  if (detail.length() > 0)
    {
      format fmt((error == NONE
		 ? "%2%"
		 : _("%1% \"%2%\"")));
      fmt % get_error(error) % detail;
      return fmt.str();
    }
  else
    return get_error(error);
}

std::string
parse_error::format_error (size_t             line,
			   type               error,
			   std::string const& detail)
{
  if (detail.length() > 0)
    {
      format fmt((error == NONE
		  ? _("line %1%: %3%")
		  : _("line %1%: %2% \"%3%\"")));
      fmt % line % get_error(error) % detail;
      return fmt.str();
    }
  else
    {
      format fmt(_("line %1%: %2%"));
      fmt % line % get_error(error);
      return fmt.str();
    }
}

std::string
parse_error::format_error (size_t             line,
			   std::string const& group,
			   type               error,
			   std::string const& detail)
{
  if (detail.length() > 0)
    {
      format fmt((error == NONE
		  ? _("line %1% [%2%]: %4%")
		  : _("line %1% [%2%]: %3% \"%4%\"")));
      fmt % line % group % get_error(error) % detail;
      return fmt.str();
    }
  else
    {
      format fmt(_("line %1% [%2%]: %3%"));
      fmt % line % group % get_error(error);
      return fmt.str();
    }
}

std::string
parse_error::format_error (size_t             line,
			   std::string const& group,
			   std::string const& key,
			   type               error,
			   std::string const& detail)
{
  if (detail.length() > 0)
    {
      format fmt((error == NONE
		  ? _("line %1% [%2%] %3%: %5%")
		  : _("line %1% [%2%] %3%: %4% \"%5%\"")));
      fmt % line % group % key % get_error(error) % detail;
      return fmt.str();
    }
  else
    {
      format fmt(_("line %1% [%2%] %3%: %4%"));
      fmt % line % group % key % get_error(error);
      return fmt.str();
    }
}

std::string
parse_error::format_error (std::string const& group,
			   type               error,
			   std::string const& detail)
{
  if (detail.length() > 0)
    {
      format fmt((error == NONE
		  ? _("[%1%]: %3%")
		  : _("[%1%]: %2% \"%3%\"")));
      fmt % group % get_error(error) % detail;
      return fmt.str();
    }
  else
    {
      format fmt(_("[%1%]: %2%"));
      fmt % group % get_error(error);
      return fmt.str();
    }
}

std::string
parse_error::format_error (std::string const& group,
			   std::string const& key,
			   type               error,
			   std::string const& detail)
{
  if (detail.length() > 0)
    {
      format fmt((error == NONE
		  ? _("[%1%] %2%: %4%")
		  : _("[%1%] %2%: %3% \"%4%\"")));
      fmt % group % key % get_error(error) % detail;
      return fmt.str();
    }
  else
    {
      format fmt(_("[%1%] %2%: %3%"));
      fmt % group % key % get_error(error);
      return fmt.str();
    }
}
