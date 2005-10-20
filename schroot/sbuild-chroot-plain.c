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

/**
 * SECTION:sbuild-chroot-plain
 * @short_description: simple chroot object
 * @title: SbuildChrootPlain
 *
 * This object represents a chroot located on a mounted filesystem.
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

#include "sbuild-chroot-plain.h"

enum
{
  PROP_0,
  PROP_LOCATION
};

static GObjectClass *parent_class;

G_DEFINE_TYPE(SbuildChrootPlain, sbuild_chroot_plain, SBUILD_TYPE_CHROOT)


/**
 * sbuild_chroot_plain_get_location:
 * @chroot: an #SbuildChrootPlain
 *
 * Get the directory location of the chroot.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const char *
sbuild_chroot_plain_get_location (const SbuildChrootPlain *restrict chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT_PLAIN(chroot), NULL);

  return chroot->location;
}

/**
 * sbuild_chroot_plain_set_location:
 * @chroot: an #SbuildChrootPlain.
 * @location: the location to set.
 *
 * Set the directory location of a chroot.
 */
void
sbuild_chroot_plain_set_location (SbuildChrootPlain *chroot,
				  const char        *location)
{
  g_return_if_fail(SBUILD_IS_CHROOT_PLAIN(chroot));

  if (chroot->location)
    {
      g_free(chroot->location);
    }
  chroot->location = g_strdup(location);
  g_object_notify(G_OBJECT(chroot), "location");
}

/**
 * sbuild_chroot_plain_set_mount_location:
 * @chroot: an #SbuildChrootPlain.
 *
 * Set the mounted directory location of a chroot.  In this case, it
 * is bound to the value of the "location" property.
 */
static void
sbuild_chroot_plain_set_mount_location (SbuildChrootPlain *chroot)
{
  g_return_if_fail(SBUILD_IS_CHROOT_PLAIN(chroot));

  sbuild_chroot_set_mount_location(SBUILD_CHROOT(chroot), chroot->location);
}

static void
sbuild_chroot_plain_print_details (SbuildChrootPlain *chroot,
				   FILE              *file)
{
  g_return_if_fail(SBUILD_IS_CHROOT_PLAIN(chroot));

  if (chroot->location)
    g_fprintf(file, "  %-22s%s\n", _("Location"), chroot->location);
}

static void
sbuild_chroot_plain_print_config (SbuildChrootPlain *chroot,
				  FILE              *file)
{
  g_return_if_fail(SBUILD_IS_CHROOT_PLAIN(chroot));

  g_fprintf(file, _("location=%s\n"), (chroot->location) ? chroot->location : "");
}

void sbuild_chroot_plain_setup_env (SbuildChrootPlain  *chroot,
				    GList        **env)
{
  g_return_if_fail(SBUILD_IS_CHROOT_PLAIN(chroot));
  g_return_if_fail(env != NULL);

  *env = g_list_append(*env,
		       g_strdup_printf("CHROOT_LOCATION=%s",
				       (chroot->location) ? chroot->location : ""));
}

static const gchar *
sbuild_chroot_plain_get_chroot_type (const SbuildChrootPlain *chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT_PLAIN(chroot), NULL);

  return "plain";
}

static gboolean
sbuild_chroot_plain_setup_lock (SbuildChrootPlain     *chroot,
				SbuildChrootSetupType  type,
				gboolean               lock)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT_PLAIN(chroot), FALSE);

  /* By default, plain chroots do no locking. */
  return TRUE;
}

static SbuildChrootSessionFlags
sbuild_chroot_plain_get_session_flags (const SbuildChrootPlain *chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT_PLAIN(chroot), 0);

  return 0;
}

static void
sbuild_chroot_plain_init (SbuildChrootPlain *chroot)
{
  g_return_if_fail(SBUILD_IS_CHROOT_PLAIN(chroot));

  chroot->location= NULL;

  g_signal_connect(G_OBJECT(chroot), "notify::location",
		   G_CALLBACK(sbuild_chroot_plain_set_mount_location), NULL);
}

static void
sbuild_chroot_plain_finalize (SbuildChrootPlain *chroot)
{
  g_return_if_fail(SBUILD_IS_CHROOT_PLAIN(chroot));

  if (chroot->location)
    {
      g_free (chroot->location);
      chroot->location = NULL;
    }

  if (parent_class->finalize)
    parent_class->finalize(G_OBJECT(chroot));
}

static void
sbuild_chroot_plain_set_property (GObject      *object,
				  guint         param_id,
				  const GValue *value,
				  GParamSpec   *pspec)
{
  SbuildChrootPlain *chroot;

  g_return_if_fail (object != NULL);
  g_return_if_fail (SBUILD_IS_CHROOT_PLAIN (object));

  chroot = SBUILD_CHROOT_PLAIN(object);

  switch (param_id)
    {
    case PROP_LOCATION:
      sbuild_chroot_plain_set_location(chroot, g_value_get_string(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
sbuild_chroot_plain_get_property (GObject    *object,
				  guint       param_id,
				  GValue     *value,
				  GParamSpec *pspec)
{
  SbuildChrootPlain *chroot;

  g_return_if_fail (object != NULL);
  g_return_if_fail (SBUILD_IS_CHROOT_PLAIN (object));

  chroot = SBUILD_CHROOT_PLAIN(object);

  switch (param_id)
    {
    case PROP_LOCATION:
      g_value_set_string(value, chroot->location);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
sbuild_chroot_plain_class_init (SbuildChrootPlainClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  SbuildChrootClass *chroot_class = SBUILD_CHROOT_CLASS (klass);
  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = (GObjectFinalizeFunc) sbuild_chroot_plain_finalize;
  gobject_class->set_property = (GObjectSetPropertyFunc) sbuild_chroot_plain_set_property;
  gobject_class->get_property = (GObjectGetPropertyFunc) sbuild_chroot_plain_get_property;

  chroot_class->print_details = (SbuildChrootPrintDetailsFunc)
    sbuild_chroot_plain_print_details;
  chroot_class->print_config = (SbuildChrootPrintConfigFunc)
    sbuild_chroot_plain_print_config;
  chroot_class->setup_env = (SbuildChrootSetupEnvFunc)
    sbuild_chroot_plain_setup_env;
  chroot_class->get_chroot_type = (SbuildChrootGetChrootTypeFunc)
    sbuild_chroot_plain_get_chroot_type;
  chroot_class->setup_lock = (SbuildChrootSetupLockFunc)
    sbuild_chroot_plain_setup_lock;
  chroot_class->get_session_flags = (SbuildChrootGetSessionFlagsFunc)
    sbuild_chroot_plain_get_session_flags;

  g_object_class_install_property
    (gobject_class,
     PROP_LOCATION,
     g_param_spec_string ("location", "Location",
			  "The directory location of the chroot",
			  "",
			  (G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT)));
}

/*
 * Local Variables:
 * mode:C
 * End:
 */
