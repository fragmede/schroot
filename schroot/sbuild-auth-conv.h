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

#ifndef SBUILD_AUTH_CONV_H
#define SBUILD_AUTH_CONV_H

#include <sigc++/sigc++.h>

#include <security/pam_appl.h>

#include "sbuild-auth-message.h"

namespace sbuild
{

  class AuthConv
  {
  public:
    typedef std::vector<AuthMessage> message_list;

    AuthConv();
    virtual ~AuthConv();

    virtual time_t get_warning_timeout () = 0;
    virtual void set_warning_timeout (time_t timeout) = 0;

    virtual time_t get_fatal_timeout () = 0;
    virtual void set_fatal_timeout (time_t timeout) = 0;

    bool conversation (message_list& messages);

    sigc::signal<bool, message_list&> signal_conversation;

  protected:
    virtual bool conversation_impl (std::vector<AuthMessage>& messages) = 0;
  };

}

#endif /* SBUILD_AUTH_CONV_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
