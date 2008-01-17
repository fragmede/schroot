/* Copyright © 2005-2007  Roger Leigh <rleigh@debian.org>
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

#ifndef SBUILD_CHROOT_H
#define SBUILD_CHROOT_H

#include <sbuild/sbuild-custom-error.h>
#include <sbuild/sbuild-environment.h>
#include <sbuild/sbuild-format-detail.h>
#include <sbuild/sbuild-keyfile.h>
#include <sbuild/sbuild-personality.h>
#include <sbuild/sbuild-regex.h>
#include <sbuild/sbuild-tr1types.h>

#include <ostream>
#include <string>

namespace sbuild
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
	EXEC_START,    ///< Start executing a command in an active chroot.
	EXEC_STOP      ///< End executing a command in an active chroot.
      };

    /// Chroot session properties
    enum session_flags
      {
	SESSION_NOFLAGS = 0,     ///< No flags are set.
	SESSION_CREATE = 1 << 0, ///< The chroot supports session creation.
	SESSION_CLONE  = 1 << 1  ///< The chroot supports cloning.
	SESSION_PURGE  = 1 << 2  ///< The chroot should be purged.
      };

    /// Error codes.
    enum error_code
      {
	CHROOT_CREATE,   ///< Chroot creation failed.
	CHROOT_DEVICE,   ///< Chroot device name not set.
	CHROOT_TYPE,     ///< Unknown chroot type.
	DEVICE_ABS,      ///< Device must have an absolute path.
	DEVICE_LOCK,     ///< Failed to lock device.
	DEVICE_NOTBLOCK, ///< File is not a block device.
	DEVICE_UNLOCK,   ///< Failed to unlock device.
	FILE_ABS,        ///< File must have an absolute path.
	FILE_LOCK,       ///< Failed to acquire lock.
	FILE_NOTREG,     ///< File is not a regular file.
	FILE_OWNER,      ///< File is not owned by user root.
	FILE_PERMS,      ///< File has write permissions for others.
	FILE_UNLOCK,     ///< Failed to discard lock.
	LOCATION_ABS,    ///< Location must have an absolute path.
	SESSION_UNLINK,  ///< Failed to unlink session file.
	SESSION_WRITE    ///< Failed to write session file.
      };

    /// Exception type.
    typedef custom_error<error_code> error;

    /// A shared_ptr to a chroot object.
    typedef std::tr1::shared_ptr<chroot> ptr;

  protected:
    /// The constructor.
    chroot ();

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
    virtual std::string const&
    get_mount_location () const;

    /**
     * Set the mount location of the chroot.
     *
     * @param location the mount location.
     */
    void
    set_mount_location (std::string const& location);

    /**
     * Get the location of the chroot.  This is the path to the root
     * of the chroot, and is typically the same as the mount location,
     * but is overridden by the chroot type if required.
     *
     * @returns the mount location.
     */
    virtual std::string const&
    get_location () const;

  protected:
    /**
     * Set the location of the chroot.  This is the path to the root
     * of the chroot, and is typically the same as the mount location,
     * but is overridden by the chroot type if required.
     *
     * @param location the mount location.
     */
    virtual void
    set_location (std::string const& location);

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
    get_path () const;

    /**
     * Get the mount device of the chroot.
     *
     * @returns the device.
     */
    virtual std::string const&
    get_mount_device () const;

    /**
     * Set the mount device of the chroot.
     *
     * @param device the device.
     */
    void
    set_mount_device (std::string const& device);

    /**
     * Get the priority of the chroot.  This is a number indicating
     * whether than a ditribution is older than another.
     *
     * @returns the priority.
     */
    unsigned int
    get_priority () const;

    /**
     * Set the priority of a chroot.  This is a number indicating
     * whether a distribution is older than another.  For example,
     * "oldstable" and "oldstable-security" might be 0, while "stable"
     * and "stable-security" 1, "testing" 2 and "unstable" 3.  The
     * values are not important, but the difference between them is.
     *
     * @param priority the priority.
     */
    void
    set_priority (unsigned int priority);

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
     * Get the environment filter of the chroot.  This is a POSIX
     * extended regular expression used to remove insecure environment
     * variables from the chroot environment.
     *
     * @returns the filter
     */
    regex const&
    get_environment_filter () const;

    /**
     * Get the environment filter of the chroot.  This is a POSIX
     * extended regular expression used to remove insecure environment
     * variables from the chroot environment.
     *
     * @param environment_filter the filter.
     */
    void
    set_environment_filter (regex const& environment_filter);

    /**
     * Get the activity status of the chroot.
     *
     * @returns true if active, false if inactive
     */
    bool
    get_active () const;

    /**
     * Set the activity status of the chroot.
     *
     * @param active true if active, false if inactive
     */
    void
    set_active (bool active);

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
     * @param original true if original, false if geneated.
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

    /**
     * Set whether chroot setup scripts will be run.
     *
     * @param run_setup_scripts true if setup scripts will be run,
     * otherwise false.
     */
    void
    set_run_setup_scripts (bool run_setup_scripts);

    /**
     * Check if chroot exec scripts will be run.
     *
     * @returns true if exec scripts will be run, otherwise false.
     */
    bool
    get_run_exec_scripts () const;

    /**
     * Set whether chroot exec scripts will be run.
     *
     * @param run_exec_scripts true if exec scripts will be run,
     * otherwise false.
     */
    void
    set_run_exec_scripts (bool run_exec_scripts);

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
     * Get the process execution domain for the chroot.
     *
     * @returns the personality.
     */
    personality const&
    get_persona () const;

    /**
     * Set the process execution domain for the chroot.
     *
     * @param persona the personality.
     */
    void
    set_persona (personality const& persona);

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
    virtual void
    setup_env (environment& env);

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
     * Get the session flags of the chroot.  These determine how the
     * Session controlling the chroot will operate.
     *
     * @returns the session flags.
     */
    virtual session_flags
    get_session_flags () const = 0;

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
     */
    friend
    keyfile const&
    operator >> (keyfile const& keyfile,
		 ptr&           rhs)
    {
      string_list used;
      rhs->set_keyfile(keyfile, used);
      keyfile.check_keys(rhs->get_name(), used);
      return keyfile;
    }

    /**
     * Chroot serialisation to a keyfile.
     */
    friend
    keyfile&
    operator << (keyfile&   keyfile,
		 ptr const& rhs)
    {
      rhs->get_keyfile(keyfile);
      return keyfile;
    }


  protected:
    /**
     * Get detailed information about the chroot for output.
     *
     * @param detail the details to output to.
     */
    virtual void
    get_details (format_detail& detail) const;

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
    virtual void
    get_keyfile (keyfile& keyfile) const;

    /**
     * Set the chroot properties from a keyfile.  The chroot name must
     * have previously been set, so that the correct keyfile group may
     * be determined.
     *
     * @param keyfile the keyfile to get the properties from.
     * @param used_keys a list of the keys used will be set.
     */
    virtual void
    set_keyfile (keyfile const& keyfile,
		 string_list&   used_keys);

  private:
    /// Chroot name.
    std::string   name;
    /// Chroot description.
    std::string   description;
    /// Chroot prioroty.
    unsigned int  priority;
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
    /// Environment filter regex.
    regex         environment_filter;
    /// Location to mount chroot in the filesystem (if any).
    std::string   mount_location;
    /// Location inside the mount location root.
    std::string   location;
    /// Block device to mount (if any).
    std::string   mount_device;
    /// Chroot activity status.
    bool          active;
    /// Was the chroot automatically generated?
    bool          original;
    /// Run chroot setup scripts?
    bool          run_setup_scripts;
    /// Run chroot exec scripts?
    bool          run_exec_scripts;
    /// Configuration of the setup and exec scripts.
    std::string   script_config;
    /// Command prefix.
    string_list   command_prefix;
    /// Process execution domain (Linux only).
    personality   persona;
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

#endif /* SBUILD_CHROOT_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
