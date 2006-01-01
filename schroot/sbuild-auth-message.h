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

#ifndef SBUILD_AUTH_MESSAGE_H
#define SBUILD_AUTH_MESSAGE_H

#include <string>
#include <vector>

#include <security/pam_appl.h>

namespace sbuild
{

  /**
   * Authentication messages.
   *
   * When Auth needs to interact with the user, it does this by
   * sending a list of AuthMessage objects to an AuthConv conversation
   * object.  These messages tell the conversation object how to
   * display the message to the user, and if necessary, whether or not
   * to ask the user for some input.  They also store the user's
   * input, if required.
   */
  class AuthMessage
  {
  public:
    /// Message type
    enum MessageType
      {
	/// Display a prompt, with no echoing of user input.
	MESSAGE_PROMPT_NOECHO = PAM_PROMPT_ECHO_OFF,
	/// Display a prompt, echoing user input.
	MESSAGE_PROMPT_ECHO = PAM_PROMPT_ECHO_ON,
	/// Display an error message.
	MESSAGE_ERROR = PAM_ERROR_MSG,
	/// Display an informational message.
	MESSAGE_INFO = PAM_TEXT_INFO
      };

    /**
     * The constructor.
     *
     * @param type the type of message.
     * @param message the message to display.
     */
    AuthMessage(MessageType        type,
		std::string const& message);

    /// The destructor.
    virtual ~AuthMessage();

    /// The type of message.
    MessageType type;
    /// The message to display.
    std::string message;
    /// The user's response (if any).
    std::string response;
  };

}

#endif /* SBUILD_AUTH_MESSAGE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
