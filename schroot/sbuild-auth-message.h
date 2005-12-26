/* sbuild-auth-message - sbuild authentication messaging types
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

#ifndef SBUILD_AUTH_MESSAGE_H
#define SBUILD_AUTH_MESSAGE_H

#include <string>
#include <vector>

#include <security/pam_appl.h>

namespace sbuild
{

  class AuthMessage
  {
  public:
    enum MessageType
      {
	MESSAGE_PROMPT_NOECHO = PAM_PROMPT_ECHO_OFF,
	MESSAGE_PROMPT_ECHO = PAM_PROMPT_ECHO_ON,
	MESSAGE_ERROR = PAM_ERROR_MSG,
	MESSAGE_INFO = PAM_TEXT_INFO
      };

    AuthMessage(MessageType        type,
		const std::string& message);
    virtual ~AuthMessage();

    MessageType type;
    std::string message;
    std::string response;
  };

}

#endif /* SBUILD_AUTH_MESSAGE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
