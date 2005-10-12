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

/**
 * SECTION:sbuild-chroot-block-device
 * @short_description: chroot block device object
 * @title: SbuildChrootBlockDevice
 *
 * This object represents a chroot stored on an unmounted block
 * device.  The device will be mounted on demand.
 */

#include <config.h>

#define _GNU_SOURCE
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "sbuild-chroot-block-device.h"

enum
{
  PROP_0,
  PROP_DEVICE,
  PROP_MOUNT_OPTIONS
};

static GObjectClass *parent_class;

G_DEFINE_TYPE(SbuildChrootBlockDevice, sbuild_chroot_block_device, SBUILD_TYPE_CHROOT)


/**
 * sbuild_chroot_block_device_get_device:
 * @chroot: an #SbuildChrootBlockDevice
 *
 * Get the block device of the chroot.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const char *
sbuild_chroot_block_device_get_device (const SbuildChrootBlockDevice *restrict chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT_BLOCK_DEVICE(chroot), NULL);

  return chroot->device;
}

/**
 * sbuild_chroot_block_device_set_device:
 * @chroot: an #SbuildChrootBlockDevice.
 * @device: the device to set.
 *
 * Set the block device of a chroot.  This is the "source" device.  It
 * may be the case that the real device is different (for example, an
 * LVM snapshot PV), but by default will be the device to mount.
 */
void
sbuild_chroot_block_device_set_device (SbuildChrootBlockDevice *chroot,
				       const char              *device)
{
  g_return_if_fail(SBUILD_IS_CHROOT_BLOCK_DEVICE(chroot));

  if (chroot->device)
    {
      g_free(chroot->device);
    }
  chroot->device = g_strdup(device);
  g_object_notify(G_OBJECT(chroot), "device");
}

/**
 * sbuild_chroot_block_device_set_mount_device:
 * @chroot: an #SbuildChrootBlockDevice.
 *
 * Set the mount block device of a chroot.  In this case, it is bound
 * to the value of the "device" property.  In derived classes, the two
 * may differ.
 */
static void
sbuild_chroot_block_device_set_mount_device (SbuildChrootBlockDevice *chroot)
{
  g_return_if_fail(SBUILD_IS_CHROOT_BLOCK_DEVICE(chroot));

  sbuild_chroot_set_mount_device(SBUILD_CHROOT(chroot), chroot->device);
}

/**
 * sbuild_chroot_block_device_get_mount_options:
 * @chroot: an #SbuildChrootBlockDevice
 *
 * Get the filesystem mount_options of the chroot block device.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const char *
sbuild_chroot_block_device_get_mount_options (const SbuildChrootBlockDevice *restrict chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT_BLOCK_DEVICE(chroot), NULL);

  return chroot->mount_options;
}

/**
 * sbuild_chroot_block_device_set_mount_options:
 * @chroot: an #SbuildChrootBlockDevice.
 * @mount_options: the device to set.
 *
 * Set the filesystem mount_options of a chroot block device.  These
 * will be passed to mount(8) when mounting the device.
 */
void
sbuild_chroot_block_device_set_mount_options (SbuildChrootBlockDevice *chroot,
					      const char              *mount_options)
{
  g_return_if_fail(SBUILD_IS_CHROOT_BLOCK_DEVICE(chroot));

  if (chroot->mount_options)
    {
      g_free(chroot->mount_options);
    }
  chroot->mount_options = g_strdup(mount_options);
  g_object_notify(G_OBJECT(chroot), "mount-options");
}

static void
sbuild_chroot_block_device_print_details (SbuildChrootBlockDevice *chroot,
					  FILE                    *file)
{
  g_return_if_fail(SBUILD_IS_CHROOT_BLOCK_DEVICE(chroot));

  if (chroot->device)
    g_fprintf(file, _("  %-22s%s\n"), _("Device"), chroot->device);
  if (chroot->mount_options)
    g_fprintf(file, _("  %-22s%s\n"), _("Mount Options"), chroot->mount_options);
}

static void
sbuild_chroot_block_device_print_config (SbuildChrootBlockDevice *chroot,
					 FILE                    *file)
{
  g_return_if_fail(SBUILD_IS_CHROOT_BLOCK_DEVICE(chroot));

  g_fprintf(file, _("device=%s\n"), (chroot->device) ? chroot->device : "");
  g_fprintf(file, _("mount-options=%s\n"),
	    (chroot->mount_options) ? chroot->mount_options : "");
}

void sbuild_chroot_block_device_setup_env (SbuildChrootBlockDevice  *chroot,
					   GList                   **env)
{
  g_return_if_fail(SBUILD_IS_CHROOT_BLOCK_DEVICE(chroot));
  g_return_if_fail(env != NULL);

  *env = g_list_append(*env,
		       g_strdup_printf("CHROOT_DEVICE=%s",
				       (chroot->device) ? chroot->device : ""));
  *env = g_list_append(*env,
		       g_strdup_printf("CHROOT_MOUNT_OPTIONS=%s",
				       (chroot->mount_options) ?
				       chroot->mount_options : ""));
}

static const gchar *
sbuild_chroot_block_device_get_chroot_type (const SbuildChrootBlockDevice *chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT_BLOCK_DEVICE(chroot), NULL);

  return "block-device";
}

