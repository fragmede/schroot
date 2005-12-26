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

#include "sbuild-i18n.h"
#include "sbuild-chroot.h"
#include "sbuild-keyfile.h"

using namespace sbuild;

/**
 * sbuild_chroot_new:
 *
 * Creates a new #Chroot.
 *
 * Returns the newly created #Chroot.
 */
Chroot::Chroot ():
  name(),
  description(),
  priority(0),
  groups(),
  root_groups(),
  aliases(),
  mount_location(),
  mount_device(),
  active(false),
  run_setup_scripts(false),
  run_session_scripts(false)
{
}

Chroot::Chroot (const keyfile&     keyfile,
		const std::string& group):
  name(),
  description(),
  priority(0),
  groups(),
  root_groups(),
  aliases(),
  mount_location(),
  mount_device(),
  active(false),
  run_setup_scripts(false),
  run_session_scripts(false)
{
  read_keyfile(keyfile, group);
}

Chroot::~Chroot()
{
}

/**
 * sbuild_chroot_get_name:
 * @chroot: an #Chroot
 *
 * Get the name of the chroot.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const std::string&
Chroot::get_name () const
{
  return this->name;
}

/**
 * sbuild_chroot_set_name:
 * @chroot: an #Chroot.
 * @name: the name to set.
 *
 * Set the name of a chroot.
 */
void
Chroot::set_name (const std::string& name)
{
  this->name = name;
}

/**
 * sbuild_chroot_get_description:
 * @chroot: an #Chroot
 *
 * Get the description of the chroot.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const std::string&
Chroot::get_description () const
{
  return this->description;
}

/**
 * sbuild_chroot_set_description:
 * @chroot: an #Chroot.
 * @description: the description to set.
 *
 * Set the description of a chroot.
 */
void
Chroot::set_description (const std::string& description)
{
  this->description = description;
}

/**
 * sbuild_chroot_get_mount_location:
 * @chroot: an #Chroot
 *
 * Get the mount location of the chroot.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const std::string&
Chroot::get_mount_location () const
{
  return this->mount_location;
}

/**
 * sbuild_chroot_set_mount_location:
 * @chroot: an #Chroot.
 * @location: the mount location to set.
 *
 * Set the mount location of a chroot.
 */
void
Chroot::set_mount_location (const std::string& location)
{
  this->mount_location = location;
}

/**
 * sbuild_chroot_get_mount_device:
 * @chroot: an #Chroot
 *
 * Get the mount device of the chroot.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const std::string&
Chroot::get_mount_device () const
{
  return this->mount_device;
}

/**
 * sbuild_chroot_set_mount_device:
 * @chroot: an #Chroot.
 * @device: the mount device to set.
 *
 * Set the mount device of a chroot.
 */
void
Chroot::set_mount_device (const std::string& device)
{
  this->mount_device = device;
}

/**
 * sbuild_chroot_get_priority:
 * @chroot: an #Chroot
 *
 * Get the priority of the chroot.  This is a number indicating
 * whether than a ditribution is older than another.
 *
 * Returns the priority.
 */
unsigned int
Chroot::get_priority () const
{
  return this->priority;
}

/**
 * sbuild_chroot_set_priority:
 * @chroot: an #Chroot.
 * @priority: the priority to set.
 *
 * Set the priority of a chroot.  This is a number indicating whether
 * a distribution is older than another.  For example, "oldstable" and
 * "oldstable-security" might be 0, while "stable" and
 * "stable-security" 1, "testing" 2 and "unstable" 3.  The values are
 * not important, but the difference between them is.
 */
void
Chroot::set_priority (unsigned int priority)
{
  this->priority = priority;
}

/**
 * sbuild_chroot_get_groups:
 * @chroot: an #Chroot
 *
 * Get the groups of the chroot.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const string_list&
Chroot::get_groups () const
{
  return this->groups;
}

/**
 * sbuild_chroot_set_groups:
 * @chroot: an #Chroot.
 * @groups: the groups to set.
 *
 * Set the groups of a chroot.
 */
