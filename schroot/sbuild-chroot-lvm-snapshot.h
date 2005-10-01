/* sbuild-chroot-lvm-snapshot - sbuild chroot lvm snapshot object
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

#ifndef SBUILD_CHROOT_LVM_SNAPSHOT_H
#define SBUILD_CHROOT_LVM_SNAPSHOT_H

#include <glib.h>
#include <glib/gprintf.h>
#include <glib-object.h>

#include "sbuild-chroot-block-device.h"

#define SBUILD_TYPE_CHROOT_LVM_SNAPSHOT		  (sbuild_chroot_lvm_snapshot_get_type ())
#define SBUILD_CHROOT_LVM_SNAPSHOT(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), SBUILD_TYPE_CHROOT_LVM_SNAPSHOT, SbuildChrootLvmSnapshot))
#define SBUILD_CHROOT_LVM_SNAPSHOT_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), SBUILD_TYPE_CHROOT_LVM_SNAPSHOT, SbuildChrootLvmSnapshotClass))
#define SBUILD_IS_CHROOT_LVM_SNAPSHOT(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SBUILD_TYPE_CHROOT_LVM_SNAPSHOT))
#define SBUILD_IS_CHROOT_LVM_SNAPSHOT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SBUILD_TYPE_CHROOT_LVM_SNAPSHOT))
#define SBUILD_CHROOT_LVM_SNAPSHOT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SBUILD_TYPE_CHROOT_LVM_SNAPSHOT, SbuildChrootLvmSnapshotClass))

typedef struct _SbuildChrootLvmSnapshot SbuildChrootLvmSnapshot;
typedef struct _SbuildChrootLvmSnapshotClass SbuildChrootLvmSnapshotClass;

struct _SbuildChrootLvmSnapshot
{
  SbuildChrootBlockDevice  parent;
  gchar                   *snapshot_device;
  gchar                   *snapshot_options;
};

struct _SbuildChrootLvmSnapshotClass
{
  SbuildChrootBlockDeviceClass parent;
};


GType
sbuild_chroot_lvm_snapshot_get_type (void);

const char *
sbuild_chroot_lvm_snapshot_get_snapshot_device (const SbuildChrootLvmSnapshot *restrict chroot);

void
sbuild_chroot_lvm_snapshot_set_snapshot_device (SbuildChrootLvmSnapshot *chroot,
						const char              *snapshot_device);

const char *
sbuild_chroot_lvm_snapshot_get_snapshot_options (const SbuildChrootLvmSnapshot *restrict chroot);

void
sbuild_chroot_lvm_snapshot_set_snapshot_options (SbuildChrootLvmSnapshot *chroot,
						 const char              *snapshot_options);

#endif /* SBUILD_CHROOT_LVM_SNAPSHOT_H */

/*
 * Local Variables:
 * mode:C
 * End:
 */
