/* Copyright © 2005-2013  Roger Leigh <rleigh@debian.org>
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

#include <sbuild/chroot/chroot.h>
#include <sbuild/chroot/config.h>
#include <sbuild/chroot/directory.h>
#include <sbuild/chroot/plain.h>
#include "chroot-custom.h"
#include "chroot-file.h"
#ifdef SBUILD_FEATURE_BLOCKDEV
#include <sbuild/chroot/block-device.h>
#endif // SBUILD_FEATURE_BLOCKDEV
#ifdef SBUILD_FEATURE_LOOPBACK
#include "chroot-loopback.h"
#endif // SBUILD_FEATURE_LOOPBACK
#ifdef SBUILD_FEATURE_LVMSNAP
#include <sbuild/chroot/lvm-snapshot.h>
#endif // SBUILD_FEATURE_LVMSNAP
#ifdef SBUILD_FEATURE_BTRFSSNAP
#include "chroot-btrfs-snapshot.h"
#endif // SBUILD_FEATURE_BTRFSSNAP
#include "chroot-facet.h"
#include "chroot-facet-personality.h"
#include "chroot-facet-session.h"
#include "chroot-facet-session-clonable.h"
#include "chroot-facet-source.h"
#include "chroot-facet-userdata.h"
#ifdef SBUILD_FEATURE_UNSHARE
#include "chroot-facet-unshare.h"
#endif // SBUILD_FEATURE_UNSHARE
#include "fdstream.h"
#include "keyfile-writer.h"
#include "lock.h"

#include <cerrno>
#include <map>
#include <utility>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <boost/format.hpp>

using boost::format;

namespace sbuild
{
  namespace chroot
  {

    template<>
    error<chroot::error_code>::map_type
    error<chroot::error_code>::error_strings =
      {
        {chroot::chroot::CHROOT_CREATE,     N_("Chroot creation failed")},
        {chroot::chroot::CHROOT_DEVICE,     N_("Device name not set")},
        // TRANSLATORS: %1% = chroot type name
        {chroot::chroot::CHROOT_TYPE,       N_("Unknown chroot type ‘%1%’")},
        {chroot::chroot::DEVICE_ABS,        N_("Device must have an absolute path")},
        {chroot::chroot::DEVICE_LOCK,       N_("Failed to lock device")},
        {chroot::chroot::DEVICE_NOTBLOCK,   N_("File is not a block device")},
        {chroot::chroot::DEVICE_UNLOCK,     N_("Failed to unlock device")},
        {chroot::chroot::DIRECTORY_ABS,     N_("Directory must have an absolute path")},
        {chroot::chroot::FACET_INVALID,     N_("Attempt to add object which is not a facet")},
        {chroot::chroot::FACET_PRESENT,     N_("Attempt to add facet which is already in use")},
        {chroot::chroot::FILE_ABS,          N_("File must have an absolute path")},
        {chroot::chroot::FILE_LOCK,         N_("Failed to acquire file lock")},
        {chroot::chroot::FILE_NOTREG,       N_("File is not a regular file")},
        {chroot::chroot::FILE_OWNER,        N_("File is not owned by user root")},
        {chroot::chroot::FILE_PERMS,        N_("File has write permissions for others")},
        {chroot::chroot::FILE_UNLOCK,       N_("Failed to discard file lock")},
        {chroot::chroot::LOCATION_ABS,      N_("Location must have an absolute path")},
        {chroot::chroot::NAME_INVALID,      N_("Invalid name")},
        {chroot::chroot::SCRIPT_CONFIG_CV,  N_("Could not set profile name from script configuration path ‘%1%’")},

        // TRANSLATORS: unlink refers to the C function which removes a file
        {chroot::chroot::SESSION_UNLINK,    N_("Failed to unlink session file")},
        {chroot::chroot::SESSION_WRITE,     N_("Failed to write session file")},
        {chroot::chroot::VERBOSITY_INVALID, N_("Message verbosity is invalid")}
      };

    chroot::chroot ():
      name(),
      description(),
      users(),
      groups(),
      root_users(),
      root_groups(),
      aliases(),
      preserve_environment(false),
      default_shell(),
      environment_filter(SBUILD_DEFAULT_ENVIRONMENT_FILTER),
      mount_location(),
      original(true),
      run_setup_scripts(true),
      script_config(),
      profile("default"),
      command_prefix(),
      message_verbosity(VERBOSITY_NORMAL),
      facets()
    {
      add_facet(chroot_facet_personality::create());
#ifdef SBUILD_FEATURE_UNSHARE
      add_facet(chroot_facet_unshare::create());
#endif // SBUILD_FEATURE_UNSHARE
      add_facet(chroot_facet_session_clonable::create());
      add_facet(chroot_facet_userdata::create());

      set_profile(get_profile());
    }

    chroot::chroot (const chroot& rhs):
      name(rhs.name),
      description(rhs.description),
      users(rhs.users),
      groups(rhs.groups),
      root_users(rhs.root_users),
      root_groups(rhs.root_groups),
      aliases(rhs.aliases),
      preserve_environment(rhs.preserve_environment),
      default_shell(rhs.default_shell),
      environment_filter(rhs.environment_filter),
      mount_location(rhs.mount_location),
      original(rhs.original),
      run_setup_scripts(rhs.run_setup_scripts),
      script_config(rhs.script_config),
      profile(rhs.profile),
      command_prefix(rhs.command_prefix),
      message_verbosity(rhs.message_verbosity),
      facets()
    {
      /// @todo Use internal version of add_facet to add chroot pointer.
      for (const auto& facet : rhs.facets)
        {
          facet_ptr fp = facet->clone();
          fp->set_chroot(*this);
          facets.push_back(fp);
        }
    }

    chroot::~chroot ()
    {
    }

    chroot::ptr
    chroot::create (std::string const& type)
    {
      chroot *new_chroot = 0;

      if (type == "directory")
        new_chroot = new directory();
      else if (type == "plain")
        new_chroot = new plain();
      else if (type == "custom")
        new_chroot = new chroot_custom();
      else if (type == "file")
        new_chroot = new chroot_file();
#ifdef SBUILD_FEATURE_BLOCKDEV
      else if (type == "block-device")
        new_chroot = new block_device();
#endif // SBUILD_FEATURE_BLOCKDEV
#ifdef SBUILD_FEATURE_LOOPBACK
      else if (type == "loopback")
        new_chroot = new chroot_loopback();
#endif // SBUILD_FEATURE_LOOPBACK
#ifdef SBUILD_FEATURE_LVMSNAP
      else if (type == "lvm-snapshot")
        new_chroot = new lvm_snapshot();
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
    chroot::get_name () const
    {
      return this->name;
    }

    void
    chroot::set_name (std::string const& name)
    {
      std::string::size_type pos = name.find_first_of(config::namespace_separator);
      if (pos != std::string::npos)
        {
          error e(name, NAME_INVALID);
          format fmt(_("Namespace separator ‘%1%’ may not be used in a chroot name"));
          fmt % config::namespace_separator;
          e.set_reason(fmt.str());
          throw e;
        }

      if (!is_valid_sessionname(name))
        {
          error e(name, NAME_INVALID);
          e.set_reason(_("Naming restrictions are documented in schroot.conf(5)"));
          throw e;
        }

      this->name = name;
    }


    std::string const&
    chroot::get_description () const
    {
      return this->description;
    }

    void
    chroot::set_description (std::string const& description)
    {
      this->description = description;
    }

    std::string const&
    chroot::get_mount_location () const
    {
      return this->mount_location;
    }

    void
    chroot::set_mount_location (std::string const& location)
    {
      if (!location.empty() && !is_absname(location))
        throw error(location, LOCATION_ABS);
      this->mount_location = location;
    }

    string_list const&
    chroot::get_users () const
    {
      return this->users;
    }

    void
    chroot::set_users (string_list const& users)
    {
      this->users = users;
    }

    string_list const&
    chroot::get_groups () const
    {
      return this->groups;
    }

    void
    chroot::set_groups (string_list const& groups)
    {
      this->groups = groups;
    }

    string_list const&
    chroot::get_root_users () const
    {
      return this->root_users;
    }

    void
    chroot::set_root_users (string_list const& users)
    {
      this->root_users = users;
    }

    string_list const&
    chroot::get_root_groups () const
    {
      return this->root_groups;
    }

    void
    chroot::set_root_groups (string_list const& groups)
    {
      this->root_groups = groups;
    }

    string_list const&
    chroot::get_aliases () const
    {
      return this->aliases;
    }

    void
    chroot::set_aliases (string_list const& aliases)
    {
      for (const auto& alias : aliases)
        {
          std::string::size_type found = alias.find_first_of(config::namespace_separator);
          if (found != std::string::npos)
            {
              error e(alias, NAME_INVALID);
              format fmt(_("Namespace separator ‘%1%’ may not be used in an alias name"));
              fmt % config::namespace_separator;
              e.set_reason(fmt.str());
              throw e;
            }

          if (!is_valid_sessionname(alias))
            {
              error e(alias, NAME_INVALID);
              e.set_reason(_("Naming restrictions are documented in schroot.conf(5)"));
              throw e;
            }
        }

      this->aliases = aliases;
    }

    bool
    chroot::get_preserve_environment () const
    {
      return this->preserve_environment;
    }

    void
    chroot::set_preserve_environment (bool preserve_environment)
    {
      this->preserve_environment = preserve_environment;
    }

    std::string const&
    chroot::get_default_shell () const
    {
      return this->default_shell;
    }

    void
    chroot::set_default_shell (std::string const& default_shell)
    {
      this->default_shell = default_shell;
    }

    regex const&
    chroot::get_environment_filter () const
    {
      return this->environment_filter;
    }

    void
    chroot::set_environment_filter (regex const& environment_filter)
    {
      this->environment_filter = environment_filter;
    }

    bool
    chroot::get_original () const
    {
      return this->original;
    }

    void
    chroot::set_original (bool original)
    {
      this->original = original;
    }

    bool
    chroot::get_run_setup_scripts () const
    {
      return this->run_setup_scripts;
    }

    void
    chroot::set_run_setup_scripts (bool run_setup_scripts)
    {
      this->run_setup_scripts = run_setup_scripts;
    }

    std::string const&
    chroot::get_script_config () const
    {
      return this->script_config;
    }

    void
    chroot::set_script_config (std::string const& script_config)
    {
      this->script_config = script_config;

      // Undo work of set_profile, so profile is completely unset.
      this->profile.clear();
      chroot_facet_userdata::ptr userdata =
        get_facet<chroot_facet_userdata>();
      if (userdata)
        {
          userdata->remove_data("setup.config");
          userdata->remove_data("setup.copyfiles");
          userdata->remove_data("setup.nssdatabases");
          userdata->remove_data("setup.fstab");
        }

    }

    std::string const&
    chroot::get_profile () const
    {
      return this->profile;
    }

    void
    chroot::set_profile (std::string const& profile)
    {
      this->profile = profile;

      chroot_facet_userdata::ptr userdata =
        get_facet<chroot_facet_userdata>();
      if (userdata)
        {
          userdata->set_system_data("setup.config", this->profile + "/config");
          userdata->set_system_data("setup.copyfiles", this->profile + "/copyfiles");
          userdata->set_system_data("setup.nssdatabases", this->profile + "/nssdatabases");
          userdata->set_system_data("setup.fstab", this->profile + "/fstab");
        }
    }

    string_list const&
    chroot::get_command_prefix () const
    {
      return this->command_prefix;
    }

    void
    chroot::set_command_prefix (string_list const& command_prefix)
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
    chroot::list_facets () const
    {
      string_list facet_names;

      for (const auto& facet : facets)
        facet_names.push_back(facet->get_name());

      return facet_names;
    }

    void
    chroot::setup_env (environment& env) const
    {
      setup_env(*this, env);

      for (const auto& facet : facets)
        facet->setup_env(*this, env);
    }

    void
    chroot::setup_env (chroot const& chroot,
                       environment&  env) const
    {
      env.add("CHROOT_TYPE", chroot.get_chroot_type());
      env.add("CHROOT_NAME", chroot.get_name());
      env.add("SESSION_ID", chroot.get_name());
      env.add("CHROOT_DESCRIPTION", chroot.get_description());
      env.add("CHROOT_MOUNT_LOCATION", chroot.get_mount_location());
      env.add("CHROOT_PATH", chroot.get_path());
      if (!chroot.get_script_config().empty())
        env.add("CHROOT_SCRIPT_CONFIG", normalname(std::string(SCHROOT_SYSCONF_DIR) +  '/' + chroot.get_script_config()));
      if (!chroot.get_profile().empty())
        {
          env.add("CHROOT_PROFILE", chroot.get_profile());
          env.add("CHROOT_PROFILE_DIR", normalname(std::string(SCHROOT_SYSCONF_DIR) +  '/' + chroot.get_profile()));
        }
      env.add("CHROOT_SESSION_CREATE",
              static_cast<bool>(chroot.get_session_flags() & SESSION_CREATE));
      env.add("CHROOT_SESSION_CLONE",
              static_cast<bool>(chroot.get_session_flags() & SESSION_CLONE));
      env.add("CHROOT_SESSION_PURGE",
              static_cast<bool>(chroot.get_session_flags() & SESSION_PURGE));
    }

    void
    chroot::setup_session_info (bool start)
    {
      /* Create or unlink session information. */
      std::string file = std::string(SCHROOT_SESSION_DIR) + "/" + get_name();

      if (start)
        {
          int fd = open(file.c_str(), O_CREAT|O_EXCL|O_WRONLY, 0664);
          if (fd < 0)
            throw error(file, SESSION_WRITE, strerror(errno));

          // Create a stream from the file descriptor.  The fd will be
          // closed when the stream is destroyed.
#ifdef BOOST_IOSTREAMS_CLOSE_HANDLE_OLD
          fdostream output(fd, true);
#else
          fdostream output(fd, boost::iostreams::close_handle);
#endif
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
          output << keyfile_writer(details);

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
    chroot::lock (setup_type type)
    {
      setup_lock(type, true, 0);
    }

    void
    chroot::unlock (setup_type type,
                    int        status)
    {
      setup_lock(type, false, status);
    }

    chroot::session_flags
    chroot::get_session_flags () const
    {
      session_flags flags = get_session_flags(*this);

      for (const auto& facet : facets)
        flags = flags | facet->get_session_flags(*this);

      return flags;
    }

    void
    chroot::get_details (format_detail& detail) const
    {
      get_details(*this, detail);

      for (const auto& facet : facets)
        facet->get_details(*this, detail);
    }

    void
    chroot::get_details (chroot const&  chroot,
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
        .add(_("Default Shell"), chroot.get_default_shell())
        .add(_("Environment Filter"), chroot.get_environment_filter())
        .add(_("Run Setup Scripts"), chroot.get_run_setup_scripts())
        .add(_("Configuration Profile"), chroot.get_profile())
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
    chroot::print_details (std::ostream& stream) const
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

    string_list
    chroot::get_used_keys () const
    {
      string_list used_keys;

      get_used_keys(used_keys);

      for (const auto& facet : facets)
        facet->get_used_keys(used_keys);

      return used_keys;
    }

    void
    chroot::get_used_keys (string_list& used_keys) const
    {
      // Keys which are used elsewhere, but should be counted as "used".
      used_keys.push_back("type");

      used_keys.push_back("active");
      used_keys.push_back("run-setup-scripts");
      used_keys.push_back("run-session-scripts");
      used_keys.push_back("run-exec-scripts");
      used_keys.push_back("profile");
      used_keys.push_back("script-config");
      used_keys.push_back("priority");
      used_keys.push_back("aliases");
      used_keys.push_back("environment-filter");
      used_keys.push_back("description");
      used_keys.push_back("users");
      used_keys.push_back("groups");
      used_keys.push_back("root-users");
      used_keys.push_back("root-groups");
      used_keys.push_back("mount-location");
      used_keys.push_back("name");
      used_keys.push_back("command-prefix");
      used_keys.push_back("message-verbosity");
      used_keys.push_back("preserve-environment");
      used_keys.push_back("shell");
    }

    void
    chroot::get_keyfile (keyfile& keyfile) const
    {
      get_keyfile(*this, keyfile);

      for (const auto& facet : facets)
        facet->get_keyfile(*this, keyfile);
    }

    void
    chroot::get_keyfile (chroot const& chroot,
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

      keyfile::set_object_value(chroot, &chroot::get_profile,
                                keyfile, chroot.get_name(),
                                "profile");

      if (!get_script_config().empty())
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

      keyfile::set_object_value(chroot, &chroot::get_default_shell,
                                keyfile, chroot.get_name(),
                                "shell");
    }

    void
    chroot::set_keyfile (keyfile const& keyfile)
    {
      set_keyfile(*this, keyfile);

      for (const auto& facet : facets)
        facet->set_keyfile(*this, keyfile);
    }

    void
    chroot::set_keyfile (chroot&        chroot,
                         keyfile const& keyfile)
    {
      // Null method for obsolete keys.
      void (chroot::* nullmethod)(bool) = 0;

      bool session = static_cast<bool>(get_facet<chroot_facet_session>());

      keyfile::get_object_value(chroot, nullmethod,
                                keyfile, chroot.get_name(),
                                "active",
                                keyfile::PRIORITY_OBSOLETE);

      // Setup scripts are run depending on the chroot type in use, and is
      // no longer user-configurable.  They need to run for all types
      // except "plain".
      keyfile::get_object_value(chroot, nullmethod,
                                keyfile, chroot.get_name(),
                                "run-setup-scripts",
                                keyfile::PRIORITY_OBSOLETE);

      // Exec scripts have been removed, so these two calls do nothing
      // except to warn the user that the options are no longer used.
      keyfile::get_object_value(chroot, nullmethod,
                                keyfile, chroot.get_name(),
                                "run-session-scripts",
                                keyfile::PRIORITY_OBSOLETE);
      keyfile::get_object_value(chroot, nullmethod,
                                keyfile, chroot.get_name(),
                                "run-exec-scripts",
                                keyfile::PRIORITY_OBSOLETE);

      keyfile::get_object_value(chroot, &chroot::set_profile,
                                keyfile, chroot.get_name(),
                                "profile",
                                keyfile::PRIORITY_OPTIONAL);

      keyfile::get_object_value(chroot, &chroot::set_script_config,
                                keyfile, chroot.get_name(),
                                "script-config",
                                session ?
                                keyfile::PRIORITY_OPTIONAL :
                                keyfile::PRIORITY_DEPRECATED);

      keyfile::get_object_value(chroot, nullmethod,
                                keyfile, chroot.get_name(),
                                "priority",
                                session ?
                                keyfile::PRIORITY_OPTIONAL :
                                keyfile::PRIORITY_OBSOLETE);

      keyfile::get_object_list_value(chroot, &chroot::set_aliases,
                                     keyfile, chroot.get_name(),
                                     "aliases",
                                     keyfile::PRIORITY_OPTIONAL);

      keyfile::get_object_value(chroot, &chroot::set_environment_filter,
                                keyfile, chroot.get_name(),
                                "environment-filter",
                                keyfile::PRIORITY_OPTIONAL);

      keyfile::get_object_value(chroot, &chroot::set_description,
                                keyfile, chroot.get_name(),
                                "description",
                                keyfile::PRIORITY_OPTIONAL);

      keyfile::get_object_list_value(chroot, &chroot::set_users,
                                     keyfile, chroot.get_name(),
                                     "users",
                                     keyfile::PRIORITY_OPTIONAL);

      keyfile::get_object_list_value(chroot, &chroot::set_groups,
                                     keyfile, chroot.get_name(),
                                     "groups",
                                     keyfile::PRIORITY_OPTIONAL);

      keyfile::get_object_list_value(chroot, &chroot::set_root_users,
                                     keyfile, chroot.get_name(),
                                     "root-users",
                                     keyfile::PRIORITY_OPTIONAL);

      keyfile::get_object_list_value(chroot, &chroot::set_root_groups,
                                     keyfile, chroot.get_name(),
                                     "root-groups",
                                     keyfile::PRIORITY_OPTIONAL);

      keyfile::get_object_value(chroot, &chroot::set_mount_location,
                                keyfile, chroot.get_name(),
                                "mount-location",
                                session ?
                                keyfile::PRIORITY_REQUIRED :
                                keyfile::PRIORITY_DISALLOWED);

      keyfile::get_object_value(chroot, &chroot::set_name,
                                keyfile, chroot.get_name(),
                                "name",
                                session ?
                                keyfile::PRIORITY_OPTIONAL :
                                keyfile::PRIORITY_DISALLOWED);

      keyfile::get_object_list_value(chroot, &chroot::set_command_prefix,
                                     keyfile, chroot.get_name(),
                                     "command-prefix",
                                     keyfile::PRIORITY_OPTIONAL);

      keyfile::get_object_value(chroot, &chroot::set_verbosity,
                                keyfile, chroot.get_name(),
                                "message-verbosity",
                                keyfile::PRIORITY_OPTIONAL);

      keyfile::get_object_value(chroot, &chroot::set_preserve_environment,
                                keyfile, chroot.get_name(),
                                "preserve-environment",
                                keyfile::PRIORITY_OPTIONAL);

      keyfile::get_object_value(chroot, &chroot::set_default_shell,
                                keyfile, chroot.get_name(),
                                "shell",
                                keyfile::PRIORITY_OPTIONAL);
    }

  }
}
