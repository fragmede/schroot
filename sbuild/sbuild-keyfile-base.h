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

#ifndef SBUILD_KEYFILE_BASE_H
#define SBUILD_KEYFILE_BASE_H

#include <sbuild/sbuild-i18n.h>
#include <sbuild/sbuild-log.h>
#include <sbuild/sbuild-parse-error.h>
#include <sbuild/sbuild-parse-value.h>
#include <sbuild/sbuild-types.h>
#include <sbuild/sbuild-tr1types.h>
#include <sbuild/sbuild-util.h>

#include <cassert>
#include <map>
#include <string>
#include <sstream>

#include <boost/format.hpp>

namespace sbuild
{

  /**
   * Base class for key-value configuration file formats.
   */
  class keyfile_base
  {
  public:
    /// Configuration parameter priority.
    enum priority
      {
	PRIORITY_OPTIONAL,   ///< The parameter is optional.
	PRIORITY_REQUIRED,   ///< The parameter is required.
	PRIORITY_DISALLOWED, ///< The parameter is not allowed in this context.
	PRIORITY_DEPRECATED, ///< The parameter is deprecated, but functional.
	PRIORITY_OBSOLETE    ///< The parameter is obsolete, and not functional.
      };

    /// Error codes.
    enum error_code
      {
	BAD_FILE,          ///< The file to parse couldn't be opened.
	DEPRECATED_KEY,    ///< The key is deprecated.
	DEPRECATED_KEY_NL, ///< The key is deprecated (no line specified).
	DISALLOWED_KEY,    ///< The key is not allowed.
	DISALLOWED_KEY_NL, ///< The key is not allowed (no line specified).
	DUPLICATE_GROUP,   ///< The group is a duplicate.
	DUPLICATE_KEY,     ///< The key is a duplicate.
	INVALID_GROUP,     ///< The group is invalid.
	INVALID_LINE,      ///< The line is invalid.
	MISSING_KEY,       ///< The key is missing.
	MISSING_KEY_NL,    ///< The key is missing (no line specified).
	NO_GROUP,          ///< No group was specified.
	NO_KEY,            ///< No key was specified.
	OBSOLETE_KEY,      ///< The key is obsolete.
	OBSOLETE_KEY_NL,   ///< The key is obsolete (no line specified).
	PASSTHROUGH_G,     ///< Pass through exception with group.
	PASSTHROUGH_GK,    ///< Pass through exception with group and key.
	PASSTHROUGH_LG,    ///< Pass through exception with line and group.
	PASSTHROUGH_LGK,   ///< Pass through exception with line, group and key.
	UNKNOWN_KEY        ///< The key is unknown.
      };

    /// Exception type.
    typedef parse_error<error_code> error;

    /// The constructor.
    keyfile_base ();

    /// The destructor.
    virtual ~keyfile_base ();

  };

}

#endif /* SBUILD_KEYFILE_BASE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
