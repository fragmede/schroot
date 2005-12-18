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

/**
 * SECTION:sbuild-auth-message
 * @short_description: SbuildAuth messages
 * @title: SbuildAuthMessage and SbuildAuthMessageVector
 *
 * When SbuildAuth needs to interact with the user, it does this by
 * sending a set of #SbuildAuthMessage structures to an
 * #SbuildAuthConv conversation object.  These messages tell the
 * conversation object how to display the message to the user, and if
 * necessary, whether or not to ask the user for some input.
 */

#include <config.h>

#include "sbuild-auth-message.h"

using namespace sbuild;

AuthMessage::AuthMessage(AuthMessage::MessageType type,
			 const std::string&       message):
  type(type),
  message(message),
  response()
{
}

AuthMessage::~AuthMessage()
{
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
