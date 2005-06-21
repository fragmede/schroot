/* sbuild-chroot - sbuild chroot object
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
 * SECTION:sbuild-chroot
 * @short_description: chroot object
 * @title: SbuildChroot
 *
 * This object contains all of the metadata associated with a single
 * chroot.  This is the in-core representation of a chroot definition
 * in the configuration file, and may be initialised directly from an
 * open #GKeyFile.
 *
 * This object is a container of information only.  The only things it
 * can do are satisfying requests for information and printing its
 * details.
 */

#include <config.h>

#include "sbuild-chroot.h"

enum
{
  PROP_0,
  PROP_NAME,
  PROP_DESCRIPTION,
  PROP_LOCATION,
  PROP_GROUPS,
  PROP_ROOT_GROUPS,
  PROP_ALIASES
};

static GObjectClass *parent_class;

G_DEFINE_TYPE(SbuildChroot, sbuild_chroot, G_TYPE_OBJECT)

/**
 * sbuild_chroot_new:
 *
 * Creates a new #SbuildChroot.
 *
 * Returns the newly created #SbuildChroot.
 */
SbuildChroot *
sbuild_chroot_new (void)
{
  return (SbuildChroot *) g_object_new (SBUILD_TYPE_CHROOT, NULL);
}

/**
 * sbuild_chroot_new_from_keyfile:
 * @keyfile: the #GKeyFile containing the chroot configuration
 * @group: the group in @keyfile to use
 *
 * Creates a new #SbuildChroot.
 *
 * Returns the newly created #SbuildChroot.
 */
SbuildChroot *
sbuild_chroot_new_from_keyfile (GKeyFile   *keyfile,
				const char *group)
{
  g_return_val_if_fail(group != NULL, NULL);

  GError *error = NULL;

  char *description =
    g_key_file_get_locale_string(keyfile, group, "description", NULL, &error);
  if (error != NULL)
    {
      g_clear_error(&error);
      error = NULL;
      description = NULL;
    }

  char *location =
    g_key_file_get_string(keyfile, group, "location", &error);
  if (error != NULL)
    {
      g_clear_error(&error);
      error = NULL;
      location = NULL;
    }

  char **groups =
    g_key_file_get_string_list(keyfile, group, "groups", NULL, &error);
  if (error != NULL)
    {
      g_clear_error(&error);
      error = NULL;
      groups = NULL;
    }

  char **root_groups =
    g_key_file_get_string_list(keyfile, group, "root-groups", NULL, &error);
  if (error != NULL)
    {
      g_clear_error(&error);
      error = NULL;
      root_groups = NULL;
    }

  char **aliases =
    g_key_file_get_string_list(keyfile, group, "aliases", NULL, &error);
  if (error != NULL)
    {
      g_clear_error(&error);
      error = NULL;
      aliases = NULL;
    }

  SbuildChroot *chroot =  (SbuildChroot *) g_object_new
    (SBUILD_TYPE_CHROOT,
     "name", group,
     "description", description,
     "location", location,
     "groups", groups,
     "root-groups", root_groups,
     "aliases", aliases,
     NULL);

  g_free(description);
  g_free(location);
  g_strfreev(groups);
  g_strfreev(root_groups);
  g_strfreev(aliases);

  return chroot;
}

/**
 * sbuild_chroot_get_name:
 * @chroot: an #SbuildChroot
 *
 * Get the name of the chroot.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const char *
sbuild_chroot_get_name (const SbuildChroot *restrict chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT(chroot), NULL);

  return chroot->name;
}

/**
 * sbuild_chroot_set_name:
 * @chroot: an #SbuildChroot.
 * @name: the name to set.
 *
 * Set the name of a chroot.
 */
void
sbuild_chroot_set_name (SbuildChroot *chroot,
			const char   *name)
{
  g_return_if_fail(SBUILD_IS_CHROOT(chroot));

  if (chroot->name)
    {
      g_free(chroot->name);
    }
  chroot->name = g_strdup(name);
  g_object_notify(G_OBJECT(chroot), "name");
}

/**
 * sbuild_chroot_get_description:
 * @chroot: an #SbuildChroot
 *
 * Get the description of the chroot.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const char *
sbuild_chroot_get_description (const SbuildChroot *restrict chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT(chroot), NULL);

  return chroot->description;
}

/**
 * sbuild_chroot_set_description:
 * @chroot: an #SbuildChroot.
 * @description: the description to set.
 *
 * Set the description of a chroot.
 */
void
sbuild_chroot_set_description (SbuildChroot *chroot,
			       const char   *description)
{
  g_return_if_fail(SBUILD_IS_CHROOT(chroot));

  if (chroot->description)
    {
      g_free(chroot->description);
    }
  chroot->description = g_strdup(description);
  g_object_notify(G_OBJECT(chroot), "description");
}

