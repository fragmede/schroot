/* Copyright © 2005  Roger Leigh <rleigh@debian.org>
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

#include <cassert>
#include <iostream>
#include <memory>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <syslog.h>

#include <boost/format.hpp>

#include <uuid/uuid.h>

#include "sbuild-i18n.h"
#include "sbuild-chroot-lvm-snapshot.h"
#include "sbuild-log.h"
#include "sbuild-session.h"
#include "sbuild-util.h"

using std::cout;
using std::endl;
using boost::format;
using namespace sbuild;

namespace
{
  /* TODO: move to utils */
  std::string
  string_list_to_string(const string_list& list,
			const std::string&         separator)
  {
    std::string ret;

    for (string_list::const_iterator cur = list.begin();
	 cur != list.end();
	 ++cur)
      {
	ret += *cur;
	if (cur + 1 != list.end())
	  ret += separator;
      }

    return ret;
  }

  char **
  string_list_to_strv(const string_list& str)
  {
    char **ret = new char *[str.size() + 1];

    for (string_list::size_type i = 0;
	 i < str.size();
	 ++i)
      {
	ret[i] = new char[str[i].length() + 1];
	std::strcpy(ret[i], str[i].c_str());
      }
    ret[str.size()] = 0;

    return ret;
  }

  char **
  env_list_to_strv(const env_list& env)
  {
    char **ret = new char *[env.size() + 1];

    for (env_list::size_type i = 0;
	 i < env.size();
	 ++i)
      {
	std::string envitem = env[i].first + "=" + env[i].second;
	ret[i] = new char[envitem.length() + 1];
	std::strcpy(ret[i], envitem.c_str());
      }
    ret[env.size()] = 0;

    return ret;
  }

  void
  strv_delete(char **strv)
  {
    for (char *cur = strv[0]; cur != 0; ++cur)
      delete cur;
    delete[] strv;
  }

  /*
   * is_group_member:
   * @group: the group to check for
   *
   * Check group membership.
   *
   * Returns true if the user is a member of @group, otherwise false.
   */
  static bool
  is_group_member (const std::string& group)
  {
    errno = 0;
    struct group *groupbuf = getgrnam(group.c_str());
    if (groupbuf == NULL)
      {
	if (errno == 0)
	  log_error() << format(_("%1%: group not found")) % group << endl;
	else
	  log_error() << format(_("%1%: group not found: %2%"))
	    % group % strerror(errno)
		      << endl;
	exit (EXIT_FAILURE);
      }

    bool group_member = false;
    if (groupbuf->gr_gid == getgid())
      {
	group_member = true;
      }
    else
      {
	int supp_group_count = getgroups(0, NULL);
	if (supp_group_count < 0)
	  {
	    log_error() << format(_("can't get supplementary group count: %1%"))
	      % strerror(errno)
			<< endl;
	    exit (EXIT_FAILURE);
	  }
	if (supp_group_count > 0)
	  {
	    gid_t *supp_groups = new gid_t[supp_group_count];
	    if (getgroups(supp_group_count, supp_groups) < 1)
	      {
		log_error() << format(_("can't get supplementary groups: %1%"))
		  % strerror(errno)
			    << endl;
		exit (EXIT_FAILURE);
	      }

	    for (int i = 0; i < supp_group_count; ++i)
	      {
		if (groupbuf->gr_gid == supp_groups[i])
		  group_member = true;
	      }
	    delete[] supp_groups;
	  }
      }

    return group_member;
  }

}

Session::Session (const std::string& service,
		  config_ptr&        config,
		  Operation          operation,
		  string_list        chroots):
  Auth(service),
  config(config),
  chroots(chroots),
  child_status(0),
  operation(operation),
  session_id(),
  force(false)
{
}

Session::~Session()
{
}

Session::config_ptr&
Session::get_config ()
{
  return this->config;
}

void
Session::set_config (config_ptr& config)
{
  this->config = config;
}

const string_list&
Session::get_chroots () const
{
  return this->chroots;
}

void
Session::set_chroots (const string_list& chroots)
{
  this->chroots = chroots;
}

