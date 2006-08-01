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

#include "sbuild-chroot-plain.h"
#include "sbuild-chroot-lvm-snapshot.h"
#include "sbuild-run-parts.h"
#include "sbuild-session.h"
#include "sbuild-util.h"

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <syslog.h>

#include <boost/format.hpp>

#include <uuid/uuid.h>

using std::cout;
using std::endl;
using boost::format;
using namespace sbuild;

namespace
{

  typedef std::pair<sbuild::session::error_code,const char *> emap;

  /**
   * This is a list of the supported error codes.  It's used to
   * construct the real error codes map.
   */
  emap init_errors[] =
    {
      // TRANSLATORS: %1% = directory
      emap(session::CHDIR,          N_("Failed to change to directory '%1%'")),
      // TRANSLATORS: %4% = directory
      emap(session::CHDIR_FB,       N_("Falling back to directory '%4%'")),
      emap(session::CHILD_CORE,     N_("Child dumped core")),
      emap(session::CHILD_FAIL,     N_("Child exited abnormally (reason unknown; not a signal or core dump)")),
      emap(session::CHILD_FORK,     N_("Failed to fork child")),
      // TRANSLATORS: %4% = signal name
      emap(session::CHILD_SIGNAL,   N_("Child terminated by signal '%4%'")),
      emap(session::CHILD_WAIT,     N_("Wait for child failed")),
      // TRANSLATORS: %1% = directory
      emap(session::CHROOT,         N_("Failed to change root to directory '%1%'")),
      // TRANSLATORS: %1% = chroot name
      emap(session::CHROOT_ALIAS,   N_("No chroot found matching name or alias '%1%'")),
      emap(session::CHROOT_LOCK,    N_("Failed to lock chroot")),
      emap(session::CHROOT_SETUP,   N_("Chroot setup failed")),
      // TRANSLATORS: %1% = chroot name
      emap(session::CHROOT_UNKNOWN, N_("Failed to find chroot '%1%'")),
      emap(session::CHROOT_UNLOCK,  N_("Failed to unlock chroot")),
      // TRANSLATORS: %1% = command
      emap(session::COMMAND_ABS,    N_("Command \"%1%\" must have an absolute path")),
      // TRANSLATORS: %1% = command
      emap(session::EXEC,           N_("Failed to execute \"%1%\"")),
      // TRANSLATORS: A supplementary group is the list of additional
      // system groups a user belongs to, in addition to their default
      // group.
      emap(session::GROUP_GET_SUP,  N_("Failed to get supplementary groups")),
      // TRANSLATORS: A supplementary group is the list of additional
      // system groups a user belongs to, in addition to their default
      // group.
      emap(session::GROUP_GET_SUPC, N_("Failed to get supplementary group count")),
      // TRANSLATORS: %1% = integer group ID
      emap(session::GROUP_SET,      N_("Failed to set group '%1%'")),
      emap(session::GROUP_SET_SUP,  N_("Failed to set supplementary groups")),
      // TRANSLATORS: %1% = group name
      emap(session::GROUP_UNKNOWN,  N_("Group '%1%' not found")),
      emap(session::PAM,            N_("PAM error")),
      emap(session::ROOT_DROP,      N_("Failed to drop root permissions")),
      // TRANSLATORS: %1% = command
      emap(session::SHELL,          N_("Shell '%1%' not available")),
      // TRANSLATORS: %4% = command
      emap(session::SHELL_FB,       N_("Falling back to shell '%4%'")),
      // TRANSLATORS: %4% = signal name
      emap(session::SIGNAL_CATCH,   N_("Caught signal '%4%'")),
      // TRANSLATORS: %4% = signal name
      emap(session::SIGNAL_SET,     N_("Failed to set signal handler '%4%'")),
      // TRANSLATORS: %1% = integer user ID
      emap(session::USER_SET,       N_("Failed to set user '%1%'")),
      // TRANSLATORS: %1% = user name
      // TRANSLATORS: %2% = user name
      // TRANSLATORS: Please translate "->" as a right arrow, e.g. U+2192
      emap(session::USER_SWITCH,    N_("(%1%->%2%): User switching is not permitted")),
    };

  /**
   * Get the current working directory.  If it can't be found, fall
   * back to root.
   *
   * @returns the current working directory.
   */
  std::string
  getcwd ()
  {
    std::string cwd;

    char *raw_cwd = ::getcwd (0, 0);
    if (raw_cwd)
      cwd = raw_cwd;
    else
      cwd = "/";
    free(raw_cwd);

    return cwd;
  }

