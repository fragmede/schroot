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

#include <schroot/util.h>

#include <schroot/options.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

using std::endl;
using boost::format;
using schroot::_;
namespace opt = boost::program_options;

namespace bin
{
  namespace schroot
  {

    const options::action_type options::ACTION_SESSION_AUTO ("session_auto");
    const options::action_type options::ACTION_SESSION_BEGIN ("session_begin");
    const options::action_type options::ACTION_SESSION_RECOVER ("session_recover");
    const options::action_type options::ACTION_SESSION_RUN ("session_run");
    const options::action_type options::ACTION_SESSION_END ("session_end");
    const options::action_type options::ACTION_LIST ("list");
    const options::action_type options::ACTION_INFO ("info");
    const options::action_type options::ACTION_LOCATION ("location");
    const options::action_type options::ACTION_CONFIG ("config");

    options::options ():
      bin::common::options (),
      chroots(),
      command(),
      directory(),
      shell(),
      user(),
      preserve(false),
      all(false),
      all_chroots(false),
      all_sessions(false),
      all_source_chroots(false),
      exclude_aliases(false),
      session_name(),
      session_force(false),
      useroptions(),
      useroptions_map(),
      chroot(_("Chroot selection")),
      chrootenv(_("Chroot environment")),
      session_actions(_("Session actions")),
      session_options(_("Session options"))
    {
    }

    options::~options ()
    {
    }

    void
    options::add_options ()
    {
      // Chain up to add basic options.
      bin::common::options::add_options();

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

      actions.add_options()
        ("list,l",
         _("List available chroots"))
        ("info,i",
         _("Show information about selected chroots"))
        ("config",
         _("Dump configuration of selected chroots"));

      chroot.add_options()
        ("chroot,c", opt::value<::schroot::string_list>(&this->chroots),
         _("Use specified chroot"))
        ("all,a",
         _("Select all chroots and active sessions"))
        ("all-chroots",
         _("Select all chroots"))
        ("all-sessions",
         _("Select all active sessions"))
        ("all-source-chroots",
         _("Select all source chroots"))
        ("exclude-aliases",
         _("Do not include aliases"));

      chrootenv.add_options()
        ("directory,d", opt::value<std::string>(&this->directory),
         _("Directory to use"))
        ("shell,s", opt::value<std::string>(&this->shell),
         _("Shell to use as login shell"))
        ("user,u", opt::value<std::string>(&this->user),
         _("Username (default current user)"))
        ("preserve-environment,p",
         _("Preserve user environment"))
        ("option,o", opt::value<::schroot::string_list>(&this->useroptions),
         _("Set option"));

      session_actions.add_options()
        ("automatic-session",
         _("Begin, run and end a session automatically (default)"))
        ("begin-session,b",
         _("Begin a session; returns a session ID"))
        ("recover-session",
         _("Recover an existing session"))
        ("run-session,r",
         _("Run an existing session"))
        ("end-session,e",
         _("End an existing session"));

      session_options.add_options()
        ("session-name,n", opt::value<std::string>(&this->session_name),
         _("Session name (defaults to an automatically generated name)"))
        ("force,f",
         _("Force operation, even if it fails"));
      hidden.add_options()
        ("command", opt::value<::schroot::string_list>(&this->command),
         _("Command to run"));

      positional.add("command", -1);

          actions.add_options()
        ("location",
         _("Print location of selected chroots"));
    }

    void
    options::add_option_groups ()
    {
      // Chain up to add basic option groups.
      bin::common::options::add_option_groups();

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
      if (!session_actions.options().empty())
#else
        if (!session_actions.primary_keys().empty())
#endif
          {
            visible.add(session_actions);
            global.add(session_actions);
          }

#ifndef BOOST_PROGRAM_OPTIONS_DESCRIPTION_OLD
      if (!session_options.options().empty())
#else
        if (!session_options.primary_keys().empty())
#endif
          {
            visible.add(session_options);
            global.add(session_options);
          }
    }

