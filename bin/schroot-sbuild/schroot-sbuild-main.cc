/* Copyright © 2005-2007  Roger Leigh <rleigh@codelibre.net>
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

#include "schroot-sbuild-main.h"
#include "schroot-sbuild-session.h"

#include <cstdlib>
#include <iostream>
#include <locale>

#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#include <boost/format.hpp>

using std::endl;
using sbuild::_;
using boost::format;
using schroot::options_base;
using namespace schroot_sbuild;

main::main (schroot::options_base::ptr& options):
  schroot::main(options)
{
}

main::~main ()
{
}

void
main::add_session_auth ()
{
  schroot::main::add_session_auth();

  if (this->session->is_group_member("sbuild"))
    this->session->get_auth()->set_ruser(std::string("sbuild"));
}
