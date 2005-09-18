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

/**
 * SECTION:sbuild-chroot-lvm-snapshot
 * @short_description: chroot lvm snapshot object
 * @title: SbuildChrootLvmSnapshot
 *
 * This object represents a chroot stored on an LVM logical volume
 * (LV).  A snapshot LV will be created and mounted on demand.
 */

#include <config.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "sbuild-chroot-lvm-snapshot.h"

enum
{
  PROP_0,
  PROP_SNAPSHOT_OPTIONS
};

static GObjectClass *parent_class;

G_DEFINE_TYPE(SbuildChrootLvmSnapshot, sbuild_chroot_lvm_snapshot, SBUILD_TYPE_CHROOT_BLOCK_DEVICE)

/**
 * sbuild_chroot_lvm_snapshot_get_snapshot_options:
 * @chroot: an #SbuildChrootLvmSnapshot
 *
 * Get the logical volume snapshot options, used by lvcreate.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const char *
sbuild_chroot_lvm_snapshot_get_snapshot_options (const SbuildChrootLvmSnapshot *restrict chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT_LVM_SNAPSHOT(chroot), NULL);

  return chroot->snapshot_options;
}

/**
 * sbuild_chroot_lvm_snapshot_set_snapshot_options:
 * @chroot: an #SbuildChrootLvmSnapshot.
 * @snapshot_options: the snapshot options to set.
 *
 * Set the logical volume snapshot options, used by lvcreate.
 */
void
sbuild_chroot_lvm_snapshot_set_snapshot_options (SbuildChrootLvmSnapshot *chroot,
						 const char              *snapshot_options)
{
  g_return_if_fail(SBUILD_IS_CHROOT_LVM_SNAPSHOT(chroot));

  if (chroot->snapshot_options)
    {
      g_free(chroot->snapshot_options);
    }
  chroot->snapshot_options = g_strdup(snapshot_options);
  g_object_notify(G_OBJECT(chroot), "lvm-snapshot-options");
}

static void
sbuild_chroot_lvm_snapshot_print_details (SbuildChrootLvmSnapshot *chroot,
					  FILE                    *file)
{
  g_return_if_fail(SBUILD_IS_CHROOT_LVM_SNAPSHOT(chroot));

  SbuildChrootClass *klass = SBUILD_CHROOT_CLASS(parent_class);
  if (klass->print_details)
    klass->print_details(SBUILD_CHROOT(chroot), file);

  g_fprintf(file, _("LVM Snapshot Options: %s\n"), chroot->snapshot_options);
}

static void
sbuild_chroot_lvm_snapshot_print_config (SbuildChrootLvmSnapshot *chroot,
					 FILE                    *file)
{
  g_return_if_fail(SBUILD_IS_CHROOT_LVM_SNAPSHOT(chroot));

  SbuildChrootClass *klass = SBUILD_CHROOT_CLASS(parent_class);
  if (klass->print_details)
    klass->print_details(SBUILD_CHROOT(chroot), file);

  g_fprintf(file, _("lvm-snapshot-options=%s\n"), chroot->snapshot_options);
}

void sbuild_chroot_lvm_snapshot_setup (SbuildChrootLvmSnapshot  *chroot,
				       GList                   **env)
{
  g_return_if_fail(SBUILD_IS_CHROOT_LVM_SNAPSHOT(chroot));
  g_return_if_fail(env != NULL);

  SbuildChrootClass *klass = SBUILD_CHROOT_CLASS(parent_class);
  if (klass->setup)
    klass->setup(SBUILD_CHROOT(chroot), env);

  *env = g_list_append(*env,
		       g_strdup_printf("CHROOT_LVM_SNAPSHOT_OPTIONS=%s",
				       chroot->snapshot_options));
}

static const gchar *
sbuild_chroot_lvm_snapshot_get_chroot_type (const SbuildChrootLvmSnapshot *chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT_LVM_SNAPSHOT(chroot), NULL);

  return "lvm-snapshot";
}

static void
sbuild_chroot_lvm_snapshot_init (SbuildChrootLvmSnapshot *chroot)
{
  g_return_if_fail(SBUILD_IS_CHROOT_LVM_SNAPSHOT(chroot));

  chroot->snapshot_options = NULL;
}

static void
sbuild_chroot_lvm_snapshot_finalize (SbuildChrootLvmSnapshot *chroot)
{
  g_return_if_fail(SBUILD_IS_CHROOT_LVM_SNAPSHOT(chroot));

  if (chroot->snapshot_options)
    {
      g_free (chroot->snapshot_options);
      chroot->snapshot_options = NULL;
    }

  if (parent_class->finalize)
    parent_class->finalize(G_OBJECT(chroot));
}

static void
sbuild_chroot_lvm_snapshot_set_property (GObject      *object,
					 guint         param_id,
					 const GValue *value,
					 GParamSpec   *pspec)
{
  SbuildChrootLvmSnapshot *chroot;

  g_return_if_fail (object != NULL);
  g_return_if_fail (SBUILD_IS_CHROOT_LVM_SNAPSHOT (object));

  chroot = SBUILD_CHROOT_LVM_SNAPSHOT(object);

  switch (param_id)
    {
    case PROP_SNAPSHOT_OPTIONS:
      sbuild_chroot_lvm_snapshot_set_snapshot_options(chroot,
						   g_value_get_string(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
sbuild_chroot_lvm_snapshot_get_property (GObject    *object,
					 guint       param_id,
					 GValue     *value,
					 GParamSpec *pspec)
{
  SbuildChrootLvmSnapshot *chroot;

  g_return_if_fail (object != NULL);
  g_return_if_fail (SBUILD_IS_CHROOT_LVM_SNAPSHOT (object));

  chroot = SBUILD_CHROOT_LVM_SNAPSHOT(object);

  switch (param_id)
    {
    case PROP_SNAPSHOT_OPTIONS:
      g_value_set_string(value, chroot->snapshot_options);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
sbuild_chroot_lvm_snapshot_class_init (SbuildChrootLvmSnapshotClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  SbuildChrootClass *chroot_class = SBUILD_CHROOT_CLASS (klass);
  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = (GObjectFinalizeFunc) sbuild_chroot_lvm_snapshot_finalize;
  gobject_class->set_property = (GObjectSetPropertyFunc) sbuild_chroot_lvm_snapshot_set_property;
  gobject_class->get_property = (GObjectGetPropertyFunc) sbuild_chroot_lvm_snapshot_get_property;

  chroot_class->print_details = (SbuildChrootPrintDetailsFunc)
    sbuild_chroot_lvm_snapshot_print_details;
  chroot_class->print_config = (SbuildChrootPrintConfigFunc)
    sbuild_chroot_lvm_snapshot_print_config;
  chroot_class->setup = (SbuildChrootSetupFunc)
    sbuild_chroot_lvm_snapshot_setup;
  chroot_class->get_chroot_type = (SbuildChrootGetChrootTypeFunc)
    sbuild_chroot_lvm_snapshot_get_chroot_type;

  g_object_class_install_property
    (gobject_class,
     PROP_SNAPSHOT_OPTIONS,
     g_param_spec_string ("lvm-snapshot-options", "LVM Snapshot Options",
			  "The LVM snapshot options for lvcreate",
			  "",
			  (G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT)));
}

/*
 * Local Variables:
 * mode:C
 * End:
 */
