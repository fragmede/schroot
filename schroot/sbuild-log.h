/* sbuild-log - sbuild message logging
 *
 * Copyright © 2005  Roger Leigh <rleigh@debian.org>
 *
 * serror is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * serror is distributed in the hope that it will be useful, but
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

  enum DebugLevel
    {
      DEBUG_NONE = -1,
      DEBUG_NOTICE = 1,
      DEBUG_INFO = 2,
      DEBUG_WARNING = 3,
      DEBUG_CRITICAL = 4
    };

  std::ostream&
  log_info();

  std::ostream&
  log_warning();

  std::ostream&
  log_error();

  std::ostream&
  log_debug(DebugLevel level);

  extern DebugLevel debug_level;

}

#endif /* SBUILD_LOG_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
