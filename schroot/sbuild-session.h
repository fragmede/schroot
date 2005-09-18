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

#ifndef SBUILD_SESSION_H
#define SBUILD_SESSION_H

#include <sys/types.h>
#include <sys/wait.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#include <glib.h>
#include <glib/gprintf.h>
#include <glib-object.h>

#include <uuid/uuid.h>

#include "sbuild-auth.h"
#include "sbuild-config.h"

typedef enum
{
  SBUILD_SESSION_ERROR_FORK,
  SBUILD_SESSION_ERROR_CHILD,
  SBUILD_SESSION_ERROR_CHROOT,
  SBUILD_SESSION_ERROR_CHROOT_SETUP
} SbuildSessionError;

#define SBUILD_SESSION_ERROR sbuild_session_error_quark()

GQuark
sbuild_session_error_quark (void);

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
  SbuildAuth     parent;
  SbuildConfig  *config;
  char         **chroots;
  int            child_status;
  uuid_t         session_id;
};

struct _SbuildSessionClass
{
  SbuildAuthClass parent;
};


GType
sbuild_session_get_type (void);

SbuildSession *
sbuild_session_new(const char    *service,
		   SbuildConfig  *config,
		   char         **chroots);

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

gchar *
sbuild_session_get_session_id (const SbuildSession  *restrict session);

void
sbuild_session_set_session_id (SbuildSession  *session,
			       const gchar    *session_id);

int
sbuild_session_get_child_status (SbuildSession *session);

#endif /* SBUILD_SESSION_H */

/*
 * Local Variables:
 * mode:C
 * End:
 */
