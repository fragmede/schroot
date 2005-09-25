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

static void
sbuild_chroot_set_active (SbuildChroot *chroot);

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
  PROP_CURRENT_USERS,
  PROP_MAX_USERS,
  PROP_ACTIVE,
  PROP_RUN_SETUP
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
 * @active: TRUE if the chroot is an active session, otherwise FALSE
 *
 * Creates a new #SbuildChroot.
 *
 * Returns the newly created #SbuildChroot.
 */
SbuildChroot *
sbuild_chroot_new_from_keyfile (GKeyFile   *keyfile,
				const char *group,
				gboolean    active)
{
  g_return_val_if_fail(group != NULL, NULL);

  GType chroot_type = 0;

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
      }
    g_free(type);
  }

  SbuildChroot *chroot = NULL;
  if (chroot_type != 0)
    chroot =  (SbuildChroot *) g_object_new(chroot_type,
					    "name", group,
					    NULL);

  if (chroot)
    {
      if (active == TRUE)
	sbuild_chroot_set_active(chroot);
      sbuild_chroot_set_properties_from_keyfile(chroot, keyfile);
    }

  return chroot;
}

/**
 * sbuild_chroot_set_properties_from_keyfile:
 * @chroot: an #SbuildChroot.
 * @keyfile: the #GKeyFile containing the chroot configuration
 *
 * Sets the properties of an existing #SbuildChroot from a #GKeyFile.
 *
 * Returns TRUE on success, FALSE on failure.
 */
gboolean
sbuild_chroot_set_properties_from_keyfile (SbuildChroot *chroot,
					   GKeyFile     *keyfile)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT(chroot), FALSE);
  g_return_val_if_fail(chroot->name != NULL, FALSE);

  GObjectClass *klass;
  GParameter *params = NULL;
  guint n_params = 0;
  guint n_alloc_params = 16;

  /* Find out which chroot type we want.  These are all linked
     statically, for reasons of security, because we don't want
     random modules loading outside our control. */
  {
    GError *error = NULL;
    char *type =
      g_key_file_get_string(keyfile, chroot->name, "type", &error);

    if (error != NULL)
      {
	g_clear_error(&error);
	type = g_strdup("plain");
      }

    if (strcmp(type, sbuild_chroot_get_chroot_type(chroot)) != 0)
      {
	g_warning("%s chroot: chroot type \"%s\" does not match keyfile type \"%s\"",
		  chroot->name, sbuild_chroot_get_chroot_type(chroot), type);
	g_free(type);
	return FALSE;
      }

    g_free(type);
  }

  gchar **keys = g_key_file_get_keys(keyfile, chroot->name, NULL, NULL);
  if (keys)
    {
      klass = G_OBJECT_CLASS(SBUILD_CHROOT_GET_CLASS(chroot));

      for (guint i = 0; keys[i] != NULL; ++i)
	{
	  gchar *key = keys[i];

	  if (strcmp(key, "type") == 0) // Used previously
	    continue;

	  GError *error = NULL;
	  GParamSpec *pspec = g_object_class_find_property(klass, key);

	  if (!pspec)
	    {
	      g_warning (_("%s chroot: chroot type '%s' has no property named '%s'"),
			 chroot->name,
			 sbuild_chroot_get_chroot_type(chroot),
			 key);
	      continue; // TODO: Should a config file error be fatal?
	    }

	  /* Only construction properties may be set if reading from a
	     configuration file, otherwise if reloading an active
	     session, writable properties may also be set. */
	  if ((chroot->active == TRUE &&
	       ((pspec->flags & G_PARAM_WRITABLE) !=
		G_PARAM_WRITABLE)) ||
	      (chroot->active == FALSE &&
	       ((pspec->flags & (G_PARAM_WRITABLE|G_PARAM_CONSTRUCT)) !=
		(G_PARAM_WRITABLE|G_PARAM_CONSTRUCT))))
	    {
	      g_warning (_("%s chroot: property '%s' is not user-settable"),
			 chroot->name, key);
	      continue; // TODO: Should a config file error be fatal?
	    }

	  if (G_PARAM_SPEC_VALUE_TYPE(pspec) == G_TYPE_BOOLEAN)
	    {
	      gboolean value =
		g_key_file_get_boolean(keyfile, chroot->name, key, &error);

	      if (error != NULL)
		g_clear_error(&error);
	      else
		g_object_set(chroot, key, value, NULL);
	    }
	  else if (G_PARAM_SPEC_VALUE_TYPE(pspec) == G_TYPE_UINT)
	    {
	      gint num =
		g_key_file_get_integer(keyfile, chroot->name, key, &error);

	      if (error != NULL)
		g_clear_error(&error);
	      else
		g_object_set(chroot, key, num, NULL);
	    }
	  else if (G_PARAM_SPEC_VALUE_TYPE(pspec) == G_TYPE_STRING)
	    {
	      char *str =
		g_key_file_get_locale_string(keyfile, chroot->name, key, NULL, &error);

	      if (error != NULL)
		g_clear_error(&error);
	      else
		g_object_set(chroot, key, str, NULL);
	    }
	  else if (G_PARAM_SPEC_VALUE_TYPE(pspec) == G_TYPE_STRV)
	    {
	      char **strv =
		g_key_file_get_string_list(keyfile, chroot->name, key, NULL, &error);

	      if (error != NULL)
		g_clear_error(&error);
	      else
		g_object_set(chroot, key, strv, NULL);
	    }
	  else
	    {
	      g_warning (_("%s chroot: property '%s' has unsupported type '%s'"),
			 chroot->name,
			 key,
			 g_type_name (G_PARAM_SPEC_VALUE_TYPE(pspec)));
	    }
	}
    }
  return TRUE;
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
 * sbuild_chroot_get_current_users:
 * @chroot: an #SbuildChroot
 *
 * Get the current number of users of the chroot.
 *
 * Returns the current number of users.
 */
