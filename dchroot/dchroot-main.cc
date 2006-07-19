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

#include "dchroot-main.h"
#include "dchroot-chroot-config.h"
#include "dchroot-session.h"

#include <cstdlib>
#include <iostream>
#include <locale>

#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#include <boost/format.hpp>

using std::endl;
using boost::format;
using schroot::options_base;
using namespace dchroot;

main::main (schroot::options_base::ptr& options):
  main_base("dchroot",
	    // TRANSLATORS: Please use an ellipsis e.g. U+2026
	    _("[OPTION...] [COMMAND] - run command or shell in a chroot"),
	    options)
{
}

main::~main ()
{
}

void
main::action_location ()
{
  sbuild::string_list chroot;
  chroot.push_back(this->options->chroot_path);
  this->config->print_chroot_location(chroot, std::cout);
}

void
main::load_config ()
{
  check_dchroot_conf();

  if (this->use_dchroot_conf)
    {
      this->config = sbuild::chroot_config::ptr(new dchroot::chroot_config);
      if (this->options->load_chroots == true)
	this->config->add(DCHROOT_CONF, false);
    }
  else
    {
      schroot::main_base::load_config();
    }
}

void
main::create_session (sbuild::session::operation sess_op)
{
  sbuild::log_debug(sbuild::DEBUG_INFO) << "Creating dchroot session" << endl;

  // Using dchroot.conf implies using dchroot_session_base, which does
  // not require user or group access.
  this->session = sbuild::session::ptr
    (new dchroot::session("schroot",
			  this->config,
			  sess_op,
			  this->chroots,
			  this->use_dchroot_conf));
}
