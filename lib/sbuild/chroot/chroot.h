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

#ifndef SBUILD_CHROOT_CHROOT_H
#define SBUILD_CHROOT_CHROOT_H

#include <sbuild/custom-error.h>
#include <sbuild/environment.h>
#include <sbuild/format-detail.h>
#include <sbuild/keyfile.h>
#include <sbuild/regex.h>

#include <list>
#include <memory>
#include <ostream>
#include <string>

namespace sbuild
{
  class chroot_facet;

  namespace chroot
  {


    /**
     * Common chroot data.  This class contains all of the metadata
     * associated with a single chroot, for all chroot types.  This is
     * the in-core representation of a chroot definition in the
     * configuration file, and may be initialised directly from an open
     * keyfile.
     */
    class chroot
    {
    public:
      /// Type of setup to perform.
      enum setup_type
        {
          SETUP_START,   ///< Activate a chroot.
          SETUP_RECOVER, ///< Reactivate a chroot.
          SETUP_STOP,    ///< Deactivate a chroot.
          EXEC_START,    ///< Prepare for executing a command.
          EXEC_STOP      ///< Clean up after executing a command.
        };

      /// Chroot session properties.
      enum session_flags
        {
          SESSION_NOFLAGS = 0,      ///< No flags are set.
          SESSION_CREATE  = 1 << 0, ///< The chroot supports session creation.
          SESSION_CLONE   = 1 << 1, ///< The chroot supports cloning.
          SESSION_PURGE   = 1 << 2  ///< The chroot should be purged.
        };

      /// Message verbosity.
      enum verbosity
        {
          VERBOSITY_QUIET,  ///< Only print essential messages.
          VERBOSITY_NORMAL, ///< Print messages (the default).
          VERBOSITY_VERBOSE ///< Print all messages.
        };

      /// Error codes.
      enum error_code
        {
          CHROOT_CREATE,    ///< Chroot creation failed.
          CHROOT_DEVICE,    ///< Chroot device name not set.
          CHROOT_TYPE,      ///< Unknown chroot type.
          DEVICE_ABS,       ///< Device must have an absolute path.
          DEVICE_LOCK,      ///< Failed to lock device.
          DEVICE_NOTBLOCK,  ///< File is not a block device.
          DEVICE_UNLOCK,    ///< Failed to unlock device.
          DIRECTORY_ABS,    ///< Directory must have an absolute path.
          FACET_INVALID,    ///< Attempt to add object which is not a facet.
          FACET_PRESENT,    ///< Attempt to add facet which is already in use.
          FILE_ABS,         ///< File must have an absolute path.
          FILE_LOCK,        ///< Failed to acquire lock.
          FILE_NOTREG,      ///< File is not a regular file.
          FILE_OWNER,       ///< File is not owned by user root.
          FILE_PERMS,       ///< File has write permissions for others.
          FILE_UNLOCK,      ///< Failed to discard lock.
          LOCATION_ABS,     ///< Location must have an absolute path.
          NAME_INVALID,     ///< Invalid name.
          SCRIPT_CONFIG_CV, ///< Could not set profile from script configuration path.
          SESSION_UNLINK,   ///< Failed to unlink session file.
          SESSION_WRITE,    ///< Failed to write session file.
          VERBOSITY_INVALID ///< Message verbosity is invalid.
        };

      /// Exception type.
      typedef custom_error<error_code> error;

      /// A shared_ptr to a chroot object.
      typedef std::shared_ptr<chroot> ptr;

      /// A shared_ptr to a const chroot object.
      typedef std::shared_ptr<const chroot> const_ptr;

    protected:
      /// The constructor.
      chroot ();

      /// The copy constructor.
      chroot (const chroot& rhs);

    public:
      /// The destructor.
      virtual ~chroot ();

      /**
       * Create a chroot.  This is a factory function.
       *
       * @param type the type of chroot to create.
       * @returns a shared_ptr to the new chroot.
       */
      static ptr
      create (std::string const& type);

      /**
       * Copy the chroot.  This is a virtual copy constructor.
       *
       * @returns a shared_ptr to the new copy of the chroot.
       */
      virtual ptr
      clone () const = 0;

      /**
       * Create a session chroot.
       *
       * @param session_id the identifier (session_id) for the new session.
       * @param alias used to initially identify the chroot.
       * @param user the user creating the session.
       * @param root true if the user has root access, otherwise false.
       * @returns a session chroot.
       */
      virtual chroot::ptr
      clone_session (std::string const& session_id,
                     std::string const& alias,
                     std::string const& user,
                     bool               root) const = 0;

