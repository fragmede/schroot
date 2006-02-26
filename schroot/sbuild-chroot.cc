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

#include <cerrno>
#include <ext/stdio_filebuf.h>

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

sbuild::chroot::chroot ():
  name(),
  description(),
  priority(0),
  groups(),
  root_groups(),
  aliases(),
  mount_location(),
  location(),
  mount_device(),
  active(false),
  run_setup_scripts(false),
  run_session_scripts(false)
{
}

sbuild::chroot::~chroot ()
{
}

sbuild::chroot::ptr
sbuild::chroot::create (std::string const& type)
{
  chroot *new_chroot = 0;

  if (type == "plain")
    new_chroot = new chroot_plain();
  else if (type == "file")
    new_chroot = new chroot_file();
  else if (type == "block-device")
    new_chroot = new chroot_block_device();
  else if (type == "lvm-snapshot")
    new_chroot = new chroot_lvm_snapshot();
  else
    {
      format fmt(_("unknown chroot type \"%1%\""));
      fmt % type;
      throw error(fmt);
    }

  if (new_chroot == 0)
    throw error(_("chroot creation failed"));

  return ptr(new_chroot);
}

std::string const&
sbuild::chroot::get_name () const
{
  return this->name;
}

void
sbuild::chroot::set_name (std::string const& name)
{
  this->name = name;
}

std::string const&
sbuild::chroot::get_description () const
{
  return this->description;
}

void
sbuild::chroot::set_description (std::string const& description)
{
  this->description = description;
}

std::string const&
sbuild::chroot::get_mount_location () const
{
  return this->mount_location;
}

void
sbuild::chroot::set_mount_location (std::string const& location)
{
  this->mount_location = location;
}

std::string const&
sbuild::chroot::get_location () const
{
  return this->location;
}

void
sbuild::chroot::set_location (std::string const& location)
{
  this->location = location;
}

std::string
sbuild::chroot::get_path () const
{
  return get_mount_location() + get_location();
}

std::string const&
sbuild::chroot::get_mount_device () const
{
  return this->mount_device;
}

void
sbuild::chroot::set_mount_device (std::string const& device)
{
  this->mount_device = device;
}

unsigned int
sbuild::chroot::get_priority () const
{
  return this->priority;
}

void
sbuild::chroot::set_priority (unsigned int priority)
{
  this->priority = priority;
}

string_list const&
sbuild::chroot::get_groups () const
{
  return this->groups;
}

void
sbuild::chroot::set_groups (string_list const& groups)
{
  this->groups = groups;
}

string_list const&
sbuild::chroot::get_root_groups () const
{
  return this->root_groups;
}

void
sbuild::chroot::set_root_groups (string_list const& groups)
{
  this->root_groups = groups;
}

string_list const&
sbuild::chroot::get_aliases () const
{
  return this->aliases;
}

void
sbuild::chroot::set_aliases (string_list const& aliases)
{
  this->aliases = aliases;
}

bool
sbuild::chroot::get_active () const
{
  return this->active;
}

void
sbuild::chroot::set_active (bool active)
{
  this->active = active;
}

bool
sbuild::chroot::get_run_setup_scripts () const
{
  return this->run_setup_scripts;
}

void
sbuild::chroot::set_run_setup_scripts (bool run_setup_scripts)
{
  this->run_setup_scripts = run_setup_scripts;
}

bool
sbuild::chroot::get_run_session_scripts () const
{
  return this->run_session_scripts;
}

void
sbuild::chroot::set_run_session_scripts (bool run_session_scripts)
{
  this->run_session_scripts = run_session_scripts;
}

void
sbuild::chroot::setup_env (environment& env)
{
  env.add("CHROOT_TYPE", get_chroot_type());
  env.add("CHROOT_NAME", get_name());
  env.add("CHROOT_DESCRIPTION", get_description());
  env.add("CHROOT_LOCATION", get_location());
  env.add("CHROOT_MOUNT_LOCATION", get_mount_location());
  env.add("CHROOT_PATH", get_path());
  env.add("CHROOT_MOUNT_DEVICE", get_mount_device());
}

