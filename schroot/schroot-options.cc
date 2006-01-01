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

/*
 * schroot_options_new:
 *
 * Parse command-line options.
 *
 * Returns a structure containing the options.
 */
Options::Options(int   argc,
		 char *argv[]):
  action(ACTION_SESSION_AUTO),
  chroots(),
  command(),
  user(),
  preserve(false),
  quiet(false),
  verbose(false),
  all(false),
  all_chroots(false),
  all_sessions(false),
  session_force(false)
{
  opt::options_description general(_("General options"));
  general.add_options()
    ("help,?",
     _("Show help options"))
    ("version,V",
     _("Print version information"))
    ("quiet,q",
     _("Show less output"))
    ("verbose,v",
     _("Show more output"))
    ("list,l",
     _("List available chroots"))
    ("info,i",
     _("Show information about selected chroots"));

  opt::options_description chroot(_("Chroot selection"));
  chroot.add_options()
    ("chroot,c", opt::value<sbuild::string_list>(&this->chroots),
     _("Use specified chroot"))
    ("all,a",
     _("Select all chroots and active sessions"))
    ("all-chroots",
     _("Select all chroots"))
    ("all-sessions",
     _("Select all active sessions"));

  opt::options_description chrootenv(_("Chroot environment"));
  chrootenv.add_options()
    ("user,u", opt::value<std::string>(&this->user),
     _("Username (default current user)"))
    ("preserve-environment,p",
     _("Preserve user environment"));

  opt::options_description session(_("Session management"));
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

  opt::options_description hidden(_("Hidden options"));
  hidden.add_options()
    ("command", opt::value<sbuild::string_list>(&this->command),
     _("Command to run"));

  opt::options_description visible;
  visible.add(general).add(chroot).add(chrootenv).add(session);

  opt::options_description global;
  global.add(general).add(chroot).add(chrootenv).add(session).add(hidden);

  opt::variables_map vm;
  opt::store(opt::parse_command_line(argc, argv, global), vm);
  //  opt::store(opt::parse_command_line(argc, argv, session), vm);
  opt::notify(vm);

  if (vm.count("help"))
    {
      std::cout
	<< _("Usage:") << '\n'
	<< _("  schroot [OPTION...] - run command or shell in a chroot") << '\n'
	<< visible << std::flush;
      exit(EXIT_SUCCESS);
    }

  if (vm.count("version"))
    set_action(ACTION_VERSION);
  if (vm.count("list"))
    set_action(ACTION_LIST);
  if (vm.count("info"))
    set_action(ACTION_INFO);

  if (vm.count("all"))
    this->all = true;
  if (vm.count("all-chroots"))
    this->all_chroots = true;
  if (vm.count("all-sessions"))
    this->all_sessions = true;

  if (vm.count("preserve-environment"))
    this->preserve = true;
  if (vm.count("quiet"))
    this->quiet = true;
  if (vm.count("verbose"))
    this->verbose = true;

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

  if (this->quiet && this->verbose)
    {
      sbuild::log_warning()
	<< _("--quiet and --verbose may not be used at the same time!")
	<< endl;
      sbuild::log_info() << _("Using verbose output.") << endl;
    }

  /* Ensure there's something to list. */
  if ((this->action == ACTION_LIST &&
       (this->all == false && this->all_chroots == false &&
	this->all_sessions == false)) ||
      (this->action == ACTION_INFO == true &&
       (this->all == false && this->all_chroots == false &&
	this->all_sessions == false &&
	(this->chroots.empty()))))
    {
      this->all = true;
    }

  if (this->all == true)
    {
      this->all_chroots = true;
      this->all_sessions = true;
    }

  /* If no chroot was specified, fall back to the "default" chroot. */
  if (this->chroots.empty())
    {
      this->chroots.push_back("default");
    }

  /* Determine which chroots to load. */
  this->load_chroots = this->all_chroots;
  this->load_sessions = this->all_sessions;
  if (this->action == ACTION_LIST &&
      this->chroots.empty())
    this->load_chroots = this->load_sessions = true;
}

Options::~Options()
{
}

void
Options::set_action (action_type action)
{
  if (this->action != ACTION_SESSION_AUTO)
    throw opt::validation_error(_("Only one action may be specified"));

  this->action = action;

}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
