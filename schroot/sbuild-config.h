/* sbuild-config - sbuild config object
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

#ifndef SBUILD_CONFIG_H
#define SBUILD_CONFIG_H

#include <glib.h>
#include <glib/gprintf.h>
#include <glib-object.h>

#include "sbuild-chroot.h"

typedef enum
{
  SBUILD_CONFIG_FILE_ERROR_STAT_FAIL,
  SBUILD_CONFIG_FILE_ERROR_OWNERSHIP,
  SBUILD_CONFIG_FILE_ERROR_PERMISSIONS,
  SBUILD_CONFIG_FILE_ERROR_NOT_REGULAR
} SbuildConfigFileError;

#define SBUILD_CONFIG_FILE_ERROR sbuild_config_file_error_quark()

GQuark
sbuild_config_file_error_quark (void);

#define SBUILD_TYPE_CONFIG		  (sbuild_config_get_type ())
#define SBUILD_CONFIG(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), SBUILD_TYPE_CONFIG, SbuildConfig))
#define SBUILD_CONFIG_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), SBUILD_TYPE_CONFIG, SbuildConfigClass))
#define SBUILD_IS_CONFIG(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SBUILD_TYPE_CONFIG))
#define SBUILD_IS_CONFIG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SBUILD_TYPE_CONFIG))
#define SBUILD_CONFIG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SBUILD_TYPE_CONFIG, SbuildConfigClass))

typedef struct _SbuildConfig SbuildConfig;
typedef struct _SbuildConfigClass SbuildConfigClass;

struct _SbuildConfig
{
  GObject  parent;
  char    *file;
  GList   *chroots;
};

struct _SbuildConfigClass
{
  GObjectClass		     parent;
};


GType
sbuild_config_get_type (void);

SbuildConfig *
sbuild_config_new_from_file (const char *file);

SbuildConfig *
sbuild_config_new_from_directory (const char *dir);

const GList *
sbuild_config_get_chroots (SbuildConfig *config);

SbuildChroot *
sbuild_config_find_chroot (SbuildConfig *config,
			   const char   *name);

SbuildChroot *
sbuild_config_find_alias (SbuildConfig *config,
			  const char   *name);

GList *
sbuild_config_get_chroot_list (SbuildConfig *config);

void
sbuild_config_print_chroot_list (SbuildConfig *config,
				 FILE         *file);

gboolean
sbuild_config_validate_chroots(SbuildConfig  *config,
			       char         **chroots);

#endif /* SBUILD_CONFIG_H */

/*
 * Local Variables:
 * mode:C
 * End:
 */