  /**
   * Check group membership.
   *
   * @param group the group to check for.
   * @returns true if the user is a member of group, otherwise false.
   */
  bool
  is_group_member (std::string const& group)
  {
    errno = 0;
    struct group *groupbuf = getgrnam(group.c_str());
    if (groupbuf == 0)
      {
	if (errno == 0)
	  {
	    session::error e(group, session::GROUP_UNKNOWN);
	    log_exception_warning(e);
	  }
	else
	  {
	    session::error e(group, session::GROUP_UNKNOWN, strerror(errno));
	    log_exception_warning(e);
	  }
	return false;
      }

    bool group_member = false;
    if (groupbuf->gr_gid == getgid())
      {
	group_member = true;
      }
    else
      {
	int supp_group_count = getgroups(0, 0);
	if (supp_group_count < 0)
	  throw session::error(session::GROUP_GET_SUPC, strerror(errno));
	if (supp_group_count > 0)
	  {
	    gid_t *supp_groups = new gid_t[supp_group_count];
	    assert (supp_groups);
	    if (getgroups(supp_group_count, supp_groups) < 1)
	      throw session::error(session::GROUP_GET_SUP, strerror(errno));

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

  volatile bool sighup_called = false;
  volatile bool sigterm_called = false;

  /**
   * Handle the SIGHUP signal.
   *
   * @param ignore the signal number.
   */
  void
  sighup_handler (int ignore)
  {
    /* This exists so that system calls get interrupted. */
    sighup_called = true;
  }

  /**
   * Handle the SIGTERM signal.
   *
   * @param ignore the signal number.
   */
  void
  sigterm_handler (int ignore)
  {
    /* This exists so that system calls get interrupted. */
    sigterm_called = true;
  }

#ifdef SBUILD_DEBUG
  volatile bool child_wait = true;
#endif

}

template<>
error<session::error_code>::map_type
error<session::error_code>::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));

session::session (std::string const&         service,
		  config_ptr&                config,
		  operation                  operation,
		  sbuild::string_list const& chroots):
  auth(service),
  config(config),
  chroots(chroots),
  chroot_status(true),
  child_status(0),
  session_operation(operation),
  session_id(),
  force(false),
  saved_sighup_signal(),
  saved_sigterm_signal(),
  saved_termios(),
  termios_ok(false),
  cwd(getcwd())
{
}

session::~session ()
{
}

session::config_ptr const&
session::get_config () const
{
  return this->config;
}

void
session::set_config (config_ptr& config)
{
  this->config = config;
}

string_list const&
session::get_chroots () const
{
  return this->chroots;
}

void
session::set_chroots (string_list const& chroots)
{
  this->chroots = chroots;
}

session::operation
session::get_operation () const
{
  return this->session_operation;
}

void
session::set_operation (operation operation)
{
  this->session_operation = operation;
}

std::string const&
session::get_session_id () const
{
  return this->session_id;
}

void
session::set_session_id (std::string const& session_id)
{
  this->session_id = session_id;
}

bool
session::get_force () const
{
  return this->force;
}

void
session::set_force (bool force)
{
  this->force = force;
}

void
session::save_termios ()
{
  int ctty = open("/dev/tty", O_RDONLY|O_NOCTTY);
  string_list const& command(auth::get_command());

  this->termios_ok = false;

  // Save if running a login shell and have a controlling terminal.
  if (ctty >= 0 &&
      (command.empty() || command[0].empty()))
    {
      if (tcgetattr(ctty, &this->saved_termios) < 0)
	{
	  sbuild::log_warning()
	    << _("Error saving terminal settings")
	    << endl;
	}
      else
	this->termios_ok = true;
    }

  if (ctty >= 0 && close(ctty))
    log_debug(DEBUG_WARNING) << "Failed to close CTTY fd " << ctty << endl;
}

void
session::restore_termios ()
{
  int ctty = open("/dev/tty", O_WRONLY|O_NOCTTY);
  string_list const& command(auth::get_command());

  // Restore if running a login shell and have a controlling terminal,
  // and have previously saved the terminal state.
  if (ctty >= 0 &&
      (command.empty() || command[0].empty()) &&
      termios_ok)
    {
      if (tcsetattr(ctty, TCSANOW, &this->saved_termios) < 0)
	sbuild::log_warning()
	  << _("Error restoring terminal settings")
	  << endl;
    }

  if (ctty >= 0 && close(ctty))
    log_debug(DEBUG_WARNING) << "Failed to close CTTY fd " << ctty << endl;
}

int
session::get_child_status () const
{
  return this->child_status;
}

auth::status
session::get_chroot_auth_status (auth::status status,
				 chroot::ptr const& chroot) const
{
  string_list const& users = chroot->get_users();
  string_list const& root_users = chroot->get_root_users();
  string_list const& groups = chroot->get_groups();
  string_list const& root_groups = chroot->get_root_groups();

  bool in_users = false;
  bool in_root_users = false;
  bool in_groups = false;
  bool in_root_groups = false;

  sbuild::string_list::const_iterator upos =
    find(users.begin(), users.end(), get_ruser());
  if (upos != users.end())
    in_users = true;

  sbuild::string_list::const_iterator rupos =
    find(root_users.begin(), root_users.end(), get_ruser());
  if (rupos != root_users.end())
    in_root_users = true;

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

  log_debug(DEBUG_INFO)
    << "In users: " << in_users << endl
    << "In groups: " << in_groups << endl
    << "In root-users: " << in_root_users << endl
    << "In root-groups: " << in_root_groups << endl;

  /*
   * No auth required if in root users or root groups and
   * changing to root, or if the uid is not changing.  If not
   * in user or group, authentication fails immediately.
   */
  if ((in_users == true || in_groups == true ||
       in_root_users == true || in_root_groups == true) &&
      this->get_ruid() == this->get_uid())
    {
      status = change_auth(status, auth::STATUS_NONE);
    }
  else if ((in_root_users == true || in_root_groups == true) &&
	   this->get_uid() == 0)
    {
      status = change_auth(status, auth::STATUS_NONE);
    }
  else if (in_users == true || in_groups == true)
    // Auth required if not in root group
    {
      status = change_auth(status, auth::STATUS_USER);
    }
  else // Not in any groups
    {
      if (this->get_ruid() == 0)
	status = change_auth(status, auth::STATUS_USER);
      else
	status = change_auth(status, auth::STATUS_FAIL);
    }

  return status;
}

auth::status
session::get_auth_status () const
{
  assert(!this->chroots.empty());
  if (this->config.get() == 0) return auth::STATUS_FAIL;

  /*
   * Note that the root user can't escape authentication.  This is
   * because pam_rootok.so should be used in the PAM configuration if
   * root should automatically be granted access.  The only exception
   * is that the root group doesn't need to be added to the groups or
   * root groups lists.
   */

  auth::status status = auth::STATUS_NONE;

  /* @todo Use set difference rather than iteration and
     is_group_member. */
  for (string_list::const_iterator cur = this->chroots.begin();
       cur != this->chroots.end();
       ++cur)
    {
      const chroot::ptr chroot = this->config->find_alias(*cur);
      if (!chroot) // Should never happen, but cater for it anyway.
	{
	  error e(*cur, CHROOT_ALIAS);
	  log_exception_warning(e);
	  status = change_auth(status, auth::STATUS_FAIL);
	}

      status = change_auth(status, get_chroot_auth_status(status, chroot));
    }

  return status;
}

void
session::run_impl ()
{
  assert(this->config.get() != 0);
  assert(!this->chroots.empty());

  try
    {
      sighup_called = false;
      set_sighup_handler();
      sigterm_called = false;
      set_sigterm_handler();

      for (string_list::const_iterator cur = this->chroots.begin();
	   cur != this->chroots.end();
	   ++cur)
	{
	  log_debug(DEBUG_NOTICE)
	    << format("Running session in %1% chroot:") % *cur
	    << endl;

	  const chroot::ptr ch = this->config->find_alias(*cur);
	  if (!ch) // Should never happen, but cater for it anyway.
	    throw error(*cur, CHROOT_UNKNOWN);

	  chroot::ptr chroot(ch->clone());

	  /* If restoring a session, set the session ID from the
	     chroot name, or else generate it.  Only chroots which
	     support session creation append a UUID to the session
	     ID. */
	  if (chroot->get_active() ||
	      !(chroot->get_session_flags() & chroot::SESSION_CREATE))
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

	  log_debug(DEBUG_INFO)
	    << format("Session ID: %1%") % get_session_id() << endl;

	  /* Activate chroot. */
	  chroot->set_active(true);

	  /* If a chroot mount location has not yet been set, and the
	     chroot is not a plain chroot, set a mount location with the
	     session id. */
	  {
	    chroot_plain *plain = dynamic_cast<chroot_plain *>(chroot.get());
	    if (chroot->get_mount_location().empty() &&
		(plain == 0 || plain->get_run_setup_scripts() == true))
	      {
		std::string location(std::string(SCHROOT_MOUNT_DIR) + "/" +
				     this->session_id);
		chroot->set_mount_location(location);
	      }
	  }

	  log_debug(DEBUG_NOTICE)
	    << format("Mount Location: %1%") % chroot->get_mount_location()
	    << endl;

	  /* Chroot types which create a session (e.g. LVM devices)
	     need the chroot name respecifying. */
	  if (chroot->get_session_flags() & chroot::SESSION_CREATE)
	    {
	      chroot->set_name(this->session_id);
	      chroot->set_aliases(string_list());
	    }

	  /* LVM devices need the snapshot device name specifying. */
	  chroot_lvm_snapshot *snapshot = 0;
	  if ((snapshot = dynamic_cast<chroot_lvm_snapshot *>(chroot.get())) != 0)
	    {
	      std::string dir(dirname(snapshot->get_device(), '/'));
	      std::string device(dir + "/" + this->session_id);
	      snapshot->set_snapshot_device(device);
	    }

	  try
	    {
	      /* Run setup-start chroot setup scripts. */
	      setup_chroot(chroot, chroot::SETUP_START);
	      if (this->session_operation == OPERATION_BEGIN)
		cout << this->session_id << endl;

	      /* Run recover scripts. */
	      setup_chroot(chroot, chroot::SETUP_RECOVER);

	      try
		{
		  /* Run exec-start scripts. */
		  setup_chroot(chroot, chroot::EXEC_START);

		  /* Run session if setup succeeded. */
		  if (this->session_operation == OPERATION_AUTOMATIC ||
		      this->session_operation == OPERATION_RUN)
		    {
		      try
			{
			  open_session();
			  save_termios();
			  run_chroot(chroot);
			}
		      catch (std::runtime_error const& e)
			{
			  log_debug(DEBUG_WARNING)
			    << "Chroot session failed" << endl;
			  restore_termios();
			  close_session();
			  throw;
			}
		      restore_termios();
		      close_session();
		    }

		}
	      catch (error const& e)
		{
		  log_debug(DEBUG_WARNING)
		    << "Chroot exec scripts or session failed" << endl;
		  setup_chroot(chroot, chroot::EXEC_STOP);
		  throw;
		}

	      /* Run exec-stop scripts whether or not there was an
		 error. */
	      setup_chroot(chroot, chroot::EXEC_STOP);
	    }
	  catch (error const& e)
	    {
	      log_debug(DEBUG_WARNING)
		<< "Chroot setup scripts, exec scripts or session failed" << endl;
	      try
		{
		  setup_chroot(chroot, chroot::SETUP_STOP);
		}
	      catch (error const& discard)
		{
		  log_debug(DEBUG_WARNING)
		    << "Chroot setup scripts failed during stop" << endl;
		}
	      chroot->set_active(false);
	      throw;
	    }

	  /* Run setup-stop chroot setup scripts whether or not there
	     was an error. */
	  setup_chroot(chroot, chroot::SETUP_STOP);

	  /* Deactivate chroot. */
	  chroot->set_active(false);
	}
    }
  catch (error const& e)
    {
      clear_sigterm_handler();
      clear_sighup_handler();

      /* If a command was not run, but something failed, the exit
	 status still needs setting. */
      if (this->child_status == 0)
	this->child_status = EXIT_FAILURE;
      throw;
    }

  clear_sigterm_handler();
  clear_sighup_handler();
}

string_list
session::get_login_directories () const
{
  string_list ret;

  // Set current working directory.
  ret.push_back(this->cwd);

  // Set $HOME.
  environment env = get_pam_environment();
  std::string home;
  if (env.get("HOME", home) &&
      std::find(ret.begin(), ret.end(), home) == ret.end())
    ret.push_back(home);

  // Set passwd home.
  if (std::find(ret.begin(), ret.end(), get_home()) == ret.end())
    ret.push_back(get_home());

  // Final fallback to root.
  if (std::find(ret.begin(), ret.end(), "/") == ret.end())
  ret.push_back("/");

  return ret;
}

string_list
session::get_command_directories () const
{
  string_list ret;

  // Set current working directory.
  ret.push_back(this->cwd);

  return ret;
}

std::string
session::get_shell () const
{
  assert (!auth::get_shell().empty());
  std::string shell = auth::get_shell();

  struct stat statbuf;
  if (stat(shell.c_str(), &statbuf) < 0)
    {
      if (shell != "/bin/sh")
	{
	  error e1(shell, SHELL, strerror(errno));
	  log_exception_warning(e1);
	  shell = "/bin/sh";
	  error e2(SHELL_FB, shell);
	  log_exception_warning(e2);
	}
    }

  return shell;
}

void
session::get_command (sbuild::chroot::ptr& session_chroot,
		      std::string&         file,
		      string_list&         command) const
{
  /* Run login shell */
  if (command.empty() ||
      command[0].empty()) // No command
    get_login_command(session_chroot, file, command);
  else
    get_user_command(session_chroot, file, command);
}

void
session::get_login_command (sbuild::chroot::ptr& session_chroot,
			    std::string&         file,
			    string_list&         command) const
{
  command.clear();

  std::string shell = get_shell();
  file = shell;

  if (get_environment().empty() &&
      session_chroot->get_command_prefix().empty())
    // Not keeping environment and can setup argv correctly; login shell
    {
      std::string shellbase = basename(shell, '/');
      std::string loginshell = "-" + shellbase;
      command.push_back(loginshell);

      log_debug(DEBUG_NOTICE)
	<< format("Running login shell: %1%") % shell << endl;
      syslog(LOG_USER|LOG_NOTICE,
	     "[%s chroot] (%s->%s) Running login shell: '%s'",
	     session_chroot->get_name().c_str(),
	     get_ruser().c_str(), get_user().c_str(),
	     shell.c_str());
    }
  else
    {
      command.push_back(shell);
      log_debug(DEBUG_NOTICE)
	<< format("Running shell: %1%") % shell << endl;
      syslog(LOG_USER|LOG_NOTICE,
	     "[%s chroot] (%s->%s) Running shell: '%s'",
	     session_chroot->get_name().c_str(),
	     get_ruser().c_str(), get_user().c_str(),
	     shell.c_str());
    }

  if (get_verbosity() != auth::VERBOSITY_QUIET)
    {
      std::string format_string;
      if (get_ruid() == get_uid())
	{
	  if (get_environment().empty() &&
	      session_chroot->get_command_prefix().empty())
	    // TRANSLATORS: %1% = chroot name
	    // TRANSLATORS: %4% = command
	    format_string = _("[%1% chroot] Running login shell: '%4%'");
	  else
	    // TRANSLATORS: %1% = chroot name
	    // TRANSLATORS: %4% = command
	    format_string = _("[%1% chroot] Running shell: '%4%'");
	}
      else
	{
	  if (get_environment().empty() &&
	      session_chroot->get_command_prefix().empty())
	    // TRANSLATORS: %1% = chroot name
	    // TRANSLATORS: %2% = user name
	    // TRANSLATORS: %3% = user name
	    // TRANSLATORS: %4% = command
	    // TRANSLATORS: Please translate "->" as a right arrow, e.g. U+2192
	    format_string = _("[%1% chroot] (%2%->%3%) Running login shell: '%4%'");
	  else
	    // TRANSLATORS: %1% = chroot name
	    // TRANSLATORS: %2% = user name
	    // TRANSLATORS: %3% = user name
	    // TRANSLATORS: %4% = command
	    // TRANSLATORS: Please translate "->" as a right arrow, e.g. U+2192
	    format_string = _("[%1% chroot] (%2%->%3%) Running shell: '%4%'");
	}

      format fmt(format_string);
      fmt % session_chroot->get_name()
	% get_ruser() % get_user()
	% shell;
      log_info() << fmt << endl;
    }
}

void
session::get_user_command (sbuild::chroot::ptr& session_chroot,
			   std::string&         file,
			   string_list&         command) const
{
  /* Search for program in path. */
  environment env = get_pam_environment();
  std::string path;
  if (!env.get("PATH", path))
    path.clear();

  file = find_program_in_path(command[0], path, "");
  if (file.empty())
    file = command[0];
  std::string commandstring = string_list_to_string(command, " ");
  log_debug(DEBUG_NOTICE)
    << format("Running command: %1%") % commandstring << endl;
  syslog(LOG_USER|LOG_NOTICE, "[%s chroot] (%s->%s) Running command: \"%s\"",
	 session_chroot->get_name().c_str(), get_ruser().c_str(), get_user().c_str(), commandstring.c_str());

  if (get_verbosity() != auth::VERBOSITY_QUIET)
    {
      std::string format_string;
      if (get_ruid() == get_uid())
	// TRANSLATORS: %1% = chroot name
	// TRANSLATORS: %4% = command
	format_string = _("[%1% chroot] Running command: \"%4%\"");
      else
	// TRANSLATORS: %1% = chroot name
	// TRANSLATORS: %2% = user name
	// TRANSLATORS: %3% = user name
	// TRANSLATORS: %4% = command
	// TRANSLATORS: Please translate "->" as a right arrow, e.g. U+2192
	format_string = (_("[%1% chroot] (%2%->%3%) Running command: \"%4%\""));

      format fmt(format_string);
      fmt % session_chroot->get_name()
	% get_ruser() % get_user()
	% commandstring;
      log_info() << fmt << endl;
    }
}

void
session::setup_chroot (sbuild::chroot::ptr&       session_chroot,
		       sbuild::chroot::setup_type setup_type)
{
  assert(!session_chroot->get_name().empty());

  if (!((this->session_operation == OPERATION_BEGIN &&
	 setup_type == chroot::SETUP_START) ||
	(this->session_operation == OPERATION_RECOVER &&
	 setup_type == chroot::SETUP_RECOVER) ||
	(this->session_operation == OPERATION_END &&
	 setup_type == chroot::SETUP_STOP) ||
	(this->session_operation == OPERATION_RUN &&
	 (setup_type == chroot::EXEC_START ||
	  setup_type == chroot::EXEC_STOP)) ||
	(this->session_operation == OPERATION_AUTOMATIC &&
	 (setup_type == chroot::SETUP_START ||
	  setup_type == chroot::SETUP_STOP  ||
	  setup_type == chroot::EXEC_START   ||
	  setup_type == chroot::EXEC_STOP))))
    return;

  if (((setup_type == chroot::SETUP_START   ||
	setup_type == chroot::SETUP_RECOVER ||
	setup_type == chroot::SETUP_STOP) &&
       session_chroot->get_run_setup_scripts() == false) ||
      ((setup_type == chroot::EXEC_START ||
	setup_type == chroot::EXEC_STOP) &&
       session_chroot->get_run_exec_scripts() == false))
    return;

  if (setup_type == chroot::SETUP_START)
    this->chroot_status = true;

  try
    {
      session_chroot->lock(setup_type);
    }
  catch (chroot::error const& e)
    {
      this->chroot_status = false;
      try
	{
	  // Release lock, which also removes session metadata.
	  session_chroot->unlock(setup_type, 0);
	}
      catch (chroot::error const& ignore)
	{
	}
      throw error(session_chroot->get_name(), CHROOT_LOCK, e);
    }

  std::string setup_type_string;
  if (setup_type == chroot::SETUP_START)
    setup_type_string = "setup-start";
  else if (setup_type == chroot::SETUP_RECOVER)
    setup_type_string = "setup-recover";
  else if (setup_type == chroot::SETUP_STOP)
    setup_type_string = "setup-stop";
  else if (setup_type == chroot::EXEC_START)
    setup_type_string = "exec-start";
  else if (setup_type == chroot::EXEC_STOP)
    setup_type_string = "exec-stop";

  std::string chroot_status_string;
  if (this->chroot_status)
    chroot_status_string = "ok";
  else
    chroot_status_string = "fail";

  string_list arg_list;
  arg_list.push_back(setup_type_string);
  arg_list.push_back(chroot_status_string);

  /* Get a complete list of environment variables to set.  We need to
     query the chroot here, since this can vary depending upon the
     chroot type. */
  environment env;
  session_chroot->setup_env(env);
  env.add("AUTH_USER", get_user());
  {
    const char *verbosity = 0;
    switch (get_verbosity())
      {
      case auth::VERBOSITY_QUIET:
	verbosity = "quiet";
	break;
      case auth::VERBOSITY_NORMAL:
	verbosity = "normal";
	break;
      case auth::VERBOSITY_VERBOSE:
	verbosity = "verbose";
	break;
      default:
	log_debug(DEBUG_CRITICAL) << format("Invalid verbosity level: %1%, falling back to 'normal'")
	  % static_cast<int>(get_verbosity())
		     << endl;
	verbosity = "normal";
	break;
      }
    env.add("AUTH_VERBOSITY", verbosity);
  }

  env.add("MOUNT_DIR", SCHROOT_MOUNT_DIR);
  env.add("LIBEXEC_DIR", SCHROOT_LIBEXEC_DIR);
  env.add("PID", getpid());
  env.add("SESSION_ID", this->session_id);

  run_parts rp((setup_type == chroot::SETUP_START ||
		setup_type == chroot::SETUP_RECOVER ||
		setup_type == chroot::SETUP_STOP)
	       ? SCHROOT_CONF_SETUP_D // Setup directory
	       : SCHROOT_CONF_EXEC_D, // Execution directory
	       true, true, 022);
  rp.set_reverse((setup_type == chroot::SETUP_STOP ||
		  setup_type == chroot::EXEC_STOP));
  rp.set_verbose(get_verbosity() == auth::VERBOSITY_VERBOSE);

  log_debug(DEBUG_INFO) << rp << std::endl;

  int exit_status = 0;
  pid_t pid;

  if ((pid = fork()) == -1)
    {
      this->chroot_status = false;
      throw error(session_chroot->get_name(), CHILD_FORK, strerror(errno));
    }
  else if (pid == 0)
    {
      try
	{
	  // The setup scripts don't use our syslog fd.
	  closelog();

	  chdir("/");
	  /* This is required to ensure the scripts run with uid=0 and gid=0,
	     otherwise setuid programs such as mount(8) will fail.  This
	     should always succeed, because our euid=0 and egid=0.*/
	  setuid(0);
	  setgid(0);
	  initgroups("root", 0);

	  int status = rp.run(arg_list, env);

	  _exit (status);
	}
      catch (std::exception const& e)
	{
	  sbuild::log_exception_error(e);
	}
      catch (...)
	{
	  sbuild::log_error()
	    << _("An unknown exception occurred") << std::endl;
	}
      _exit(EXIT_FAILURE);
    }
  else
    {
      wait_for_child(pid, exit_status);
    }

  try
    {
      session_chroot->unlock(setup_type, exit_status);
    }
  catch (chroot::error const& e)
    {
      this->chroot_status = false;
      throw error(session_chroot->get_name(), CHROOT_UNLOCK, e);
    }

  if (exit_status != 0)
    {
      this->chroot_status = false;

      format fmt(_("stage=%1%"));
      fmt % setup_type_string;
      throw error(session_chroot->get_name(), CHROOT_SETUP, fmt.str());
    }
}

void
session::run_child (sbuild::chroot::ptr& session_chroot)
{
  assert(!session_chroot->get_name().empty());

  assert(!get_user().empty());
  assert(!get_shell().empty());
  assert(auth::pam != 0); // PAM must be initialised

  // Store before chroot call.
  this->cwd = getcwd();
  log_debug(DEBUG_INFO) << "CWD=" << this->cwd << std::endl;

  std::string location(session_chroot->get_path());
  log_debug(DEBUG_INFO) << "location=" << location << std::endl;

  /* Set group ID and supplementary groups */
  if (setgid (get_gid()))
    throw error(get_gid(), GROUP_SET, strerror(errno));
  log_debug(DEBUG_NOTICE) << "Set GID=" << get_gid() << std::endl;
  if (initgroups (get_user().c_str(), get_gid()))
    throw error(GROUP_SET_SUP, strerror(errno));
  log_debug(DEBUG_NOTICE) << "Set supplementary groups" << std::endl;

  /* Set the process execution domain. */
  /* Will throw on failure. */
  session_chroot->get_persona().set();
  log_debug(DEBUG_NOTICE) << "Set personality="
			  << session_chroot->get_persona()<< std::endl;

  /* Enter the chroot */
  if (chdir (location.c_str()))
    throw error(location, CHDIR, strerror(errno));
  log_debug(DEBUG_NOTICE) << "Changed directory to " << location << std::endl;
  if (::chroot (location.c_str()))
    throw error(location, CHROOT, strerror(errno));
  log_debug(DEBUG_NOTICE) << "Changed root to " << location << std::endl;

  /* Set uid and check we are not still root */
  if (setuid (get_uid()))
    throw error(get_uid(), USER_SET, strerror(errno));
  log_debug(DEBUG_NOTICE) << "Set UID=" << get_uid() << std::endl;
  if (!setuid (0) && get_uid())
    throw error(ROOT_DROP);
  if (get_uid())
    log_debug(DEBUG_NOTICE) << "Dropped root privileges" << std::endl;

  std::string file;
  string_list command(auth::get_command());

  string_list dlist;
  if (command.empty() ||
      command[0].empty()) // No command
    dlist = get_login_directories();
  else
    dlist = get_command_directories();
  log_debug(DEBUG_INFO)
    << format("Directory fallbacks: %1%") % string_list_to_string(dlist, ", ") << endl;

  /* Attempt to chdir to current directory. */
  for (string_list::const_iterator dpos = dlist.begin();
       dpos != dlist.end();
       ++dpos)
    {
      if (chdir ((*dpos).c_str()) < 0)
	{
	  error e(*dpos, CHDIR, strerror(errno));

	  if (dpos + 1 == dlist.end())
	    throw e;
	  else
	    log_exception_warning(e);
	}
      else
	{
	  log_debug(DEBUG_NOTICE) << "Changed directory to "
				  << *dpos << std::endl;
	  if (dpos != dlist.begin())
	    {
	      error e(CHDIR_FB, *dpos);
	      log_exception_warning(e);
	    }
	  break;
	}
    }

  /* Fix up the command for exec. */
  get_command(session_chroot, file, command);
  log_debug(DEBUG_NOTICE) << "command="
			  << string_list_to_string(command, ", ")
			  << std::endl;

  /* Set up environment */
  environment env;
  env.set_filter(session_chroot->get_environment_filter());
  env += get_pam_environment();

  log_debug(DEBUG_INFO) << "Set environment:\n" << env;

  // The user's command does not use our syslog fd.
  closelog();

  // Add command prefix.
  string_list full_command(session_chroot->get_command_prefix());
  if (full_command.size() > 0)
    file = full_command[0];
  for (string_list::const_iterator pos = command.begin();
       pos != command.end();
       ++pos)
    full_command.push_back(*pos);

  /* Execute */
  if (exec (file, full_command, env))
    throw error(file, EXEC, strerror(errno));

  /* This should never be reached */
  _exit(EXIT_FAILURE);
}

void
session::wait_for_child (pid_t pid,
			 int&  child_status)
{
  child_status = EXIT_FAILURE; // Default exit status

  int status;
  bool child_killed = false;

  while (1)
    {
      if ((sighup_called || sigterm_called) && !child_killed)
	{
	  if (sighup_called)
	    {
	      error e(SIGNAL_CATCH, strsignal(SIGHUP),
		      _("terminating immediately"));
	      log_exception_error(e);
	      kill(pid, SIGHUP);
	    }
	  else // SIGTERM
	    {
	      error e(SIGNAL_CATCH, strsignal(SIGTERM),
		      _("terminating immediately"));
	      log_exception_error(e);
	      kill(pid, SIGTERM);
	    }
	  this->chroot_status = false;
	  child_killed = true;
	}

      if (wait(&status) != pid)
	{
	  if (errno == EINTR && (sighup_called || sigterm_called))
	    continue; // Kill child and wait again.
	  else
	    throw error(CHILD_WAIT, strerror(errno));
	}
      else if (sighup_called)
	{
	  sighup_called = false;
	  throw error(SIGNAL_CATCH, strsignal(SIGHUP));
	}
      else if (sigterm_called)
	{
	  sigterm_called = false;
	  throw error(SIGNAL_CATCH, strsignal(SIGTERM));
	}
      else
	break;
    }

  if (!WIFEXITED(status))
    {
      if (WIFSIGNALED(status))
	throw error(CHILD_SIGNAL, strsignal(WTERMSIG(status)));
      else if (WCOREDUMP(status))
	throw error(CHILD_CORE);
      else
	throw error(CHILD_FAIL);
    }

  child_status = WEXITSTATUS(status);
}

void
session::run_chroot (sbuild::chroot::ptr& session_chroot)
{
  assert(!session_chroot->get_name().empty());

  pid_t pid;
  if ((pid = fork()) == -1)
    {
      throw error(CHILD_FORK, strerror(errno));
    }
  else if (pid == 0)
    {
#ifdef SBUILD_DEBUG
      while (child_wait)
	;
#endif
      try
	{
	  run_child(session_chroot);
	}
      catch (std::runtime_error const& e)
	{
	  log_exception_error(e);
	}
      catch (...)
	{
	  sbuild::log_error()
	    << _("An unknown exception occurred") << std::endl;
	}
      _exit (EXIT_FAILURE);
    }
  else
    {
      wait_for_child(pid, this->child_status);
    }
}

void
session::set_sighup_handler ()
{
  set_signal_handler(SIGHUP, &this->saved_sighup_signal, sighup_handler);
}

void
session::clear_sighup_handler ()
{
  clear_signal_handler(SIGHUP, &this->saved_sighup_signal);
}

void
session::set_sigterm_handler ()
{
  set_signal_handler(SIGTERM, &this->saved_sigterm_signal, sigterm_handler);
}

void
session::clear_sigterm_handler ()
{
  clear_signal_handler(SIGTERM, &this->saved_sigterm_signal);
}

void
session::set_signal_handler (int                signal,
			     struct sigaction  *saved_signal,
			     void             (*handler)(int))
{
  struct sigaction new_sa;
  sigemptyset(&new_sa.sa_mask);
  new_sa.sa_flags = 0;
  new_sa.sa_handler = handler;

  if (sigaction(signal, &new_sa, saved_signal) != 0)
    throw error(SIGNAL_SET, strsignal(signal), strerror(errno));
}

void
session::clear_signal_handler (int               signal,
			       struct sigaction *saved_signal)
{
  /* Restore original handler */
  sigaction (signal, saved_signal, 0);
}
