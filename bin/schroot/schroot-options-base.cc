/* Copyright © 2005-2007  Roger Leigh <rleigh@debian.org>
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

const options_base::action_type options_base::ACTION_SESSION_AUTO ("session_auto");
const options_base::action_type options_base::ACTION_SESSION_BEGIN ("session_begin");
const options_base::action_type options_base::ACTION_SESSION_RECOVER ("session_recover");
const options_base::action_type options_base::ACTION_SESSION_RUN ("session_run");
const options_base::action_type options_base::ACTION_SESSION_END ("session_end");
const options_base::action_type options_base::ACTION_LIST ("list");
const options_base::action_type options_base::ACTION_INFO ("info");
const options_base::action_type options_base::ACTION_LOCATION ("location");
const options_base::action_type options_base::ACTION_CONFIG ("config");

options_base::options_base ():
  schroot_base::options (),
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
  // Chain up to add basic options.
  schroot_base::options::add_options();

  action.add(ACTION_SESSION_AUTO);
  action.set_default(ACTION_SESSION_AUTO);
  action.add(ACTION_SESSION_BEGIN);
  action.add(ACTION_SESSION_RECOVER);
  action.add(ACTION_SESSION_RUN);
  action.add(ACTION_SESSION_END);
  action.add(ACTION_LIST);
  action.add(ACTION_INFO);
  action.add(ACTION_LOCATION);
  action.add(ACTION_CONFIG);

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
  // Chain up to add basic option groups.
  schroot_base::options::add_option_groups();

#ifndef BOOST_PROGRAM_OPTIONS_DESCRIPTION_OLD
  if (!chroot.options().empty())
#else
  if (!chroot.primary_keys().empty())
#endif
    {
      visible.add(chroot);
      global.add(chroot);
    }
#ifndef BOOST_PROGRAM_OPTIONS_DESCRIPTION_OLD
  if (!chrootenv.options().empty())
#else
  if (!chrootenv.primary_keys().empty())
#endif
    {
      visible.add(chrootenv);
      global.add(chrootenv);
    }
#ifndef BOOST_PROGRAM_OPTIONS_DESCRIPTION_OLD
  if (!session.options().empty())
#else
  if (!session.primary_keys().empty())
#endif
    {
      visible.add(session);
      global.add(session);
    }
}

void
options_base::check_options ()
{
  // Chain up to check basic options.
  schroot_base::options::check_options();

  if (vm.count("list"))
    this->action = ACTION_LIST;
  if (vm.count("info"))
    this->action = ACTION_INFO;
  if (vm.count("config"))
    this->action = ACTION_CONFIG;
}

void
options_base::check_actions ()
{
  // Chain up to check basic actions.
  schroot_base::options::check_actions();

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
  if (this->action == ACTION_SESSION_AUTO)
    {
      // Only allow normal chroots
      this->load_chroots = true;
      this->load_sessions = false;
      this->all = this->all_sessions = false;

      // If no chroot was specified, fall back to the "default" chroot.
      if (this->chroots.empty() && all_used() == false)
	this->chroots.push_back("default");
    }
  else if (this->action == ACTION_SESSION_BEGIN)
    {
      // Only allow one session chroot
      this->load_chroots = true;
      this->load_sessions = false;
      if (this->chroots.size() != 1 || all_used())
	throw opt::validation_error(_("Exactly one chroot must be specified when beginning a session"));

      this->all = this->all_chroots = this->all_sessions = false;
    }
  else if (this->action == ACTION_SESSION_RECOVER ||
	   this->action == ACTION_SESSION_RUN ||
	   this->action == ACTION_SESSION_AUTO)
    {
      // Session operations work on all chroots.
      this->load_chroots = this->load_sessions = true;
    }
  else if (this->action == ACTION_HELP ||
	   this->action == ACTION_VERSION)
    {
      // Chroots don't make sense here.
      this->load_chroots = this->load_sessions = false;
      this->all = this->all_chroots = this->all_sessions = false;
    }
  else if (this->action == ACTION_LIST)
    {
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
    }
  else if (this->action == ACTION_INFO ||
	   this->action == ACTION_LOCATION ||
	   this->action == ACTION_CONFIG)
    {
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
    }
  else
    {
      // Something went wrong
      this->load_chroots = this->load_sessions = false;
      this->all = this->all_chroots = this->all_sessions = false;
      throw opt::validation_error(_("Unknown action specified"));
    }
}
