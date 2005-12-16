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
 * @title: SbuildSession
 *
 * This object provides the session handling for schroot.  It derives
 * from SbuildAuth, which performs all the necessary PAM actions,
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

namespace
{
  /* TODO: move to utils */
  std::string
  string_list_to_string(const SbuildChroot::string_list& list,
			const std::string&               separator)
  {
    std::string ret;

    for (SbuildChroot::string_list::const_iterator cur = list.begin();
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
  string_list_to_strv(const SbuildAuth::string_list& str)
  {
    char **ret = new char *[str.size() + 1];

    for (SbuildAuth::string_list::size_type i = 0;
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
  env_list_to_strv(const SbuildAuth::env_list& env)
  {
    char **ret = new char *[env.size() + 1];

    for (SbuildAuth::env_list::size_type i = 0;
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
  static gboolean
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

    gboolean group_member = FALSE;
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
 * Get the SBUILD_SESSION_ERROR domain number.
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

SbuildSession::SbuildSession (const std::string&                  service,
			      std::tr1::shared_ptr<SbuildConfig>& config,
			      SbuildSessionOperation              operation,
			      string_list                         chroots):
  SbuildAuth(service),
  config(config),
  chroots(chroots),
  child_status(0),
  session_id(),
  force(false)
{
}

SbuildSession::~SbuildSession()
{
}

/**
 * sbuild_session_get_config:
 * @session: an #SbuildSession
 *
 * Get the configuration associated with @session.
 *
 * Returns an #SbuildConfig.
 */
std::tr1::shared_ptr<SbuildConfig>&
SbuildSession::get_config ()
{
  return this->config;
}

/**
 * sbuild_session_set_config:
 * @session: an #SbuildSession
 * @config: an #SbuildConfig
 *
 * Set the configuration associated with @session.
 */
void
SbuildSession::set_config (std::tr1::shared_ptr<SbuildConfig>& config)
{
  this->config = config;
}

/**
 * sbuild_session_get_chroots:
 * @session: an #SbuildSession
 *
 * Get the chroots to use in @session.
 *
 * Returns a string vector. This string vector points to internally
 * allocated storage in the chroot and must not be freed, modified or
 * stored.
 */
const SbuildSession::string_list&
SbuildSession::get_chroots () const
{
  return this->chroots;
}

/**
 * sbuild_session_set_chroots:
 * @session: an #SbuildSession
 * @chroots: the chroots to use
 *
 * Set the chroots to use in @session.
 */
void
SbuildSession::set_chroots (const SbuildSession::string_list& chroots)
{
  this->chroots = chroots;
}

/**
 * sbuild_session_get_operation:
 * @session: an #SbuildSession
 *
 * Get the operation @session will perform.
 *
 * Returns an #SbuildConfig.
 */
SbuildSessionOperation
SbuildSession::get_operation () const
{
  return this->operation;
}

/**
 * sbuild_session_set_operation:
 * @session: an #SbuildSession
 * @operation: an #SbuildSessionOperation
 *
 * Set the operation @session will perform.
 */
void
SbuildSession::set_operation (SbuildSessionOperation  operation)
{
  this->operation = operation;
}

/**
 * sbuild_session_get_session_id:
 * @session: an #SbuildSession
 *
 * Get the session identifier.  The session identifier is a unique
 * string to identify a session.
 *
 * Returns a string.  The string must be freed by the caller.
 */
const std::string&
SbuildSession::get_session_id () const
{
  return this->session_id;
}

/**
 * sbuild_session_set_session_id:
 * @session: an #SbuildSession
 * @session_id: an string containing a valid session id
 *
 * Set the session identifier for @session.
 */
void
SbuildSession::set_session_id (const std::string& session_id)
{
  this->session_id = session_id;
}

/**
 * sbuild_session_get_force:
 * @session: an #SbuildSession
 *
 * Get the force status of @session.
 *
 * Returns TRUE if operation will be forced, otherwise FALSE.
 */
bool
SbuildSession::get_force () const
{
  return this->force;
}

/**
 * sbuild_session_set_force:
 * @session: an #SbuildSession
 * @force: TRUE to force session operation, otherwise FALSE
 *
 * Set the force status of @session.
 */
void
SbuildSession::set_force (bool force)
{
  this->force = force;
}

/**
 * sbuild_session_get_child_status:
 * @session: an #SbuildSession
 *
 * Get the exit (wait) status of the last child process to run in this
 * session.
 *
 * Returns the exit status.
 */
int
SbuildSession::get_child_status () const
{
  return this->child_status;
}

/**
 * sbuild_session_require_auth:
 * @session: an #SbuildSession
 *
 * Check if authentication is required for @session.  Group membership
 * is checked for all chroots, and depending on which user will be run
 * in the chroot, password authentication or no authentication may be
 * required.
 *
 * Returns the authentication type.
 */
SbuildAuthStatus
SbuildSession::get_auth_status () const
{
  g_return_val_if_fail(!this->chroots.empty(), SBUILD_AUTH_STATUS_FAIL);
  g_return_val_if_fail(this->config.get() != NULL, SBUILD_AUTH_STATUS_FAIL);

  SbuildAuthStatus status = SBUILD_AUTH_STATUS_NONE;

  /* TODO set difference. */
  for (string_list::const_iterator cur = this->chroots.begin();
       cur != this->chroots.end();
       ++cur)
    {
      const SbuildChroot *chroot = this->config->find_alias(*cur);
      if (chroot == NULL) // Should never happen, but cater for it anyway.
	{
	  g_warning(_("No chroot found matching alias '%s'"), cur->c_str());
	  status = change_auth(status, SBUILD_AUTH_STATUS_FAIL);
	}

      const SbuildChroot::string_list& groups = chroot->get_groups();
      const SbuildChroot::string_list& root_groups = chroot->get_root_groups();

      if (!groups.empty())
	{
	  gboolean in_groups = FALSE;
	  gboolean in_root_groups = FALSE;

	  if (!groups.empty())
	    {
	      for (SbuildChroot::string_list::const_iterator gp = groups.begin();
		   gp != groups.end();
		   ++gp)
		if (is_group_member(*gp))
		  in_groups = TRUE;
	    }

	  if (!root_groups.empty())
	    {
	      for (SbuildChroot::string_list::const_iterator gp = root_groups.begin();
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
	      status = change_auth(status, SBUILD_AUTH_STATUS_NONE);
	    }
	  else if (in_groups == TRUE) // Auth required if not in root group
	    {
	      status = change_auth(status, SBUILD_AUTH_STATUS_USER);
	    }
	  else // Not in any groups
	    {
	      status = change_auth(status, SBUILD_AUTH_STATUS_FAIL);
	    }
	}
      else // No available groups entries means no access to anyone
	{
	  status = change_auth(status, SBUILD_AUTH_STATUS_FAIL);
	}
    }

  return status;
}

/**
 * sbuild_session_run:
 * @session: an #SbuildSession
 * @error: a #GError, or NULL to ignore errors
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
bool
SbuildSession::run_impl (GError **error)
{
  g_return_val_if_fail(this->config.get() != NULL, FALSE);
  g_return_val_if_fail(!this->chroots.empty(), FALSE);

  GError *tmp_error = NULL;

  for (SbuildChroot::string_list::const_iterator cur = this->chroots.begin();
       cur != this->chroots.end();
       ++cur)
    {
      g_debug("Running session in %s chroot:\n", cur->c_str());
      const SbuildChroot *ch = this->config->find_alias(*cur);
      if (chroot == NULL) // Should never happen, but cater for it anyway.
	{
	  g_set_error(&tmp_error,
		      SBUILD_SESSION_ERROR, SBUILD_SESSION_ERROR_CHROOT,
		      _("%s: Failed to find chroot"), cur->c_str());
	}
      else
	{
	  std::auto_ptr<SbuildChroot> chroot(ch->clone());

	  /* If restoring a session, set the session ID from the
	     chroot name, or else generate it.  Only chroots which
	     support session creation append a UUID to the session
	     ID. */
	  if (chroot->get_active() ||
	      !(chroot->get_session_flags() & SBUILD_CHROOT_SESSION_CREATE))
	    {
	      set_session_id(chroot->get_name());
	    }
	  else
	    {
	      uuid_t uuid;
	      gchar uuid_str[37];
	      uuid_generate(uuid);
	      uuid_unparse(uuid, uuid_str);
	      uuid_clear(uuid);
	      gchar *session_id = g_strconcat(chroot->get_name().c_str(), "-", uuid_str, NULL);
	      set_session_id(session_id);
	      g_free(session_id);
	    }

	  /* Activate chroot. */
	  chroot->set_active(true);

	  /* If a chroot mount location has not yet been set, set one
	     with the session id. */
	  if (chroot->get_mount_location().empty())
	    {
	      gchar *location = g_strconcat(SCHROOT_MOUNT_DIR, "/",
					    this->session_id.c_str(), NULL);
	      chroot->set_mount_location(location);
	      g_free(location);
	    }

	  /* Chroot types which create a session (e.g. LVM devices)
	     need the chroot name respecifying. */
	  if (chroot->get_session_flags() & SBUILD_CHROOT_SESSION_CREATE)
	    chroot->set_name(this->session_id);

	  /* LVM devices need the snapshot device name specifying. */
	  SbuildChrootLvmSnapshot *snapshot = 0;
	  if ((snapshot = dynamic_cast<SbuildChrootLvmSnapshot *>(chroot.get())) != 0)
	    {
	      gchar *dir =
		g_path_get_dirname(snapshot->get_device().c_str());
	      gchar *device = g_strconcat(dir, "/",
					  this->session_id.c_str(), NULL);
	      snapshot->set_snapshot_device(device);
	      g_free(device);
	      g_free(dir);
	    }

	  /* Run setup-start chroot setup scripts. */
	  if (this->operation == SBUILD_SESSION_OPERATION_AUTOMATIC ||
	      this->operation == SBUILD_SESSION_OPERATION_BEGIN)
	    {
	      if (chroot->get_run_setup_scripts() == TRUE)
		setup_chroot(*chroot,
			     SBUILD_CHROOT_SETUP_START,
			     &tmp_error);
	      if (this->operation == SBUILD_SESSION_OPERATION_BEGIN)
		g_fprintf(stdout, "%s\n", this->session_id.c_str());
	    }

	  /* Run recover scripts. */
	  if (this->operation == SBUILD_SESSION_OPERATION_RECOVER)
	    {
	      if (chroot->get_run_setup_scripts() == TRUE)
		setup_chroot(*chroot,
			     SBUILD_CHROOT_SETUP_RECOVER,
			     &tmp_error);
	    }

	  if (tmp_error == NULL &&
	      (this->operation == SBUILD_SESSION_OPERATION_AUTOMATIC ||
	       this->operation == SBUILD_SESSION_OPERATION_RUN))
	    {
	      /* Run run-start scripts. */
	      if (chroot->get_run_session_scripts() == TRUE)
		setup_chroot(*chroot,
			     SBUILD_CHROOT_RUN_START,
			     &tmp_error);

	      /* Run session if setup succeeded. */
	      if (tmp_error == NULL)
		run_chroot(*chroot, &tmp_error);

	      /* Run run-stop scripts whether or not there was an
		 error. */
	      if (chroot->get_run_session_scripts() == TRUE)
		setup_chroot(*chroot,
			     SBUILD_CHROOT_RUN_STOP,
			     (tmp_error != NULL) ? NULL : &tmp_error);
	    }

	  /* Run setup-stop chroot setup scripts whether or not there
	     was an error. */
	  if (this->operation == SBUILD_SESSION_OPERATION_AUTOMATIC ||
	      this->operation == SBUILD_SESSION_OPERATION_END)
	    {
	      if (chroot->get_run_setup_scripts() == TRUE)
		setup_chroot(*chroot,
			     SBUILD_CHROOT_SETUP_STOP,
			     (tmp_error != NULL) ? NULL : &tmp_error);
	    }

	  /* Deactivate chroot. */
	  chroot->set_active(false);
	}

      if (tmp_error != NULL)
	break;
    }

  if (tmp_error != NULL)
    {
      g_propagate_error(error, tmp_error);
      /* If a command was not run, but something failed, the exit
	 status still needs setting. */
      if (this->child_status == 0)
	this->child_status = EXIT_FAILURE;
      return FALSE;
    }
  else
    return TRUE;
}

/**
 * sbuild_session_setup_chroot_child_setup:
 * @session: an #SbuildSession
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
 * @session: an #SbuildSession
 * @session_chroot: an #SbuildChroot (which must be present in the @session configuration)
 * @setup_type: an #SbuildChrootSetupType
 * @error: a #GError
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
bool
SbuildSession::setup_chroot (SbuildChroot&           session_chroot,
			     SbuildChrootSetupType   setup_type,
			     GError                **error)
{
  g_return_val_if_fail(!session_chroot.get_name().empty(), FALSE);
  g_return_val_if_fail(setup_type == SBUILD_CHROOT_SETUP_START ||
		       setup_type == SBUILD_CHROOT_SETUP_RECOVER ||
		       setup_type == SBUILD_CHROOT_SETUP_STOP ||
		       setup_type == SBUILD_CHROOT_RUN_START ||
		       setup_type == SBUILD_CHROOT_RUN_STOP, FALSE);

  GError *setup_lock_error = NULL;
  if (session_chroot.setup_lock(setup_type,
				TRUE, &setup_lock_error) == FALSE)
    {
      g_set_error(error,
		  SBUILD_SESSION_ERROR, SBUILD_SESSION_ERROR_CHROOT_SETUP,
		  _("Chroot setup failed to lock chroot: %s"),
		  setup_lock_error->message);
      g_error_free(setup_lock_error);
      return FALSE;
    }

  gchar *setup_type_string = NULL;
  if (setup_type == SBUILD_CHROOT_SETUP_START)
    setup_type_string = "setup-start";
  else if (setup_type == SBUILD_CHROOT_SETUP_RECOVER)
    setup_type_string = "setup-recover";
  else if (setup_type == SBUILD_CHROOT_SETUP_STOP)
    setup_type_string = "setup-stop";
  else if (setup_type == SBUILD_CHROOT_RUN_START)
    setup_type_string = "run-start";
  else if (setup_type == SBUILD_CHROOT_RUN_STOP)
    setup_type_string = "run-stop";

  gchar **argv = g_new(gchar *, 8);
  {
    guint i = 0;
    argv[i++] = g_strdup(RUN_PARTS); // Run run-parts(8)
    if (get_verbosity() == SBUILD_AUTH_VERBOSITY_VERBOSE)
      argv[i++] = g_strdup("--verbose");
    argv[i++] = g_strdup("--lsbsysinit");
    argv[i++] = g_strdup("--exit-on-error");
    if (setup_type == SBUILD_CHROOT_SETUP_STOP ||
	setup_type == SBUILD_CHROOT_RUN_STOP)
      argv[i++] = g_strdup("--reverse");
    argv[i++] = g_strdup_printf("--arg=%s", setup_type_string);
    if (setup_type == SBUILD_CHROOT_SETUP_START ||
	setup_type == SBUILD_CHROOT_SETUP_RECOVER ||
	setup_type == SBUILD_CHROOT_SETUP_STOP)
      argv[i++] = g_strdup(SCHROOT_CONF_SETUP_D); // Setup directory
    else
      argv[i++] = g_strdup(SCHROOT_CONF_RUN_D); // Run directory
    argv[i++] = NULL;
  }

  /* Get a complete list of environment variables to set.  We need to
     query the chroot here, since this can vary depending upon the
     chroot type. */
  SbuildChroot::env_list env;
  session_chroot.setup_env(env);
  setup_env_var(env, "AUTH_USER",
		get_user());
  {
    const char *verbosity = NULL;
    switch (get_verbosity())
      {
      case SBUILD_AUTH_VERBOSITY_QUIET:
	verbosity = "quiet";
	break;
      case SBUILD_AUTH_VERBOSITY_NORMAL:
	verbosity = "normal";
	break;
      case SBUILD_AUTH_VERBOSITY_VERBOSE:
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
  gchar **envp = NULL;
  if (!env.empty())
    {
      guint num_vars = env.size();
      envp = g_new(gchar *, num_vars + 1);

      for (guint i = 0; i < num_vars; ++i)
	{
	  envp[i] = g_strdup(std::string(env[i].first + "=" + env[i].second).c_str());
	}
      envp[num_vars] = NULL;
    }

  gint exit_status = 0;
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

  GError *setup_unlock_error = NULL;
  if (session_chroot.setup_lock(setup_type,
				FALSE, &setup_unlock_error) == FALSE)
    {
      if (tmp_error == NULL)
	g_set_error(&tmp_error,
		    SBUILD_SESSION_ERROR, SBUILD_SESSION_ERROR_CHROOT_SETUP,
		    _("Chroot setup failed to unlock chroot: %s"),
		    setup_unlock_error->message);
      g_error_free(setup_unlock_error);
    }

  if (tmp_error != NULL)
    {
      g_propagate_error(error, tmp_error);
      return FALSE;
    }

  if (exit_status != 0)
    {
      g_set_error(error,
		  SBUILD_SESSION_ERROR, SBUILD_SESSION_ERROR_CHROOT_SETUP,
		  _("Chroot setup failed during chroot %s"),
		  setup_type_string);
      return FALSE;
    }

  return TRUE;
}

/**
 * sbuild_session_run_child:
 * @session: an #SbuildSession
 * @session_chroot: an #SbuildChroot (which must be present in the @session configuration)
 *
 * Run a command or login shell as a child process in the specified chroot.
 *
 */
void
SbuildSession::run_child (SbuildChroot& session_chroot)
{
  g_assert(!session_chroot.get_name().empty());
  g_assert(!session_chroot.get_mount_location().empty());

  g_assert(!get_user().empty());
  g_assert(!get_shell().empty());
  g_assert(SbuildAuth::pam != NULL); // PAM must be initialised

  const std::string& location = session_chroot.get_mount_location();
  char *cwd = g_get_current_dir();
  /* Child errors result in immediate exit().  Errors are not
     propagated back via a GError, because there is no longer any
     higher-level handler to catch them. */
  GError *pam_error = NULL;
  open_session(&pam_error);
  if (pam_error != NULL)
    {
      g_printerr(_("PAM error: %s\n"), pam_error->message);
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

      if (get_verbosity() != SBUILD_AUTH_VERBOSITY_QUIET)
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
      if (get_verbosity() != SBUILD_AUTH_VERBOSITY_QUIET)
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
 * @session: an #SbuildSession
 * @error: a #GError
 *
 * Wait for a child process to complete, and check its exit status.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
bool
SbuildSession::wait_for_child (int             pid,
			       GError        **error)
{
  this->child_status = EXIT_FAILURE; // Default exit status

  int status;
  if (wait(&status) != pid)
    {
      g_set_error(error,
		  SBUILD_SESSION_ERROR, SBUILD_SESSION_ERROR_CHILD,
		  _("wait for child failed: %s\n"), g_strerror (errno));
      return FALSE;
    }

  GError *pam_error = NULL;
  close_session(&pam_error);
  if (pam_error != NULL)
    {
      g_propagate_error(error, pam_error);
      return FALSE;
    }

  if (!WIFEXITED(status))
    {
      if (WIFSIGNALED(status))
	g_set_error(error,
		    SBUILD_SESSION_ERROR, SBUILD_SESSION_ERROR_CHILD,
		    _("Child terminated by signal '%s'"),
		    strsignal(WTERMSIG(status)));
      else if (WCOREDUMP(status))
	g_set_error(error,
		    SBUILD_SESSION_ERROR, SBUILD_SESSION_ERROR_CHILD,
		    _("Child dumped core"));
      else
	g_set_error(error,
		    SBUILD_SESSION_ERROR, SBUILD_SESSION_ERROR_CHILD,
		    _("Child exited abnormally (reason unknown; not a signal or core dump)"));
      return FALSE;
    }

  this->child_status = WEXITSTATUS(status);

  if (this->child_status)
    {
      g_set_error(error,
		  SBUILD_SESSION_ERROR, SBUILD_SESSION_ERROR_CHILD,
		  _("Child exited abnormally with status '%d'"),
		  this->child_status);
      return FALSE;
    }

  /* Should never be reached. */
  return TRUE;
}

/**
 * sbuild_session_run_command:
 * @session: an #SbuildSession
 * @session_chroot: an #SbuildChroot (which must be present in the @session configuration)
 * @error: a #GError
 *
 * Run the session command or login shell in the specified chroot.
 *
 * Returns TRUE on success, FALSE on failure (@error will be set to
 * indicate the cause of the failure).
 */
bool
SbuildSession::run_chroot (SbuildChroot&   session_chroot,
			   GError        **error)
{
  g_return_val_if_fail(!session_chroot.get_name().empty(), FALSE);

  pid_t pid;
  if ((pid = fork()) == -1)
    {
      g_set_error(error,
		  SBUILD_SESSION_ERROR, SBUILD_SESSION_ERROR_FORK,
		  _("Failed to fork child: %s"), g_strerror(errno));
      return FALSE;
    }
  else if (pid == 0)
    {
      run_child(session_chroot);
      exit (EXIT_FAILURE); /* Should never be reached. */
    }
  else
    {
      GError *tmp_error = NULL;
      wait_for_child(pid, &tmp_error);

      if (tmp_error != NULL)
	{
	  g_propagate_error(error, tmp_error);
	  return FALSE;
	}
      return TRUE;
    }
}

int
SbuildSession::exec (const std::string& file,
		     const string_list& command,
		     const env_list& env)
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