      /**
       * Create a source chroot.
       *
       * @returns a source chroot.
       */
      virtual chroot::ptr
      clone_source () const = 0;

      /**
       * Get the name of the chroot.
       *
       * @returns the name.
       */
      std::string const&
      get_name () const;

      /**
       * Set the name of the chroot.
       *
       * @param name the name.
       */
      void
      set_name (std::string const& name);

      /**
       * Get the description of the chroot.
       *
       * @returns the description.
       */
      std::string const&
      get_description () const;

      /**
       * Set the description of the chroot.
       *
       * @param description the description.
       */
      void
      set_description (std::string const& description);

      /**
       * Get the mount location of the chroot.
       *
       * @returns the mount location.
       */
      std::string const&
      get_mount_location () const;

      /**
       * Set the mount location of the chroot.
       *
       * @param location the mount location.
       */
      void
      set_mount_location (std::string const& location);

    public:
      /**
       * Get the path to the chroot.  This is the absolute path to the
       * root of the chroot, and is typically the same as the mount
       * location and location concatenated together, but is overridden
       * by the chroot type if required.
       *
       * @returns the path.
       */
      virtual std::string
      get_path () const = 0;

      /**
       * Get the users allowed to access the chroot.
       *
       * @returns a list of users.
       */
      string_list const&
      get_users () const;

      /**
       * Set the users allowed to access the chroot.
       *
       * @param users a list of users.
       */
      void
      set_users (string_list const& users);

      /**
       * Get the groups allowed to access the chroot.
       *
       * @returns a list of groups.
       */
      string_list const&
      get_groups () const;

      /**
       * Set the users allowed to access the chroot.
       *
       * @param groups a list of groups.
       */
      void
      set_groups (string_list const& groups);

      /**
       * Get the users allowed to access the chroot as root.  Members
       * of these users can switch to root without authenticating
       * themselves.
       *
       * @returns a list of users.
       */
      string_list const&
      get_root_users () const;

      /**
       * Set the users allowed to access the chroot as root.  Members
       * of these users can switch to root without authenticating
       * themselves.
       *
       * @param users a list of users.
       */
      void
      set_root_users (string_list const& users);

      /**
       * Get the groups allowed to access the chroot as root.  Members
       * of these groups can switch to root without authenticating
       * themselves.
       *
       * @returns a list of groups.
       */
      string_list const&
      get_root_groups () const;

      /**
       * Set the groups allowed to access the chroot as root.  Members
       * of these groups can switch to root without authenticating
       * themselves.
       *
       * @param groups a list of groups.
       */
      void
      set_root_groups (string_list const& groups);

      /**
       * Get the aliases of the chroot.  These are alternative names for
       * the chroot.
       *
       * @returns a list of names.
       */
      string_list const&
      get_aliases () const;

      /**
       * Set the aliases of the chroot.  These are alternative names for
       * the chroot.
       *
       * @param aliases a list of names.
       */
      void
      set_aliases (string_list const& aliases);

      /**
       * Check if the environment should be preserved in the chroot.
       *
       * @returns true to preserve or false to clean.
       */
      bool
      get_preserve_environment () const;

      /**
       * Set if the environment should be preserved in the chroot.
       *
       * @param preserve_environment true to preserve or false to clean.
       */
      void
      set_preserve_environment (bool preserve_environment);

      /**
       * Get default shell.
       *
       * @returns default shell, or empty string if unset
       */
      std::string const&
      get_default_shell () const;

      /**
       * Set the default shell.  This is the default interactive shell.
       *
       * @param default_shell the default shell.
       */
      void
      set_default_shell (std::string const& default_shell);

      /**
       * Get the environment filter of the chroot.  This is a POSIX
       * extended regular expression used to remove insecure environment
       * variables from the chroot environment.
       *
       * @returns the filter
       */
      regex const&
      get_environment_filter () const;

      /**
       * Set the environment filter of the chroot.  This is a POSIX
       * extended regular expression used to remove insecure environment
       * variables from the chroot environment.
       *
       * @param environment_filter the filter.
       */
      void
      set_environment_filter (regex const& environment_filter);

      /**
       * Get the activity status of the chroot.  The chroot is active if
       * it has been cloned as a session.
       *
       * @returns true if active, false if inactive
       */
      bool
      get_active () const;

      /**
       * Get the originality of the chroot.
       *
       * @returns true if original, false if generated.
       */
      bool
      get_original () const;

      /**
       * Set the originality of the chroot.
       *
       * @param original true if original, false if generated.
       */
      void
      set_original (bool original);