/**
 * sbuild_chroot_get_location:
 * @chroot: an #SbuildChroot
 *
 * Get the location of the chroot.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const char *
sbuild_chroot_get_location (const SbuildChroot *restrict chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT(chroot), NULL);

  return chroot->location;
}

/**
 * sbuild_chroot_set_location:
 * @chroot: an #SbuildChroot.
 * @location: the location to set.
 *
 * Set the location of a chroot.
 */
void
sbuild_chroot_set_location (SbuildChroot *chroot,
			    const char   *location)
{
  g_return_if_fail(SBUILD_IS_CHROOT(chroot));

  if (chroot->location)
    {
      g_free(chroot->location);
    }
  chroot->location = g_strdup(location);
  g_object_notify(G_OBJECT(chroot), "location");
}

/**
 * sbuild_chroot_get_groups:
 * @chroot: an #SbuildChroot
 *
 * Get the groups of the chroot.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
char **
sbuild_chroot_get_groups (const SbuildChroot *restrict chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT(chroot), NULL);

  return chroot->groups;
}

/**
 * sbuild_chroot_set_groups:
 * @chroot: an #SbuildChroot.
 * @groups: the groups to set.
 *
 * Set the groups of a chroot.
 */
void
sbuild_chroot_set_groups (SbuildChroot  *chroot,
			  char         **groups)
{
  g_return_if_fail(SBUILD_IS_CHROOT(chroot));

  if (chroot->groups)
    {
      g_strfreev(chroot->groups);
    }
  chroot->groups = g_strdupv(groups);
  g_object_notify(G_OBJECT(chroot), "groups");
}

/**
 * sbuild_chroot_get_root_groups:
 * @chroot: an #SbuildChroot
 *
 * Get the root groups of the chroot.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
char **
sbuild_chroot_get_root_groups (const SbuildChroot *restrict chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT(chroot), NULL);

  return chroot->root_groups;
}

/**
 * sbuild_chroot_set_root_groups:
 * @chroot: an #SbuildChroot.
 * @groups: the groups to set.
 *
 * Set the groups of a chroot.
 */
void
sbuild_chroot_set_root_groups (SbuildChroot  *chroot,
			       char         **groups)
{
  g_return_if_fail(SBUILD_IS_CHROOT(chroot));

  if (chroot->root_groups)
    {
      g_strfreev(chroot->root_groups);
    }
  chroot->root_groups = g_strdupv(groups);
  g_object_notify(G_OBJECT(chroot), "root-groups");
}

/**
 * sbuild_chroot_get_aliases:
 * @chroot: an #SbuildChroot
 *
 * Get the aliases of the chroot.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
char **
sbuild_chroot_get_aliases (const SbuildChroot *restrict chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT(chroot), NULL);

  return chroot->aliases;
}

/**
 * sbuild_chroot_set_aliases:
 * @chroot: an #SbuildChroot.
 * @aliases: the aliases to set.
 *
 * Set the aliases of a chroot.
 */
void
sbuild_chroot_set_aliases (SbuildChroot  *chroot,
			   char         **aliases)
{
  g_return_if_fail(SBUILD_IS_CHROOT(chroot));

  if (chroot->aliases)
    {
      g_strfreev(chroot->aliases);
    }
  chroot->aliases = g_strdupv(aliases);
  g_object_notify(G_OBJECT(chroot), "aliases");
}

/**
 * sbuild_chroot_print_details:
 * @chroot: an #SbuildChroot.
 * @file: the file to output to.
 *
 * Print detailed information about @chroot to @file.  The information
 * is printed in plain text with one line per property.
 */
void sbuild_chroot_print_details (SbuildChroot *chroot,
				  FILE         *file)
{
  g_fprintf(file, "Name: %s\nDescription: %s\nLocation: %s\n",
	    chroot->name, chroot->description, chroot->location);
  g_fprintf(file, "Groups:");
  if (chroot->groups)
    for (guint i=0; chroot->groups[i] != NULL; ++i)
      g_fprintf(file, " %s", chroot->groups[i]);
  g_fprintf(file, "\nRoot Groups:");
  if (chroot->root_groups)
    for (guint i=0; chroot->root_groups[i] != NULL; ++i)
      g_fprintf(file, " %s", chroot->root_groups[i]);
  g_fprintf(file, "\nAliases:");
  if (chroot->aliases)
    for (guint i=0; chroot->aliases[i] != NULL; ++i)
      g_fprintf(file, " %s", chroot->aliases[i]);
  g_fprintf(file, "\n\n");
}

static void
sbuild_chroot_init (SbuildChroot *chroot)
{
  g_return_if_fail(SBUILD_IS_CHROOT(chroot));

  chroot->name = NULL;
  chroot->description = NULL;
  chroot->location = NULL;
  chroot->groups = NULL;
  chroot->root_groups = NULL;
  chroot->aliases = NULL;
}

