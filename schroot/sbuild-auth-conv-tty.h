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

#ifndef SBUILD_AUTH_CONV_TTY_H
#define SBUILD_AUTH_CONV_TTY_H

#include <sys/types.h>
#include <sys/wait.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#include <security/pam_appl.h>
#include <security/pam_misc.h>

#include "sbuild-auth-conv.h"

namespace sbuild
{
  /**
   * @brief Authentication conversation handler for terminal devices.
   *
   * This class is an implementation of the auth_conv interface, and
   * is used to interact with the user on a terminal (TTY) interface.
   *
   * In order to implement timeouts, this class uses alarm(2).  This
   * has some important implications.  Global state is modified by the
   * object, so only one may be used at once in a single process.  In
   * addition, no other part of the process may set or unset the
   * SIGALRM handlers and the alarm(2) timer during the time PAM
   * authentication is proceeding.
   */
  class auth_conv_tty : public auth_conv
  {
  public:
    /// The constructor.
    auth_conv_tty();
    /// The destructor.
    virtual ~auth_conv_tty();

    virtual time_t get_warning_timeout ();
    virtual void set_warning_timeout (time_t timeout);

    virtual time_t get_fatal_timeout ();
    virtual void set_fatal_timeout (time_t timeout);

    virtual bool conversation (message_list& messages);

  private:
    /**
     * @brief Get the time delay before the next SIGALRM signal.
     *
     * If either the warning timeout or the fatal timeout have
     * expired, a message to notify the user is printed to stderr.  If
     * the fatal timeout is reached, an exception is thrown.
     *
     * @returns the delay in seconds, or 0 if no delay is set.
     */
    int get_delay ();

    /**
     * @brief Read user input from standard input.
     *
     * The prompt message is printed to prompt the user for input.  If
     * echo is true, the user input it echoed back to the terminal,
     * but if false, echoing is suppressed using termios(3).
     *
     * If the SIGALRM timer expires while waiting for input, this is
     * handled by re-checking the delay time which will warn the user
     * or cause the input routine to terminate if the fatal timeout
     * has expired.
     *
     * @param message the message to prompt the user for input.
     * @param echo echo user input to screen.
     * @returns a string, which is empty on failure.
     */
    std::string
    read_string (std::string message,
		 bool        echo);

    /// The time to warn at.
    time_t  warning_timeout;
    /// The time to end at.
    time_t  fatal_timeout;
    /// The time the current delay was obtained at.
    time_t  start_time;
  };

}

#endif /* SBUILD_AUTH_CONV_TTY_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
