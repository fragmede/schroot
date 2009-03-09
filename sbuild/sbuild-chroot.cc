/* Copyright © 2005-2008  Roger Leigh <rleigh@debian.org>
 *
 * schroot is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * schroot is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 *********************************************************************/

#include <config.h>

#include "sbuild-chroot.h"
#include "sbuild-chroot-directory.h"
#include "sbuild-chroot-plain.h"
#include "sbuild-chroot-file.h"
#include "sbuild-chroot-block-device.h"
#include "sbuild-chroot-loopback.h"
#include "sbuild-chroot-lvm-snapshot.h"
#include "sbuild-lock.h"

#include <cerrno>
#include <map>
#include <utility>

#include <ext/stdio_filebuf.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

namespace
{

  typedef std::pair<sbuild::chroot::error_code,const char *> emap;

  /**
   * This is a list of the supported error codes.  It's used to
   * construct the real error codes map.
   */
  emap init_errors[] =
    {
      emap(sbuild::chroot::CHROOT_CREATE,   N_("Chroot creation failed")),
      emap(sbuild::chroot::CHROOT_DEVICE,   N_("Device name not set")),
      // TRANSLATORS: %1% = chroot type name
      emap(sbuild::chroot::CHROOT_TYPE,     N_("Unknown chroot type '%1%'")),
      emap(sbuild::chroot::DEVICE_ABS,      N_("Device must have an absolute path")),
      emap(sbuild::chroot::DEVICE_LOCK,     N_("Failed to lock device")),
      emap(sbuild::chroot::DEVICE_NOTBLOCK, N_("File is not a block device")),
      emap(sbuild::chroot::DEVICE_UNLOCK,   N_("Failed to unlock device")),
      emap(sbuild::chroot::FILE_ABS,        N_("File must have an absolute path")),
      emap(sbuild::chroot::FILE_LOCK,       N_("Failed to acquire file lock")),
      emap(sbuild::chroot::FILE_NOTREG,     N_("File is not a regular file")),
      emap(sbuild::chroot::FILE_OWNER,      N_("File is not owned by user root")),
      emap(sbuild::chroot::FILE_PERMS,      N_("File has write permissions for others")),
      emap(sbuild::chroot::FILE_UNLOCK,     N_("Failed to discard file lock")),
      emap(sbuild::chroot::LOCATION_ABS,    N_("Location must have an absolute path")),
      // TRANSLATORS: unlink refers to the C function which removes a file
      emap(sbuild::chroot::SESSION_UNLINK,  N_("Failed to unlink session file")),
      emap(sbuild::chroot::SESSION_WRITE,   N_("Failed to write session file"))
    };

}

template<>
error<sbuild::chroot::error_code>::map_type
error<sbuild::chroot::error_code>::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));

sbuild::chroot::chroot ():
  name(),
  description(),
  priority(0),
  users(),
  groups(),
  root_users(),
  root_groups(),
  aliases(),
  environment_filter("^(BASH_ENV|CDPATH|ENV|HOSTALIASES|IFS|KRB5_CONFIG|KRBCONFDIR|KRBTKFILE|KRB_CONF|LD_.*|LOCALDOMAIN|NLSPATH|PATH_LOCALE|RES_OPTIONS|TERMINFO|TERMINFO_DIRS|TERMPATH)$"),
  mount_location(),
  location(),
  mount_device(),
  active(false),
  original(true),
  run_setup_scripts(false),
  run_exec_scripts(false),
  script_config("script-defaults"),
  command_prefix(),
  persona(
#ifdef __linux__
	  personality("linux")
#else
	  personality("undefined")
#endif
	  )
{
}

sbuild::chroot::~chroot ()
{
}

