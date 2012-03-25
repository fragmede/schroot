/* Copyright © 2005-2008  Roger Leigh <rleigh@debian.org>
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

#include "sbuild-keyfile-base.h"

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

namespace
{

  typedef std::pair<keyfile_base::error_code,const char *> emap;

  /**
   * This is a list of the supported error codes.  It's used to
   * construct the real error codes map.
   */
  emap init_errors[] =
    {
      // TRANSLATORS: %1% = file
      emap(keyfile_base::BAD_FILE,
	   N_("Can't open file ‘%1%’")),
      // TRANSLATORS: %1% = line number in configuration file
      // TRANSLATORS: %2% = group name ("[groupname]" in configuration file)
      // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
      emap(keyfile_base::DEPRECATED_KEY,
	   N_("line %1% [%2%]: Deprecated key ‘%4%’ used")),
      // TRANSLATORS: %1% = group name ("[groupname]" in configuration file)
      // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
      emap(keyfile_base::DEPRECATED_KEY_NL,
	   N_("[%1%]: Deprecated key ‘%4%’ used")),
      // TRANSLATORS: %1% = line number in configuration file
      // TRANSLATORS: %2% = group name ("[groupname]" in configuration file)
      // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
      emap(keyfile_base::DISALLOWED_KEY,
	   N_("line %1% [%2%]: Disallowed key ‘%4%’ used")),
      // TRANSLATORS: %1% = group name ("[groupname]" in configuration file)
      // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
      emap(keyfile_base::DISALLOWED_KEY_NL,
	   N_("[%1%]: Disallowed key ‘%4%’ used")),
      // TRANSLATORS: %1% = line number in configuration file
      // TRANSLATORS: %4% = group name ("[groupname]" in configuration file)
      emap(keyfile_base::DUPLICATE_GROUP,
	   N_("line %1%: Duplicate group ‘%4%’")),
      // TRANSLATORS: %1% = line number in configuration file
      // TRANSLATORS: %2% = group name ("[groupname]" in configuration file)
      // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
      emap(keyfile_base::DUPLICATE_KEY,
	   N_("line %1% [%2%]: Duplicate key ‘%4%’")),
      // TRANSLATORS: %1% = line number in configuration file
      // TRANSLATORS: %4% = line contents as read from the configuration file
      emap(keyfile_base::INVALID_GROUP,
	   N_("line %1%: Invalid group: “%4%”")),
      // TRANSLATORS: %1% = line number in configuration file
      // TRANSLATORS: %2% = group name ("[groupname]" in configuration file)
      // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
      emap(keyfile_base::INVALID_KEY,
	   N_("line %1% [%2%]: Invalid key ‘%4%’ used")),
      // TRANSLATORS: %1% = line number in configuration file
      // TRANSLATORS: %4% = line contents as read from the configuration file
      emap(keyfile_base::INVALID_LINE,
	   N_("line %1%: Invalid line: “%4%”")),
      // TRANSLATORS: %1% = line number in configuration file
      // TRANSLATORS: %2% = group name ("[groupname]" in configuration file)
      // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
      emap(keyfile_base::MISSING_KEY,
	   N_("line %1% [%2%]: Required key ‘%4%’ is missing")),
      // TRANSLATORS: %1% = group name ("[groupname]" in configuration file)
      // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
      emap(keyfile_base::MISSING_KEY_NL,
	   N_("[%1%]: Required key ‘%4%’ is missing")),
      // TRANSLATORS: %1% = line number in configuration file
      // TRANSLATORS: %4% = line contents as read from the configuration file
      emap(keyfile_base::NO_GROUP,
	   N_("line %1%: No group specified: “%4%”")),
      // TRANSLATORS: %1% = line number in configuration file
      // TRANSLATORS: %4% = line contents as read from the configuration file
      emap(keyfile_base::NO_KEY,
	   N_("line %1%: No key specified: “%4%”")),
      // TRANSLATORS: %1% = line number in configuration file
      // TRANSLATORS: %2% = group name ("[groupname]" in configuration file)
      // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
      emap(keyfile_base::OBSOLETE_KEY,
	   N_("line %1% [%2%]: Obsolete key ‘%4%’ used")),
      // TRANSLATORS: %1% = group name ("[groupname]" in configuration file)
      // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
      emap(keyfile_base::OBSOLETE_KEY_NL,
	   N_("[%1%]: Obsolete key ‘%4%’ used")),
      // TRANSLATORS: %1% = group name ("[groupname]" in configuration file)
      // TRANSLATORS: %4% = additional details
      emap(keyfile_base::PASSTHROUGH_G,
	   N_("[%1%]: %4%")),
      // TRANSLATORS: %1% = group name ("[groupname]" in configuration file)
      // TRANSLATORS: %2% = key name ("keyname=value" in configuration file)
      // TRANSLATORS: %4% = additional details
      emap(keyfile_base::PASSTHROUGH_GK,
	   N_("[%1%] %2%: %4%")),
      // TRANSLATORS: %1% = line number in configuration file
      // TRANSLATORS: %2% = group name ("[groupname]" in configuration file)
      // TRANSLATORS: %4% = additional details
      emap(keyfile_base::PASSTHROUGH_LG,
	   N_("line %1% [%2%]: %4%")),
      // TRANSLATORS: %1% = line number in configuration file
      // TRANSLATORS: %2% = group name ("[groupname]" in configuration file)
      // TRANSLATORS: %3% = key name ("keyname=value" in configuration file)
      // TRANSLATORS: %4% = additional details
      emap(keyfile_base::PASSTHROUGH_LGK,
	   N_("line %1% [%2%] %3%: %4%")),
      // TRANSLATORS: %1% = line number in configuration file
      // TRANSLATORS: %2% = group name ("[groupname]" in configuration file)
      // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
      emap(keyfile_base::UNKNOWN_KEY,
	   N_("line %1% [%2%]: Unknown key ‘%4%’ used"))
    };

}

template<>
error<keyfile_base::error_code>::map_type
error<keyfile_base::error_code>::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));

keyfile_base::keyfile_base ()
{
}

keyfile_base::~keyfile_base()
{
}
