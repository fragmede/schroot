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
#include "sbuild-chroot-config.h"
#include "sbuild-chroot-directory.h"
#include "sbuild-chroot-plain.h"
#include "sbuild-chroot-file.h"
#ifdef SBUILD_FEATURE_BLOCKDEV
#include "sbuild-chroot-block-device.h"
#endif // SBUILD_FEATURE_BLOCKDEV
#ifdef SBUILD_FEATURE_LOOPBACK
#include "sbuild-chroot-loopback.h"
#endif // SBUILD_FEATURE_LOOPBACK
#ifdef SBUILD_FEATURE_LVMSNAP
#include "sbuild-chroot-lvm-snapshot.h"
#endif // SBUILD_FEATURE_LVMSNAP
#ifdef SBUILD_FEATURE_BTRFSSNAP
#include "sbuild-chroot-btrfs-snapshot.h"
#endif // SBUILD_FEATURE_BTRFSSNAP
#include "sbuild-chroot-facet.h"
#include "sbuild-chroot-facet-personality.h"
#include "sbuild-chroot-facet-session.h"
#include "sbuild-chroot-facet-session-clonable.h"
#include "sbuild-chroot-facet-source.h"
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
      emap(sbuild::chroot::CHROOT_CREATE,     N_("Chroot creation failed")),
      emap(sbuild::chroot::CHROOT_DEVICE,     N_("Device name not set")),
      // TRANSLATORS: %1% = chroot type name
      emap(sbuild::chroot::CHROOT_TYPE,       N_("Unknown chroot type '%1%'")),
      emap(sbuild::chroot::DEVICE_ABS,        N_("Device must have an absolute path")),
      emap(sbuild::chroot::DEVICE_LOCK,       N_("Failed to lock device")),
      emap(sbuild::chroot::DEVICE_NOTBLOCK,   N_("File is not a block device")),
      emap(sbuild::chroot::DEVICE_UNLOCK,     N_("Failed to unlock device")),
      emap(sbuild::chroot::DIRECTORY_ABS,     N_("Directory must have an absolute path")),
      emap(sbuild::chroot::FACET_INVALID,     N_("Attempt to add object which is not a facet")),
      emap(sbuild::chroot::FACET_PRESENT,     N_("Attempt to add facet which is already in use")),
      emap(sbuild::chroot::FILE_ABS,          N_("File must have an absolute path")),
      emap(sbuild::chroot::FILE_LOCK,         N_("Failed to acquire file lock")),
      emap(sbuild::chroot::FILE_NOTREG,       N_("File is not a regular file")),
      emap(sbuild::chroot::FILE_OWNER,        N_("File is not owned by user root")),
      emap(sbuild::chroot::FILE_PERMS,        N_("File has write permissions for others")),
      emap(sbuild::chroot::FILE_UNLOCK,       N_("Failed to discard file lock")),
      emap(sbuild::chroot::LOCATION_ABS,      N_("Location must have an absolute path")),
      emap(sbuild::chroot::NAME_INVALID,      N_("Invalid name")),
      // TRANSLATORS: unlink refers to the C function which removes a file
      emap(sbuild::chroot::SESSION_UNLINK,    N_("Failed to unlink session file")),
      emap(sbuild::chroot::SESSION_WRITE,     N_("Failed to write session file")),
      emap(sbuild::chroot::VERBOSITY_INVALID, N_("Message verbosity is invalid"))
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
  users(),
  groups(),
  root_users(),
  root_groups(),
  aliases(),
  preserve_environment(false),
  environment_filter(SBUILD_DEFAULT_ENVIRONMENT_FILTER),
  mount_location(),
  original(true),
  run_setup_scripts(true),
  script_config("default/config"),
  command_prefix(),
  message_verbosity(VERBOSITY_NORMAL),
  facets()
{
  add_facet(sbuild::chroot_facet_personality::create());
  add_facet(sbuild::chroot_facet_session_clonable::create());
}