Session::Operation
Session::get_operation () const
{
  return this->operation;
}

void
Session::set_operation (Operation operation)
{
  this->operation = operation;
}

const std::string&
Session::get_session_id () const
{
  return this->session_id;
}

void
Session::set_session_id (const std::string& session_id)
{
  this->session_id = session_id;
}

bool
Session::get_force () const
{
  return this->force;
}

void
Session::set_force (bool force)
{
  this->force = force;
}

int
Session::get_child_status () const
{
  return this->child_status;
}

Auth::Status
Session::get_auth_status () const
{
  assert(!this->chroots.empty());
  if (this->config.get() == 0) return Auth::STATUS_FAIL;

  Auth::Status status = Auth::STATUS_NONE;

  /* TODO set difference. */
  for (string_list::const_iterator cur = this->chroots.begin();
       cur != this->chroots.end();
       ++cur)
    {
      const Chroot *chroot = this->config->find_alias(*cur);
      if (chroot == NULL) // Should never happen, but cater for it anyway.
	{
	  log_warning() << format(_("No chroot found matching alias '%1%'"))
	    % *cur
			<< endl;
	  status = change_auth(status, Auth::STATUS_FAIL);
	}

      const string_list& groups = chroot->get_groups();
      const string_list& root_groups = chroot->get_root_groups();

      if (!groups.empty())
	{
	  bool in_groups = false;
	  bool in_root_groups = false;

	  if (!groups.empty())
	    {
	      for (string_list::const_iterator gp = groups.begin();
		   gp != groups.end();
		   ++gp)
		if (is_group_member(*gp))
		  in_groups = true;
	    }

	  if (!root_groups.empty())
	    {
	      for (string_list::const_iterator gp = root_groups.begin();
		   gp != root_groups.end();
		   ++gp)
		if (is_group_member(*gp))
		  in_root_groups = true;
	    }

	  /*
	   * No auth required if in root groups and changing to root,
	   * or if the uid is not changing.  If not in a group,
	   * authentication fails immediately.
	   */
	  if (in_groups == true &&
	      ((this->get_uid() == 0 && in_root_groups == true) ||
	       (this->get_ruid() == this->get_uid())))
	    {
	      status = change_auth(status, Auth::STATUS_NONE);
	    }
	  else if (in_groups == true) // Auth required if not in root group
	    {
	      status = change_auth(status, Auth::STATUS_USER);
	    }
	  else // Not in any groups
	    {
	      status = change_auth(status, Auth::STATUS_FAIL);
	    }
	}
      else // No available groups entries means no access to anyone
	{
	  status = change_auth(status, Auth::STATUS_FAIL);
	}
    }

  return status;
}

