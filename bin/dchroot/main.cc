/* Copyright © 2005-2013  Roger Leigh <rleigh@codelibre.net>
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

#include <dchroot/main.h>
#include <dchroot/session.h>

#include <cstdlib>
#include <iostream>
#include <locale>

#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#include <boost/format.hpp>

using std::endl;
using schroot::_;
using boost::format;

namespace bin
{
  namespace dchroot
  {

    main::main (schroot_common::options::ptr& options):
      dchroot_common::main("dchroot",
                           // TRANSLATORS: '...' is an ellipsis e.g. U+2026, and '-'
                           // is an em-dash.
                           _("[OPTION…] [COMMAND] — run command or shell in a chroot"),
                           options)
    {
    }

    main::~main ()
    {
    }

    void
    main::create_session (schroot::session::operation sess_op)
    {
      schroot::log_debug(schroot::DEBUG_INFO) << "Creating dchroot session" << endl;

      this->session = schroot::session::ptr
        (new dchroot::session("schroot",
                              sess_op,
                              this->chroot_objects));
    }

  }
}
