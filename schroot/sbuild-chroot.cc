/* Copyright © 2005-2006  Roger Leigh <rleigh@debian.org>
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

#include "sbuild.h"

#include <boost/format.hpp>

using boost::format;
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

Chroot::Chroot (keyfile const&     keyfile,
		std::string const& group):
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
  set_name(group);

  // This is set not in the configuration file, but set in the keyfile
  // manually.  The user must not have the ability to set this option.
  bool active(false);
  if (keyfile.get_value(group, "active",
			keyfile::PRIORITY_REQUIRED, active))
    set_active(active);

  bool run_setup_scripts(false);
  if (keyfile.get_value(group, "run-setup-scripts",
			keyfile::PRIORITY_OPTIONAL, run_setup_scripts))
    set_run_setup_scripts(run_setup_scripts);

  bool run_session_scripts(false);
  if (keyfile.get_value(group, "run-session-scripts",
			keyfile::PRIORITY_OPTIONAL, run_session_scripts))
    set_run_session_scripts(run_session_scripts);

  int priority(0);
  if (keyfile.get_value(group, "priority",
			keyfile::PRIORITY_OPTIONAL, priority))
    set_priority(priority);

  string_list aliases;
  if (keyfile.get_list_value(group, "aliases",
			     keyfile::PRIORITY_OPTIONAL, aliases))
    set_aliases(aliases);

  std::string description;
  if (keyfile.get_value(group, "description",
			keyfile::PRIORITY_OPTIONAL, description))
    set_description(description);

  string_list groups;
  if (keyfile.get_list_value(group, "groups",
			     keyfile::PRIORITY_REQUIRED, groups))
    set_groups(groups);

  string_list root_groups;
  if (keyfile.get_list_value(group, "root-groups",
			     keyfile::PRIORITY_OPTIONAL, root_groups))
    set_root_groups(root_groups);
}

Chroot::~Chroot()
{
}

Chroot::chroot_ptr
Chroot::create (std::string const& type)
{
  Chroot *new_chroot = 0;

  if (type == "plain")
    new_chroot = new ChrootPlain();
  else if (type == "block-device")
    new_chroot = new ChrootBlockDevice();
  else if (type == "lvm-snapshot")
    new_chroot = new ChrootLvmSnapshot();
  else
    {
      format fmt(_("unknown chroot type \"%1%\""));
      fmt % type;
      throw error(fmt);
    }

  if (new_chroot == 0)
    throw error(_("chroot creation failed"));

  return chroot_ptr(new_chroot);
}

Chroot::chroot_ptr
Chroot::create (keyfile const&     keyfile,
		std::string const& group)
{
  std::string type = "plain"; // "plain" is the default type.
  keyfile.get_value(group, "type", type);

  Chroot *new_chroot = 0;

  if (type == "plain")
    new_chroot = new ChrootPlain(keyfile, group);
  else if (type == "block-device")
    new_chroot = new ChrootBlockDevice(keyfile, group);
  else if (type == "lvm-snapshot")
    new_chroot = new ChrootLvmSnapshot(keyfile, group);
  else
    {
      format fmt(_("unknown chroot type \"%1%\""));
      fmt % type;
      throw error(fmt);
    }

  if (new_chroot == 0)
    {
      format fmt(_("%1% chroot creation failed"));
      fmt % group;
      throw error(fmt);
    }

  return chroot_ptr(new_chroot);
}


std::string const&
Chroot::get_name () const
{
  return this->name;
}

void
Chroot::set_name (std::string const& name)
{
  this->name = name;
}

std::string const&
Chroot::get_description () const
{
  return this->description;
}

void
Chroot::set_description (std::string const& description)
{
  this->description = description;
}

std::string const&
Chroot::get_mount_location () const
{
  return this->mount_location;
}

void
Chroot::set_mount_location (std::string const& location)
{
  this->mount_location = location;
}

std::string const&
Chroot::get_mount_device () const
{
  return this->mount_device;
}

void
Chroot::set_mount_device (std::string const& device)
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

string_list const&
Chroot::get_groups () const
{
  return this->groups;
}

void
Chroot::set_groups (string_list const& groups)
{
  this->groups = groups;
}

string_list const&
Chroot::get_root_groups () const
{
  return this->root_groups;
}

void
Chroot::set_root_groups (string_list const& groups)
{
  this->root_groups = groups;
}

string_list const&
Chroot::get_aliases () const
{
  return this->aliases;
}

void
Chroot::set_aliases (string_list const& aliases)
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
Chroot::setup_env (environment& env)
{
  env.add("CHROOT_TYPE", get_chroot_type());
  env.add("CHROOT_NAME", get_name());
  env.add("CHROOT_DESCRIPTION", get_description());
  env.add("CHROOT_MOUNT_LOCATION", get_mount_location());
  env.add("CHROOT_MOUNT_DEVICE", get_mount_device());
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
  stream << format_details(_("Name"), get_name())
	 << format_details(_("Description"), get_description())
	 << format_details(_("Type"), get_chroot_type())
	 << format_details(_("Priority"), get_priority())
	 << format_details(_("Groups"), get_groups())
	 << format_details(_("Root Groups"), get_root_groups())
	 << format_details(_("Aliases"), get_aliases())
	 << format_details(_("Run Setup Scripts"), get_run_setup_scripts())
	 << format_details(_("Run Session Scripts"),
			   get_run_session_scripts());

  /* Non user-settable properties are listed last. */
  if (!this->mount_location.empty())
    stream << format_details(_("Mount Location"),
			     get_mount_location());
  if (!this->mount_device.empty())
    stream << format_details(_("Mount Device"), get_mount_device());
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

/*
 * Local Variables:
 * mode:C++
 * End:
 */
