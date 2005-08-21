/* sbuild-auth-conv-tty - sbuild auth terminal conversation object
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

#ifndef SBUILD_AUTH_CONV_TTY_H
#define SBUILD_AUTH_CONV_TTY_H

#include <sys/types.h>
#include <sys/wait.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#include <security/pam_appl.h>
#include <security/pam_misc.h>

#include <glib.h>
#include <glib/gprintf.h>
#include <glib-object.h>

#include "sbuild-auth-conv.h"
#include "sbuild-config.h"

#define SBUILD_TYPE_AUTH_CONV_TTY            (sbuild_auth_conv_tty_get_type ())
#define SBUILD_AUTH_CONV_TTY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SBUILD_TYPE_AUTH_CONV_TTY, SbuildAuthConvTty))
#define SBUILD_AUTH_CONV_TTY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  SBUILD_TYPE_AUTH_CONV_TTY, SbuildAuthConvTtyClass))
#define SBUILD_IS_AUTH_CONV_TTY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SBUILD_TYPE_AUTH_CONV_TTY))
#define SBUILD_IS_AUTH_CONV_TTY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  SBUILD_TYPE_AUTH_CONV_TTY))
#define SBUILD_AUTH_CONV_TTY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  SBUILD_TYPE_AUTH_CONV_TTY, SbuildAuthConvTtyClass))

typedef struct _SbuildAuthConvTty SbuildAuthConvTty;
typedef struct _SbuildAuthConvTtyClass SbuildAuthConvTtyClass;

struct _SbuildAuthConvTty
{
  GObject parent;
  time_t  warning_timeout;
  time_t  fatal_timeout;
  time_t  start_time;
};

struct _SbuildAuthConvTtyClass
{
  GObjectClass parent;
};


GType
sbuild_auth_conv_tty_get_type (void);

SbuildAuthConvTty *
sbuild_auth_conv_tty_new (void);

#endif /* SBUILD_AUTH_CONV_TTY_H */

/*
 * Local Variables:
 * mode:C
 * End:
 */