void
Chroot::set_groups (const string_list& groups)
{
  this->groups = groups;
}

/**
 * sbuild_chroot_get_root_groups:
 * @chroot: an #Chroot
 *
 * Get the root groups of the chroot.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const string_list&
Chroot::get_root_groups () const
{
  return this->root_groups;
}

/**
 * sbuild_chroot_set_root_groups:
 * @chroot: an #Chroot.
 * @groups: the groups to set.
 *
 * Set the groups of a chroot.
 */
void
Chroot::set_root_groups (const string_list& groups)
{
  this->root_groups = groups;
}

/**
 * sbuild_chroot_get_aliases:
 * @chroot: an #Chroot
 *
 * Get the aliases of the chroot.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */

const string_list&
Chroot::get_aliases () const
{
  return this->aliases;
}

/**
 * sbuild_chroot_set_aliases:
 * @chroot: an #Chroot.
 * @aliases: the aliases to set.
 *
 * Set the aliases of a chroot.
 */
void
Chroot::set_aliases (const string_list& aliases)
{
  this->aliases = aliases;
}

/**
 * sbuild_chroot_get_active:
 * @chroot: an #Chroot
 *
 * Get the activity status of the chroot.
 *
 * Returns true if active, false if inactive.
 */
bool
Chroot::get_active () const
{
  return this->active;
}

/**
 * sbuild_chroot_set_active:
 * @chroot: an #Chroot.
 * @active: true if active, false if inactive.
 *
 * Set the activity status of the chroot.
 */
void
Chroot::set_active (bool active)
{
  this->active = active;
}

/**
 * sbuild_chroot_get_run_setup_scripts:
 * @chroot: an #Chroot
 *
 * Check if chroot setup scripts will be run.
 *
 * Returns true if setup scripts will be run, otherwise false.
 */
bool
Chroot::get_run_setup_scripts () const
{
  return this->run_setup_scripts;
}

/**
 * sbuild_chroot_set_run_setup_scripts:
 * @chroot: an #Chroot.
 * @run_setup_scripts: true to run setup scripts, otherwise false.
 *
 * Set whether chroot setup scripts should be run or not.
 */
void
Chroot::set_run_setup_scripts (bool run_setup_scripts)
{
  this->run_setup_scripts = run_setup_scripts;
}

/**
 * sbuild_chroot_set_run_session_scripts:
 * @chroot: an #Chroot.
 * @run_session_scripts: true to run session scripts, otherwise false.
 *
 * Set whether chroot session scripts should be run or not.
 */
bool
Chroot::get_run_session_scripts () const
{
  return this->run_session_scripts;
}

/**
 * sbuild_chroot_get_run_session_scripts:
 * @chroot: an #Chroot
 *
 * Check if chroot session scripts will be run.
 *
 * Returns true if session scripts will be run, otherwise false.
 */
void
Chroot::set_run_session_scripts (bool run_session_scripts)
{
  this->run_session_scripts = run_session_scripts;
}

/**
 * sbuild_chroot_setup_env:
 * @chroot: an #Chroot.
 * @env: the environment to set.
 *
 * This function is used to set the environment that the setup scripts
 * will see during execution.
 */
void
Chroot::setup_env (env_list& env)
{
  setup_env_var(env, "CHROOT_TYPE",
		get_chroot_type());
  setup_env_var(env, "CHROOT_NAME",
		get_name());
  setup_env_var(env, "CHROOT_DESCRIPTION",
		get_description());
  setup_env_var(env, "CHROOT_MOUNT_LOCATION",
		get_mount_location());
  setup_env_var(env, "CHROOT_MOUNT_DEVICE",
		get_mount_device());
}

