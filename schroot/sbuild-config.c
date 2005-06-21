/* sbuild-config - sbuild config object
 *
 * Copyright (C) 2005  Roger Leigh <rleigh@debian.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *********************************************************************/

/**
 * SECTION:sbuild-config
 * @short_description: config object
 * @title: SbuildConfig
 *
 */

#define _GNU_SOURCE
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "sbuild-config.h"

GQuark
sbuild_config_file_error_quark (void)
{
  static GQuark error_quark = 0;

  if (error_quark == 0)
    error_quark = g_quark_from_static_string ("sbuild-config-file-error-quark");

  return error_quark;
}

enum
{
  PROP_0,
  PROP_CONFIG_FILE
};

static GObjectClass *parent_class;

G_DEFINE_TYPE(SbuildConfig, sbuild_config, G_TYPE_OBJECT)

/**
 * sbuild_config_new:
 *
 * Creates a new #SbuildConfig.
 *
 * Returns the newly created #SbuildConfig.
 */
SbuildConfig *
sbuild_config_new (const char *file)
{
  return (SbuildConfig *) g_object_new (SBUILD_TYPE_CONFIG,
					"config-file", file,
					NULL);
}

static gboolean
sbuild_config_check_security(int      fd,
			     GError **error)
{
  struct stat statbuf;
  if (fstat(fd, &statbuf) < 0)
    {
      g_set_error(error,
		  SBUILD_CONFIG_FILE_ERROR, SBUILD_CONFIG_FILE_ERROR_STAT_FAIL,
		  "failed to stat file: %s", g_strerror(errno));
      return FALSE;
    }

  if (statbuf.st_uid != 0)
    {
      g_set_error(error,
		  SBUILD_CONFIG_FILE_ERROR, SBUILD_CONFIG_FILE_ERROR_OWNERSHIP,
		  "not owned by user root");
      return FALSE;
    }

  if (statbuf.st_mode & S_IWOTH)
    {
      g_set_error(error,
		  SBUILD_CONFIG_FILE_ERROR, SBUILD_CONFIG_FILE_ERROR_PERMISSIONS,
		  "others have write permission");
      return FALSE;
    }

  if (!S_ISREG(statbuf.st_mode))
    {
      g_set_error(error,
		  SBUILD_CONFIG_FILE_ERROR, SBUILD_CONFIG_FILE_ERROR_NOT_REGULAR,
		  "not a regular file");
      return FALSE;
    }

  return TRUE;
}

static GList *
sbuild_config_load (const char *file)
{
  /* Use a UNIX fd, for security (no races) */
  int fd = open(file, O_RDONLY|O_NOFOLLOW);
  if (fd < 0)
    {
      g_printerr("%s: failed to load configuration: %s\n", file, g_strerror(errno));
      exit (EXIT_FAILURE);
    }

  GError *security_error = NULL;
  sbuild_config_check_security(fd, &security_error);
  if (security_error)
    {
      g_printerr("%s: security failure: %s\n", file, security_error->message);
      exit (EXIT_FAILURE);
    }

  /* Now create an IO Channel and read in the data */
  GIOChannel *channel = g_io_channel_unix_new(fd);
  gchar *data = NULL;
  gsize size = 0;
  GError *read_error = NULL;

  g_io_channel_set_encoding(channel, NULL, NULL);
  g_io_channel_read_to_end(channel, &data, &size, &read_error);
  if (read_error)
    {
      g_printerr("%s: read failure: %s\n", file, read_error->message);
      exit (EXIT_FAILURE);
    }

  GError *close_error = NULL;
  g_io_channel_shutdown(channel, FALSE, &close_error);
  if (close_error)
    {
      g_printerr("%s: close failure: %s\n", file, close_error->message);
      exit (EXIT_FAILURE);
    }

  /* Create key file */
  GKeyFile *keyfile = g_key_file_new();
  g_key_file_set_list_separator(keyfile, ',');
  GError *parse_error = NULL;
  g_key_file_load_from_data(keyfile, data, size, G_KEY_FILE_NONE, &parse_error);

  if (parse_error)
    {
      g_printerr("%s: parse failure: %s\n", file, parse_error->message);
      exit (EXIT_FAILURE);
    }

  /* Create SbuildChroot objects from key file */
  char **groups = g_key_file_get_groups(keyfile, NULL);
  GList *list = NULL;
  for (guint i=0; groups[i] != NULL; ++i)
    {
      SbuildChroot *chroot = sbuild_chroot_new_from_keyfile(keyfile, groups[i]);
      //      sbuild_chroot_print_details(chroot, stdout);
      list = g_list_append(list, chroot);
    }
  g_strfreev(groups);

  return list;
}

/**
 * sbuild_config_set_name:
 * @config: an #SbuildConfig.
 * @name: the name to set.
 *
 * Set the name of a config.
 */
static void
sbuild_config_set_config_file (SbuildConfig *config,
			       const char   *file)
{
  g_return_if_fail(SBUILD_IS_CONFIG(config));

  if (config->file)
    {
      g_free(config->file);
    }
  config->file = g_strdup(file);

  g_assert(config->chroots == NULL);
  config->chroots = sbuild_config_load(config->file);

  g_object_notify(G_OBJECT(config), "config_file");
}

const GList *
sbuild_config_get_chroots (SbuildConfig *config)
{
  g_return_val_if_fail(SBUILD_IS_CONFIG(config), NULL);

  return config->chroots;
}

