/* sbuild-session - sbuild session object
 *
 * Copyright © 2005  Roger Leigh <rleigh@debian.org>
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

/**
 * SECTION:sbuild-session
 * @short_description: session object
 * @title: Session
 *
 * This object provides the session handling for schroot.  It derives
 * from Auth, which performs all the necessary PAM actions,
 * specialising it by overriding its virtual functions.  This allows
 * more sophisticated handling of user authorisation (groups and
 * root-groups membership in the configuration file) and session
 * management (setting up the session, entering the chroot and running
 * the requested commands or shell).
 */

#include <config.h>

#include <memory>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <syslog.h>

#include <glib.h>
#include <glib/gi18n.h>

#include <uuid/uuid.h>

#include "sbuild-session.h"
#include "sbuild-chroot-lvm-snapshot.h"

using namespace sbuild;

namespace
{
  /* TODO: move to utils */
  std::string
  string_list_to_string(const Chroot::string_list& list,
			const std::string&         separator)
  {
    std::string ret;

    for (Chroot::string_list::const_iterator cur = list.begin();
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
  string_list_to_strv(const Auth::string_list& str)
  {
    char **ret = new char *[str.size() + 1];

    for (Auth::string_list::size_type i = 0;
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
  env_list_to_strv(const Auth::env_list& env)
  {
    char **ret = new char *[env.size() + 1];

    for (Auth::env_list::size_type i = 0;
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

  /**
   * is_group_member:
   * @group: the group to check for
   *
   * Check group membership.
   *
   * Returns TRUE if the user is a member of @group, otherwise FALSE.
   */
  static bool
  is_group_member (const std::string& group)
  {
    errno = 0;
    struct group *groupbuf = getgrnam(group.c_str());
    if (groupbuf == NULL)
      {
	if (errno == 0)
	  g_printerr(_("%s: group not found\n"), group.c_str());
	else
	  g_printerr(_("%s: group not found: %s\n"), group.c_str(), g_strerror(errno));
	exit (EXIT_FAILURE);
      }

    bool group_member = FALSE;
    if (groupbuf->gr_gid == getgid())
      {
	group_member = TRUE;
      }
    else
      {
	int supp_group_count = getgroups(0, NULL);
	if (supp_group_count < 0)
	  {
	    g_printerr(_("can't get supplementary group count: %s\n"), g_strerror(errno));
	    exit (EXIT_FAILURE);
	  }
	if (supp_group_count > 0)
	  {
	    gid_t supp_groups[supp_group_count];
	    if (getgroups(supp_group_count, supp_groups) < 1)
	      {
		g_printerr(_("can't get supplementary groups: %s\n"), g_strerror(errno));
		exit (EXIT_FAILURE);
	      }

	    for (int i = 0; i < supp_group_count; ++i)
	      {
		if (groupbuf->gr_gid == supp_groups[i])
		  group_member = TRUE;
	      }
	  }
      }

    return group_member;
  }

}

/**
 * sbuild_session_error_quark:
 *
 * Get the ERROR domain number.
 *
 * Returns the domain.
 */
GQuark
sbuild_session_error_quark (void)
{
  static GQuark error_quark = 0;

  if (error_quark == 0)
    error_quark = g_quark_from_static_string ("sbuild-session-error-quark");

  return error_quark;
}

Session::Session (const std::string&            service,
		  std::tr1::shared_ptr<Config>& config,
		  Operation                     operation,
		  string_list                   chroots):
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

/**
 * sbuild_session_get_config:
 * @session: an #Session
 *
 * Get the configuration associated with @session.
 *
 * Returns an #Config.
 */
std::tr1::shared_ptr<Config>&
Session::get_config ()
{
  return this->config;
}

/**
 * sbuild_session_set_config:
 * @session: an #Session
 * @config: an #Config
 *
 * Set the configuration associated with @session.
 */
void
Session::set_config (std::tr1::shared_ptr<Config>& config)
{
  this->config = config;
}

/**
 * sbuild_session_get_chroots:
 * @session: an #Session
 *
 * Get the chroots to use in @session.
 *
 * Returns a string vector. This string vector points to internally
 * allocated storage in the chroot and must not be freed, modified or
 * stored.
 */
const Session::string_list&
Session::get_chroots () const
{
  return this->chroots;
}

/**
 * sbuild_session_set_chroots:
 * @session: an #Session
 * @chroots: the chroots to use
 *
 * Set the chroots to use in @session.
 */
void
Session::set_chroots (const Session::string_list& chroots)
{
  this->chroots = chroots;
}

/**
 * sbuild_session_get_operation:
 * @session: an #Session
 *
 * Get the operation @session will perform.
 *
 * Returns an #Config.
 */
Session::Operation
Session::get_operation () const
{
  return this->operation;
}

/**
 * sbuild_session_set_operation:
 * @session: an #Session
 * @operation: an #Operation
 *
 * Set the operation @session will perform.
 */
void
Session::set_operation (Operation operation)
{
  this->operation = operation;
}

/**
 * sbuild_session_get_session_id:
 * @session: an #Session
 *
 * Get the session identifier.  The session identifier is a unique
 * string to identify a session.
 *
 * Returns a string.  The string must be freed by the caller.
 */
const std::string&
Session::get_session_id () const
{
  return this->session_id;
}

/**
 * sbuild_session_set_session_id:
 * @session: an #Session
 * @session_id: an string containing a valid session id
 *
 * Set the session identifier for @session.
 */
void
Session::set_session_id (const std::string& session_id)
{
  this->session_id = session_id;
}

/**
 * sbuild_session_get_force:
 * @session: an #Session
 *
 * Get the force status of @session.
 *
 * Returns TRUE if operation will be forced, otherwise FALSE.
 */
bool
Session::get_force () const
{
  return this->force;
}

/**
 * sbuild_session_set_force:
 * @session: an #Session
 * @force: TRUE to force session operation, otherwise FALSE
 *
 * Set the force status of @session.
 */
void
Session::set_force (bool force)
{
  this->force = force;
}

/**
 * sbuild_session_get_child_status:
 * @session: an #Session
 *
 * Get the exit (wait) status of the last child process to run in this
 * session.
 *
 * Returns the exit status.
 */
int
Session::get_child_status () const
{
  return this->child_status;
}

/**
 * sbuild_session_require_auth:
 * @session: an #Session
 *
 * Check if authentication is required for @session.  Group membership
 * is checked for all chroots, and depending on which user will be run
 * in the chroot, password authentication or no authentication may be
 * required.
 *
 * Returns the authentication type.
 */
Auth::Status
Session::get_auth_status () const
{
  g_return_val_if_fail(!this->chroots.empty(), Auth::STATUS_FAIL);
  g_return_val_if_fail(this->config.get() != NULL, Auth::STATUS_FAIL);

  Auth::Status status = Auth::STATUS_NONE;

  /* TODO set difference. */
  for (string_list::const_iterator cur = this->chroots.begin();
       cur != this->chroots.end();
       ++cur)
    {
      const Chroot *chroot = this->config->find_alias(*cur);
      if (chroot == NULL) // Should never happen, but cater for it anyway.
	{
	  g_warning(_("No chroot found matching alias '%s'"), cur->c_str());
	  status = change_auth(status, Auth::STATUS_FAIL);
	}

      const Chroot::string_list& groups = chroot->get_groups();
      const Chroot::string_list& root_groups = chroot->get_root_groups();

      if (!groups.empty())
	{
	  bool in_groups = FALSE;
	  bool in_root_groups = FALSE;

	  if (!groups.empty())
	    {
	      for (Chroot::string_list::const_iterator gp = groups.begin();
		   gp != groups.end();
		   ++gp)
		if (is_group_member(*gp))
		  in_groups = TRUE;
	    }

	  if (!root_groups.empty())
	    {
	      for (Chroot::string_list::const_iterator gp = root_groups.begin();
		   gp != root_groups.end();
		   ++gp)
		if (is_group_member(*gp))
		  in_root_groups = TRUE;
	    }

	  /*
	   * No auth required if in root groups and changing to root,
	   * or if the uid is not changing.  If not in a group,
	   * authentication fails immediately.
	   */
	  if (in_groups == TRUE &&
	      ((this->get_uid() == 0 && in_root_groups == TRUE) ||
	       (this->get_ruid() == this->get_uid())))
	    {
	      status = change_auth(status, Auth::STATUS_NONE);
	    }
	  else if (in_groups == TRUE) // Auth required if not in root group
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

/**
 * sbuild_session_run:
 * @session: an #Session
 *
 * Run a session.  If a command has been specified, this will be run
 * in each of the specified chroots.  If no command has been
 * specified, a login shell will run in the specified chroot.
 *
 * If required, the user may be required to authenticate themselves.
 * This usually occurs when running as a different user.  The user
 * must be a member of the appropriate groups in order to satisfy the
 * groups and root-groups requirements in the chroot configuration.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
void
Session::run_impl ()
{
  g_return_if_fail(this->config.get() != NULL);
  g_return_if_fail(!this->chroots.empty());

try
  {
    for (Chroot::string_list::const_iterator cur = this->chroots.begin();
	 cur != this->chroots.end();
	 ++cur)
      {
	g_debug("Running session in %s chroot:\n", cur->c_str());
	const Chroot *ch = this->config->find_alias(*cur);
	if (chroot == NULL) // Should never happen, but cater for it anyway.
	  {
	    throw error(*cur + _("Failed to find chroot"),
			ERROR_CHROOT);
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
		char *session_id = g_strconcat(chroot->get_name().c_str(), "-", uuid_str, NULL);
		set_session_id(session_id);
		g_free(session_id);
	      }

	    /* Activate chroot. */
	    chroot->set_active(true);

	    /* If a chroot mount location has not yet been set, set one
	       with the session id. */
	    if (chroot->get_mount_location().empty())
	      {
		char *location = g_strconcat(SCHROOT_MOUNT_DIR, "/",
					     this->session_id.c_str(), NULL);
		chroot->set_mount_location(location);
		g_free(location);
	      }

	    /* Chroot types which create a session (e.g. LVM devices)
	       need the chroot name respecifying. */
	    if (chroot->get_session_flags() & Chroot::SESSION_CREATE)
	      chroot->set_name(this->session_id);

	    /* LVM devices need the snapshot device name specifying. */
	    ChrootLvmSnapshot *snapshot = 0;
	    if ((snapshot = dynamic_cast<ChrootLvmSnapshot *>(chroot.get())) != 0)
	      {
		char *dir =
		  g_path_get_dirname(snapshot->get_device().c_str());
		char *device = g_strconcat(dir, "/",
					    this->session_id.c_str(), NULL);
		snapshot->set_snapshot_device(device);
		g_free(device);
		g_free(dir);
	      }

	    try
	      {
		/* Run setup-start chroot setup scripts. */
		setup_chroot(*chroot, Chroot::SETUP_START);
		if (this->operation == OPERATION_BEGIN)
		  g_fprintf(stdout, "%s\n", this->session_id.c_str());

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

/**
 * sbuild_session_setup_chroot_child_setup:
 * @session: an #Session
 *
 * A helper to make sure the child process is real and effective root
 * before running run-parts.
 */
void
sbuild_session_setup_chroot_child_setup ()
{
  /* This is required to ensure the scripts run with uid=0 and gid=0,
     otherwise setuid programs such as mount(8) will fail.  This
     should always succeed, because our euid=0 and egid=0.*/
  setuid(0);
  setgid(0);
  initgroups("root", 0);
}

/**
 * sbuild_session_setup_chroot:
 * @session: an #Session
 * @session_chroot: an #Chroot (which must be present in the @session configuration)
 * @setup_type: an #ChrootSetupType
 *
 * Setup a chroot.  This runs all of the commands in setup.d.
 *
 * The environment variables CHROOT_NAME, CHROOT_DESCRIPTION,
 * CHROOT_LOCATION, AUTH_USER and AUTH_VERBOSITY are set for use in
 * setup scripts.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
void
Session::setup_chroot (Chroot&           session_chroot,
		       Chroot::SetupType setup_type)
{
  g_return_if_fail(!session_chroot.get_name().empty());

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
      session_chroot.setup_lock(setup_type, TRUE);
    }
  catch (const Chroot::error &e)
    {
      throw error(std::string(_("Chroot setup failed to lock chroot: %s"))
		  + e.what(),
		  ERROR_CHROOT_SETUP);
    }

  char *setup_type_string = NULL;
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

  char **argv = g_new(char *, 8);
  {
    guint i = 0;
    argv[i++] = g_strdup(RUN_PARTS); // Run run-parts(8)
    if (get_verbosity() == Auth::VERBOSITY_VERBOSE)
      argv[i++] = g_strdup("--verbose");
    argv[i++] = g_strdup("--lsbsysinit");
    argv[i++] = g_strdup("--exit-on-error");
    if (setup_type == Chroot::SETUP_STOP ||
	setup_type == Chroot::RUN_STOP)
      argv[i++] = g_strdup("--reverse");
    argv[i++] = g_strdup_printf("--arg=%s", setup_type_string);
    if (setup_type == Chroot::SETUP_START ||
	setup_type == Chroot::SETUP_RECOVER ||
	setup_type == Chroot::SETUP_STOP)
      argv[i++] = g_strdup(SCHROOT_CONF_SETUP_D); // Setup directory
    else
      argv[i++] = g_strdup(SCHROOT_CONF_RUN_D); // Run directory
    argv[i++] = NULL;
  }

  /* Get a complete list of environment variables to set.  We need to
     query the chroot here, since this can vary depending upon the
     chroot type. */
  Chroot::env_list env;
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
	g_warning(_("Invalid verbosity level: %d, falling back to \"normal\""), get_verbosity());
	verbosity = "normal";
	break;
      }
    setup_env_var(env, "AUTH_VERBOSITY", verbosity);
  }

  setup_env_var(env, "MOUNT_DIR", SCHROOT_MOUNT_DIR);
  setup_env_var(env, "LIBEXEC_DIR", SCHROOT_LIBEXEC_DIR);
  setup_env_var(env, "PID", getpid());
  setup_env_var(env, "SESSION_ID", this->session_id);

  /* Move the strings into the envp vector. */
  char **envp = NULL;
  if (!env.empty())
    {
      guint num_vars = env.size();
      envp = g_new(char *, num_vars + 1);

      for (guint i = 0; i < num_vars; ++i)
	{
	  envp[i] = g_strdup(std::string(env[i].first + "=" + env[i].second).c_str());
	}
      envp[num_vars] = NULL;
    }

  int exit_status = 0;
  GError *tmp_error = NULL;

  g_spawn_sync("/",          // working directory
	       argv,         // arguments
	       envp,         // environment
	       static_cast<GSpawnFlags>(0), // spawn flags
	       (GSpawnChildSetupFunc)
	         sbuild_session_setup_chroot_child_setup, // child_setup
	       NULL,         // child_setup user_data
	       NULL,         // standard_output
	       NULL,         // standard_error
	       &exit_status, // child exit status
	       &tmp_error);  // error

  g_strfreev(argv);
  argv = NULL;
  g_strfreev(envp);
  envp = NULL;

  // TODO: Check tmp_error

  try
    {
      session_chroot.setup_lock(setup_type, false);
    }
  catch (const Chroot::error &e)
    {
      throw error(std::string(_("Chroot setup failed to unlock chroot: %s"))
		  + e.what(),
		  ERROR_CHROOT_SETUP);
    }

  if (exit_status != 0)
    throw error(std::string(_("Chroot setup failed during chroot "))
			    + setup_type_string,
			    ERROR_CHROOT_SETUP);
}

/**
 * sbuild_session_run_child:
 * @session: an #Session
 * @session_chroot: an #Chroot (which must be present in the @session configuration)
 *
 * Run a command or login shell as a child process in the specified chroot.
 *
 */
void
Session::run_child (Chroot& session_chroot)
{
  g_assert(!session_chroot.get_name().empty());
  g_assert(!session_chroot.get_mount_location().empty());

  g_assert(!get_user().empty());
  g_assert(!get_shell().empty());
  g_assert(Auth::pam != NULL); // PAM must be initialised

  const std::string& location = session_chroot.get_mount_location();
  char *cwd = g_get_current_dir();
  /* Child errors result in immediate exit().  Errors are not
     propagated back via an exception, because there is no longer any
     higher-level handler to catch them. */
  try
    {
      open_session();
    }
  catch (const Auth::error& e)
    {
      g_printerr(_("PAM error: %s\n"), e.what());
      exit (EXIT_FAILURE);
    }

  /* Set group ID and supplementary groups */
  if (setgid (get_gid()))
    {
      fprintf (stderr, _("Could not set gid to '%lu'\n"), (unsigned long) get_gid());
      exit (EXIT_FAILURE);
    }
  if (initgroups (get_user().c_str(), get_gid()))
    {
      fprintf (stderr, _("Could not set supplementary group IDs\n"));
      exit (EXIT_FAILURE);
    }

  /* Enter the chroot */
  if (chdir (location.c_str()))
    {
      fprintf (stderr, _("Could not chdir to '%s': %s\n"), location.c_str(),
	       g_strerror (errno));
      exit (EXIT_FAILURE);
    }
  if (chroot (location.c_str()))
    {
      fprintf (stderr, _("Could not chroot to '%s': %s\n"), location.c_str(),
	       g_strerror (errno));
      exit (EXIT_FAILURE);
    }

  /* Set uid and check we are not still root */
  if (setuid (get_uid()))
    {
      fprintf (stderr, _("Could not set uid to '%lu'\n"), (unsigned long) get_uid());
      exit (EXIT_FAILURE);
    }
  if (!setuid (0) && get_uid())
    {
      fprintf (stderr, _("Failed to drop root permissions.\n"));
      exit (EXIT_FAILURE);
    }

  /* chdir to current directory */
  if (chdir (cwd))
    {
      fprintf (stderr, _("warning: Could not chdir to '%s': %s\n"), cwd,
	       g_strerror (errno));
    }
  g_free(cwd);

  /* Set up environment */
  env_list env = get_pam_environment();
  for (env_list::const_iterator cur = env.begin();
       cur != env.end();
       ++cur)
    g_debug("Set environment: %s=%s", cur->first.c_str(), cur->second.c_str());

  /* Run login shell */
  char *file = NULL;

  string_list command = get_command();
  if (command.empty() ||
      command[0].empty()) // No command
    {
      g_assert (!get_shell().empty());

      file = g_strdup(get_shell().c_str());
      if (get_environment().empty()) // Not keeping environment; login shell
	{
	  char *shellbase = g_path_get_basename(get_shell().c_str());
	  char *loginshell = g_strconcat("-", shellbase, NULL);
	  g_free(shellbase);
	  command.push_back(loginshell);
	  g_debug("Login shell: %s", command[1].c_str());
	}
      else
	{
	  command.push_back(get_shell());
	}

      if (get_environment().empty())
	{
	  g_debug("Running login shell: %s", get_shell().c_str());
	  syslog(LOG_USER|LOG_NOTICE, "[%s chroot] (%s->%s) Running login shell: \"%s\"",
		 session_chroot.get_name().c_str(), get_ruser().c_str(), get_user().c_str(), get_shell().c_str());
	}
      else
	{
	  g_debug("Running shell: %s", get_shell().c_str());
	  syslog(LOG_USER|LOG_NOTICE, "[%s chroot] (%s->%s) Running shell: \"%s\"",
		 session_chroot.get_name().c_str(), get_ruser().c_str(), get_user().c_str(), get_shell().c_str());
	}

      if (get_verbosity() != Auth::VERBOSITY_QUIET)
	{
	  if (get_ruid() == get_uid())
	    {
	      if (get_environment().empty())
		g_printerr(_("[%s chroot] Running login shell: \"%s\"\n"),
			   session_chroot.get_name().c_str(), get_shell().c_str());
	      else
		g_printerr(_("[%s chroot] Running shell: \"%s\"\n"),
			   session_chroot.get_name().c_str(), get_shell().c_str());
	    }
	  else
	    {
	      if (get_environment().empty())
		g_printerr(_("[%s chroot] (%s->%s) Running login shell: \"%s\"\n"),
			   session_chroot.get_name().c_str(), get_ruser().c_str(), get_user().c_str(), get_shell().c_str());
	      else
		g_printerr(_("[%s chroot] (%s->%s) Running shell: \"%s\"\n"),
			   session_chroot.get_name().c_str(), get_ruser().c_str(), get_user().c_str(), get_shell().c_str());
	    }
	}
    }
  else
    {
      /* Search for program in path. */
      file = g_find_program_in_path(command[0].c_str());
      if (file == NULL)
	file = g_strdup(command[0].c_str());
      std::string commandstring = string_list_to_string(command, " ");
      g_debug("Running command: %s", commandstring.c_str());
      syslog(LOG_USER|LOG_NOTICE, "[%s chroot] (%s->%s) Running command: \"%s\"",
	     session_chroot.get_name().c_str(), get_ruser().c_str(), get_user().c_str(), commandstring.c_str());
      if (get_verbosity() != Auth::VERBOSITY_QUIET)
	{
	  if (get_ruid() == get_uid())
	    g_printerr(_("[%s chroot] Running command: \"%s\"\n"),
		       session_chroot.get_name().c_str(), commandstring.c_str());
	  else
	    g_printerr(_("[%s chroot] (%s->%s) Running command: \"%s\"\n"),
		       session_chroot.get_name().c_str(), get_ruser().c_str(), get_user().c_str(), commandstring.c_str());
	}
    }

  /* Execute */
  /* TODO: wrap execve. */
  if (exec (file, command, env))
    {
      fprintf (stderr, _("Could not exec \"%s\": %s\n"), command[0].c_str(),
	       g_strerror (errno));
      exit (EXIT_FAILURE);
    }
  /* This should never be reached */
  exit(EXIT_FAILURE);
}

/**
 * sbuild_session_wait_for_child:
 * @session: an #Session
 *
 * Wait for a child process to complete, and check its exit status.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
void
Session::wait_for_child (int pid)
{
  this->child_status = EXIT_FAILURE; // Default exit status

  int status;
  if (wait(&status) != pid)
    throw error(std::string(_("wait for child failed: ")) + g_strerror (errno),
		ERROR_CHILD);

  try
    {
      close_session();
    }
  catch (const Auth::error& e)
    {
      throw error(e.what(), ERROR_CHILD);
    }

  if (!WIFEXITED(status))
    {
      if (WIFSIGNALED(status))
	throw error(std::string(_("Child terminated by signal ")) + strsignal(WTERMSIG(status)),
		    ERROR_CHILD);
      else if (WCOREDUMP(status))
	throw error(_("Child dumped core"), ERROR_CHILD);
      else
	throw error(_("Child exited abnormally (reason unknown; not a signal or core dump)"),
		    ERROR_CHILD);
    }

  this->child_status = WEXITSTATUS(status);

  if (this->child_status)
    {
      char *gstr = g_strdup_printf(_("Child exited abnormally with status '%d'"),
				   this->child_status);
      std::string err(gstr);
      g_free(gstr);
      throw error(err, ERROR_CHILD);
    }
}

/**
 * sbuild_session_run_command:
 * @session: an #Session
 * @session_chroot: an #Chroot (which must be present in the @session configuration)
 *
 * Run the session command or login shell in the specified chroot.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
void
Session::run_chroot (Chroot&   session_chroot)
{
  g_return_if_fail(!session_chroot.get_name().empty());

  pid_t pid;
  if ((pid = fork()) == -1)
    {
      throw error(std::string(_("Failed to fork child: ")) + g_strerror(errno),
		  ERROR_FORK);
    }
  else if (pid == 0)
    {
      run_child(session_chroot);
      exit (EXIT_FAILURE); /* Should never be reached. */
    }
  else
    {
      wait_for_child(pid);
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
