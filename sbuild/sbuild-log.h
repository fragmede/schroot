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

#ifndef SBUILD_LOG_H
#define SBUILD_LOG_H

#include <ostream>

namespace sbuild
{

  /// Debugging level.
  enum DebugLevel
    {
      DEBUG_NONE = -1,   ///< No debugging.
      DEBUG_NOTICE = 1,  ///< Notification messages.
      DEBUG_INFO = 2,    ///< Informational messages.
      DEBUG_WARNING = 3, ///< Warning messages.
      DEBUG_CRITICAL = 4 ///< Critical messages.
    };

  /**
   * Log an informational message.
   *
   * @returns an ostream.
   */
  std::ostream&
  log_info ();

  /**
   * Log a warning message.
   *
   * @returns an ostream.
   */
  std::ostream&
  log_warning ();

  /**
   * Log an error message.
   *
   * @returns an ostream.
   */
  std::ostream&
  log_error ();

  /**
   * Log a debug message.
   *
   * @param level the debug level of the message being logged.
   * @returns an ostream.  This will be a valid stream if level is
   * greater or equal to debug_level, or else a null stream will be
   * returned, resulting in no output.
   */
  std::ostream&
  log_debug (DebugLevel level);

  /**
   * Log an exception.
   *
   * @param e the exception to log.
   */
  void
  log_exception (std::exception const& e);

  /// The debugging level in use.
  extern DebugLevel debug_level;

}

#endif /* SBUILD_LOG_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
