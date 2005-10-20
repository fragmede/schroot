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

#define _GNU_SOURCE
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "sbuild-chroot-lvm-snapshot.h"
#include "sbuild-lock.h"

static void
sbuild_chroot_lvm_snapshot_set_mount_device (SbuildChrootLvmSnapshot *chroot);

enum
{
  PROP_0,
  PROP_SNAPSHOT_DEVICE,
  PROP_SNAPSHOT_OPTIONS
};

static GObjectClass *parent_class;

G_DEFINE_TYPE(SbuildChrootLvmSnapshot, sbuild_chroot_lvm_snapshot, SBUILD_TYPE_CHROOT_BLOCK_DEVICE)

/**
 * sbuild_chroot_lvm_snapshot_get_snapshot_device:
 * @chroot: an #SbuildChrootLvmSnapshot
 *
 * Get the logical volume snapshot device name, used by lvcreate.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const char *
sbuild_chroot_lvm_snapshot_get_snapshot_device (const SbuildChrootLvmSnapshot *restrict chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT_LVM_SNAPSHOT(chroot), NULL);

  return chroot->snapshot_device;
}

/**
 * sbuild_chroot_lvm_snapshot_set_snapshot_device:
 * @chroot: an #SbuildChrootLvmSnapshot.
 * @snapshot_device: the snapshot device to set.
 *
 * Set the logical volume snapshot device name, used by lvcreate.
 */
void
sbuild_chroot_lvm_snapshot_set_snapshot_device (SbuildChrootLvmSnapshot *chroot,
						const char              *snapshot_device)
{
  g_return_if_fail(SBUILD_IS_CHROOT_LVM_SNAPSHOT(chroot));

  if (chroot->snapshot_device)
    {
      g_free(chroot->snapshot_device);
    }
  chroot->snapshot_device = g_strdup(snapshot_device);
  sbuild_chroot_lvm_snapshot_set_mount_device(chroot);
  g_object_notify(G_OBJECT(chroot), "lvm-snapshot-device");
}

/**
 * sbuild_chroot_lvm_snapshot_set_mount_device:
 * @chroot: an #SbuildChrootLvmSnapshot.
 *
 * Set the mount block device of a chroot.  In this case, it is bound
 * to the value of the "snapshot-device" property.  In derived
 * classes, the two may differ.
 */
static void
sbuild_chroot_lvm_snapshot_set_mount_device (SbuildChrootLvmSnapshot *chroot)
{
  g_return_if_fail(SBUILD_IS_CHROOT_LVM_SNAPSHOT(chroot));

  sbuild_chroot_set_mount_device(SBUILD_CHROOT(chroot), chroot->snapshot_device);
}

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

  if (chroot->snapshot_device)
    g_fprintf(file, "  %-22s%s\n", _("LVM Snapshot Device"),
	      chroot->snapshot_device);
  if (chroot->snapshot_options)
    g_fprintf(file, "  %-22s%s\n", _("LVM Snapshot Options"),
	      chroot->snapshot_options);
}

static void
sbuild_chroot_lvm_snapshot_print_config (SbuildChrootLvmSnapshot *chroot,
					 FILE                    *file)
{
  g_return_if_fail(SBUILD_IS_CHROOT_LVM_SNAPSHOT(chroot));

  SbuildChrootClass *klass = SBUILD_CHROOT_CLASS(parent_class);
  if (klass->print_details)
    klass->print_details(SBUILD_CHROOT(chroot), file);

  if (chroot->snapshot_device)
    g_fprintf(file, _("lvm-snapshot-device=%s\n"),
	      (chroot->snapshot_device) ? chroot->snapshot_device : "");
  if (chroot->snapshot_options)
    g_fprintf(file, _("lvm-snapshot-options=%s\n"),
	      (chroot->snapshot_options) ? chroot->snapshot_options : "");
}