/**
 * sbuild_chroot_print_details:
 * @chroot: an #Chroot.
 * @file: the file to output to.
 *
 * Print detailed information about @chroot to @file.  The information
 * is printed in plain text with one line per property.
 */
void
Chroot::print_details (std::ostream& stream) const
{
  if (this->active == true)
    stream << _("  --- Session ---\n");
  else
    stream << _("  --- Chroot ---\n");
  stream << format_detail_string(_("Name"), get_name())
	 << format_detail_string(_("Description"), get_description())
	 << format_detail_string(_("Type"), get_chroot_type())
	 << format_detail_int(_("Priority"), get_priority())
	 << format_detail_strv(_("Groups"), get_groups())
	 << format_detail_strv(_("Root Groups"), get_root_groups())
	 << format_detail_strv(_("Aliases"), get_aliases())
	 << format_detail_bool(_("Run Setup Scripts"),
			       get_run_setup_scripts())
	 << format_detail_bool(_("Run Session Scripts"),
			       get_run_session_scripts());
  /* Non user-settable properties are listed last. */
  if (this->active == true)
    {
      if (!this->mount_location.empty())
	stream << format_detail_string(_("Mount Location"),
				       get_mount_location());
      if (!this->mount_device.empty())
	stream << format_detail_string(_("Mount Device"), get_mount_device());
    }
}

/**
 * sbuild_chroot_print_config:
 * @chroot: an #Chroot.
 * @file: the file to output to.
 *
 * Print the configuration group for a chroot in the format required
 * by schroot.conf.
 */
void
Chroot::print_config (std::ostream& stream) const
{
  // TODO: Replace with proper serialisation code, once GKeyFile is
  // gone.

  stream << '[' << get_name() << "]\n";
  if (get_active())
    stream << "active=true\n";
  if (!this->description.empty())
    stream << "description=" << this->description << '\n';
  stream << "type=" << get_chroot_type() << '\n'
	 << "priority=" << get_priority() << '\n';

  if (!this->groups.empty())
    {
      stream << "groups=" << string_list_to_string(this->groups, ",") << '\n';
    }

  if (!this->root_groups.empty())
    {
      stream << "root-groups=" << string_list_to_string(this->root_groups, ",")
	     << '\n';
    }

  if (!this->aliases.empty())
    {
      stream << "aliases=" << string_list_to_string(this->aliases, ",") << '\n';
    }

  const char *setup = (this->run_setup_scripts == true) ? "true" : "false";
  const char *session = (this->run_session_scripts == true) ? "true" : "false";
  stream << "run-setup-scripts=" << setup << '\n'
	 << "run-session-scripts=" << session << '\n';

  /* Non user-settable properties are listed last. */
  if (!this->mount_location.empty())
    stream << "mount-location=" << get_mount_location() << '\n';
  if (!this->mount_device.empty())
    stream << "mount-device=" << get_mount_device() << '\n';
}

void
Chroot::read_keyfile (const keyfile&     keyfile,
		      const std::string& group)
{
  bool active(false);
  if (keyfile.get_value(group, "active", active))
    set_active(active);

  bool run_setup_scripts(false);
  if (keyfile.get_value(group, "run-setup-scripts", run_setup_scripts))
    set_run_setup_scripts(run_setup_scripts);

  bool run_session_scripts(false);
  if (keyfile.get_value(group, "run-session-scripts", run_session_scripts))
    set_run_session_scripts(run_session_scripts);

  string_list aliases;
  if (keyfile.get_list_value(group, "aliases", aliases))
    set_aliases(aliases);

  std::string description;
  if (keyfile.get_value(group, "description", description))
    set_description(description);

  std::string name;
  if (keyfile.get_value(group, "name", name))
    set_name(name);

  string_list groups;
  if (keyfile.get_list_value(group, "groups", groups))
    set_groups(groups);

  string_list root_groups;
  if (keyfile.get_list_value(group, "root-groups", root_groups))
    set_root_groups(root_groups);
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