void
Session::run_impl ()
{
  assert(this->config.get() != NULL);
  assert(!this->chroots.empty());

try
  {
    for (string_list::const_iterator cur = this->chroots.begin();
	 cur != this->chroots.end();
	 ++cur)
      {
	log_debug(DEBUG_NOTICE)
	  << format("Running session in %1% chroot:") % *cur
	  << endl;
	const Chroot *ch = this->config->find_alias(*cur);
	if (chroot == NULL) // Should never happen, but cater for it anyway.
	  {
	    format fmt(_("%1%: Failed to find chroot"));
	    fmt % *cur;
	    throw error(fmt);
	  }
	else
	  {
	    std::auto_ptr<Chroot> chroot(ch->clone());

	    /* If restoring a session, set the session ID from the
	       chroot name, or else generate it.  Only chroots which
	       support session creation append a UUID to the session
	       ID. */
	    if (chroot->get_active() ||
		!(chroot->get_session_flags() & Chroot::SESSION_CREATE))
	      {
		set_session_id(chroot->get_name());
	      }
	    else
	      {
		uuid_t uuid;
		char uuid_str[37];
		uuid_generate(uuid);
		uuid_unparse(uuid, uuid_str);
		uuid_clear(uuid);
		std::string session_id(chroot->get_name() + "-" + uuid_str);
		set_session_id(session_id);
	      }

	    /* Activate chroot. */
	    chroot->set_active(true);

	    /* If a chroot mount location has not yet been set, set one
	       with the session id. */
	    if (chroot->get_mount_location().empty())
	      {
		std::string location(std::string(SCHROOT_MOUNT_DIR) + "/" +
				     this->session_id);
		chroot->set_mount_location(location);
	      }

	    /* Chroot types which create a session (e.g. LVM devices)
	       need the chroot name respecifying. */
	    if (chroot->get_session_flags() & Chroot::SESSION_CREATE)
	      chroot->set_name(this->session_id);

	    /* LVM devices need the snapshot device name specifying. */
	    ChrootLvmSnapshot *snapshot = 0;
	    if ((snapshot = dynamic_cast<ChrootLvmSnapshot *>(chroot.get())) != 0)
	      {
		std::string dir(dirname(snapshot->get_device(), '/'));
		std::string device(dir + "/" + this->session_id);
		snapshot->set_snapshot_device(device);
	      }

	    try
	      {
		/* Run setup-start chroot setup scripts. */
		setup_chroot(*chroot, Chroot::SETUP_START);
		if (this->operation == OPERATION_BEGIN)
		  cout << this->session_id << endl;

		/* Run recover scripts. */
		setup_chroot(*chroot, Chroot::SETUP_RECOVER);

		try
		  {
		    /* Run run-start scripts. */
		    setup_chroot(*chroot, Chroot::RUN_START);

		    /* Run session if setup succeeded. */
		    run_chroot(*chroot);

		    /* Run run-stop scripts whether or not there was an
		       error. */
		    setup_chroot(*chroot, Chroot::RUN_STOP);
		  }
		catch (const error& e)
		  {
		    setup_chroot(*chroot, Chroot::RUN_STOP);
		    throw e;
		  }

	    /* Run setup-stop chroot setup scripts whether or not there
	       was an error. */
		setup_chroot(*chroot, Chroot::SETUP_STOP);
		chroot->set_active(false);
	      }
	    catch (const error& e)
	      {
		try
		  {
		    setup_chroot(*chroot, Chroot::SETUP_STOP);
		  }
		catch (const error& discard)
		  {
		  }
		chroot->set_active(false);
		throw e;
	      }

	    /* Deactivate chroot. */
	    chroot->set_active(false);
	  }
      }
  }
catch (const error& e)
  {
    /* If a command was not run, but something failed, the exit
       status still needs setting. */
    if (this->child_status == 0)
      this->child_status = EXIT_FAILURE;
    throw e;
  }
}

