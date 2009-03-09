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

#include <config.h>

#include "sbuild-ctty.h"

#include <cerrno>
#include <cstring>

#include <ext/stdio_filebuf.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace sbuild;

namespace
{

  typedef std::pair<ctty_error_code,const char *> emap;

  /**
   * This is a list of the supported error codes.  It's used to
   * construct the real error codes map.
   */
  emap init_errors[] =
    {
      emap(CTTY_CLOEXEC, N_("The controlling terminal close-on-execute flag could not be set")),
      emap(CTTY_DUP,     N_("The controlling terminal file descriptor could not be duplicated"))
    };

  /**
   * Set close-on-exec flag.  An error will be thrown on failure.
   *
   * @param fd the file descriptor to set.
   */
  void
  set_cloexec (int fd)
  {
    int flags = fcntl(fd, F_GETFD);
    flags |= FD_CLOEXEC;
    if (fcntl(fd, F_SETFD, flags) < 0)
      throw ctty_error(CTTY_CLOEXEC, strerror(errno));
  }

  /**
   * Open the controlling terminal and return its file descriptor.
   *
   * @returns the CTTY fd, or -1 on failure.
   */
  int
  open_ctty ()
  {
    int ctty = open("/dev/tty", O_RDWR|O_NOCTTY);
    if (ctty >= 0)
      {
	set_cloexec(ctty);
      }
    else
      {
	ctty = -1;
      }

    return ctty;
  }

}

template<>
error<ctty_error_code>::map_type
error<ctty_error_code>::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));

const int sbuild::CTTY_FILENO(open_ctty());

namespace
{

  /**
   * Get the file descriptor for cttybuf.  An error will be thrown on
   * failure.
   *
   * @returns the CTTY_FILENO file descriptor if there is a CTTY, or a
   * duplicated file descriptor for stdin otherwise.
   */
  int
  cttybuf_fd ()
  {
    int ctty = CTTY_FILENO;

    if (ctty < 0)
      {
	ctty = dup(STDIN_FILENO);

	if (ctty < 0)
	  throw ctty_error(CTTY_DUP, strerror(errno));

	set_cloexec(ctty);
      }

    return ctty;
  }

  /// A streambuf for cctty.
  __gnu_cxx::stdio_filebuf<char> cttybuf(cttybuf_fd(),
					 std::ios::in|std::ios::out);

}

std::iostream sbuild::cctty(&cttybuf);