      /**
       * Check if chroot setup scripts will be run.
       *
       * @returns true if setup scripts will be run, otherwise false.
       */
      bool
      get_run_setup_scripts () const;

    protected:
      /**
       * Set whether chroot setup scripts will be run.
       *
       * @param run_setup_scripts true if setup scripts will be run,
       * otherwise false.
       */
      void
      set_run_setup_scripts (bool run_setup_scripts);

    public:
      /**
       * Get the script configuration file for the chroot.  This is a
       * filename, either relative to the configured pkgsysconfdir or an
       * absolute path.
       *
       * @returns the configuration file name.
       */
      std::string const&
      get_script_config () const;

      /**
       * Set the script configuration file for the chroot.  This is a
       * filename, either relative to the configured pkgsysconfdir or an
       * absolute path.
       *
       * @param script_config the script configuration file.
       */
      void
      set_script_config (std::string const& script_config);

      /**
       * Get the configuration profile for the chroot.  This is a
       * directory, either relative to the configured pkgsysconfdir or
       * an absolute path.
       *
       * @returns the configuration file name.
       */
      std::string const&
      get_profile () const;

      /**
       * Set configuration profile for the chroot.  This is a directory,
       * either relative to the configured pkgsysconfdir or an absolute
       * path.
       *
       * @param profile the script configuration file.
       */
      void
      set_profile (std::string const& profile);

      /**
       * Get the command_prefix for the chroot.  This is a command to
       * prefix to any command run in the chroot.
       *
       * @returns the command prefix.
       */
      string_list const&
      get_command_prefix () const;

      /**
       * Set the command_prefix for the chroot.  This is a command to
       * prefix to any command run in the chroot.
       *
       * @param command_prefix the command prefix.
       */
      void
      set_command_prefix (string_list const& command_prefix);

      /**
       * Get the message verbosity.
       *
       * @returns the verbosity level.
       */
      verbosity
      get_verbosity () const;

      /**
       * Get the message verbosity as a readable string.
       *
       * @returns the verbosity level as a readable string.
       */
      const char *
      get_verbosity_string () const;

      /**
       * Set the message verbosity.
       *
       * @param verbosity the verbosity level.
       */
      void
      set_verbosity (verbosity verbosity);

      /**
       * Set the message verbosity.
       *
       * @param verbosity the verbosity level.
       */
      void
      set_verbosity (std::string const& verbosity);

      /**
       * Get the type of the chroot.
       *
       * @returns the chroot type.
       */
      virtual std::string const&
      get_chroot_type () const = 0;

      /**
       * Set environment.  Set the environment that the setup scripts
       * will see during execution.
       *
       * @param env the environment to set.
       */
      void
      setup_env (environment& env) const;

      /**
       * Set environment.  Set the environment that the setup scripts
       * will see during execution.
       *
       * @param chroot the chroot to use.
       * @param env the environment to set.
       */
      virtual void
      setup_env (chroot const& chroot,
                 environment& env) const = 0;

      /**
       * Lock a chroot during setup.  The locking technique (if any) may
       * vary depending upon the chroot type and setup stage.  For
       * example, during creation of an LVM snapshot a block device
       * might require locking, but afterwards this will change to the
       * new block device.
       *
       * An error will be thrown on failure.
       *
       * @param type the type of setup being performed
       */
      void
      lock (setup_type type);

      /**
       * Unlock a chroot during setup.  The locking technique (if any) may
       * vary depending upon the chroot type and setup stage.  For
       * example, during creation of an LVM snapshot a block device
       * might require locking, but afterwards this will change to the
       * new block device.
       *
       * An error will be thrown on failure.
       *
       * @param type the type of setup being performed
       * @param status the exit status of the setup commands (0 for
       * success, nonzero for failure).
       */
      void
      unlock (setup_type type,
              int        status);

    protected:
      /**
       * Set up persistent session information.
       *
       * @param start true if startion, or false if ending a session.
       */
      virtual void
      setup_session_info (bool start);

      /**
       * Unlock a chroot during setup.  The locking technique (if any) may
       * vary depending upon the chroot type and setup stage.  For
       * example, during creation of an LVM snapshot a block device
       * might require locking, but afterwards this will change to the
       * new block device.
       *
       * An error will be thrown on failure.
       *
       * @param type the type of setup being performed
       * @param lock true to lock, false to unlock
       * @param status the exit status of the setup commands (0 for
       * success, nonzero for failure).
       */
      virtual void
      setup_lock(setup_type type,
                 bool       lock,
                 int        status) = 0;