static void
sbuild_chroot_finalize (SbuildChroot *chroot)
{
  g_return_if_fail(SBUILD_IS_CHROOT(chroot));

  if (chroot->name)
    {
      g_free (chroot->name);
      chroot->name = NULL;
    }
  if (chroot->description)
    {
      g_free (chroot->description);
      chroot->description = NULL;
    }
  if (chroot->location)
    {
      g_free (chroot->location);
      chroot->location = NULL;
    }
  if (chroot->groups)
    {
      g_strfreev(chroot->groups);
      chroot->groups = NULL;
    }
  if (chroot->root_groups)
    {
      g_strfreev(chroot->root_groups);
      chroot->root_groups = NULL;
    }
  if (chroot->aliases)
    {
      g_strfreev(chroot->aliases);
      chroot->aliases = NULL;
    }

  if (parent_class->finalize)
    parent_class->finalize(G_OBJECT(chroot));
}

static void
sbuild_chroot_set_property (GObject      *object,
			    guint         param_id,
			    const GValue *value,
			    GParamSpec   *pspec)
{
  SbuildChroot *chroot;

  g_return_if_fail (object != NULL);
  g_return_if_fail (SBUILD_IS_CHROOT (object));

  chroot = SBUILD_CHROOT(object);

  switch (param_id)
    {
    case PROP_NAME:
      sbuild_chroot_set_name(chroot, g_value_get_string(value));
      break;
    case PROP_DESCRIPTION:
      sbuild_chroot_set_description(chroot, g_value_get_string(value));
      break;
    case PROP_LOCATION:
      sbuild_chroot_set_location(chroot, g_value_get_string(value));
      break;
    case PROP_GROUPS:
      sbuild_chroot_set_groups(chroot, g_value_get_boxed(value));
      break;
    case PROP_ROOT_GROUPS:
      sbuild_chroot_set_root_groups(chroot, g_value_get_boxed(value));
      break;
    case PROP_ALIASES:
      sbuild_chroot_set_aliases(chroot, g_value_get_boxed(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
sbuild_chroot_get_property (GObject    *object,
			    guint       param_id,
			    GValue     *value,
			    GParamSpec *pspec)
{
  SbuildChroot *chroot;

  g_return_if_fail (object != NULL);
  g_return_if_fail (SBUILD_IS_CHROOT (object));

  chroot = SBUILD_CHROOT(object);

  switch (param_id)
    {
    case PROP_NAME:
      g_value_set_string(value, chroot->name);
      break;
    case PROP_DESCRIPTION:
      g_value_set_string(value, chroot->description);
      break;
    case PROP_LOCATION:
      g_value_set_string(value, chroot->location);
      break;
    case PROP_GROUPS:
      g_value_set_boxed(value, chroot->groups);
      break;
    case PROP_ROOT_GROUPS:
      g_value_set_boxed(value, chroot->root_groups);
      break;
    case PROP_ALIASES:
      g_value_set_boxed(value, chroot->aliases);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
sbuild_chroot_class_init (SbuildChrootClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = (GObjectFinalizeFunc) sbuild_chroot_finalize;
  gobject_class->set_property = (GObjectSetPropertyFunc) sbuild_chroot_set_property;
  gobject_class->get_property = (GObjectGetPropertyFunc) sbuild_chroot_get_property;

  g_object_class_install_property
    (gobject_class,
     PROP_NAME,
     g_param_spec_string ("name", "Name",
			  "The name of the chroot",
			  "",
			  (G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT)));

  g_object_class_install_property
    (gobject_class,
     PROP_DESCRIPTION,
     g_param_spec_string ("description", "Description",
			  "The description of the chroot",
			  "",
			  (G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT)));

  g_object_class_install_property
    (gobject_class,
     PROP_LOCATION,
     g_param_spec_string ("location", "Location",
			  "The location (path) of the chroot",
			  "",
			  (G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT)));

  g_object_class_install_property
    (gobject_class,
     PROP_GROUPS,
     g_param_spec_boxed ("groups", "Groups",
                         "The groups allowed to use this chroot",
                         G_TYPE_STRV,
                         (G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT)));

  g_object_class_install_property
    (gobject_class,
     PROP_ROOT_GROUPS,
     g_param_spec_boxed ("root-groups", "Root Groups",
                         "The groups allowed to use this chroot as root",
                         G_TYPE_STRV,
                         (G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT)));

  g_object_class_install_property
    (gobject_class,
     PROP_ALIASES,
     g_param_spec_boxed ("aliases", "Aliases",
                         "Alternate names for this chroot",
                         G_TYPE_STRV,
                         (G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT)));
}

/*
 * Local Variables:
 * mode:C
 * End:
 */