sbuild::chroot::ptr
sbuild::chroot::create (std::string const& type)
{
  chroot *new_chroot = 0;

  if (type == "directory")
    new_chroot = new chroot_directory();
  else if (type == "plain")
    new_chroot = new chroot_plain();
  else if (type == "file")
    new_chroot = new chroot_file();
  else if (type == "block-device")
    new_chroot = new chroot_block_device();
  else if (type == "loopback")
    new_chroot = new chroot_loopback();
  else if (type == "lvm-snapshot")
    new_chroot = new chroot_lvm_snapshot();
  else
    throw error(type, CHROOT_TYPE);

  if (new_chroot == 0)
    throw error(CHROOT_CREATE);

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
  if (!location.empty() && !is_absname(location))
    throw error(location, LOCATION_ABS);
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
  if (!location.empty() && !is_absname(location))
    throw error(location, LOCATION_ABS);

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
  if (!device.empty() && !is_absname(device))
    throw error(device, DEVICE_ABS);
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
sbuild::chroot::get_users () const
{
  return this->users;
}

void
sbuild::chroot::set_users (string_list const& users)
{
  this->users = users;
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
sbuild::chroot::get_root_users () const
{
  return this->root_users;
}

void
sbuild::chroot::set_root_users (string_list const& users)
{
  this->root_users = users;
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

regex const&
sbuild::chroot::get_environment_filter () const
{
  return this->environment_filter;
}

void
sbuild::chroot::set_environment_filter (regex const& environment_filter)
{
  this->environment_filter = environment_filter;
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
sbuild::chroot::get_original () const
{
  return this->original;
}

void
sbuild::chroot::set_original (bool original)
{
  this->original = original;
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
sbuild::chroot::get_run_exec_scripts () const
{
  return this->run_exec_scripts;
}

void
sbuild::chroot::set_run_exec_scripts (bool run_exec_scripts)
{
  this->run_exec_scripts = run_exec_scripts;
}

std::string const&
sbuild::chroot::get_script_config () const
{
  return this->script_config;
}

void
sbuild::chroot::set_script_config (std::string const& script_config)
{
  this->script_config = script_config;
}

string_list const&
sbuild::chroot::get_command_prefix () const
{
  return this->command_prefix;
}

void
sbuild::chroot::set_command_prefix (string_list const& command_prefix)
{
  this->command_prefix = command_prefix;
}

personality const&
sbuild::chroot::get_persona () const
{
  return this->persona;
}

void
sbuild::chroot::set_persona (personality const& persona)
{
  this->persona = persona;
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
  env.add("CHROOT_SCRIPT_CONFIG", normalname(std::string(PACKAGE_SYSCONF_DIR) +  '/' + get_script_config()));
  env.add("CHROOT_SESSION_CREATE",
	  static_cast<bool>(get_session_flags() & SESSION_CREATE));
  env.add("CHROOT_SESSION_CLONE",
	  static_cast<bool>(get_session_flags() & SESSION_CLONE));
  env.add("CHROOT_SESSION_PURGE",
	  static_cast<bool>(get_session_flags() & SESSION_PURGE));
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
	throw error(file, SESSION_WRITE, strerror(errno));

      // Create a stream buffer from the file descriptor.  The fd will
      // be closed when the buffer is destroyed.
      __gnu_cxx::stdio_filebuf<char> fdbuf(fd, std::ios::out);
      std::ostream output(&fdbuf);
      output.imbue(std::locale::classic());

      sbuild::file_lock lock(fd);
      try
	{
	  lock.set_lock(lock::LOCK_EXCLUSIVE, 2);
	}
      catch (lock::error const& e)
	{
	  throw error(file, FILE_LOCK, e);
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
	  throw error(file, FILE_UNLOCK, e);
	}
    }
  else /* start == false */
    {
      if (unlink(file.c_str()) != 0)
	throw error(file, SESSION_UNLINK, strerror(errno));
    }
}

void
sbuild::chroot::lock (setup_type type)
{
  setup_lock(type, true, 0);
}

void
sbuild::chroot::unlock (setup_type type,
			int        status)
{
  setup_lock(type, false, status);
}


void
sbuild::chroot::get_details (format_detail& detail) const
{
  detail
    .add(_("Name"), get_name())
    .add(_("Description"), get_description())
    .add(_("Type"), get_chroot_type())
    .add(_("Priority"), get_priority())
    .add(_("Users"), get_users())
    .add(_("Groups"), get_groups())
    .add(_("Root Users"), get_root_users())
    .add(_("Root Groups"), get_root_groups())
    .add(_("Aliases"), get_aliases())
    .add(_("Environment Filter"), get_environment_filter())
    .add(_("Run Setup Scripts"), get_run_setup_scripts())
    .add(_("Run Execution Scripts"), get_run_exec_scripts())
    .add(_("Script Configuration"), get_script_config())
    .add(_("Session Managed"),
	 static_cast<bool>(get_session_flags() & chroot::SESSION_CREATE))
    .add(_("Session Cloned"),
	 static_cast<bool>(get_session_flags() & chroot::SESSION_CLONE))
    .add(_("Session Purged"),
	 static_cast<bool>(get_session_flags() & chroot::SESSION_PURGE));

  if (!get_command_prefix().empty())
    detail.add(_("Command Prefix"), get_command_prefix());

  // TRANSLATORS: "Personality" is the Linux kernel personality
  // (process execution domain).  See schroot.conf(5).
  detail.add(_("Personality"), get_persona().get_name());

  /* Non user-settable properties are listed last. */
  if (!get_location().empty())
    detail.add(_("Location"), get_location());
  if (!get_mount_location().empty())
    detail.add(_("Mount Location"), get_mount_location());
  if (!get_path().empty())
    detail.add(_("Path"), get_path());
  if (!get_mount_device().empty())
    // TRANSLATORS: The system device node to mount containing the chroot
    detail.add(_("Mount Device"), get_mount_device());
}

void
sbuild::chroot::print_details (std::ostream& stream) const
{
  format_detail fmt((this->active == true ? _("Session") : _("Chroot")),
		    stream.getloc());

  get_details(fmt);

  stream << fmt;
}

void
sbuild::chroot::get_keyfile (keyfile& keyfile) const
{
  keyfile.remove_group(get_name());

  keyfile::set_object_value(*this, &chroot::get_chroot_type,
			    keyfile, get_name(), "type");

  keyfile::set_object_value(*this, &chroot::get_active,
			    keyfile, get_name(), "active");

  keyfile::set_object_value(*this, &chroot::get_run_setup_scripts,
			    keyfile, get_name(), "run-setup-scripts");

  keyfile::set_object_value(*this, &chroot::get_run_exec_scripts,
			    keyfile, get_name(), "run-exec-scripts");

  keyfile::set_object_value(*this, &chroot::get_script_config,
			    keyfile, get_name(), "script-config");

  keyfile::set_object_value(*this, &chroot::get_priority,
			    keyfile, get_name(), "priority");

  keyfile::set_object_list_value(*this, &chroot::get_aliases,
				 keyfile, get_name(), "aliases");

  keyfile::set_object_value(*this, &chroot::get_environment_filter,
			    keyfile, get_name(), "environment-filter");

  keyfile::set_object_value(*this, &chroot::get_description,
			    keyfile, get_name(), "description");

  keyfile::set_object_list_value(*this, &chroot::get_users,
				 keyfile, get_name(), "users");

  keyfile::set_object_list_value(*this, &chroot::get_groups,
				 keyfile, get_name(), "groups");

  keyfile::set_object_list_value(*this, &chroot::get_root_users,
				 keyfile, get_name(), "root-users");

  keyfile::set_object_list_value(*this, &chroot::get_root_groups,
				 keyfile, get_name(), "root-groups");

  if (get_active())
    keyfile::set_object_value(*this, &chroot::get_mount_location,
			      keyfile, get_name(), "mount-location");

  if (get_active())
    keyfile::set_object_value(*this, &chroot::get_mount_device,
			      keyfile, get_name(), "mount-device");

  keyfile::set_object_list_value(*this, &chroot::get_command_prefix,
				 keyfile, get_name(), "command-prefix");

  keyfile::set_object_value(*this, &chroot::get_persona,
			    keyfile, get_name(), "personality");
}

void
sbuild::chroot::set_keyfile (keyfile const& keyfile,
			     string_list&   used_keys)
{
  // Keys which are used elsewhere, but should be counted as "used".
  used_keys.push_back("type");

  string_list keys = keyfile.get_keys(get_name());
  for (string_list::const_iterator pos = keys.begin();
       pos != keys.end();
       ++pos)
    {
      static regex description_keys("^description\\[.*\\]$");
      if (regex_search(*pos, description_keys))
	used_keys.push_back(*pos);
    }

  // This is set not in the configuration file, but set in the keyfile
  // manually.  The user must not have the ability to set this option.
  keyfile::get_object_value(*this, &chroot::set_active,
			    keyfile, get_name(), "active",
			    keyfile::PRIORITY_REQUIRED);
  used_keys.push_back("active");

  keyfile::get_object_value(*this, &chroot::set_run_setup_scripts,
			    keyfile, get_name(), "run-setup-scripts",
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("run-setup-scripts");

  keyfile::get_object_value(*this, &chroot::set_run_exec_scripts,
			    keyfile, get_name(), "run-session-scripts",
			    keyfile::PRIORITY_DEPRECATED);
  used_keys.push_back("run-session-scripts");
  keyfile::get_object_value(*this, &chroot::set_run_exec_scripts,
			    keyfile, get_name(), "run-exec-scripts",
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("run-exec-scripts");

  keyfile::get_object_value(*this, &chroot::set_script_config,
			    keyfile, get_name(), "script-config",
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("script-config");

  keyfile::get_object_value(*this, &chroot::set_priority,
			    keyfile, get_name(), "priority",
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("priority");

  keyfile::get_object_list_value(*this, &chroot::set_aliases,
				 keyfile, get_name(), "aliases",
				 keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("aliases");

  keyfile::get_object_value(*this, &chroot::set_environment_filter,
			    keyfile, get_name(), "environment-filter",
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("environment-filter");

  keyfile::get_object_value(*this, &chroot::set_description,
			    keyfile, get_name(), "description",
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("description");

  keyfile::get_object_list_value(*this, &chroot::set_users,
				 keyfile, get_name(), "users",
				 keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("users");

  keyfile::get_object_list_value(*this, &chroot::set_groups,
				 keyfile, get_name(), "groups",
				 keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("groups");

  keyfile::get_object_list_value(*this, &chroot::set_root_users,
				 keyfile, get_name(), "root-users",
				 keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("root-users");

  keyfile::get_object_list_value(*this, &chroot::set_root_groups,
				 keyfile, get_name(), "root-groups",
				 keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("root-groups");

  keyfile::get_object_value(*this, &chroot::set_mount_location,
			    keyfile, get_name(), "mount-location",
			    get_active() ?
			    keyfile::PRIORITY_REQUIRED :
			    keyfile::PRIORITY_DISALLOWED);
  used_keys.push_back("mount-location");

  keyfile::get_object_value(*this, &chroot::set_mount_device,
			    keyfile, get_name(), "mount-device",
			    get_active() ?
			    keyfile::PRIORITY_OPTIONAL :
			    keyfile::PRIORITY_DISALLOWED);
  used_keys.push_back("mount-device");

  keyfile::get_object_list_value(*this, &chroot::set_command_prefix,
				 keyfile, get_name(), "command-prefix",
				 keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("command-prefix");

  keyfile::get_object_value(*this, &chroot::set_persona,
			    keyfile, get_name(), "personality",
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("personality");
}
