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

#include <string.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "sbuild-chroot.h"
#include "sbuild-chroot-plain.h"
#include "sbuild-chroot-block-device.h"
#include "sbuild-chroot-lvm-snapshot.h"

enum
{
  PROP_0,
  PROP_NAME,
  PROP_DESCRIPTION,
  PROP_PRIORITY,
  PROP_GROUPS,
  PROP_ROOT_GROUPS,
  PROP_ALIASES,
  PROP_MOUNT_LOCATION,
  PROP_MOUNT_DEVICE,
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

  GType chroot_type = 0;
  GObjectClass *klass;
  GParameter *params = NULL;
  guint n_params = 0;
  guint n_alloc_params = 16;

  gchar **keys = g_key_file_get_keys(keyfile, group, NULL, NULL);
  if (keys)
    {
      /* Find out which chroot type we want.  These are all linked
	 statically, for reasons of security, because we don't want
	 random modules loading outside our control. */
      {
	GError *error = NULL;
	char *type =
	  g_key_file_get_string(keyfile, group, "type", &error);

	if (error != NULL)
	  {
	    g_clear_error(&error);
	    chroot_type = SBUILD_TYPE_CHROOT_PLAIN;
	  }
	else
	  {
	    if (strcmp(type, "plain") == 0)
	      chroot_type = SBUILD_TYPE_CHROOT_PLAIN;
	    else if (strcmp(type, "block-device") == 0)
	      chroot_type = SBUILD_TYPE_CHROOT_BLOCK_DEVICE;
	    else if (strcmp(type, "lvm-snapshot") == 0)
	      chroot_type = SBUILD_TYPE_CHROOT_LVM_SNAPSHOT;
	    else // Invalid chroot type
	      {
		g_warning (_("%s chroot: chroot type '%s' is invalid"),
			   group, type);
		chroot_type = 0;
	      }
	    g_free(type);
	  }
      }

      klass = g_type_class_ref (chroot_type);

      params = g_new (GParameter, n_alloc_params);

      params[n_params].name = "name";
      params[n_params].value.g_type = 0;
      g_value_init (&params[n_params].value,
		    G_TYPE_STRING);
      g_value_set_string(&params[n_params].value, group);
      ++n_params;

      for (guint i = 0; keys[i] != NULL; ++i)
	{
	  gchar *key = keys[i];

	  if (strcmp(key, "type") == 0) // Used previously
	    continue;

	  GError *error = NULL;
	  GParamSpec *pspec = g_object_class_find_property(klass, key);

	  if (!pspec)
	    {
	      /* TODO: Use proper schroot type description, rather
		 than the GType name. */
	      g_warning (_("%s chroot: chroot type '%s' has no property named '%s'"),
			 group,
			 g_type_name (chroot_type),
			 key);
	      continue; // TODO: Should a config file error be fatal?
	    }

	  /* Only construction properties may be set. */
	  if ((pspec->flags & (G_PARAM_WRITABLE|G_PARAM_CONSTRUCT)) !=
	      (G_PARAM_WRITABLE|G_PARAM_CONSTRUCT))
	    {
	      g_warning (_("%s chroot: property '%s' is not user-settable"),
			 group, key);
	      continue; // TODO: Should a config file error be fatal?
	    }

	  if (n_params >= n_alloc_params)
	    {
	      n_alloc_params += 16;
	      params = g_renew (GParameter, params, n_alloc_params);
	    }

	  if (G_PARAM_SPEC_VALUE_TYPE(pspec) == G_TYPE_UINT)
	    {
	      gint num =
		g_key_file_get_integer(keyfile, group, key, &error);

	      if (error != NULL)
		g_clear_error(&error);
	      else
		{
		  params[n_params].name = key;
		  params[n_params].value.g_type = 0;
		  g_value_init (&params[n_params].value,
				G_PARAM_SPEC_VALUE_TYPE (pspec));
		  g_value_set_uint(&params[n_params].value, (guint) num);
		  ++n_params;
		}
	    }
	  else if (G_PARAM_SPEC_VALUE_TYPE(pspec) == G_TYPE_STRING)
	    {
	      char *str =
		g_key_file_get_locale_string(keyfile, group, key, NULL, &error);

	      if (error != NULL)
		g_clear_error(&error);
	      else
		{
		  params[n_params].name = key;
		  params[n_params].value.g_type = 0;
		  g_value_init (&params[n_params].value,
				G_PARAM_SPEC_VALUE_TYPE (pspec));
		  g_value_set_string(&params[n_params].value, str);
		  g_free(str);
		  ++n_params;
		}
	    }
	  else if (G_PARAM_SPEC_VALUE_TYPE(pspec) == G_TYPE_STRV)
	    {
	      char **strv =
		g_key_file_get_string_list(keyfile, group, key, NULL, &error);

	      if (error != NULL)
		g_clear_error(&error);
	      else
		{
		  params[n_params].name = key;
		  params[n_params].value.g_type = 0;
		  g_value_init (&params[n_params].value,
				G_PARAM_SPEC_VALUE_TYPE (pspec));
		  g_value_set_boxed(&params[n_params].value, strv);
		  g_strfreev(strv);
		  ++n_params;
		}
	    }
	  else
	    {
	      g_warning (_("%s chroot: property '%s' has unsupported type '%s'"),
			 group,
			 key,
			 g_type_name (G_PARAM_SPEC_VALUE_TYPE(pspec)));
	    }
	}
      g_type_class_unref (klass);
    }