void
sbuild::chroot::setup_session_info (bool start)
{
  /* Create or unlink session information. */
  std::string file = std::string(SCHROOT_SESSION_DIR) + "/" + get_name();

  if (start)
    {
      int fd = open(file.c_str(), O_CREAT|O_EXCL|O_WRONLY, 0664);
      if (fd < 0)
	{
	  format fmt(_("%1%: failed to create session file: %2%\n"));
	  fmt % file % strerror(errno);
	  throw error(fmt);
	}

      // Create a stream buffer from the file descriptor.  The fd will
      // be closed when the buffer is destroyed.
      __gnu_cxx::stdio_filebuf<char> fdbuf(fd, std::ios::out);
      std::ostream output(&fdbuf);
      output.imbue(std::locale("C"));

      sbuild::file_lock lock(fd);
      try
	{
	  lock.set_lock(lock::LOCK_EXCLUSIVE, 2);
	}
      catch (lock::error const& e)
	{
	  format fmt(_("%1%: lock acquisition failure: %2%\n"));
	  fmt % file % e.what();
	  throw error(fmt);
	}

      keyfile details;
      get_keyfile(details);
      output << details;

      try
	{
	  lock.unset_lock();
	}
      catch (lock::error const& e)
	{
	  format fmt(_("%1%: lock discard failure: %2%\n"));
	  fmt % file % e.what();
	  throw error(fmt);
	}
    }
  else /* start == false */
    {
      if (unlink(file.c_str()) != 0)
	{
	  format fmt(_("%1%: failed to unlink session file: %2%\n"));
	  fmt % file % strerror(errno);
	  throw error(fmt);
	}
    }
}

void
sbuild::chroot::print_details (std::ostream& stream) const
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
			   get_run_session_scripts())
	 << format_details(_("Session Managed"),
			   static_cast<bool>(get_session_flags() &
					     chroot::SESSION_CREATE));

  /* Non user-settable properties are listed last. */
  if (!get_location().empty())
    stream << format_details(_("Location"),
			     get_location());
  if (!get_mount_location().empty())
    stream << format_details(_("Mount Location"),
			     get_mount_location());
  if (!get_path().empty())
    stream << format_details(_("Path"),
			     get_path());
  if (!get_mount_device().empty())
    stream << format_details(_("Mount Device"), get_mount_device());
}

void
sbuild::chroot::get_keyfile (keyfile& keyfile) const
{
  keyfile.remove_group(this->name);

  keyfile.set_value(this->name, "type",
		    get_chroot_type());

  keyfile.set_value(this->name, "active",
		    get_active());

  keyfile.set_value(this->name, "run-setup-scripts",
		    get_run_setup_scripts());

  keyfile.set_value(this->name, "run-session-scripts",
		    get_run_session_scripts());

  keyfile.set_value(this->name, "priority",
		    get_priority());

  keyfile.set_list_value(this->name, "aliases",
			 get_aliases());

  keyfile.set_value(this->name, "description",
		    get_description());

  keyfile.set_list_value(this->name, "groups",
			 get_groups());

  keyfile.set_list_value(this->name, "root-groups",
			 get_root_groups());

  if (get_active())
    keyfile.set_value(this->name, "mount-location",
		      get_mount_location());

  if (get_active())
    keyfile.set_value(this->name, "mount-device",
		      get_mount_device());
}

void
sbuild::chroot::set_keyfile (keyfile const& keyfile)
{
  // This is set not in the configuration file, but set in the keyfile
  // manually.  The user must not have the ability to set this option.
  bool active(false);
  if (keyfile.get_value(this->name, "active",
			keyfile::PRIORITY_REQUIRED, active))
    set_active(active);

  bool run_setup_scripts(false);
  if (keyfile.get_value(this->name, "run-setup-scripts",
			keyfile::PRIORITY_OPTIONAL, run_setup_scripts))
    set_run_setup_scripts(run_setup_scripts);

  bool run_session_scripts(false);
  if (keyfile.get_value(this->name, "run-session-scripts",
			keyfile::PRIORITY_OPTIONAL, run_session_scripts))
    set_run_session_scripts(run_session_scripts);

  int priority(0);
  if (keyfile.get_value(this->name, "priority",
			keyfile::PRIORITY_OPTIONAL, priority))
    set_priority(priority);

  string_list aliases;
  if (keyfile.get_list_value(this->name, "aliases",
			     keyfile::PRIORITY_OPTIONAL, aliases))
    set_aliases(aliases);

  std::string description;
  if (keyfile.get_locale_string(this->name, "description",
				keyfile::PRIORITY_OPTIONAL, description))
    set_description(description);

  string_list groups;
  if (keyfile.get_list_value(this->name, "groups",
			     keyfile::PRIORITY_REQUIRED, groups))
    set_groups(groups);

  string_list root_groups;
  if (keyfile.get_list_value(this->name, "root-groups",
			     keyfile::PRIORITY_OPTIONAL, root_groups))
    set_root_groups(root_groups);

  std::string mount_location;
  if (keyfile.get_value(this->name, "mount-location",
			get_active() ?
			keyfile::PRIORITY_REQUIRED : keyfile::PRIORITY_DISALLOWED,
			mount_location))
    set_mount_location(mount_location);

  std::string mount_device;
  if (keyfile.get_value(this->name, "mount-device",
			get_active() ?
			keyfile::PRIORITY_OPTIONAL : keyfile::PRIORITY_DISALLOWED,
			mount_device))
    set_mount_device(mount_device);
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