guint
sbuild_chroot_get_current_users (const SbuildChroot *restrict chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT(chroot), 0);

  return chroot->current_users;
}

/**
 * sbuild_chroot_set_current_users:
 * @chroot: an #SbuildChroot.
 * @current_users: the current_users to set.
 *
 * Set the current number of users of a chroot.
 */
void
sbuild_chroot_set_current_users (SbuildChroot *chroot,
				 guint         current_users)
{
  g_return_if_fail(SBUILD_IS_CHROOT(chroot));

  chroot->current_users = current_users;
  g_object_notify(G_OBJECT(chroot), "current-users");
}

/**
 * sbuild_chroot_get_max_users:
 * @chroot: an #SbuildChroot
 *
 * Get the maximum number of users of the chroot.
 *
 * Returns the max number of users.
 */
guint
sbuild_chroot_get_max_users (const SbuildChroot *restrict chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT(chroot), 0);

  return chroot->max_users;
}

/**
 * sbuild_chroot_set_max_users:
 * @chroot: an #SbuildChroot.
 * @max_users: the max_users to set.
 *
 * Set the maximum number of users of a chroot.
 */
void
sbuild_chroot_set_max_users (SbuildChroot *chroot,
				 guint         max_users)
{
  g_return_if_fail(SBUILD_IS_CHROOT(chroot));

  chroot->max_users = max_users;
  g_object_notify(G_OBJECT(chroot), "max-users");
}

/**
 * sbuild_chroot_set_current_users:
 * @chroot: an #SbuildChroot.
 * @current_users: the current_users to set.
 *
 * Set the current number of users of a chroot.
 */
static void
sbuild_chroot_set_active (SbuildChroot *chroot)
{
  g_return_if_fail(SBUILD_IS_CHROOT(chroot));

  chroot->active = TRUE;
  g_object_notify(G_OBJECT(chroot), "active");
}

/**
 * sbuild_chroot_get_run_setup:
 * @chroot: an #SbuildChroot
 *
 * Check if chroot setup scripts will be run.
 *
 * Returns TRUE if setup scripts will be run, otherwise FALSE.
 */