sbuild::chroot::chroot (const chroot& rhs):
  name(rhs.name),
  description(rhs.description),
  users(rhs.users),
  groups(rhs.groups),
  root_users(rhs.root_users),
  root_groups(rhs.root_groups),
  aliases(rhs.aliases),
  preserve_environment(rhs.preserve_environment),
  environment_filter(rhs.environment_filter),
  mount_location(rhs.mount_location),
  original(rhs.original),
  run_setup_scripts(rhs.run_setup_scripts),
  script_config(rhs.script_config),
  command_prefix(rhs.command_prefix),
  message_verbosity(rhs.message_verbosity),
  facets()
{
  /// @todo Use internal version of add_facet to add chroot pointer.
  for (facet_list::const_iterator pos = rhs.facets.begin();
       pos != rhs.facets.end();
       ++pos)
    {
      facet_ptr fp = (*pos)->clone();
      fp->set_chroot(*this);
      facets.push_back(fp);
    }
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
#ifdef SBUILD_FEATURE_BLOCKDEV
  else if (type == "block-device")
    new_chroot = new chroot_block_device();
#endif // SBUILD_FEATURE_BLOCKDEV
#ifdef SBUILD_FEATURE_LOOPBACK
  else if (type == "loopback")
    new_chroot = new chroot_loopback();
#endif // SBUILD_FEATURE_LOOPBACK
#ifdef SBUILD_FEATURE_LVMSNAP
  else if (type == "lvm-snapshot")
    new_chroot = new chroot_lvm_snapshot();
#endif // SBUILD_FEATURE_LVMSNAP
#ifdef SBUILD_FEATURE_BTRFSSNAP
  else if (type == "btrfs-snapshot")
    new_chroot = new chroot_btrfs_snapshot();
#endif // SBUILD_FEATURE_BTRFSSNAP
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
  std::string::size_type pos = name.find_first_of(chroot_config::namespace_separator);
  if (pos != std::string::npos)
    {
      error e(name, NAME_INVALID);
      format fmt("Namespace separator '%1%' may not be used in a chroot name");
      fmt % chroot_config::namespace_separator;
      e.set_reason(fmt.str());
      throw e;
    }

  if (!is_valid_sessionname(name))
    {
      error e(name, NAME_INVALID);
      e.set_reason("Only alphanumeric characters, underscores and hyphens are allowed");
      throw e;
    }

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
  for (string_list::const_iterator pos = aliases.begin();
       pos != aliases.end();
       ++pos)
    {
      std::string::size_type found = pos->find_first_of(chroot_config::namespace_separator);
      if (found != std::string::npos)
	{
	  error e(*pos, NAME_INVALID);
	  format fmt("Namespace separator '%1%' may not be used in an alias name");
	  fmt % chroot_config::namespace_separator;
	  e.set_reason(fmt.str());
	  throw e;
	}
    }

  this->aliases = aliases;
}

bool
sbuild::chroot::get_preserve_environment () const
{
  return this->preserve_environment;
}