  SbuildChroot *chroot = NULL;
  if (chroot_type != 0)
    chroot =  (SbuildChroot *) g_object_newv(chroot_type, n_params, params);

  while (n_params--)
    g_value_unset (&params[n_params].value);
  g_free (params);

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
 * sbuild_chroot_get_mount_location:
 * @chroot: an #SbuildChroot
 *
 * Get the mount location of the chroot.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const char *
sbuild_chroot_get_mount_location (const SbuildChroot *restrict chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT(chroot), NULL);

  return chroot->mount_location;
}

/**
 * sbuild_chroot_set_mount_location:
 * @chroot: an #SbuildChroot.
 * @location: the mount location to set.
 *
 * Set the mount location of a chroot.
 */
void
sbuild_chroot_set_mount_location (SbuildChroot *chroot,
				  const char   *location)
{
  g_return_if_fail(SBUILD_IS_CHROOT(chroot));

  if (chroot->mount_location)
    {
      g_free(chroot->mount_location);
    }
  chroot->mount_location = g_strdup(location);
  g_object_notify(G_OBJECT(chroot), "mount-location");
}

/**
 * sbuild_chroot_get_mount_device:
 * @chroot: an #SbuildChroot
 *
 * Get the mount device of the chroot.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const char *
sbuild_chroot_get_mount_device (const SbuildChroot *restrict chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT(chroot), NULL);

  return chroot->mount_device;
}

/**
 * sbuild_chroot_set_mount_device:
 * @chroot: an #SbuildChroot.
 * @device: the mount device to set.
 *
 * Set the mount device of a chroot.
 */
void
sbuild_chroot_set_mount_device (SbuildChroot *chroot,
				const char   *device)
{
  g_return_if_fail(SBUILD_IS_CHROOT(chroot));

  if (chroot->mount_device)
    {
      g_free(chroot->mount_device);
    }
  chroot->mount_device = g_strdup(device);
  g_object_notify(G_OBJECT(chroot), "mount-device");
}

/**
 * sbuild_chroot_get_priority:
 * @chroot: an #SbuildChroot
 *
 * Get the priority of the chroot.  This is a number indicating
 * whether than a ditribution is older than another.
 *
 * Returns the priority.
 */
guint
sbuild_chroot_get_priority (const SbuildChroot *restrict chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT(chroot), 0);

  return chroot->priority;
}

/**
 * sbuild_chroot_set_priority:
 * @chroot: an #SbuildChroot.
 * @priority: the priority to set.
 *
 * Set the priority of a chroot.  This is a number indicating whether
 * a distribution is older than another.  For example, "oldstable" and
 * "oldstable-security" might be 0, while "stable" and
 * "stable-security" 1, "testing" 2 and "unstable" 3.  The values are
 * not important, but the difference between them is.
 */
void
sbuild_chroot_set_priority (SbuildChroot *chroot,
			    guint         priority)
{
  g_return_if_fail(SBUILD_IS_CHROOT(chroot));

  chroot->priority = priority;
  g_object_notify(G_OBJECT(chroot), "priority");
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
 * sbuild_chroot_get_chroot_type:
 * @chroot: an #SbuildChroot
 *
 * Get the type of the chroot.  This returns a user-readable string
 * which is equivalant to the type name of the subclassed
 * #SbuildChroot.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const gchar *
sbuild_chroot_get_chroot_type (const SbuildChroot  *chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT(chroot), NULL);

  SbuildChrootClass *klass = SBUILD_CHROOT_GET_CLASS(chroot);
  if (klass->get_chroot_type)
    return klass->get_chroot_type(chroot);
  else
    {
      g_error(_("%s chroot: chroot type is unset; error in derived class\n"),
	      chroot->name);
      return NULL;
    }
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
  g_return_if_fail(SBUILD_IS_CHROOT(chroot));

  g_fprintf(file, _("Name: %s\n"), chroot->name);
  g_fprintf(file, _("Description: %s\n"), chroot->description);
  g_fprintf(file, _("Type: %s\n"), sbuild_chroot_get_chroot_type(chroot));
  g_fprintf(file, _("Priority: %u\n"), chroot->priority);
  g_fprintf(file, _("Groups:"));
  if (chroot->groups)
    for (guint i=0; chroot->groups[i] != NULL; ++i)
      g_fprintf(file, " %s", chroot->groups[i]);
  g_fprintf(file, "\n%s", _( "Root Groups:"));
  if (chroot->root_groups)
    for (guint i=0; chroot->root_groups[i] != NULL; ++i)
      g_fprintf(file, " %s", chroot->root_groups[i]);
  g_fprintf(file, "\n%s", _( "Aliases:"));
  if (chroot->aliases)
    for (guint i=0; chroot->aliases[i] != NULL; ++i)
      g_fprintf(file, " %s", chroot->aliases[i]);
  g_fprintf(file, "\n");

  SbuildChrootClass *klass = SBUILD_CHROOT_GET_CLASS(chroot);
  if (klass->print_details)
    klass->print_details(chroot, file);

  /* Non user-settable properties are listed last. */
  if (chroot->mount_location)
    g_fprintf(file, _("Mount Location: %s\n"), chroot->mount_location);
  if (chroot->mount_device)
    g_fprintf(file, _("Mount Device: %s\n"), chroot->mount_device);

}

