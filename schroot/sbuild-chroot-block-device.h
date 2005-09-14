/* sbuild-chroot-block-device - sbuild chroot block device object
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

#ifndef SBUILD_CHROOT_BLOCK_DEVICE_H
#define SBUILD_CHROOT_BLOCK_DEVICE_H

#include <glib.h>
#include <glib/gprintf.h>
#include <glib-object.h>

#include "sbuild-chroot.h"

#define SBUILD_TYPE_CHROOT_BLOCK_DEVICE		  (sbuild_chroot_block_device_get_type ())
#define SBUILD_CHROOT_BLOCK_DEVICE(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), SBUILD_TYPE_CHROOT_BLOCK_DEVICE, SbuildChrootBlockDevice))
#define SBUILD_CHROOT_BLOCK_DEVICE_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), SBUILD_TYPE_CHROOT_BLOCK_DEVICE, SbuildChrootBlockDeviceClass))
#define SBUILD_IS_CHROOT_BLOCK_DEVICE(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SBUILD_TYPE_CHROOT_BLOCK_DEVICE))
#define SBUILD_IS_CHROOT_BLOCK_DEVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SBUILD_TYPE_CHROOT_BLOCK_DEVICE))
#define SBUILD_CHROOT_BLOCK_DEVICE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SBUILD_TYPE_CHROOT_BLOCK_DEVICE, SbuildChrootBlockDeviceClass))

typedef struct _SbuildChrootBlockDevice SbuildChrootBlockDevice;
typedef struct _SbuildChrootBlockDeviceClass SbuildChrootBlockDeviceClass;

struct _SbuildChrootBlockDevice
{
  SbuildChroot  parent;
  gchar        *device;
  gchar        *mount_options;
};

struct _SbuildChrootBlockDeviceClass
{
  SbuildChrootClass parent;
};


GType
sbuild_chroot_block_device_get_type (void);

const char *
sbuild_chroot_block_device_get_device (const SbuildChrootBlockDevice *restrict chroot);

void
sbuild_chroot_block_device_set_device (SbuildChrootBlockDevice *chroot,
				       const char              *device);

const char *
sbuild_chroot_block_device_get_mount_options
(const SbuildChrootBlockDevice *restrict chroot);

void
sbuild_chroot_block_device_set_mount_options
(SbuildChrootBlockDevice *chroot,
 const char              *device);

#endif /* SBUILD_CHROOT_BLOCK_DEVICE_H */

/*
 * Local Variables:
 * mode:C
 * End:
 */