    public:
      /**
       * Get a chroot facet.  This is a templated method; use the
       * correct type for the facet required.
       *
       * @returns a shared_ptr to the facet, or to NULL if the facet
       * does not exist.
       */
      template <typename T>
      std::shared_ptr<T>
      get_facet ();

      /**
       * Get a chroot facet.  This is a templated method; use the
       * correct type for the facet required.
       *
       * @returns a shared_ptr to the facet, or to NULL if the facet
       * does not exist.
       */
      template <typename T>
      const std::shared_ptr<const T>
      get_facet () const;

      /**
       * Add a chroot facet.
       *
       * @param facet the facet to add.
       */
      template <typename T>
      void
      add_facet (std::shared_ptr<T> facet);

      /**
       * Remove a chroot facet.  This is a templated method; use the
       * correct type for the facet to remove.
       */
      template <typename T>
      void
      remove_facet ();

      /**
       * Remove a chroot facet.
       *
       * @param facet the facet to remove.
       */
      template <typename T>
      void
      remove_facet (std::shared_ptr<T> facet);

      /**
       * Replace an existing chroot facet with a new facet.
       *
       * @param facet the replacement facet.
       */
      template <typename T>
      void
      replace_facet (std::shared_ptr<T> facet);

      /**
       * List all registered chroot facets.
       *
       * @returns a list of facets.
       */
      string_list
      list_facets () const;

      /**
       * Get the session flags of the chroot.  These determine how the
       * Session controlling the chroot will operate.
       *
       * @returns the session flags.
       */
      session_flags
      get_session_flags () const;

      /**
       * Get the session flags of the chroot.  These determine how the
       * Session controlling the chroot will operate.
       *
       * @param chroot the chroot to use.
       * @returns the session flags.
       */
      virtual chroot::session_flags
      get_session_flags (chroot const& chroot) const = 0;

      /**
       * Print detailed information about the chroot to a stream.  The
       * information is printed in plain text with one line per
       * property.
       *
       * @param stream the stream to output to.
       * @param rhs the chroot to output.
       * @returns the stream.
       */
      friend std::ostream&
      operator << (std::ostream& stream,
                   ptr const&    rhs)
      {
        rhs->print_details(stream);
        return stream;
      }

      /**
       * Chroot initialisation from a keyfile.
       *
       * @param keyfile the keyfile to get the properties from.
       * @param rhs the chroot to output.
       * @returns the keyfile.
       */
      friend
      keyfile const&
      operator >> (keyfile const& keyfile,
                   ptr&           rhs)
      {
        rhs->set_keyfile(keyfile);
        return keyfile;
      }

      /**
       * Chroot serialisation to a keyfile.
       *
       * @param keyfile the keyfile to use.
       * @param rhs the chroot to output.
       * @returns the keyfile.
       */
      friend
      keyfile&
      operator << (keyfile&   keyfile,
                   ptr const& rhs)
      {
        rhs->get_keyfile(keyfile);
        return keyfile;
      }

      /**
       * Get detailed information about the chroot for output.
       *
       * @param detail the details to output to.
       */
      void
      get_details (format_detail& detail) const;

      /**
       * Get detailed information about the chroot for output.
       *
       * @param chroot the chroot to use.
       * @param detail the details to output to.
       */
      virtual void
      get_details (chroot const&  chroot,
                   format_detail& detail) const = 0;

      /**
       * Print detailed information about the chroot to a stream.  The
       * information is printed in plain text with one line per
       * property.
       *
       * @param stream the stream to output to.
       */
      void
      print_details (std::ostream& stream) const;

      /**
       * Copy the chroot properties into a keyfile.  The keyfile group
       * with the name of the chroot will be set; if it already exists,
       * it will be removed before setting it.
       *
       * @param keyfile the keyfile to use.
       */
      void
      get_keyfile (keyfile& keyfile) const;

    protected:
      /**
       * Copy the chroot properties into a keyfile.  The keyfile group
       * with the name of the chroot will be set; if it already exists,
       * it will be removed before setting it.
       *
       * @param chroot the chroot to use.
       * @param keyfile the keyfile to use.
       */
      virtual void
      get_keyfile (chroot const& chroot,
                   keyfile&      keyfile) const = 0;

    public:
      /**
       * Set the chroot properties from a keyfile.  The chroot name must
       * have previously been set, so that the correct keyfile group may
       * be determined.
       *
       * @param keyfile the keyfile to get the properties from.
       */
      void
      set_keyfile (keyfile const& keyfile);

      /**
       * Get a list of the keys used during keyfile parsing.
       *
       * @returns a list of key names.
       */
      string_list
      get_used_keys () const;