void sbuild_chroot_lvm_snapshot_setup_env (SbuildChrootLvmSnapshot  *chroot,
					   GList                   **env)
{
  g_return_if_fail(SBUILD_IS_CHROOT_LVM_SNAPSHOT(chroot));
  g_return_if_fail(env != NULL);

  SbuildChrootClass *klass = SBUILD_CHROOT_CLASS(parent_class);
  if (klass->setup_env)
    klass->setup_env(SBUILD_CHROOT(chroot), env);

  gchar *name = g_path_get_basename(chroot->snapshot_device);
  *env = g_list_append(*env,
		       g_strdup_printf("CHROOT_LVM_SNAPSHOT_NAME=%s",
				       name));
  g_free(name);

  *env = g_list_append(*env,
		       g_strdup_printf("CHROOT_LVM_SNAPSHOT_DEVICE=%s",
				       (chroot->snapshot_device) ?
				       chroot->snapshot_device : ""));
  *env = g_list_append(*env,
		       g_strdup_printf("CHROOT_LVM_SNAPSHOT_OPTIONS=%s",
				       (chroot->snapshot_options) ?
				       chroot->snapshot_options : ""));
}

static const gchar *
sbuild_chroot_lvm_snapshot_get_chroot_type (const SbuildChrootLvmSnapshot *chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT_LVM_SNAPSHOT(chroot), NULL);

  return "lvm-snapshot";
}

static gboolean
sbuild_chroot_lvm_snapshot_setup_lock (SbuildChrootLvmSnapshot *chroot,
				       SbuildChrootSetupType    type,
				       gboolean                 lock)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT_LVM_SNAPSHOT(chroot), FALSE);

  const char *device;
  struct stat statbuf;

  /* Lock is removed by setup script on setup stop.  Unlocking here
     would fail: the LVM snapshot device no longer exists. */
  if (!(type == SBUILD_CHROOT_SETUP_STOP && lock == FALSE))
    {
      if (type == SBUILD_CHROOT_SETUP_START)
	device = sbuild_chroot_block_device_get_device(SBUILD_CHROOT_BLOCK_DEVICE(chroot));
      else
	device = chroot->snapshot_device;

      if (device == NULL)
	{
	  g_error(_("%s chroot: device name not set\n"));
	  return FALSE;
	}
      if (stat(device, &statbuf) == -1)
	{
	  g_printerr(_("%s chroot: failed to stat device %s: %s\n"),
		     sbuild_chroot_get_name(SBUILD_CHROOT(chroot)),
		     device, g_strerror(errno));
	  return FALSE;
	}
      if (!S_ISBLK(statbuf.st_mode))
	{
	  g_printerr(_("%s chroot: %s is not a block device\n"),
		     sbuild_chroot_get_name(SBUILD_CHROOT(chroot)),
		     device);
	  return FALSE;
	}

      /* Lock is preserved while running a command. */
      if (type == SBUILD_CHROOT_RUN_START && lock == FALSE ||
	  type == SBUILD_CHROOT_RUN_STOP && lock == TRUE)
	return TRUE;

      GError *error = NULL;
      if (lock)
	{
	  if (sbuild_lock_set_device_lock(device,
					  SBUILD_LOCK_EXCLUSIVE,
					  15,
					  &error) == FALSE)
	    {
	      g_printerr(_("%s: failed to lock device: %s\n"),
			 device, error->message);
	      return FALSE;
	    }
	}
      else
	{
	  if (sbuild_lock_unset_device_lock(device,
					    &error) == FALSE)
	    {
	      g_printerr(_("%s: failed to unlock device: %s\n"),
			 device, error->message);
	      return FALSE;
	    }
	}
    }

  /* Create or unlink session information. */
  if (type == SBUILD_CHROOT_SETUP_START && lock == TRUE ||
      type == SBUILD_CHROOT_SETUP_STOP && lock == FALSE)
    {
      char *file = g_strconcat(SCHROOT_SESSION_DIR, "/",
			       sbuild_chroot_get_name(SBUILD_CHROOT(chroot)),
			       NULL);
      int fd;

      if (type == SBUILD_CHROOT_SETUP_START)
	fd = open(file, O_CREAT|O_EXCL|O_WRONLY, 0664);
      else
	fd = open(file, O_WRONLY);
      if (fd < 0)
	{
	  g_printerr(_("%s: failed to create session file: %s\n"), file, g_strerror(errno));
	  g_free(file);
	  return FALSE;
	}

      GError *lock_error = NULL;
      sbuild_lock_set_lock(fd, SBUILD_LOCK_EXCLUSIVE, 2, &lock_error);
      if (lock_error)
	{
	  g_printerr(_("%s: lock acquisition failure: %s\n"), file, lock_error->message);
	  g_free(file);
	  return FALSE;
	}

      FILE *sess_file = fdopen(fd, "w");
      if (file == NULL)
	{
	  g_printerr(_("%s: failed to create FILE from fd: %s\n"), file, g_strerror(errno));
	  g_free(file);
	  return FALSE;
	}

      if (type == SBUILD_CHROOT_SETUP_START)
	sbuild_chroot_print_details(SBUILD_CHROOT(chroot), sess_file);
      else
	{
	  if (unlink(file) != 0)
	    {
	      g_printerr(_("%s: failed to unlink session file: %s\n"),
			 file, g_strerror(errno));
	      g_free(file);
	      return FALSE;
	    }
	}

      GError *unlock_error = NULL;
      sbuild_lock_unset_lock(fd, &unlock_error);
      if (unlock_error)
	{
	  g_printerr(_("%s: lock discard failure: %s\n"), file, unlock_error->message);
	  g_free(file);
	  return FALSE;
	}

      if (fclose(sess_file) != 0)
	{
	  g_printerr(_("%s: failed to close session file: %s\n"), file, g_strerror(errno));
	  g_free(file);
	  return FALSE;
	}

      g_free(file);
    }

  return TRUE;
}