void
sbuild::chroot::set_preserve_environment (bool preserve_environment)
{
  this->preserve_environment = preserve_environment;
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

chroot::verbosity
chroot::get_verbosity () const
{
  return this->message_verbosity;
}

const char *
chroot::get_verbosity_string () const
{
  const char *verbosity = 0;

  switch (this->message_verbosity)
    {
    case chroot::VERBOSITY_QUIET:
      verbosity = "quiet";
      break;
    case chroot::VERBOSITY_NORMAL:
      verbosity = "normal";
      break;
    case chroot::VERBOSITY_VERBOSE:
      verbosity = "verbose";
      break;
    default:
      log_debug(DEBUG_CRITICAL) << format("Invalid verbosity level: %1%, falling back to 'normal'")
	% static_cast<int>(this->message_verbosity)
				<< std::endl;
      verbosity = "normal";
      break;
    }

  return verbosity;
}

void
chroot::set_verbosity (chroot::verbosity verbosity)
{
  this->message_verbosity = verbosity;
}

void
chroot::set_verbosity (std::string const& verbosity)
{
  if (verbosity == "quiet")
    this->message_verbosity = VERBOSITY_QUIET;
  else if (verbosity == "normal")
    this->message_verbosity = VERBOSITY_NORMAL;
  else if (verbosity == "verbose")
    this->message_verbosity = VERBOSITY_VERBOSE;
  else
    throw error(verbosity, VERBOSITY_INVALID);
}

string_list
sbuild::chroot::list_facets () const
{
  string_list fnames;

  for (facet_list::const_iterator pos = facets.begin();
       pos != facets.end();
       ++pos)
    {
      fnames.push_back((*pos)->get_name());
    }

  return fnames;
}

void
sbuild::chroot::setup_env (environment& env) const
{
  setup_env(*this, env);

  for (facet_list::const_iterator pos = facets.begin();
       pos != facets.end();
       ++pos)
    {
      (*pos)->setup_env(*this, env);
    }
}

void
sbuild::chroot::setup_env (chroot const& chroot,
			   environment&  env) const
{
  env.add("CHROOT_TYPE", chroot.get_chroot_type());
  env.add("CHROOT_NAME", chroot.get_name());
  //  env.add("CHROOT_SESSION_NAME", chroot.get_name());
  env.add("CHROOT_DESCRIPTION", chroot.get_description());
  env.add("CHROOT_MOUNT_LOCATION", chroot.get_mount_location());
  env.add("CHROOT_PATH", chroot.get_path());
  env.add("CHROOT_SCRIPT_CONFIG", normalname(std::string(SCHROOT_SYSCONF_DIR) +  '/' + chroot.get_script_config()));
  env.add("CHROOT_SESSION_CREATE",
	  static_cast<bool>(chroot.get_session_flags() & SESSION_CREATE));
  env.add("CHROOT_SESSION_CLONE",
	  static_cast<bool>(chroot.get_session_flags() & SESSION_CLONE));
  env.add("CHROOT_SESSION_PURGE",
	  static_cast<bool>(chroot.get_session_flags() & SESSION_PURGE));
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

      file_lock lock(fd);
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

sbuild::chroot::session_flags
sbuild::chroot::get_session_flags () const
{
  session_flags flags = get_session_flags(*this);

  for (facet_list::const_iterator pos = facets.begin();
       pos != facets.end();
       ++pos)
    {
      flags = flags | (*pos)->get_session_flags(*this);
    }

  return flags;
}

void
sbuild::chroot::get_details (format_detail& detail) const
{
  get_details(*this, detail);

  for (facet_list::const_iterator pos = facets.begin();
       pos != facets.end();
       ++pos)
    {
      (*pos)->get_details(*this, detail);
    }
}

void
sbuild::chroot::get_details (chroot const&  chroot,
			     format_detail& detail) const
{
  detail.add(_("Name"), chroot.get_name());

  detail
    .add(_("Description"), chroot.get_description())
    .add(_("Type"), chroot.get_chroot_type())
    .add(_("Message Verbosity"), chroot.get_verbosity_string())
    .add(_("Users"), chroot.get_users())
    .add(_("Groups"), chroot.get_groups())
    .add(_("Root Users"), chroot.get_root_users())
    .add(_("Root Groups"), chroot.get_root_groups())
    .add(_("Aliases"), chroot.get_aliases())
    .add(_("Preserve Environment"), chroot.get_preserve_environment())
    .add(_("Environment Filter"), chroot.get_environment_filter())
    .add(_("Run Setup Scripts"), chroot.get_run_setup_scripts())
    .add(_("Script Configuration"), chroot.get_script_config())
    .add(_("Session Managed"),
	 static_cast<bool>(chroot.get_session_flags() & chroot::SESSION_CREATE))
    .add(_("Session Cloned"),
	 static_cast<bool>(chroot.get_session_flags() & chroot::SESSION_CLONE))
    .add(_("Session Purged"),
	 static_cast<bool>(chroot.get_session_flags() & chroot::SESSION_PURGE));

  if (!chroot.get_command_prefix().empty())
    detail.add(_("Command Prefix"), chroot.get_command_prefix());

  /* Non user-settable properties are listed last. */
  if (!chroot.get_mount_location().empty())
    detail.add(_("Mount Location"), chroot.get_mount_location());
  if (!chroot.get_path().empty())
    detail.add(_("Path"), chroot.get_path());
}

void
sbuild::chroot::print_details (std::ostream& stream) const
{
  std::string title(_("Chroot"));

  if (get_facet<chroot_facet_session>())
    title = _("Session");
  if (get_facet<chroot_facet_source>())
    title = _("Source");

  format_detail fmt(title, stream.getloc());

  get_details(fmt);

  stream << fmt;
}

void
sbuild::chroot::get_keyfile (keyfile& keyfile) const
{
  get_keyfile(*this, keyfile);

  for (facet_list::const_iterator pos = facets.begin();
       pos != facets.end();
       ++pos)
    {
      (*pos)->get_keyfile(*this, keyfile);
    }
}

void
sbuild::chroot::get_keyfile (chroot const& chroot,
			     keyfile&      keyfile) const
{
  keyfile.remove_group(chroot.get_name());

  bool session = static_cast<bool>(get_facet<chroot_facet_session>());

  if (session)
    keyfile::set_object_value(chroot, &chroot::get_name,
			      keyfile, chroot.get_name(),
			      "name");

  keyfile::set_object_value(chroot, &chroot::get_chroot_type,
			    keyfile, chroot.get_name(),
			    "type");

  keyfile::set_object_value(chroot, &chroot::get_script_config,
			    keyfile, chroot.get_name(),
			    "script-config");

  keyfile::set_object_list_value(chroot, &chroot::get_aliases,
				 keyfile, chroot.get_name(),
				 "aliases");

  keyfile::set_object_value(chroot, &chroot::get_environment_filter,
			    keyfile, chroot.get_name(),
			    "environment-filter");

  keyfile::set_object_value(chroot, &chroot::get_description,
			    keyfile, chroot.get_name(),
			    "description");

  keyfile::set_object_list_value(chroot, &chroot::get_users,
				 keyfile, chroot.get_name(),
				 "users");

  keyfile::set_object_list_value(chroot, &chroot::get_groups,
				 keyfile, chroot.get_name(),
				 "groups");

  keyfile::set_object_list_value(chroot, &chroot::get_root_users,
				 keyfile, chroot.get_name(),
				 "root-users");

  keyfile::set_object_list_value(chroot, &chroot::get_root_groups,
				 keyfile, chroot.get_name(),
				 "root-groups");

  if (session)
    keyfile::set_object_value(chroot, &chroot::get_mount_location,
			      keyfile, chroot.get_name(),
			      "mount-location");

  keyfile::set_object_list_value(chroot, &chroot::get_command_prefix,
				 keyfile, chroot.get_name(),
				 "command-prefix");

  keyfile::set_object_value(chroot, &chroot::get_verbosity_string,
			    keyfile, chroot.get_name(),
			    "message-verbosity");

  keyfile::set_object_value(chroot, &chroot::get_preserve_environment,
			    keyfile, chroot.get_name(),
			    "preserve-environment");
}

void
sbuild::chroot::set_keyfile (keyfile const& keyfile,
			     string_list&   used_keys)
{
  set_keyfile(*this, keyfile, used_keys);

  for (facet_list::const_iterator pos = facets.begin();
       pos != facets.end();
       ++pos)
    {
      (*pos)->set_keyfile(*this, keyfile, used_keys);
    }
}

void
sbuild::chroot::set_keyfile (chroot&        chroot,
			     keyfile const& keyfile,
			     string_list&   used_keys)
{
  // Null method for obsolete keys.
  void (sbuild::chroot::* nullmethod)(bool) = 0;

  bool session = static_cast<bool>(get_facet<chroot_facet_session>());

  // Keys which are used elsewhere, but should be counted as "used".
  used_keys.push_back("type");

  string_list keys = keyfile.get_keys(chroot.get_name());
  for (string_list::const_iterator pos = keys.begin();
       pos != keys.end();
       ++pos)
    {
      static regex description_keys("^description\\[.*\\]$");
      if (regex_search(*pos, description_keys))
	used_keys.push_back(*pos);
    }

  keyfile::get_object_value(chroot, nullmethod,
			    keyfile, chroot.get_name(),
			    "active",
			    keyfile::PRIORITY_OBSOLETE);
  used_keys.push_back("active");

  // Setup scripts are run depending on the chroot type in use, and is
  // no longer user-configurable.  They need to run for all types
  // except "plain".
  keyfile::get_object_value(chroot, nullmethod,
			    keyfile, chroot.get_name(),
			    "run-setup-scripts",
			    keyfile::PRIORITY_OBSOLETE);
  used_keys.push_back("run-setup-scripts");

  // Exec scripts have been removed, so these two calls do nothing
  // except to warn the user that the options are no longer used.
  keyfile::get_object_value(chroot, nullmethod,
			    keyfile, chroot.get_name(),
			    "run-session-scripts",
			    keyfile::PRIORITY_OBSOLETE);
  used_keys.push_back("run-session-scripts");
  keyfile::get_object_value(chroot, nullmethod,
			    keyfile, chroot.get_name(),
			    "run-exec-scripts",
			    keyfile::PRIORITY_OBSOLETE);
  used_keys.push_back("run-exec-scripts");

  keyfile::get_object_value(chroot, &chroot::set_script_config,
			    keyfile, chroot.get_name(),
			    "script-config",
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("script-config");

  keyfile::get_object_value(chroot, nullmethod,
			    keyfile, chroot.get_name(),
			    "priority",
			    session ?
			    keyfile::PRIORITY_OPTIONAL :
			    keyfile::PRIORITY_OBSOLETE);
  used_keys.push_back("priority");

  keyfile::get_object_list_value(chroot, &chroot::set_aliases,
				 keyfile, chroot.get_name(),
				 "aliases",
				 keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("aliases");

  keyfile::get_object_value(chroot, &chroot::set_environment_filter,
			    keyfile, chroot.get_name(),
			    "environment-filter",
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("environment-filter");

  keyfile::get_object_value(chroot, &chroot::set_description,
			    keyfile, chroot.get_name(),
			    "description",
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("description");

  keyfile::get_object_list_value(chroot, &chroot::set_users,
				 keyfile, chroot.get_name(),
				 "users",
				 keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("users");

  keyfile::get_object_list_value(chroot, &chroot::set_groups,
				 keyfile, chroot.get_name(),
				 "groups",
				 keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("groups");

  keyfile::get_object_list_value(chroot, &chroot::set_root_users,
				 keyfile, chroot.get_name(),
				 "root-users",
				 keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("root-users");

  keyfile::get_object_list_value(chroot, &chroot::set_root_groups,
				 keyfile, chroot.get_name(),
				 "root-groups",
				 keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("root-groups");

  keyfile::get_object_value(chroot, &chroot::set_mount_location,
			    keyfile, chroot.get_name(),
			    "mount-location",
			    session ?
			    keyfile::PRIORITY_REQUIRED :
			    keyfile::PRIORITY_DISALLOWED);
  used_keys.push_back("mount-location");

  keyfile::get_object_value(chroot, &chroot::set_name,
			    keyfile, chroot.get_name(),
			    "name",
			    session ?
			    keyfile::PRIORITY_OPTIONAL :
			    keyfile::PRIORITY_DISALLOWED);
  used_keys.push_back("name");

  keyfile::get_object_list_value(chroot, &chroot::set_command_prefix,
				 keyfile, chroot.get_name(),
				 "command-prefix",
				 keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("command-prefix");

  keyfile::get_object_value(chroot, &chroot::set_verbosity,
			    keyfile, chroot.get_name(),
			    "message-verbosity",
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("message-verbosity");

  keyfile::get_object_value(chroot, &chroot::set_preserve_environment,
			    keyfile, chroot.get_name(),
			    "preserve-environment",
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("preserve-environment");
}