void
Session::setup_chroot (Chroot&           session_chroot,
		       Chroot::SetupType setup_type)
{
  assert(!session_chroot.get_name().empty());

  if (!((this->operation == OPERATION_BEGIN &&
	 setup_type == Chroot::SETUP_START) ||
	(this->operation == OPERATION_RECOVER &&
	 setup_type == Chroot::SETUP_RECOVER) ||
	(this->operation == OPERATION_END &&
	 setup_type == Chroot::SETUP_STOP) ||
	(this->operation == OPERATION_RUN &&
	 (setup_type == Chroot::RUN_START ||
	  setup_type == Chroot::RUN_STOP)) ||
	(this->operation == OPERATION_AUTOMATIC &&
	 (setup_type == Chroot::SETUP_START ||
	  setup_type == Chroot::SETUP_STOP  ||
	  setup_type == Chroot::SETUP_STOP  ||
	  setup_type == Chroot::SETUP_STOP))))
    return;

  if (((setup_type == Chroot::SETUP_START   ||
	setup_type == Chroot::SETUP_RECOVER ||
	setup_type == Chroot::SETUP_STOP) &&
       session_chroot.get_run_setup_scripts() == false) ||
      ((setup_type == Chroot::RUN_START ||
	setup_type == Chroot::RUN_STOP) &&
       session_chroot.get_run_setup_scripts() == false))
    return;

  try
    {
      session_chroot.setup_lock(setup_type, true);
    }
  catch (const Chroot::error &e)
    {
      format fmt(_("Chroot setup failed to lock chroot: %1%"));
      fmt % e.what();
      throw error(fmt);
    }

  std::string setup_type_string;
  if (setup_type == Chroot::SETUP_START)
    setup_type_string = "setup-start";
  else if (setup_type == Chroot::SETUP_RECOVER)
    setup_type_string = "setup-recover";
  else if (setup_type == Chroot::SETUP_STOP)
    setup_type_string = "setup-stop";
  else if (setup_type == Chroot::RUN_START)
    setup_type_string = "run-start";
  else if (setup_type == Chroot::RUN_STOP)
    setup_type_string = "run-stop";

  string_list arg_list;
  arg_list.push_back(RUN_PARTS); // Run run-parts(8)
  if (get_verbosity() == Auth::VERBOSITY_VERBOSE)
    arg_list.push_back("--verbose");
  arg_list.push_back("--lsbsysinit");
  arg_list.push_back("--exit-on-error");
  if (setup_type == Chroot::SETUP_STOP ||
      setup_type == Chroot::RUN_STOP)
    arg_list.push_back("--reverse");
  format arg_fmt("--arg=%1%");
  arg_fmt % setup_type_string;
  arg_list.push_back(arg_fmt.str());
  if (setup_type == Chroot::SETUP_START ||
      setup_type == Chroot::SETUP_RECOVER ||
      setup_type == Chroot::SETUP_STOP)
    arg_list.push_back(SCHROOT_CONF_SETUP_D); // Setup directory
  else
    arg_list.push_back(SCHROOT_CONF_RUN_D); // Run directory

  /* Get a complete list of environment variables to set.  We need to
     query the chroot here, since this can vary depending upon the
     chroot type. */
  env_list env;
  session_chroot.setup_env(env);
  setup_env_var(env, "AUTH_USER",
		get_user());
  {
    const char *verbosity = NULL;
    switch (get_verbosity())
      {
      case Auth::VERBOSITY_QUIET:
	verbosity = "quiet";
	break;
      case Auth::VERBOSITY_NORMAL:
	verbosity = "normal";
	break;
      case Auth::VERBOSITY_VERBOSE:
	verbosity = "verbose";
	break;
      default:
	log_debug(DEBUG_CRITICAL) << format(_("Invalid verbosity level: %1%, falling back to \"normal\""))
	  % static_cast<int>(get_verbosity())
		     << endl;
	verbosity = "normal";
	break;
      }
    setup_env_var(env, "AUTH_VERBOSITY", verbosity);
  }

  setup_env_var(env, "MOUNT_DIR", SCHROOT_MOUNT_DIR);
  setup_env_var(env, "LIBEXEC_DIR", SCHROOT_LIBEXEC_DIR);
  setup_env_var(env, "PID", getpid());
  setup_env_var(env, "SESSION_ID", this->session_id);

  int exit_status = 0;
  pid_t pid;

  if ((pid = fork()) == -1)
    {
      format fmt(_("Failed to fork child: %1%"));
      fmt % strerror(errno);
      throw error(fmt);
    }
  else if (pid == 0)
    {
      chdir("/");
      /* This is required to ensure the scripts run with uid=0 and gid=0,
	 otherwise setuid programs such as mount(8) will fail.  This
	 should always succeed, because our euid=0 and egid=0.*/
      setuid(0);
      setgid(0);
      initgroups("root", 0);
      if (exec (arg_list[0], arg_list, env))
	{
	  log_error() << format(_("Could not exec \"%1%\": %2%"))
	    % arg_list[0] % strerror(errno)
		      << endl;
	  exit (EXIT_FAILURE);
	}
      exit (EXIT_FAILURE); /* Should never be reached. */
    }
  else
    {
      wait_for_child(pid, exit_status);
    }

  try
    {
      session_chroot.setup_lock(setup_type, false);
    }
  catch (const Chroot::error &e)
    {
      format fmt(_("Chroot setup failed to unlock chroot: %1%"));
      fmt % e.what();
      throw error(fmt);
    }

  if (exit_status != 0)
    {
      format fmt(_("Chroot setup failed during chroot \"%1%\" stage"));
      fmt % setup_type_string;
      throw error(fmt);
    }
}

