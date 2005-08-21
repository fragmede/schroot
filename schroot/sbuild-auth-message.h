/* sbuild-auth-message - sbuild authentication messaging types
 *
 * Copyright © 2005  Roger Leigh <rleigh@debian.org>
 *
 * serror is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * serror is distributed in the hope that it will be useful, but
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

#ifndef SBUILD_AUTH_MESSAGE_H
#define SBUILD_AUTH_MESSAGE_H

#include <glib-object.h>

#include <security/pam_appl.h>

#define SBUILD_TYPE_AUTH_MESSAGE (sbuild_auth_message_get_type ())
#define SBUILD_TYPE_AUTH_MESSAGE_VECTOR (sbuild_auth_message_vector_get_type ())

GType
sbuild_auth_message_get_type (void);

GType
sbuild_auth_message_vector_get_type (void);

typedef enum
{
  SBUILD_AUTH_MESSAGE_PROMPT_NOECHO = PAM_PROMPT_ECHO_OFF,
  SBUILD_AUTH_MESSAGE_PROMPT_ECHO = PAM_PROMPT_ECHO_ON,
  SBUILD_AUTH_MESSAGE_ERROR = PAM_ERROR_MSG,
  SBUILD_AUTH_MESSAGE_INFO = PAM_TEXT_INFO
} SbuildAuthMessageType;

typedef struct _SbuildAuthMessage SbuildAuthMessage;
typedef struct _SbuildAuthMessageVector SbuildAuthMessageVector;

struct _SbuildAuthMessage
{
  SbuildAuthMessageType  type;
  const gchar           *message;
  gchar                 *response;
  gpointer               user_data;
};

struct _SbuildAuthMessageVector
{
  volatile gint       refcount;
  SbuildAuthMessage **messages;
};

SbuildAuthMessage *
sbuild_auth_message_copy (SbuildAuthMessage *message);

void
sbuild_auth_message_free (SbuildAuthMessage *message);

SbuildAuthMessageVector *
sbuild_auth_message_vector_new (guint size);

SbuildAuthMessageVector *
sbuild_auth_message_vector_copy (SbuildAuthMessageVector *messagev);

void
sbuild_auth_message_vector_free (SbuildAuthMessageVector *messagev);

#endif /* SBUILD_AUTH_MESSAGE_H */

/*
 * Local Variables:
 * mode:C
 * End:
 */