static gboolean
sbuild_chroot_block_device_setup_lock (const SbuildChrootBlockDevice *chroot,
				       SbuildChrootSetupType          type,
				       gboolean                       lock)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT_BLOCK_DEVICE(chroot), FALSE);

  struct stat statbuf;

  /* TODO: Use liblockdev. */

  if (stat(chroot->device, &statbuf) == -1)
    {
      g_printerr(_("%s chroot: failed to stat device %s: %s\n"),
		 sbuild_chroot_get_name(SBUILD_CHROOT(chroot)),
		 chroot->device, g_strerror(errno));
      return FALSE;
    }
  if (!S_ISBLK(statbuf.st_mode))
    {
      g_printerr(_("%s chroot: %s is not a block device\n"),
		 sbuild_chroot_get_name(SBUILD_CHROOT(chroot)),
		 chroot->device);
      return FALSE;
    }

/*   return g_strdup_printf("block-%llu-%llu\n", */
/* 			 (unsigned long long) gnu_dev_major(statbuf.st_rdev), */
/* 			 (unsigned long long) gnu_dev_major(statbuf.st_rdev)); */

  return TRUE;
}

static SbuildChrootSessionFlags
sbuild_chroot_block_device_get_session_flags (const SbuildChrootBlockDevice *chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT_BLOCK_DEVICE(chroot), 0);

  return SBUILD_CHROOT_SESSION_AUTOCREATE;
}

static void
sbuild_chroot_block_device_init (SbuildChrootBlockDevice *chroot)
{
  g_return_if_fail(SBUILD_IS_CHROOT_BLOCK_DEVICE(chroot));

  chroot->device = NULL;
  chroot->mount_options = NULL;

  g_signal_connect(G_OBJECT(chroot), "notify::device",
		   G_CALLBACK(sbuild_chroot_block_device_set_mount_device), NULL);
}

static void
sbuild_chroot_block_device_finalize (SbuildChrootBlockDevice *chroot)
{
  g_return_if_fail(SBUILD_IS_CHROOT_BLOCK_DEVICE(chroot));

  if (chroot->device)
    {
      g_free (chroot->device);
      chroot->device = NULL;
    }
  if (chroot->mount_options)
    {
      g_free (chroot->mount_options);
      chroot->mount_options = NULL;
    }

  if (parent_class->finalize)
    parent_class->finalize(G_OBJECT(chroot));
}

static void
sbuild_chroot_block_device_set_property (GObject      *object,
					 guint         param_id,
					 const GValue *value,
					 GParamSpec   *pspec)
{
  SbuildChrootBlockDevice *chroot;

  g_return_if_fail (object != NULL);
  g_return_if_fail (SBUILD_IS_CHROOT_BLOCK_DEVICE (object));

  chroot = SBUILD_CHROOT_BLOCK_DEVICE(object);

  switch (param_id)
    {
    case PROP_DEVICE:
      sbuild_chroot_block_device_set_device(chroot, g_value_get_string(value));
      break;
    case PROP_MOUNT_OPTIONS:
      sbuild_chroot_block_device_set_mount_options(chroot,
						   g_value_get_string(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
sbuild_chroot_block_device_get_property (GObject    *object,
					 guint       param_id,
					 GValue     *value,
					 GParamSpec *pspec)
{
  SbuildChrootBlockDevice *chroot;

  g_return_if_fail (object != NULL);
  g_return_if_fail (SBUILD_IS_CHROOT_BLOCK_DEVICE (object));

  chroot = SBUILD_CHROOT_BLOCK_DEVICE(object);

  switch (param_id)
    {
    case PROP_DEVICE:
      g_value_set_string(value, chroot->device);
      break;
    case PROP_MOUNT_OPTIONS:
      g_value_set_string(value, chroot->mount_options);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
sbuild_chroot_block_device_class_init (SbuildChrootBlockDeviceClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  SbuildChrootClass *chroot_class = SBUILD_CHROOT_CLASS (klass);
  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = (GObjectFinalizeFunc) sbuild_chroot_block_device_finalize;
  gobject_class->set_property = (GObjectSetPropertyFunc) sbuild_chroot_block_device_set_property;
  gobject_class->get_property = (GObjectGetPropertyFunc) sbuild_chroot_block_device_get_property;

  chroot_class->print_details = (SbuildChrootPrintDetailsFunc)
    sbuild_chroot_block_device_print_details;
  chroot_class->print_config = (SbuildChrootPrintConfigFunc)
    sbuild_chroot_block_device_print_config;
  chroot_class->setup_env = (SbuildChrootSetupEnvFunc)
    sbuild_chroot_block_device_setup_env;
  chroot_class->get_chroot_type = (SbuildChrootGetChrootTypeFunc)
    sbuild_chroot_block_device_get_chroot_type;
  chroot_class->setup_lock = (SbuildChrootSetupLockFunc)
    sbuild_chroot_block_device_setup_lock;
  chroot_class->get_session_flags = (SbuildChrootGetSessionFlagsFunc)
    sbuild_chroot_block_device_get_session_flags;

  g_object_class_install_property
    (gobject_class,
     PROP_DEVICE,
     g_param_spec_string ("device", "Device",
			  "The block device name of the chroot",
			  "",
			  (G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT)));

  g_object_class_install_property
    (gobject_class,
     PROP_MOUNT_OPTIONS,
     g_param_spec_string ("mount-options", "Mount Options",
			  "The filesystem mount options for the chroot block device",
			  "",
			  (G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT)));
}

/*
 * Local Variables:
 * mode:C
 * End:
 */