    protected:
      virtual void
      get_used_keys (string_list& used_keys) const = 0;

      /**
       * Set the chroot properties from a keyfile.  The chroot name must
       * have previously been set, so that the correct keyfile group may
       * be determined.
       *
       * @param chroot the chroot to use.
       * @param keyfile the keyfile to get the properties from.
       * @param used_keys a list of the keys used will be set.
       */
      virtual void
      set_keyfile (chroot&        chroot,
                   keyfile const& keyfile) = 0;

    private:
      /// Chroot name.
      std::string   name;
      /// Chroot description.
      std::string   description;
      /// Users allowed to access the chroot.
      string_list   users;
      /// Groups allowed to access the chroot.
      string_list   groups;
      /// Users allowed to access the chroot as root.
      string_list   root_users;
      /// Groups allowed to access the chroot as root.
      string_list   root_groups;
      /// Alternative names for the chroot.
      string_list   aliases;
      /// Preserve environment?
      bool          preserve_environment;
      /// Default shell
      std::string   default_shell;
      /// Environment filter regex.
      regex         environment_filter;
      /// Location to mount chroot in the filesystem (if any).
      std::string   mount_location;
      /// Was the chroot automatically generated?
      bool          original;
      /// Run chroot setup scripts?
      bool          run_setup_scripts;
      /// Configuration of the setup and exec scripts.
      std::string   script_config;
      /// Configuration profile for setup scripts (replaces script_config).
      std::string   profile;
      /// Command prefix.
      string_list   command_prefix;
      /// The message verbosity.
      verbosity     message_verbosity;

      /// A shared pointer to a chroot facet.
      typedef std::shared_ptr<chroot_facet> facet_ptr;
      /// A list of chroot facets.
      typedef std::list<facet_ptr> facet_list;
      /// Contained chroot facets
      facet_list facets;
    };

    /**
     * Bitwise-OR of specifed session properties
     * @param lhs session properties
     * @param rhs session properties
     * @returns result of OR.
     */
    chroot::session_flags
    inline operator | (chroot::session_flags const& lhs,
                       chroot::session_flags const& rhs)
    {
      return static_cast<chroot::session_flags>
        (static_cast<int>(lhs) | static_cast<int>(rhs));
    }

    /**
     * Bitwise-AND of specifed session properties
     * @param lhs session properties
     * @param rhs session properties
     * @returns result of AND.
     */
    chroot::session_flags
    inline operator & (chroot::session_flags const& lhs,
                       chroot::session_flags const& rhs)
    {
      return static_cast<chroot::session_flags>
        (static_cast<int>(lhs) & static_cast<int>(rhs));
    }

  }
}

#include <sbuild/chroot-facet.h>

namespace sbuild
{
  namespace chroot
  {

    template <typename T>
    std::shared_ptr<T>
    chroot::get_facet ()
    {
      std::shared_ptr<T> ret;

      for (const auto& facet : facets)
        {
          if (ret = std::dynamic_pointer_cast<T>(facet))
            break;
        }

      return ret;
    }

    template <typename T>
    const std::shared_ptr<const T>
    chroot::get_facet () const
    {
      std::shared_ptr<T> ret;

      for (const auto& facet : facets)
        {
          if (ret = std::dynamic_pointer_cast<T>(facet))
            break;
        }

      return std::const_pointer_cast<T>(ret);
    }

    template <typename T>
    void
    chroot::add_facet (std::shared_ptr<T> facet)
    {
      facet_ptr new_facet = std::dynamic_pointer_cast<chroot_facet>(facet);
      if (!new_facet)
        throw error(FACET_INVALID);

      for (const auto& facet : facets)
        {
          if (std::dynamic_pointer_cast<T>(facet))
            throw error(FACET_PRESENT);
        }

      new_facet->set_chroot(*this);
      facets.push_back(new_facet);
    }

    template <typename T>
    void
    chroot::remove_facet ()
    {
      for (facet_list::iterator facet = facets.begin();
           facet != facets.end();
           ++facet)
        {
          if (std::dynamic_pointer_cast<T>(*facet))
            {
              facets.erase(facet);
              break;
            }
        }
    }

    template <typename T>
    void
    chroot::remove_facet (std::shared_ptr<T> facet)
    {
      remove_facet<T>();
    }

    template <typename T>
    void
    chroot::replace_facet (std::shared_ptr<T> facet)
    {
      remove_facet<T>();
      add_facet(facet);
    }

  }
}

#endif /* SBUILD_CHROOT_CHROOT_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
