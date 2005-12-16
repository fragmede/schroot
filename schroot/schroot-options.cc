/* schroot-options - schroot options parser
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
 * SECTION:schroot-options
 * @short_description: schroot options parser
 * @title: SchrootOptions
 *
 * This structure is used to contain the results of command-line
 * option parsing.
 */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "schroot-options.h"

static gboolean
parse_session_options(const gchar  *option_name,
		      const gchar  *value,
		      gpointer      data,
		      GError      **error);

/**
 * schroot_options_new:
 *
 * Parse command-line options.
 *
 * Returns a structure containing the options.
 */
SchrootOptions::SchrootOptions(int argc,
			       char *argv[]):
  chroots(),
  command(),
  user(),
  preserve(false),
  quiet(false),
  verbose(false),
  list(false),
  info(false),
  all(false),
  all_chroots(false),
  all_sessions(false),
  version(false),
  session_operation(SBUILD_SESSION_OPERATION_AUTOMATIC),
  session_force(false)
{
  char **chroots;
  char **command;
  char *user;

  GOptionEntry entries[] =
    {
      { "all", 'a', 0, G_OPTION_ARG_NONE, &this->all,
	N_("Select all chroots and active sessions"), NULL },
      { "all-chroots", 0, 0, G_OPTION_ARG_NONE, &this->all_chroots,
	N_("Select all chroots"), NULL },
      { "all-sessions", 0, 0, G_OPTION_ARG_NONE, &this->all_sessions,
	N_("Select all active sessions"), NULL },
      { "chroot", 'c', 0, G_OPTION_ARG_STRING_ARRAY, &chroots,
	N_("Use specified chroot"), "chroot" },
      { "user", 'u', 0, G_OPTION_ARG_STRING, &user,
	N_("Username (default current user)"), "user" },
      { "list", 'l', 0, G_OPTION_ARG_NONE, &this->list,
	N_("List available chroots"), NULL },
      { "info", 'i', 0, G_OPTION_ARG_NONE, &this->info,
	N_("Show information about chroot"), NULL },
      { "preserve-environment", 'p', 0, G_OPTION_ARG_NONE, &this->preserve,
	N_("Preserve user environment"), NULL },
      { "quiet", 'q', 0, G_OPTION_ARG_NONE, &this->quiet,
	N_("Show less output"), NULL },
      { "verbose", 'v', 0, G_OPTION_ARG_NONE, &this->verbose,
	N_("Show more output"), NULL },
      { "version", 'V', 0, G_OPTION_ARG_NONE, &this->version,
	N_("Print version information"), NULL },
      { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &command,
	NULL, NULL },
      { NULL }
    };

  GOptionEntry session_entries[] =
    {
      { "begin-session", 'b', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	(void *) parse_session_options,
	N_("Begin a session; returns a session UUID"), NULL },
      { "recover-session", 0, G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	(void *) parse_session_options,
	N_("Recover an existing session"), NULL },
      { "run-session", 'r', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	(void *) parse_session_options,
	N_("Run an existing session"), NULL },
      { "end-session", 'e', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	(void *) parse_session_options,
	N_("End an existing session"), NULL },
      { "force", 'f', 0, G_OPTION_ARG_NONE, &this->session_force,
	N_("Force operation, even if it fails"), NULL },
      { NULL }
    };

  GError *error = NULL;

  GOptionContext *context = g_option_context_new (_("- run command or shell in a chroot"));
  g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);

  GOptionGroup* session_group =
    g_option_group_new("session", N_("Session Options"),
		       N_("Session management options"),
		       static_cast<void *>(this), NULL);
  g_option_group_set_translation_domain(session_group, GETTEXT_PACKAGE);
  g_option_group_add_entries (session_group, session_entries);
  g_option_context_add_group(context, session_group);

  g_option_context_parse (context, &argc, &argv, &error);
  g_option_context_free (context);
  if (error != NULL)
    {
      g_printerr(_("Error parsing options: %s\n"), error->message);
      exit (EXIT_FAILURE);
    }

  if (this->quiet && this->verbose)
    g_printerr(_("--quiet and --verbose may not be used at the same time!\nUsing verbose output.\n"));

  for (char *chroot = chroots[0]; chroot != NULL; ++chroot)
    this->chroots.push_back(chroot);
  g_strfreev(chroots);

  this->user = user;
  g_free(user);

  for (char *command_part = command[0]; command_part != NULL; ++command_part)
    this->command.push_back(command_part);
  g_strfreev(command);

  /* Ensure there's something to list. */
  if ((this->list == true &&
       (this->all == false && this->all_chroots == false &&
	this->all_sessions == false)) ||
      (this->info == true &&
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
  if (this->list == false &&
      this->chroots.empty())
    this->load_chroots = this->load_sessions = true;
}

SchrootOptions::~SchrootOptions()
{
}

/**
 * parse_session_options:
 *
 * Parse command-line session options.  The options are placed in the
 * session_opt structure.  This is a #GOptionArgFunc.
 *
 * Returns true on success, false on failure (and error will also be
 * set).
 */
static gboolean
parse_session_options(const gchar  *option_name,
		      const gchar  *value,
		      gpointer      data,
		      GError      **error)
{
  SchrootOptions *options = static_cast<SchrootOptions *>(data);

  if (strcmp(option_name, "-b") == 0 ||
      strcmp(option_name, "--begin-session") == 0)
    {
      options->session_operation = SBUILD_SESSION_OPERATION_BEGIN;
    }
  else if (strcmp(option_name, "--recover-session") == 0)
    {
      options->session_operation = SBUILD_SESSION_OPERATION_RECOVER;
    }
  else if (strcmp(option_name, "-r") == 0 ||
	   strcmp(option_name, "--run-session") == 0)
    {
      options->session_operation = SBUILD_SESSION_OPERATION_RUN;
    }
  else if (strcmp(option_name, "-e") == 0 ||
	   strcmp(option_name, "--end-session") == 0)
    {
      options->session_operation = SBUILD_SESSION_OPERATION_END;
    }
  else
    {
      g_set_error(error,
		  G_OPTION_ERROR, G_OPTION_ERROR_FAILED,
		  _("Invalid session option %s"), option_name);
      return false;
    }

  return true;
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
