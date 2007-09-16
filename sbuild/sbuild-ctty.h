/* Copyright © 2006-2007  Roger Leigh <rleigh@debian.org>
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

#ifndef SBUILD_CTTY_H
#define SBUILD_CTTY_H

#include <sbuild/sbuild-parse-error.h>

#include <streambuf>
#include <ostream>

namespace sbuild
{

  enum ctty_error_code
    {
      CTTY_CLOEXEC, ///< The CTTY FD_CLOEXEC flag could not be set.
      CTTY_DUP      ///< The CTTY file descriptor could not be duplicated.
    };

  typedef parse_error<ctty_error_code> ctty_error;

  /**
   * CTTY fd.  The fd number of the Controlling TTY, or -1 if not
   * available.
   */
  extern const int CTTY_FILENO;

  /**
   * CTTY stream.  A stream to the Controlling TTY, or standard input
   * if not available.
   */
  extern std::iostream cctty;

}

#endif /* SBUILD_CTTY_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
