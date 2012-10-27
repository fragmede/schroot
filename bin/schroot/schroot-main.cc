/* Copyright © 2005-2007  Roger Leigh <rleigh@debian.org>
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

#include <sbuild/sbuild-config.h>

#include "schroot-main.h"

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <locale>

#include <termios.h>
#include <unistd.h>

#include <boost/format.hpp>

using std::endl;
using boost::format;
using sbuild::_;
using namespace schroot;

main::main (options_base::ptr& options):
  main_base("schroot",
	    _("[OPTION…] [COMMAND] — run command or shell in a chroot"),
	    options,
	    true)
{
}

main::~main ()
{
}

void
main::action_list ()
{
  // This list is pre-validated.
  for(sbuild::string_list::const_iterator pos = this->chroot_names.begin();
      pos != this->chroot_names.end();
      ++pos)
    std::cout << *pos << '\n';
  std::cout << std::flush;
}

void
main::create_session(sbuild::session::operation sess_op)
{
  sbuild::log_debug(sbuild::DEBUG_INFO) << "Creating schroot session" << endl;

  this->session = sbuild::session::ptr
    (new sbuild::session("schroot", sess_op, this->chroot_objects));
}

void
main::add_session_auth ()
{
  main_base::add_session_auth();

  if (!this->options->user.empty())
    this->session->get_auth()->set_user(this->options->user);
}