void
Session::run_child (Chroot& session_chroot)
{
  assert(!session_chroot.get_name().empty());
  assert(!session_chroot.get_mount_location().empty());

  assert(!get_user().empty());
  assert(!get_shell().empty());
  assert(Auth::pam != NULL); // PAM must be initialised

  const std::string& location = session_chroot.get_mount_location();
  std::string cwd;
  {
    char *raw_cwd = getcwd (NULL, 0);
    if (raw_cwd)
      cwd = raw_cwd;
    else
      cwd = "/";
    free(raw_cwd);
  }
  /* Child errors result in immediate exit().  Errors are not
     propagated back via an exception, because there is no longer any
     higher-level handler to catch them. */
  try
    {
      open_session();
    }
  catch (const Auth::error& e)
    {
      log_error() << format(_("PAM error: %1%")) % e.what()
		  << endl;
      exit (EXIT_FAILURE);
    }

  /* Set group ID and supplementary groups */
  if (setgid (get_gid()))
    {
      log_error() << format(_("Could not set gid to '%1%'")) % get_gid()
		  << endl;
      exit (EXIT_FAILURE);
    }
  if (initgroups (get_user().c_str(), get_gid()))
    {
      log_error() << _("Could not set supplementary group IDs") << endl;
      exit (EXIT_FAILURE);
    }

  /* Enter the chroot */
  if (chdir (location.c_str()))
    {
      log_error() << format(_("Could not chdir to '%1%': %2%"))
	% location % strerror(errno)
		  << endl;
      exit (EXIT_FAILURE);
    }
  if (chroot (location.c_str()))
    {
      log_error() << format(_("Could not chroot to '%1%': %2%"))
	% location % strerror(errno)
		  << endl;
      exit (EXIT_FAILURE);
    }

  /* Set uid and check we are not still root */
  if (setuid (get_uid()))
    {
      log_error() << format(_("Could not set uid to '%1%'")) % get_uid()
		  << endl;
      exit (EXIT_FAILURE);
    }
  if (!setuid (0) && get_uid())
    {
      log_error() << _("Failed to drop root permissions.")
		  << endl;
      exit (EXIT_FAILURE);
    }

  /* chdir to current directory */
  if (chdir (cwd.c_str()))
    {
      log_error() << format(_("warning: Could not chdir to '%1%': %2%"))
	% cwd % strerror(errno)
		 << endl;
    }

  /* Set up environment */
  env_list env = get_pam_environment();
  for (env_list::const_iterator cur = env.begin();
       cur != env.end();
       ++cur)
    log_debug(DEBUG_INFO)
      << format("Set environment: %1%=%2%") % cur->first % cur->second
      << endl;

  /* Run login shell */
  std::string file;

  string_list command = get_command();
  if (command.empty() ||
      command[0].empty()) // No command
    {
      assert (!get_shell().empty());

      file = get_shell();
      if (get_environment().empty()) // Not keeping environment; login shell
	{
	  std::string shellbase = basename(get_shell(), '/');
	  std::string loginshell = "-" + shellbase;
	  command.push_back(loginshell);
	  log_debug(DEBUG_INFO)
	    << format("Login shell: %1%") % command[1] << endl;
	}
      else
	{
	  command.push_back(get_shell());
	}

      if (get_environment().empty())
	{
	  log_debug(DEBUG_NOTICE)
	    << format("Running login shell: %1%") % get_shell() << endl;
	  syslog(LOG_USER|LOG_NOTICE, "[%s chroot] (%s->%s) Running login shell: \"%s\"",
		 session_chroot.get_name().c_str(), get_ruser().c_str(), get_user().c_str(), get_shell().c_str());
	}
      else
	{
	  log_debug(DEBUG_NOTICE)
	    << format("Running shell: %1%") % get_shell() << endl;
	  syslog(LOG_USER|LOG_NOTICE, "[%s chroot] (%s->%s) Running shell: \"%s\"",
		 session_chroot.get_name().c_str(), get_ruser().c_str(), get_user().c_str(), get_shell().c_str());
	}

      if (get_verbosity() != Auth::VERBOSITY_QUIET)
	{
	  if (get_ruid() == get_uid())
	    log_info()
	      << format(_(get_environment().empty() ?
			  "[%1% chroot] Running login shell: \"%2%\"" :
			  "[%1% chroot] Running shell: \"%2%\""))
	      % session_chroot.get_name() % get_shell()
	      << endl;
	  else
	    log_info()
	      << format(_(get_environment().empty() ?
			  "[%1% chroot] (%2%->%3%) Running login shell: \"%4%\"" :
			  "[%1% chroot] (%2%->%3%) Running shell: \"%4%\""))
	      % session_chroot.get_name()
	      % get_ruser() % get_user()
	      % get_shell()
	      << endl;
	}
    }
  else
    {
      /* Search for program in path. */
      file = find_program_in_path(command[0]);
      if (file.empty())
	file = command[0];
      std::string commandstring = string_list_to_string(command, " ");
      log_debug(DEBUG_NOTICE)
	<< format("Running command: %1%") % commandstring << endl;
      syslog(LOG_USER|LOG_NOTICE, "[%s chroot] (%s->%s) Running command: \"%s\"",
	     session_chroot.get_name().c_str(), get_ruser().c_str(), get_user().c_str(), commandstring.c_str());
      if (get_verbosity() != Auth::VERBOSITY_QUIET)
	{
	  if (get_ruid() == get_uid())
	    log_info() << format(_("[%1% chroot] Running command: \"%2%\""))
	      % session_chroot.get_name() % commandstring
		       << endl;
	  else
	    log_info() << format(_("[%1% chroot] (%2%->%3%) Running command: \"%4%\""))
	      % session_chroot.get_name()
	      % get_ruser() % get_user()
	      % commandstring
		       << endl;
	}
    }

  /* Execute */
  if (exec (file, command, env))
    {
      log_error() << format(_("Could not exec \"%1%\": %2%"))
	% command[0] % strerror(errno)
		  << endl;
      exit (EXIT_FAILURE);
    }
  /* This should never be reached */
  exit(EXIT_FAILURE);
}

