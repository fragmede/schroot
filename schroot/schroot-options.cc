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

#include <iostream>

#include <stdlib.h>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include "sbuild.h"

#include "schroot-options.h"

using std::endl;
using boost::format;
namespace opt = boost::program_options;
using namespace schroot;

options::options (int   argc,
		  char *argv[]):
  options_base(argc, argv)
{
  add_options();
  parse_options(argc, argv);
  check_options();
  check_actions();
}

options::~options ()
{
}

void
options::add_options ()
{
  options_base::add_options();

  general.add_options()
    ("location",
     _("Print location of selected chroots"));

  chroot.add_options()
    ("all,a",
     _("Select all chroots and active sessions"))
    ("all-chroots",
     _("Select all chroots"))
    ("all-sessions",
     _("Select all active sessions"));

  chrootenv.add_options()
    ("user,u", opt::value<std::string>(&this->user),
     _("Username (default current user)"))
    ("preserve-environment,p",
     _("Preserve user environment"));

  session.add_options()
    ("begin-session,b",
     _("Begin a session; returns a session ID"))
    ("recover-session",
     _("Recover an existing session"))
    ("run-session,r",
     _("Run an existing session"))
    ("end-session,e",
     _("End an existing session"))
    ("force,f",
     _("Force operation, even if it fails"));
}


void
options::check_options ()
{
  if (vm.count("help"))
    {
      std::cout
	<< _("Usage:") << "\n  "
	<< "schroot"
	<< "  "
	<< _("[OPTION...] [COMMAND] - run command or shell in a chroot")
	<< '\n';
      std::cout << visible << std::flush;
      exit(EXIT_SUCCESS);
    }

  options_base::check_options();

  if (vm.count("location"))
    set_action(ACTION_LOCATION);

  if (vm.count("all"))
    this->all = true;
  if (vm.count("all-chroots"))
    this->all_chroots = true;
  if (vm.count("all-sessions"))
    this->all_sessions = true;

  if (vm.count("preserve-environment"))
    this->preserve = true;

  if (vm.count("begin-session"))
    set_action(ACTION_SESSION_BEGIN);
  if (vm.count("recover-session"))
    set_action(ACTION_SESSION_RECOVER);
  if (vm.count("run-session"))
    set_action(ACTION_SESSION_RUN);
  if (vm.count("end-session"))
    set_action(ACTION_SESSION_END);
  if (vm.count("force"))
    this->session_force = true;

  if (this->all == true)
    {
      this->all_chroots = true;
      this->all_sessions = true;
    }
}
