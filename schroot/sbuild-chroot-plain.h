/* sbuild-chroot-plain - sbuild simple chroot object
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

#ifndef SBUILD_CHROOT_PLAIN_H
#define SBUILD_CHROOT_PLAIN_H

#include <glib.h>
#include <glib/gprintf.h>
#include <glib-object.h>

#include "sbuild-chroot.h"

#define SBUILD_TYPE_CHROOT_PLAIN		  (sbuild_chroot_plain_get_type ())
#define SBUILD_CHROOT_PLAIN(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), SBUILD_TYPE_CHROOT_PLAIN, SbuildChrootPlain))
#define SBUILD_CHROOT_PLAIN_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), SBUILD_TYPE_CHROOT_PLAIN, SbuildChrootPlainClass))
#define SBUILD_IS_CHROOT_PLAIN(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SBUILD_TYPE_CHROOT_PLAIN))
#define SBUILD_IS_CHROOT_PLAIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SBUILD_TYPE_CHROOT_PLAIN))
#define SBUILD_CHROOT_PLAIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SBUILD_TYPE_CHROOT_PLAIN, SbuildChrootPlainClass))

typedef struct _SbuildChrootPlain SbuildChrootPlain;
typedef struct _SbuildChrootPlainClass SbuildChrootPlainClass;

struct _SbuildChrootPlain
{
  SbuildChroot  parent;
  gchar        *location;
};

struct _SbuildChrootPlainClass
{
  SbuildChrootClass parent;
};


GType
sbuild_chroot_plain_get_type (void);

const char *
sbuild_chroot_plain_get_location (const SbuildChrootPlain *restrict chroot);

void
sbuild_chroot_plain_set_location (SbuildChrootPlain *chroot,
				  const char   *location);

#endif /* SBUILD_CHROOT_PLAIN_H */

/*
 * Local Variables:
 * mode:C
 * End:
 */
