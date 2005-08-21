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

/**
 * SECTION:sbuild-auth-message
 * @short_description: SbuildAuth messages
 * @title: SbuildAuthMessage and SbuildAuthMessageVector
 *
 * When SbuildAuth needs to interact with the user, it does this by
 * sending a set of #SbuildAuthMessage structures to an
 * #SbuildAuthConv conversation object.  These messages tell the
 * conversation object how to display the message to the user, and if
 * necessary, whether or not to ask the user for some input.
 */

#include <config.h>

#include "sbuild-auth-message.h"

GType
sbuild_auth_message_get_type (void)
{
  static GType type = 0;

  if (type == 0)
    type = g_boxed_type_register_static ("SbuildAuthMessage",
                                         (GBoxedCopyFunc) sbuild_auth_message_copy,
                                         (GBoxedFreeFunc) sbuild_auth_message_free);

  return type;
}

/**
 * sbuild_auth_message_assign:
 * @source: the source message
 * @dest: the destination message
 *
 * Assign the contents of one message to another.  This does not copy
 * the user_data member.
 */
static inline void
sbuild_auth_message_assign (const SbuildAuthMessage *restrict source,
			    SbuildAuthMessage *dest)
{
  dest->type = source->type;
  dest->message = source->message;
  dest->response = g_strdup(source->response);
  dest->user_data = NULL;
}

/**
 * sbuild_auth_message_copy:
 * @message: an #SbuildAuthMessage
 *
 * Copy a message.
 *
 * Returns a newly allocated copy of @message.  This must be freed by
 * the caller.
 */
SbuildAuthMessage *
sbuild_auth_message_copy (SbuildAuthMessage *message)
{
  if (message == NULL)
    return NULL;

  SbuildAuthMessage *copy = g_new(SbuildAuthMessage, 1);

  sbuild_auth_message_assign(message, copy);

  return copy;
}

/**
 * sbuild_auth_message_clear:
 * @message: an #SbuildAuthMessage
 *
 * Clear the allocated contents of @message
 */
static inline void
sbuild_auth_message_clear (SbuildAuthMessage *message)
{
  if (message == NULL)
    return;

  g_free (message->response);
}

/**
 * sbuild_auth_message_free:
 * @message: an #SbuildAuthMessage
 *
 * Free @message.
 */
void
sbuild_auth_message_free (SbuildAuthMessage *message)
{
  if (message == NULL)
    return;

  sbuild_auth_message_clear (message);
  g_free (message);
}

GType
sbuild_auth_message_vector_get_type (void)
{
  static GType type = 0;

  if (type == 0)
    type = g_boxed_type_register_static ("SbuildAuthMessageVector",
                                         (GBoxedCopyFunc) sbuild_auth_message_vector_copy,
                                         (GBoxedFreeFunc) sbuild_auth_message_vector_free);

  return type;
}

/**
 * sbuild_auth_message_vector_new:
 * @size: the number of messages.
 *
 * Create a new #SbuildAuthMessageVector with space for @size
 * messages.
 *
 * Returns a new #SbuildAuthMessageVector.
 */
SbuildAuthMessageVector *
sbuild_auth_message_vector_new (guint size)
{
  SbuildAuthMessageVector *mv = g_new(SbuildAuthMessageVector, 1);

  mv->refcount = 1;
  mv->messages = g_new(SbuildAuthMessage *, size + 1);

  for (guint i = 0; i < size; ++i)
    {
      SbuildAuthMessage *msg = g_new(SbuildAuthMessage, 1);
      msg->type = 0;
      msg->message = NULL;
      msg->response = NULL;
      msg->user_data = NULL;

      mv->messages[i] = msg;
    }
  mv->messages[size] = NULL;

  return mv;
}

/**
 * sbuild_auth_message_vector_copy:
 * @messagev: an #SbuildAuthMessageVector
 *
 * Copy a message vector.  This increases the reference count of
 * @messagev, so no actual copying takes place.
 *
 * Returns @messagev.
 */
SbuildAuthMessageVector *
sbuild_auth_message_vector_copy (SbuildAuthMessageVector *messagev)
{
  if (messagev == NULL)
    return NULL;

  g_atomic_int_inc (&messagev->refcount);

  return messagev;
}

/**
 * sbuild_auth_message_vector_free:
 * @messagev: an #SbuildAuthMessageVector
 *
 * Free a message vector.  This decreases the reference count of
 * @messagev, and finally frees it when the reference count reaches 0.
 */
void
sbuild_auth_message_vector_free (SbuildAuthMessageVector *messagev)
{
  if (messagev == NULL)
    return;

  gboolean is_zero = g_atomic_int_dec_and_test (&messagev->refcount);

  if (G_UNLIKELY (is_zero))
    {
      for (guint i = 0; messagev->messages[i] != NULL; ++i)
	sbuild_auth_message_free(messagev->messages[i]);
      g_free(messagev->messages);
      g_free(messagev);
    }
}

/*
 * Local Variables:
 * mode:C
 * End:
 */