/**
 * sbuild_chroot_setup:
 * @chroot: an #SbuildChroot.
 * @env: the environment to set.
 *
 * Set up a chroot by runnning setup scripts.  This function is used
 * to set the environment that the scripts will see during execution.
 * Environment variables should be added to @env as "key=value"
 * strings (the format expected by execve envp).  These strings should
 * be allocated with g_free (or related allocation functions such as
 * g_strdup), and they must not be freed.
 */
void sbuild_chroot_setup (SbuildChroot  *chroot,
			  GList        **env)
{
  g_return_if_fail(SBUILD_IS_CHROOT(chroot));
  g_return_if_fail(env != NULL);

  *env = g_list_append(*env,
		       g_strdup_printf("CHROOT_TYPE=%s",
				       sbuild_chroot_get_chroot_type(chroot)));
  *env = g_list_append(*env,
		       g_strdup_printf("CHROOT_NAME=%s",
				       sbuild_chroot_get_name(chroot)));
  *env = g_list_append(*env,
		       g_strdup_printf("CHROOT_DESCRIPTION=%s",
				       sbuild_chroot_get_description(chroot)));
  *env = g_list_append(*env,
		       g_strdup_printf("CHROOT_MOUNT_LOCATION=%s",
				       sbuild_chroot_get_mount_location(chroot)));
  *env = g_list_append(*env,
		       g_strdup_printf("CHROOT_MOUNT_DEVICE=%s",
				       sbuild_chroot_get_mount_device(chroot)));

  SbuildChrootClass *klass = SBUILD_CHROOT_GET_CLASS(chroot);
  if (klass->setup)
    klass->setup(chroot, env);
}

static void
sbuild_chroot_init (SbuildChroot *chroot)
{
  g_return_if_fail(SBUILD_IS_CHROOT(chroot));

  chroot->name = NULL;
  chroot->description = NULL;
  chroot->priority = 0;
  chroot->groups = NULL;
  chroot->root_groups = NULL;
  chroot->aliases = NULL;
  chroot->mount_location = NULL;
  chroot->mount_device = NULL;
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
  if (chroot->mount_location)
    {
      g_free (chroot->mount_location);
      chroot->mount_location = NULL;
    }
  if (chroot->mount_device)
    {
      g_free (chroot->mount_device);
      chroot->mount_device = NULL;
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
    case PROP_PRIORITY:
      sbuild_chroot_set_priority(chroot, g_value_get_uint(value));
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
    case PROP_MOUNT_LOCATION:
      sbuild_chroot_set_mount_location(chroot, g_value_get_string(value));
      break;
    case PROP_MOUNT_DEVICE:
      sbuild_chroot_set_mount_device(chroot, g_value_get_string(value));
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
    case PROP_PRIORITY:
      g_value_set_uint(value, chroot->priority);
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
    case PROP_MOUNT_LOCATION:
      g_value_set_string(value, chroot->mount_location);
      break;
    case PROP_MOUNT_DEVICE:
      g_value_set_string(value, chroot->mount_device);
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

  klass->print_details = NULL;
  klass->setup = NULL;
  klass->get_chroot_type = NULL;

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
     PROP_PRIORITY,
     g_param_spec_uint ("priority", "Priority",
			"The priority of the chroot distribution, the lower the older the distribution",
			0, G_MAXUINT, 0,
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

  g_object_class_install_property
    (gobject_class,
     PROP_MOUNT_LOCATION,
     g_param_spec_string ("mount-location", "Mount Location",
			  "The mounted location (path) of the chroot",
			  "",
			  (G_PARAM_READABLE | G_PARAM_WRITABLE)));

  g_object_class_install_property
    (gobject_class,
     PROP_MOUNT_DEVICE,
     g_param_spec_string ("mount-device", "Mount Device",
			  "The block device for of the chroot",
			  "",
			  (G_PARAM_READABLE | G_PARAM_WRITABLE)));
}

/*
 * Local Variables:
 * mode:C
 * End:
 */
