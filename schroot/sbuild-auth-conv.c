/* sbuild-auth-conv - sbuild auth conversation interface
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
 * SECTION:sbuild-auth-conv
 * @short_description: authentication conversation interface
 * @title: SbuildAuthConv
 *
 * SbuildAuthConv is an interface which should be implemented by
 * objects which handle interaction with the user during
 * authentication.
 *
 * This is a wrapper around the struct pam_conv PAM conversation
 * interface, and is used by #SbuildAuth when interacting with the
 * user during authentication.
 *
 * A simple implementation is provided in the form of
 * #SbuildAuthConvTty.  However, more complex implementations might
 * hook into the GLib main loop, or implement it in a #GtkWidget.
 *
 * The interface allows the setting of optional warning timeout and
 * fatal timeout values, which default to 0 (not enabled).  This is an
 * absolute time after which a warning is displayed or the
 * conversation ends with an error.
 */

#include <config.h>

#define _GNU_SOURCE
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <syslog.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "sbuild-auth-conv.h"
#include "sbuild-marshallers.h"
#include "sbuild-typebuiltins.h"

static gboolean
sbuild_auth_conv_conversation_accumulator (GSignalInvocationHint *ihint,
					   GValue                *return_accu,
					   const GValue          *handler_return,
					   gpointer               data);

/**
 * sbuild_auth_conv_get_warning_timeout:
 * @auth_conv: an #SbuildAuthConv.
 *
 * Get the warning timeout for @auth_conv.
 *
 * Returns the timeout.
 */
time_t sbuild_auth_conv_get_warning_timeout (SbuildAuthConv *auth_conv)
{
  return SBUILD_AUTH_CONV_GET_INTERFACE(auth_conv)->get_warning_timeout(auth_conv);
}

/**
 * sbuild_auth_conv_set_warning_timeout:
 * @auth_conv: an #SbuildAuthConv.
 * @timeout: the timeout to set.
 *
 * Set the warning timeout for @auth_conv.
 */
void sbuild_auth_conv_set_warning_timeout (SbuildAuthConv *auth_conv,
					   time_t          timeout)
{
  SBUILD_AUTH_CONV_GET_INTERFACE(auth_conv)->set_warning_timeout(auth_conv,
								 timeout);
}

/**
 * sbuild_auth_conv_get_fatal_timeout:
 * @auth_conv: an #SbuildAuthConv.
 *
 * Get the fatal timeout for @auth_conv.
 *
 * Returns the timeout.
 */
time_t sbuild_auth_conv_get_fatal_timeout (SbuildAuthConv *auth_conv)
{
  return SBUILD_AUTH_CONV_GET_INTERFACE(auth_conv)->get_fatal_timeout(auth_conv);
}

/**
 * sbuild_auth_conv_set_fatal_timeout:
 * @auth_conv: an #SbuildAuthConv.
 * @timeout: the timeout to set.
 *
 * Set the fatal timeout for @auth_conv.
 */
void sbuild_auth_conv_set_fatal_timeout (SbuildAuthConv *auth_conv,
					 time_t          timeout)
{
  SBUILD_AUTH_CONV_GET_INTERFACE(auth_conv)->set_fatal_timeout(auth_conv,
							       timeout);
}

/**
 * sbuild_auth_conv_conversation:
 * @auth_conv: an #SbuildAuthConv.
 * @num_messages: the number of messages
 * @messages: the messages to display to the user
 *
 * Hold a conversation with the user.
 *
 * Each of the messages detailed in @messages should be displayed to
 * the user, asking for input where required.  The type of message is
 * indicated in the type field of the #SbuildAuthMessage.  The
 * response field of the message vector #SbuildAuthMessage should be
 * filled in if input is required.  This will be automatically freed
 * on success or failure.  The user_data field of #SbuildAuthMessage
 * is for the use of #SbuildAuthConv implementations, and will be
 * otherwise ignored.
 *
 * Returns TRUE on success, FALSE on failure.
 */
gboolean sbuild_auth_conv_conversation (SbuildAuthConv          *auth_conv,
					guint                    num_messages,
					SbuildAuthMessageVector *messages)
{
  g_return_val_if_fail(SBUILD_IS_AUTH_CONV(auth_conv), FALSE);

  gboolean retval = FALSE;
  g_signal_emit_by_name(auth_conv, "conversation", num_messages, messages, &retval);

  return retval;
}

static void
sbuild_auth_conv_base_init (gpointer g_class)
{
  static gboolean initialised = FALSE;

  if (!initialised)
    {
      g_object_interface_install_property
	(g_class,
	 g_param_spec_int ("warning-timeout", "Warning Timeout",
			   "The time to wait until warning the user",
			   0, G_MAXINT, 0,
			   G_PARAM_READWRITE));

      g_object_interface_install_property
	(g_class,
	 g_param_spec_int ("fatal-timeout", "Fatal Timeout",
			   "The time to wait until aborting",
			   0, G_MAXINT, 0,
			   G_PARAM_READWRITE));

      g_signal_new ("conversation",
		    SBUILD_TYPE_AUTH_CONV,
		    G_SIGNAL_RUN_LAST,
		    G_STRUCT_OFFSET (SbuildAuthConvInterface, conversation),
		    sbuild_auth_conv_conversation_accumulator, NULL,
		    sbuild_cclosure_marshal_BOOLEAN__UINT_BOXED,
		    G_TYPE_BOOLEAN, 2, G_TYPE_UINT, SBUILD_TYPE_AUTH_MESSAGE_VECTOR);

      initialised = TRUE;
    }
}

/**
 * sbuild_auth_conv_conversation_accumulator:
 * @ihint: an invocation hint
 * @return_accu: a GValue to return to the caller
 * @handler_return: a GValue containing the return value of the last signal handler.
 * @data: user data
 *
 * Accumulate the results of "conversation" handlers.
 *
 * Returns FALSE to ensure only the first handler gets run.
 *
 */
static gboolean
sbuild_auth_conv_conversation_accumulator (GSignalInvocationHint *ihint,
					   GValue                *return_accu,
					   const GValue          *handler_return,
					   gpointer               data)
{
  gboolean signal_return = g_value_get_boolean(handler_return);
  g_value_set_boolean(return_accu, signal_return);


  return FALSE;
}

GType
sbuild_auth_conv_get_type (void)
{
  static GType type = 0;

  if (type == 0)
    {
      static const GTypeInfo info =
	{
	  sizeof (SbuildAuthConvInterface),
	  sbuild_auth_conv_base_init,   /* base_init */
	  NULL,   /* base_finalize */
	  NULL,   /* class_init */
	  NULL,   /* class_finalize */
	  NULL,   /* class_data */
	  0,
	  0,      /* n_preallocs */
	  NULL    /* instance_init */
	};

      type = g_type_register_static (G_TYPE_INTERFACE,
				     "SbuildAuthConv",
				     &info, 0);

      g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
    }

  return type;
}

/*
 * Local Variables:
 * mode:C
 * End:
 */
