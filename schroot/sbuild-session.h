/* sbuild-session - sbuild session object
 *
 * Copyright (C) 2005  Roger Leigh <rleigh@debian.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *********************************************************************/

#ifndef SBUILD_SESSION_H
#define SBUILD_SESSION_H

#include <security/pam_appl.h>
#include <security/pam_misc.h>

#include <glib.h>
#include <glib/gprintf.h>
#include <glib-object.h>

#include "sbuild-config.h"

#define SBUILD_TYPE_SESSION		  (sbuild_session_get_type ())
#define SBUILD_SESSION(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), SBUILD_TYPE_SESSION, SbuildSession))
#define SBUILD_SESSION_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), SBUILD_TYPE_SESSION, SbuildSessionClass))
#define SBUILD_IS_SESSION(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SBUILD_TYPE_SESSION))
#define SBUILD_IS_SESSION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SBUILD_TYPE_SESSION))
#define SBUILD_SESSION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SBUILD_TYPE_SESSION, SbuildSessionClass))

typedef struct _SbuildSession SbuildSession;
typedef struct _SbuildSessionClass SbuildSessionClass;

struct _SbuildSession
{
  GObject        parent;
  uid_t          uid;
  gid_t          gid;
  gchar         *user;
  gchar        **command;
  gchar         *shell;
  uid_t          ruid;
  gchar         *ruser;
  SbuildConfig  *config;
  char         **chroots;
  pam_handle_t  *pam;
};

struct _SbuildSessionClass
{
  GObjectClass parent;
};


GType
sbuild_session_get_type (void);

SbuildSession *
sbuild_session_new(SbuildConfig  *config,
		   char         **chroots);

const char *
sbuild_session_get_user (const SbuildSession *restrict session);

void
sbuild_session_set_user (SbuildSession *session,
			 const char   *user);

char **
sbuild_session_get_command (const SbuildSession *restrict session);

void
sbuild_session_set_command (SbuildSession  *session,
			    char          **command);

SbuildConfig *
sbuild_session_get_config (const SbuildSession *restrict session);

void
sbuild_session_set_config (SbuildSession *session,
			   SbuildConfig  *config);

char **
sbuild_session_get_chroots (const SbuildSession *restrict session);

void
sbuild_session_set_chroots (SbuildSession  *session,
			    char         **chroots);

void
sbuild_session_run (SbuildSession *session);

#endif /* SBUILD_SESSION_H */

/*
 * Local Variables:
 * mode:C
 * End:
 */