void
Session::wait_for_child (int  pid,
			 int& child_status)
{
  this->child_status = EXIT_FAILURE; // Default exit status

  int status;
  if (wait(&status) != pid)
    {
      format fmt(_("wait for child failed: %1%"));
      fmt % strerror(errno);
      throw error(fmt);
    }

  try
    {
      close_session();
    }
  catch (const Auth::error& e)
    {
      throw error(e.what());
    }

  if (!WIFEXITED(status))
    {
      if (WIFSIGNALED(status))
	{
	  format fmt(_("Child terminated by signal %1%"));
	  fmt % strsignal(WTERMSIG(status));
	  throw error(fmt);
	}
      else if (WCOREDUMP(status))
	throw error(_("Child dumped core"));
      else
	throw error(_("Child exited abnormally (reason unknown; not a signal or core dump)"));
    }

  child_status = WEXITSTATUS(status);

  if (child_status)
    {
      format fmt(_("Child exited abnormally with status '%1%'"));
      fmt % child_status;
      throw error(fmt);
    }
}

void
Session::run_chroot (Chroot&   session_chroot)
{
  assert(!session_chroot.get_name().empty());

  pid_t pid;
  if ((pid = fork()) == -1)
    {
      format fmt(_("Failed to fork child: %1%"));
      fmt % strerror(errno);
      throw error(fmt);
    }
  else if (pid == 0)
    {
      run_child(session_chroot);
      exit (EXIT_FAILURE); /* Should never be reached. */
    }
  else
    {
      wait_for_child(pid, this->child_status);
    }
}

int
Session::exec (const std::string& file,
	       const string_list& command,
	       const env_list&    env)
{
  char **argv = string_list_to_strv(command);
  char **envp = env_list_to_strv(env);
  int status;

  if ((status = execve(file.c_str(), argv, envp)) != 0)
    {
      strv_delete(argv);
      strv_delete(envp);
    }

  return status;
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
