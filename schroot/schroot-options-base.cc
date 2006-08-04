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

#include "schroot-options.h"

#include <cstdlib>
#include <iostream>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

using std::endl;
using boost::format;
using sbuild::_;
namespace opt = boost::program_options;
using namespace schroot;

options_base::options_base ():
  schroot_base::options (),
  action(ACTION_SESSION_AUTO),
  chroots(),
  command(),
  user(),
  preserve(false),
  all(false),
  all_chroots(false),
  all_sessions(false),
  session_force(false),
  chroot(_("Chroot selection")),
  chrootenv(_("Chroot environment")),
  session(_("Session management"))
{
}

options_base::~options_base ()
{
}

void
options_base::add_options ()
{
  schroot_base::options::add_options();

  general.add_options()
    ("list,l",
     _("List available chroots"))
    ("info,i",
     _("Show information about selected chroots"))
    ("config",
     _("Dump configuration of selected chroots"));

  chroot.add_options()
    ("chroot,c", opt::value<sbuild::string_list>(&this->chroots),
     _("Use specified chroot"));

  hidden.add_options()
    ("command", opt::value<sbuild::string_list>(&this->command),
     _("Command to run"));

  positional.add("command", -1);
}

void
options_base::add_option_groups ()
{
  schroot_base::options::add_option_groups();

  if (!chroot.options().empty())
    {
      visible.add(chroot);
      global.add(chroot);
    }
  if (!chrootenv.options().empty())
    {
      visible.add(chrootenv);
      global.add(chrootenv);
    }
  if (!session.options().empty())
    {
      visible.add(session);
      global.add(session);
    }
}

void
options_base::check_options ()
{
  schroot_base::options::check_options();

  if (vm.count("help"))
    set_action(ACTION_HELP);
  if (vm.count("version"))
    set_action(ACTION_VERSION);
  if (vm.count("list"))
    set_action(ACTION_LIST);
  if (vm.count("info"))
    set_action(ACTION_INFO);
  if (vm.count("config"))
    set_action(ACTION_CONFIG);
}

void
options_base::check_actions ()
{
  if (this->quiet && this->verbose)
    {
      sbuild::log_warning()
	<< _("--quiet and --verbose may not be used at the same time")
	<< endl;
      sbuild::log_info() << _("Using verbose output") << endl;
    }

  if (!this->chroots.empty() && all_used())
    {
      sbuild::log_warning()
	<< _("--chroot and --all may not be used at the same time")
	<< endl;
      sbuild::log_info() << _("Using --chroots only") << endl;
      this->all = this->all_chroots = this->all_sessions = false;
    }

  /* Determine which chroots to load and use. */
  switch (this->action)
    {
    case ACTION_SESSION_AUTO:
      // Only allow normal chroots
      this->load_chroots = true;
      this->load_sessions = false;
      this->all = this->all_sessions = false;

      // If no chroot was specified, fall back to the "default" chroot.
      if (this->chroots.empty() && all_used() == false)
	this->chroots.push_back("default");

      break;
    case ACTION_SESSION_BEGIN:
      // Only allow one session chroot
      this->load_chroots = true;
      this->load_sessions = false;
      if (this->chroots.size() != 1 || all_used())
	throw opt::validation_error(_("Exactly one chroot must be specified when beginning a session"));

      this->all = this->all_chroots = this->all_sessions = false;
      break;
    case ACTION_SESSION_RECOVER:
    case ACTION_SESSION_RUN:
    case ACTION_SESSION_END:
      // Session operations work on all chroots.
      this->load_chroots = this->load_sessions = true;
      break;
    case ACTION_HELP:
    case ACTION_VERSION:
      // Chroots don't make sense here.
      this->load_chroots = this->load_sessions = false;
      this->all = this->all_chroots = this->all_sessions = false;
      break;
    case ACTION_LIST:
      // If not specified otherwise, load normal chroots, but allow
      // --all options.
      if (!all_used())
	this->load_chroots = true;
      if (this->all_chroots)
	this->load_chroots = true;
      if (this->all_sessions)
	this->load_sessions = true;
      if (!this->chroots.empty())
	throw opt::validation_error(_("--chroot may not be used with --list"));
      break;
    case ACTION_INFO:
    case ACTION_LOCATION:
    case ACTION_CONFIG:
      // If not specified otherwise, load normal chroots, but allow
      // --all options.
      if (!this->chroots.empty()) // chroot specified
	this->load_chroots = this->load_sessions = true;
      else if (!all_used()) // no chroots specified
	{
	  this->all_chroots = true;
	  this->load_chroots = true;
	}
      if (this->all_chroots)
	this->load_chroots = true;
      if (this->all_sessions)
	this->load_sessions = true;
      break;
    default: // Something went wrong
      this->load_chroots = this->load_sessions = false;
      this->all = this->all_chroots = this->all_sessions = false;
      throw opt::validation_error(_("Unknown action specified"));
    }
}

void
options_base::set_action (action_type action)
{
  if (this->action != ACTION_SESSION_AUTO)
    throw opt::validation_error(_("Only one action may be specified"));

  this->action = action;
}
