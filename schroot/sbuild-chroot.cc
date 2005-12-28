/* Copyright © 2005  Roger Leigh <rleigh@debian.org>
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

#include <config.h>

#include "sbuild-i18n.h"
#include "sbuild-chroot.h"
#include "sbuild-keyfile.h"

using namespace sbuild;

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

const std::string&
Chroot::get_name () const
{
  return this->name;
}

void
Chroot::set_name (const std::string& name)
{
  this->name = name;
}

const std::string&
Chroot::get_description () const
{
  return this->description;
}

void
Chroot::set_description (const std::string& description)
{
  this->description = description;
}

const std::string&
Chroot::get_mount_location () const
{
  return this->mount_location;
}

void
Chroot::set_mount_location (const std::string& location)
{
  this->mount_location = location;
}

const std::string&
Chroot::get_mount_device () const
{
  return this->mount_device;
}

void
Chroot::set_mount_device (const std::string& device)
{
  this->mount_device = device;
}

unsigned int
Chroot::get_priority () const
{
  return this->priority;
}

void
Chroot::set_priority (unsigned int priority)
{
  this->priority = priority;
}

const string_list&
Chroot::get_groups () const
{
  return this->groups;
}

void
Chroot::set_groups (const string_list& groups)
{
  this->groups = groups;
}

const string_list&
Chroot::get_root_groups () const
{
  return this->root_groups;
}

void
Chroot::set_root_groups (const string_list& groups)
{
  this->root_groups = groups;
}

const string_list&
Chroot::get_aliases () const
{
  return this->aliases;
}

void
Chroot::set_aliases (const string_list& aliases)
{
  this->aliases = aliases;
}

bool
Chroot::get_active () const
{
  return this->active;
}

void
Chroot::set_active (bool active)
{
  this->active = active;
}

bool
Chroot::get_run_setup_scripts () const
{
  return this->run_setup_scripts;
}

void
Chroot::set_run_setup_scripts (bool run_setup_scripts)
{
  this->run_setup_scripts = run_setup_scripts;
}

bool
Chroot::get_run_session_scripts () const
{
  return this->run_session_scripts;
}

void
Chroot::set_run_session_scripts (bool run_session_scripts)
{
  this->run_session_scripts = run_session_scripts;
}

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

/*
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

void
Chroot::print_config (std::ostream& stream) const
{
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
