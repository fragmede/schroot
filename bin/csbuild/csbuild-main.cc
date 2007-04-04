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

#include <config.h>

#include "csbuild-main.h"

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
using namespace csbuild;

main::main (schroot::options_base::ptr& options):
  main_base("csbuild",
	    // TRANSLATORS: '...' is an ellipsis e.g. U+2026, and '-'
	    // is an em-dash.
	    _("[OPTION...] [COMMAND] - run command or shell in a chroot"),
	    options,
	    true)
{
}

main::~main ()
{
}

void
main::create_session(sbuild::session::operation sess_op)
{
  sbuild::log_debug(sbuild::DEBUG_INFO) << "Creating schroot session" << endl;

  this->session = sbuild::session::ptr
    (new sbuild::session("schroot", this->config, sess_op, this->chroots));

  if (!this->options->user.empty())
    this->session->set_user(this->options->user);
}
