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

#include <glib.h>
#include <glib/gi18n.h>

#include "sbuild-chroot-block-device.h"

enum
{
  PROP_0,
  PROP_DEVICE
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
 * Set the block device of a chroot.
 */
void
sbuild_chroot_block_device_set_device (SbuildChrootBlockDevice *chroot,
				       const char   *device)
{
  g_return_if_fail(SBUILD_IS_CHROOT_BLOCK_DEVICE(chroot));

  if (chroot->device)
    {
      g_free(chroot->device);
    }
  chroot->device = g_strdup(device);
  g_object_notify(G_OBJECT(chroot), "device");
}

static void
sbuild_chroot_block_device_print_details (SbuildChrootBlockDevice *chroot,
					  FILE         *file)
{
  g_return_if_fail(SBUILD_IS_CHROOT_BLOCK_DEVICE(chroot));

  g_fprintf(file, _("Device: %s\n"), chroot->device);
}

void sbuild_chroot_block_device_setup (SbuildChrootBlockDevice  *chroot,
			  GList        **env)
{
  g_return_if_fail(SBUILD_IS_CHROOT_BLOCK_DEVICE(chroot));
  g_return_if_fail(env != NULL);

  *env = g_list_append(*env,
		       g_strdup_printf("CHROOT_DEVICE=%s",
				       chroot->device));
}

static const gchar *
sbuild_chroot_block_device_get_chroot_type (const SbuildChrootBlockDevice *chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT_BLOCK_DEVICE(chroot), NULL);

  return "block-device";
}

static void
sbuild_chroot_block_device_init (SbuildChrootBlockDevice *chroot)
{
  g_return_if_fail(SBUILD_IS_CHROOT_BLOCK_DEVICE(chroot));

  chroot->device= NULL;
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
  chroot_class->setup = (SbuildChrootSetupFunc)
    sbuild_chroot_block_device_setup;
  chroot_class->get_chroot_type = (SbuildChrootGetChrootTypeFunc)
    sbuild_chroot_block_device_get_chroot_type;

  g_object_class_install_property
    (gobject_class,
     PROP_DEVICE,
     g_param_spec_string ("device", "Device",
			  "The block device name of the chroot",
			  "",
			  (G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT)));
}

/*
 * Local Variables:
 * mode:C
 * End:
 */