gboolean
sbuild_chroot_get_run_setup (const SbuildChroot *restrict chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT(chroot), FALSE);

  return chroot->run_setup;
}

/**
 * sbuild_chroot_set_run_setup:
 * @chroot: an #SbuildChroot.
 * @run_setup: TRUE to run setup scripts, otherwise FALSE.
 *
 * Set whether chroot setup scripts should be run or not.
 */
void
sbuild_chroot_set_run_setup (SbuildChroot *chroot,
			     gboolean      run_setup)
{
  g_return_if_fail(SBUILD_IS_CHROOT(chroot));

  chroot->run_setup = run_setup;
  g_object_notify(G_OBJECT(chroot), "run-setup");
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
 * sbuild_chroot_get_session_flags:
 * @chroot: an #SbuildChroot
 *
 * Get the session flags of the chroot.  These determine how the
 * #SbuildSession controlling the chroot will operate.
 *
 * Returns the #SbuildChrootSessionFlags.
 */
SbuildChrootSessionFlags
sbuild_chroot_get_session_flags (const SbuildChroot  *chroot)
{
  g_return_val_if_fail(SBUILD_IS_CHROOT(chroot), 0);

  SbuildChrootClass *klass = SBUILD_CHROOT_GET_CLASS(chroot);
  if (klass->get_session_flags)
    return klass->get_session_flags(chroot);
  else
    {
      g_error(_("%s chroot: chroot session flags unset; error in derived class\n"),
	      chroot->name);
      return 0;
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

  g_fprintf(file, _("  --- Chroot ---\n"));
  g_fprintf(file, "  %-22s%s\n", _("Name"), chroot->name);
  g_fprintf(file, "  %-22s%s\n", _("Description"), chroot->description);
  g_fprintf(file, "  %-22s%s\n", _("Type"), sbuild_chroot_get_chroot_type(chroot));
  g_fprintf(file, "  %-22s%u\n", _("Priority"), chroot->priority);

  char *group_list = (chroot->groups) ? g_strjoinv(" ", chroot->groups) : NULL;
  g_fprintf(file, "  %-22s%s\n", _("Groups"),
	    (group_list) ? group_list : "");
  g_free(group_list);

  char *root_group_list = (chroot->root_groups) ?
    g_strjoinv(" ", chroot->root_groups) : NULL;
  g_fprintf(file, "  %-22s%s\n", _("Root Groups"),
	    (root_group_list) ? root_group_list : NULL);
  g_free(root_group_list);

  char *alias_list = (chroot->aliases) ? g_strjoinv(" ", chroot->aliases) : NULL;
  g_fprintf(file, "  %-22s%s\n", _("Aliases"),
	    (alias_list) ? alias_list : NULL);
  g_free(alias_list);

  g_fprintf(file, _("  %-22s%u\n"), _("Maximum Users"), chroot->max_users);
  g_fprintf(file, _("  %-22s%s\n"), _("Run Setup"),
	    (chroot->run_setup == TRUE) ? "true" : "false");

  SbuildChrootClass *klass = SBUILD_CHROOT_GET_CLASS(chroot);
  if (klass->print_details)
    klass->print_details(chroot, file);

  /* Non user-settable properties are listed last. */
  if (chroot->active == TRUE)
    {
      if (chroot->mount_location)
	g_fprintf(file, "  %-22s%s\n", _("Mount Location"), chroot->mount_location);
      if (chroot->mount_device)
	g_fprintf(file, "  %-22s%s\n", _("Mount Device"), chroot->mount_device);
      g_fprintf(file, _("  %-22s%u\n"), _("Current Users"), chroot->current_users);
    }
}

/**
 * sbuild_chroot_print_config:
 * @chroot: an #SbuildChroot.
 * @file: the file to output to.
 *
 * Print the configuration group for a chroot in the format required
 * by schroot.conf.
 */
void sbuild_chroot_print_config (SbuildChroot *chroot,
				  FILE         *file)
{
  g_return_if_fail(SBUILD_IS_CHROOT(chroot));

  g_fprintf(file, "[%s]\n", chroot->name);
  g_fprintf(file, "description=%s\n", chroot->description);
  g_fprintf(file, "type=%s\n", sbuild_chroot_get_chroot_type(chroot));
  g_fprintf(file, "priority=%u\n", chroot->priority);

  if (chroot->groups)
    {
      char *group_list = g_strjoinv(",", chroot->groups);
      g_fprintf(file, "groups=%s\n", group_list);
      g_free(group_list);
    }

  if (chroot->root_groups)
    {
      char *root_group_list = g_strjoinv(",", chroot->root_groups);
      g_fprintf(file, "root-groups=%s\n", root_group_list);
      g_free(root_group_list);
    }

  if (chroot->aliases)
    {
      char *alias_list = g_strjoinv(",", chroot->aliases);
      g_fprintf(file, "aliases=%s\n", alias_list);
      g_free(alias_list);
    }

  g_fprintf(file, "max-users=%u\n", chroot->max_users);
  g_fprintf(file, _("run-setup=%s\n"),
	    (chroot->run_setup == TRUE) ? "true" : "false");

  SbuildChrootClass *klass = SBUILD_CHROOT_GET_CLASS(chroot);
  if (klass->print_config)
    klass->print_config(chroot, file);

  /* Non user-settable properties are listed last. */
  if (chroot->mount_location)
    g_fprintf(file, "mount-location=%s\n", chroot->mount_location);
  if (chroot->mount_device)
    g_fprintf(file, "mount-device%s\n", chroot->mount_device);
  g_fprintf(file, "current-users=%u\n", chroot->current_users);
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
  *env = g_list_append(*env,
		       g_strdup_printf("CHROOT_CURRENT_USERS=%u",
				       chroot->current_users));
  *env = g_list_append(*env,
		       g_strdup_printf("CHROOT_MAX_USERS=%u",
				       chroot->max_users));

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
  chroot->current_users = 0;
  chroot->max_users = 0;
  chroot->active = FALSE;
  chroot->run_setup = FALSE;
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
    case PROP_CURRENT_USERS:
      sbuild_chroot_set_current_users(chroot, g_value_get_uint(value));
      break;
    case PROP_MAX_USERS:
      sbuild_chroot_set_max_users(chroot, g_value_get_uint(value));
      break;
    case PROP_RUN_SETUP:
      sbuild_chroot_set_run_setup(chroot, g_value_get_boolean(value));
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
    case PROP_CURRENT_USERS:
      g_value_set_uint(value, chroot->current_users);
      break;
    case PROP_MAX_USERS:
      g_value_set_uint(value, chroot->max_users);
      break;
    case PROP_ACTIVE:
      g_value_set_boolean(value, chroot->active);
      break;
    case PROP_RUN_SETUP:
      g_value_set_boolean(value, chroot->run_setup);
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
  klass->print_config = NULL;
  klass->setup = NULL;
  klass->get_chroot_type = NULL;
  klass->get_session_flags = NULL;

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

  g_object_class_install_property
    (gobject_class,
     PROP_CURRENT_USERS,
     g_param_spec_uint ("current-users", "Current Users",
			"The number of users currently using this chroot",
			0, G_MAXUINT, 1,
			(G_PARAM_READABLE | G_PARAM_WRITABLE)));

  g_object_class_install_property
    (gobject_class,
     PROP_MAX_USERS,
     g_param_spec_uint ("max-users", "Maximum Users",
			"The maximum number of users able to use this chroot",
			1, G_MAXUINT, 1,
			(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT)));

  g_object_class_install_property
    (gobject_class,
     PROP_ACTIVE,
     g_param_spec_boolean ("active", "Active",
			   "Is the chroot currently in use?",
			   FALSE,
			   (G_PARAM_READABLE)));

  g_object_class_install_property
    (gobject_class,
     PROP_RUN_SETUP,
     g_param_spec_boolean ("run-setup", "Run Setup",
			   "Run chroot setup scripts?",
			   FALSE,
			   (G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT)));
}

/*
 * Local Variables:
 * mode:C
 * End:
 */