static SbuildChroot *
sbuild_config_find_generic (SbuildConfig *config,
			    const char   *name,
			    GCompareFunc  func)
{
  g_return_val_if_fail(SBUILD_IS_CONFIG(config), NULL);

  SbuildChroot *example = sbuild_chroot_new();
  sbuild_chroot_set_name(example, name);

  GList *elem = g_list_find_custom(config->chroots, example, (GCompareFunc) func);

  g_object_unref(example);
  example = NULL;

  if (elem)
    return (SbuildChroot *) elem->data;
  else
    return NULL;
}

static gint
chroot_findfunc (SbuildChroot *a,
		 SbuildChroot *b)
{
  return strcmp(sbuild_chroot_get_name(a),
		sbuild_chroot_get_name(b));
}


SbuildChroot *
sbuild_config_find_chroot (SbuildConfig *config,
			   const char   *name)
{
  g_return_val_if_fail(SBUILD_IS_CONFIG(config), NULL);

  return sbuild_config_find_generic(config, name, (GCompareFunc) chroot_findfunc);
}

static gint
alias_findfunc (SbuildChroot *a,
		SbuildChroot *b)
{
  gchar **aliases = sbuild_chroot_get_aliases(a);
  for (guint i = 0; aliases[i] != NULL; ++i)
    {
      if (strcmp(aliases[i], sbuild_chroot_get_name(b)) == 0)
	return 0;
    }
  return 1;
}


SbuildChroot *
sbuild_config_find_alias (SbuildConfig *config,
			  const char   *name)
{
  g_return_val_if_fail(SBUILD_IS_CONFIG(config), NULL);

  SbuildChroot *chroot = sbuild_config_find_chroot(config, name);
  if (chroot == NULL)
    chroot = sbuild_config_find_generic(config, name, (GCompareFunc) alias_findfunc);
  return chroot;
}

static void
sbuild_config_get_chroot_list_foreach (SbuildChroot  *chroot,
				       GList        **list)
{
  *list = g_list_append(*list, (gpointer) sbuild_chroot_get_name(chroot));

  gchar **aliases = sbuild_chroot_get_aliases(chroot);
  for (guint i = 0; aliases[i] != NULL; ++i)
    *list = g_list_append(*list, aliases[i]);
}

GList *
sbuild_config_get_chroot_list (SbuildConfig *config)
{
  g_return_val_if_fail(SBUILD_IS_CONFIG(config), NULL);

  GList *list = NULL;

  g_list_foreach(config->chroots, (GFunc) sbuild_config_get_chroot_list_foreach, &list);
  list = g_list_sort(list, (GCompareFunc) strcmp);

  return list;
}

static void
sbuild_config_print_chroot_list_foreach (const char *name,
					 FILE       *file)
{
  g_print("%s\n", name);
}

void
sbuild_config_print_chroot_list (SbuildConfig *config,
				 FILE         *file)
{
  GList *list = sbuild_config_get_chroot_list(config);
  g_list_foreach(list, (GFunc) sbuild_config_print_chroot_list_foreach, file);
  g_list_free(list);
}

gboolean
sbuild_config_validate_chroots(SbuildConfig  *config,
			       char         **chroots)
{
  gboolean success = TRUE;
  for (guint i=0; chroots[i] != NULL; ++i)
    {
      SbuildChroot *chroot = sbuild_config_find_alias(config, chroots[i]);
      if (chroot == NULL)
	{
	  g_printerr("%s: No such chroot\n", chroots[i]);
	  success = FALSE;
	}
    }
  return success;
}

static void
sbuild_config_init (SbuildConfig *config)
{
  g_return_if_fail(SBUILD_IS_CONFIG(config));

  config->file = NULL;
  config->chroots = NULL;
}

static void
sbuild_config_finalize (SbuildConfig *config)
{
  g_return_if_fail(SBUILD_IS_CONFIG(config));

  if (config->file)
    {
      g_free (config->file);
      config->file = NULL;
    }
  if (config->chroots)
    {
      g_list_foreach(config->chroots, (GFunc) g_object_unref, NULL);
      g_list_free(config->chroots);
    }

  if (parent_class->finalize)
    parent_class->finalize(G_OBJECT(config));
}

static void
sbuild_config_set_property (GObject      *object,
			    guint         param_id,
			    const GValue *value,
			    GParamSpec   *pspec)
{
  SbuildConfig *config;

  g_return_if_fail (object != NULL);
  g_return_if_fail (SBUILD_IS_CONFIG (object));

  config = SBUILD_CONFIG(object);

  switch (param_id)
    {
    case PROP_CONFIG_FILE:
      sbuild_config_set_config_file(config, g_value_get_string(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
sbuild_config_get_property (GObject    *object,
			    guint       param_id,
			    GValue     *value,
			    GParamSpec *pspec)
{
  SbuildConfig *config;

  g_return_if_fail (object != NULL);
  g_return_if_fail (SBUILD_IS_CONFIG (object));

  config = SBUILD_CONFIG(object);

  switch (param_id)
    {
    case PROP_CONFIG_FILE:
      g_value_set_string(value, config->file);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
sbuild_config_class_init (SbuildConfigClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  parent_class = g_type_class_peek_parent (klass);

  /* Override the virtual finalize, set_property and
     get_property methods in the GObject class vtable (which
     is contained in CseCanvasClass). */
  gobject_class->finalize = (GObjectFinalizeFunc) sbuild_config_finalize;
  gobject_class->set_property = (GObjectSetPropertyFunc) sbuild_config_set_property;
  gobject_class->get_property = (GObjectGetPropertyFunc) sbuild_config_get_property;

  g_object_class_install_property
    (gobject_class,
     PROP_CONFIG_FILE,
     g_param_spec_string ("config-file", "Configuration File",
			  "The file containing the chroot configuration",
			  "",
			  (G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY)));
}

/*
 * Local Variables:
 * mode:C
 * End:
 */
