/* sbuild-chroot - sbuild chroot object
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

#ifndef SBUILD_CHROOT_H
#define SBUILD_CHROOT_H

#include <glib.h>
#include <glib/gprintf.h>
#include <glib-object.h>

#define SBUILD_TYPE_CHROOT		  (sbuild_chroot_get_type ())
#define SBUILD_CHROOT(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), SBUILD_TYPE_CHROOT, SbuildChroot))
#define SBUILD_CHROOT_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), SBUILD_TYPE_CHROOT, SbuildChrootClass))
#define SBUILD_IS_CHROOT(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SBUILD_TYPE_CHROOT))
#define SBUILD_IS_CHROOT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SBUILD_TYPE_CHROOT))
#define SBUILD_CHROOT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SBUILD_TYPE_CHROOT, SbuildChrootClass))

typedef enum
{
  SBUILD_CHROOT_SETUP_START,
  SBUILD_CHROOT_SETUP_STOP,
  SBUILD_CHROOT_RUN_START,
  SBUILD_CHROOT_RUN_STOP
} SbuildChrootSetupType;

typedef enum
{
  SBUILD_CHROOT_SESSION_CREATE = 1 << 0
} SbuildChrootSessionFlags;

typedef struct _SbuildChroot SbuildChroot;
typedef struct _SbuildChrootClass SbuildChrootClass;

typedef void (*SbuildChrootPrintDetailsFunc)(SbuildChroot *chroot,
					     FILE         *file);
typedef void (*SbuildChrootPrintConfigFunc)(SbuildChroot *chroot,
					    FILE         *file);
typedef const gchar *(*SbuildChrootGetChrootTypeFunc)(const SbuildChroot  *chroot);
typedef void (*SbuildChrootSetupEnvFunc)(SbuildChroot  *chroot,
					 GList        **env);
typedef gboolean (*SbuildChrootSetupLockFunc)(SbuildChroot          *chroot,
					      SbuildChrootSetupType  type,
					      gboolean               lock);
typedef SbuildChrootSessionFlags (*SbuildChrootGetSessionFlagsFunc)(const SbuildChroot  *chroot);

struct _SbuildChroot
{
  GObject    parent;
  gchar     *name;
  gchar     *description;
  guint      priority;
  char     **groups;
  char     **root_groups;
  char     **aliases;
  gchar     *mount_location;
  gchar     *mount_device;
  gboolean   active;
  gboolean   run_setup_scripts;
  gboolean   run_session_scripts;
};

struct _SbuildChrootClass
{
  GObjectClass                    parent;
  SbuildChrootPrintDetailsFunc    print_details;
  SbuildChrootPrintConfigFunc     print_config;
  SbuildChrootSetupEnvFunc        setup_env;
  SbuildChrootGetChrootTypeFunc   get_chroot_type;
  SbuildChrootSetupLockFunc       setup_lock;
  SbuildChrootGetSessionFlagsFunc get_session_flags;
};


GType
sbuild_chroot_get_type (void);

SbuildChroot *
sbuild_chroot_new (void);

SbuildChroot *
sbuild_chroot_new_from_keyfile (GKeyFile   *keyfile,
				const char *group,
				gboolean    active);

gboolean
sbuild_chroot_set_properties_from_keyfile (SbuildChroot *chroot,
					   GKeyFile     *keyfile);

const char *
sbuild_chroot_get_name (const SbuildChroot *restrict chroot);

void
sbuild_chroot_set_name (SbuildChroot *chroot,
			const char   *name);

const char *
sbuild_chroot_get_description (const SbuildChroot *restrict chroot);

void
sbuild_chroot_set_description (SbuildChroot *chroot,
			       const char   *description);

const char *
sbuild_chroot_get_mount_location (const SbuildChroot *restrict chroot);

void
sbuild_chroot_set_mount_location (SbuildChroot *chroot,
				  const char   *location);

const char *
sbuild_chroot_get_mount_device (const SbuildChroot *restrict chroot);

void
sbuild_chroot_set_mount_device (SbuildChroot *chroot,
				const char   *device);

guint
sbuild_chroot_get_priority (const SbuildChroot *restrict chroot);

void
sbuild_chroot_set_priority (SbuildChroot *chroot,
			    guint         priority);

char **
sbuild_chroot_get_groups (const SbuildChroot *restrict chroot);

void
sbuild_chroot_set_groups (SbuildChroot  *chroot,
			  char         **groups);

char **
sbuild_chroot_get_root_groups (const SbuildChroot *restrict chroot);

void
sbuild_chroot_set_root_groups (SbuildChroot  *chroot,
			       char         **groups);

char **
sbuild_chroot_get_aliases (const SbuildChroot *restrict chroot);

void
sbuild_chroot_set_aliases (SbuildChroot  *chroot,
			   char         **aliases);

gboolean
sbuild_chroot_get_run_setup_scripts (const SbuildChroot *restrict chroot);

void
sbuild_chroot_set_run_setup_scripts (SbuildChroot *chroot,
				     gboolean      run_setup_scripts);

gboolean
sbuild_chroot_get_run_session_scripts (const SbuildChroot *restrict chroot);

void
sbuild_chroot_set_run_session_scripts (SbuildChroot *chroot,
				       gboolean      run_session_scripts);

const gchar *
sbuild_chroot_get_chroot_type (const SbuildChroot  *chroot);

void
sbuild_chroot_setup_env (SbuildChroot  *chroot,
			 GList        **env);

gboolean
sbuild_chroot_setup_lock (SbuildChroot          *chroot,
			  SbuildChrootSetupType  type,
			  gboolean               lock);

SbuildChrootSessionFlags
sbuild_chroot_get_session_flags (const SbuildChroot  *chroot);

void
sbuild_chroot_print_details (SbuildChroot *chroot,
			     FILE         *file);

void
sbuild_chroot_print_config (SbuildChroot *chroot,
			    FILE         *file);

#endif /* SBUILD_CHROOT_H */

/*
 * Local Variables:
 * mode:C
 * End:
 */