static SbuildChrootSessionFlags
sbuild_chroot_lvm_snapshot_get_session_flags (const SbuildChrootLvmSnapshot *chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT_LVM_SNAPSHOT(chroot), 0);

  return SBUILD_CHROOT_SESSION_PERSISTENT | SBUILD_CHROOT_SESSION_MULTIUSER;
}

static void
sbuild_chroot_lvm_snapshot_init (SbuildChrootLvmSnapshot *chroot)
{
  g_return_if_fail(SBUILD_IS_CHROOT_LVM_SNAPSHOT(chroot));

  chroot->snapshot_device = NULL;
  chroot->snapshot_options = NULL;

  /* Override SbuildChrootBlockDevice::device closure as well. */
  g_signal_connect(G_OBJECT(chroot), "notify::device",
		   G_CALLBACK(sbuild_chroot_lvm_snapshot_set_mount_device), NULL);
  g_signal_connect(G_OBJECT(chroot), "notify::snapshot-device",
		   G_CALLBACK(sbuild_chroot_lvm_snapshot_set_mount_device), NULL);
}

static void
sbuild_chroot_lvm_snapshot_finalize (SbuildChrootLvmSnapshot *chroot)
{
  g_return_if_fail(SBUILD_IS_CHROOT_LVM_SNAPSHOT(chroot));

  if (chroot->snapshot_device)
    {
      g_free (chroot->snapshot_device);
      chroot->snapshot_device = NULL;
    }
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
    case PROP_SNAPSHOT_DEVICE:
      sbuild_chroot_lvm_snapshot_set_snapshot_device(chroot,
						     g_value_get_string(value));
      break;
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
    case PROP_SNAPSHOT_DEVICE:
      g_value_set_string(value, chroot->snapshot_device);
      break;
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
  chroot_class->setup_env = (SbuildChrootSetupEnvFunc)
    sbuild_chroot_lvm_snapshot_setup_env;
  chroot_class->get_chroot_type = (SbuildChrootGetChrootTypeFunc)
    sbuild_chroot_lvm_snapshot_get_chroot_type;
  chroot_class->setup_lock = (SbuildChrootSetupLockFunc)
    sbuild_chroot_lvm_snapshot_setup_lock;
  chroot_class->get_session_flags = (SbuildChrootGetSessionFlagsFunc)
    sbuild_chroot_lvm_snapshot_get_session_flags;

  g_object_class_install_property
    (gobject_class,
     PROP_SNAPSHOT_DEVICE,
     g_param_spec_string ("lvm-snapshot-device", "LVM Snapshot Device",
			  "The LVM snapshot device name for lvcreate",
			  "",
			  (G_PARAM_READABLE | G_PARAM_WRITABLE)));

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