    void
    options::check_options ()
    {
      // Chain up to check basic options.
      bin::common::options::check_options();

      if (vm.count("list"))
        this->action = ACTION_LIST;
      if (vm.count("info"))
        this->action = ACTION_INFO;
      if (vm.count("config"))
        this->action = ACTION_CONFIG;
      if (vm.count("location"))
        this->action = ACTION_LOCATION;

      if (vm.count("all"))
        this->all = true;
      if (vm.count("all-chroots"))
        this->all_chroots = true;
      if (vm.count("all-sessions"))
        this->all_sessions = true;
      if (vm.count("all-source-chroots"))
        this->all_source_chroots = true;
      if (vm.count("exclude-aliases"))
        this->exclude_aliases = true;

      if (vm.count("preserve-environment"))
        this->preserve = true;

      if (vm.count("automatic-session"))
        this->action = ACTION_SESSION_AUTO;
      if (vm.count("begin-session"))
        this->action = ACTION_SESSION_BEGIN;
      if (vm.count("recover-session"))
        this->action = ACTION_SESSION_RECOVER;
      if (vm.count("run-session"))
        this->action = ACTION_SESSION_RUN;
      if (vm.count("end-session"))
        this->action = ACTION_SESSION_END;
      if (vm.count("force"))
        this->session_force = true;

      if (this->all == true)
        {
          this->all_chroots = true;
          this->all_sessions = true;
          this->all_source_chroots = true;
        }

      for (const auto& useroption : this->useroptions)
        {
          std::string::size_type sep = useroption.find_first_of('=', 0);
          std::string key = useroption.substr(0,sep);
          ++sep;
          std::string value = useroption.substr(sep);
          this->useroptions_map.insert(std::make_pair(key,value));
        }
    }

    void
    options::check_actions ()
    {
      // Chain up to check basic actions.
      bin::common::options::check_actions();

      if (this->quiet && this->verbose)
        {
          ::schroot::log_warning()
            << _("--quiet and --verbose may not be used at the same time")
            << endl;
          ::schroot::log_info() << _("Using verbose output") << endl;
        }

      if (!this->chroots.empty() && all_used())
        {
          ::schroot::log_warning()
            << _("--chroot and --all may not be used at the same time")
            << endl;
          ::schroot::log_info() << _("Using --chroots only") << endl;
          this->all = this->all_chroots = this->all_source_chroots = this->all_sessions = false;
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
            throw error
              (_("Exactly one chroot must be specified when beginning a session"));

          this->all = this->all_chroots = this->all_source_chroots = this->all_sessions = false;
        }
      else if (this->action == ACTION_SESSION_RECOVER ||
               this->action == ACTION_SESSION_RUN ||
               this->action == ACTION_SESSION_END)
        {
          // Session operations work on all chroots.
          this->load_chroots = this->load_sessions = true;

          if (!this->session_name.empty())
            throw error
              (_("--session-name is not permitted for the specified action; did you mean to use --chroot?"));
        }
      else if (this->action == ACTION_HELP ||
               this->action == ACTION_VERSION)
        {
          // Chroots don't make sense here.
          this->load_chroots = this->load_sessions = false;
          this->all = this->all_chroots = this->all_source_chroots = this->all_sessions = false;
        }
      else if (this->action == ACTION_LIST ||
               this->action == ACTION_INFO ||
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
              if (this->action == ACTION_LIST || this->action == ACTION_INFO)
                this->all_source_chroots = true;
              this->load_chroots = true;
            }
          if (this->all_chroots || this->all_source_chroots)
            this->load_chroots = true;
          if (this->all_sessions)
            this->load_chroots = this->load_sessions = true;
        }
      else
        {
          // Something went wrong
          this->load_chroots = this->load_sessions = false;
          this->all = this->all_chroots = this->all_source_chroots = this->all_sessions = false;
          throw error(_("Unknown action specified"));
        }

      if (!this->session_name.empty() && this->action != ACTION_SESSION_BEGIN)
        throw error
          (_("--session-name is not permitted for the specified action"));

      if (!this->session_name.empty() &&
          !::schroot::is_valid_sessionname(this->session_name))
        throw error(_("Invalid session name"));
    }

  }
}
