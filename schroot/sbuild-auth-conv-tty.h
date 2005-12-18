/* sbuild-auth-conv-tty - sbuild auth terminal conversation object
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

#ifndef SBUILD_AUTH_CONV_TTY_H
#define SBUILD_AUTH_CONV_TTY_H

#include <sys/types.h>
#include <sys/wait.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#include <sigc++/sigc++.h>

#include <security/pam_appl.h>
#include <security/pam_misc.h>

#include <glib.h>
#include <glib/gprintf.h>

#include "sbuild-auth-conv.h"
#include "sbuild-config.h"

namespace sbuild
{

  class AuthConvTty : public AuthConv, sigc::trackable
  {
  public:
    AuthConvTty();
    virtual ~AuthConvTty();

    virtual time_t get_warning_timeout ();
    virtual void set_warning_timeout (time_t timeout);

    virtual time_t get_fatal_timeout ();
    virtual void set_fatal_timeout (time_t timeout);

  protected:
    virtual bool conversation_impl (std::vector<AuthMessage>& messages);

  private:
    int get_delay ();
    std::string * read_string (std::string message,
			       bool        echo);

    time_t  warning_timeout;
    time_t  fatal_timeout;
    time_t  start_time;
  };

}

#endif /* SBUILD_AUTH_CONV_TTY_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
