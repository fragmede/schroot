/* sbuild-auth-conv - sbuild auth conversation interface
 *
 * Copyright © 2005  Roger Leigh <rleigh@debian.org>
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

/**
 * SECTION:sbuild-auth-conv
 * @short_description: authentication conversation interface
 * @title: SbuildAuthConv
 *
 * SbuildAuthConv is an interface which should be implemented by
 * objects which handle interaction with the user during
 * authentication.
 *
 * This is a wrapper around the struct pam_conv PAM conversation
 * interface, and is used by #SbuildAuth when interacting with the
 * user during authentication.
 *
 * A simple implementation is provided in the form of
 * #SbuildAuthConvTty.  However, more complex implementations might
 * hook into the GLib main loop, or implement it in a #GtkWidget.
 *
 * The interface allows the setting of optional warning timeout and
 * fatal timeout values, which default to 0 (not enabled).  This is an
 * absolute time after which a warning is displayed or the
 * conversation ends with an error.
 */

#include <config.h>

#include "sbuild-auth-conv.h"

SbuildAuthConv::SbuildAuthConv()
{
}

SbuildAuthConv::~SbuildAuthConv()
{
}

/**
 * sbuild_auth_conv_conversation:
 * @auth_conv: an #SbuildAuthConv.
 * @num_messages: the number of messages
 * @messages: the messages to display to the user
 *
 * Hold a conversation with the user.
 *
 * Each of the messages detailed in @messages should be displayed to
 * the user, asking for input where required.  The type of message is
 * indicated in the type field of the #SbuildAuthMessage.  The
 * response field of the message vector #SbuildAuthMessage should be
 * filled in if input is required.  This will be automatically freed
 * on success or failure.  The user_data field of #SbuildAuthMessage
 * is for the use of #SbuildAuthConv implementations, and will be
 * otherwise ignored.
 *
 * Returns TRUE on success, FALSE on failure.
 */
bool
SbuildAuthConv::conversation (SbuildAuthConv::message_list& messages)
{
  return this->signal_conversation(messages);
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
