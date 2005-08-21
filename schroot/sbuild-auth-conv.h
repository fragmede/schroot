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

#ifndef SBUILD_AUTH_CONV_H
#define SBUILD_AUTH_CONV_H

#include <security/pam_appl.h>

#include <glib.h>
#include <glib/gprintf.h>
#include <glib-object.h>

#include "sbuild-auth-message.h"

#define SBUILD_TYPE_AUTH_CONV                (sbuild_auth_conv_get_type ())
#define SBUILD_AUTH_CONV(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj),     SBUILD_TYPE_AUTH_CONV, SbuildAuthConv))
#define SBUILD_IS_AUTH_CONV(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj),     SBUILD_TYPE_AUTH_CONV))
#define SBUILD_AUTH_CONV_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE ((inst), SBUILD_TYPE_AUTH_CONV, SbuildAuthConvInterface))

typedef struct _SbuildAuthConv SbuildAuthConv;
typedef struct _SbuildAuthConvInterface SbuildAuthConvInterface;

typedef time_t (*SbuildAuthConvGetWarningTimeoutFunc)(SbuildAuthConv *auth_conv);
typedef void (*SbuildAuthConvSetWarningTimeoutFunc)(SbuildAuthConv *auth_conv,
						    time_t          timeout);
typedef time_t (*SbuildAuthConvGetFatalTimeoutFunc)(SbuildAuthConv *auth_conv);
typedef void (*SbuildAuthConvSetFatalTimeoutFunc)(SbuildAuthConv *auth_conv,
						  time_t          timeout);
typedef gboolean (*SbuildAuthConvConversationFunc)(SbuildAuthConv          *auth_conv,
						   guint                    num_messages,
						   SbuildAuthMessageVector *messages);

struct _SbuildAuthConvInterface
{
  GTypeInterface                      parent;
  SbuildAuthConvGetWarningTimeoutFunc get_warning_timeout;
  SbuildAuthConvSetWarningTimeoutFunc set_warning_timeout;
  SbuildAuthConvGetFatalTimeoutFunc   get_fatal_timeout;
  SbuildAuthConvSetFatalTimeoutFunc   set_fatal_timeout;
  SbuildAuthConvConversationFunc      conversation;
};


GType
sbuild_auth_conv_get_type (void);

time_t sbuild_auth_conv_get_warning_timeout (SbuildAuthConv *auth_conv);

void sbuild_auth_conv_set_warning_timeout (SbuildAuthConv *auth_conv,
					   time_t          timeout);

time_t sbuild_auth_conv_get_fatal_timeout (SbuildAuthConv *auth_conv);

void sbuild_auth_conv_set_fatal_timeout (SbuildAuthConv *auth_conv,
					 time_t          timeout);

gboolean sbuild_auth_conv_conversation (SbuildAuthConv          *auth_conv,
					guint                    num_messages,
					SbuildAuthMessageVector *messages);

#endif /* SBUILD_AUTH_CONV_H */

/*
 * Local Variables:
 * mode:C
 * End:
 */
