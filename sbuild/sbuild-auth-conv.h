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

#ifndef SBUILD_AUTH_CONV_H
#define SBUILD_AUTH_CONV_H

#include <sbuild/sbuild-auth-message.h>
#include <sbuild/sbuild-error.h>

#include <vector>

#include <security/pam_appl.h>

namespace sbuild
{

  /**
   * @brief Authentication conversation handler interface.
   *
   * This interface should be implemented by objects which handle
   * interaction with the user during authentication.
   *
   * This is a wrapper around the struct pam_conv PAM conversation
   * interface, and is used by auth when interacting with the user
   * during authentication.
   *
   * A simple implementation is provided in the form of auth_conv_tty.
   * However, more complex implementations might hook into an event
   * loop for GUI widget system.
   *
   * The interface allows the setting of optional warning timeout and
   * fatal timeout values, which should default to 0 (not enabled).
   * This is an absolute time after which a warning is displayed or
   * the conversation ends with an error.
   */
  class auth_conv
  {
  public:
    /// A list of messages.
    typedef std::vector<auth_message> message_list;

    /// The constructor.
    auth_conv ();
    /// The destructor.
    virtual ~auth_conv ();

    /**
     * @brief Get the time at which the user will be warned.
     *
     * @returns the time.
     */
    virtual time_t
    get_warning_timeout () = 0;

    /**
     * @brief Set the time at which the user will be warned.
     *
     * @param timeout the time to set.
     */
    virtual void
    set_warning_timeout (time_t timeout) = 0;

    /**
     * @brief Get the time at which the conversation will be
     * terminated with an error.
     *
     * @returns the time.
     */
    virtual time_t
    get_fatal_timeout () = 0;

    /**
     * @brief Set the time at which the conversation will be
     * terminated with an error.
     *
     * @param timeout the time to set.
     */
    virtual void
    set_fatal_timeout (time_t timeout) = 0;

    /**
     * @brief Hold a conversation with the user.
     *
     * Each of the messages detailed in messages should be displayed
     * to the user, asking for input where required.  The type of
     * message is indicated in the auth_message::message_type field of
     * the auth_message.  The auth_message::response field of the
     * auth_message should be filled in if input is required.
     *
     * @param messages the messages to display to the user, and
     * responses to return to the caller.
     * @returns true on success, false on failure.
     */
    virtual bool
    conversation (message_list& messages) = 0;
  };

}

#endif /* SBUILD_AUTH_CONV_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
