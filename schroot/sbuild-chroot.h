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

#ifndef SBUILD_CHROOT_H
#define SBUILD_CHROOT_H

#include "sbuild-config.h"

#include <iomanip>
#include <ostream>
#include <string>
#include <vector>

#ifdef HAVE_TR1_MEMORY
#include <tr1/memory>
#elif HAVE_BOOST_SHARED_PTR_HPP
#include <boost/shared_ptr.hpp>
namespace std { namespace tr1 { using boost::shared_ptr; } }
#else
#error A shared_ptr implementation is not available
#endif

#include "sbuild-error.h"
#include "sbuild-environment.h"
#include "sbuild-keyfile.h"
#include "sbuild-personality.h"
#include "sbuild-util.h"

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
	SESSION_CREATE = 1 << 0 ///< The chroot supports session creation.
      };

    /// Exception type.
    typedef runtime_error_custom<chroot> error;

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
     * @returns the mount location.
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
     * Get the groups allowed to access the chroot.
     *
     * @returns a list of groups.
     */
    string_list const&
    get_groups () const;

    /**
     * Set the groups allowed to access the chroot.
     *
     * @param groups a list of groups.
     */
    void
    set_groups (string_list const& groups);

    /**
     * Get the groups allowed to access the chroot as root.  Mmebers
     * of these groups can switch to root without authenticating
     * themselves.
     *
     * @returns a list of groups.
     */
    string_list const&
    get_root_groups () const;

    /**
     * Set the groups allowed to access the chroot as root.  Mmebers
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
     * @returns the command prefix.
     */
    personality const&
    get_persona () const;

    /**
     * Set the process execution domain for the chroot.
     *
     * @param persona the command prefix.
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
      rhs->set_keyfile(keyfile);
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
     * Print detailed information about the chroot to a stream.  The
     * information is printed in plain text with one line per
     * property.
     *
     * @param stream the stream to output to.
     */
    virtual void
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
     */
    virtual void
    set_keyfile (keyfile const& keyfile);

  private:
    /// Chroot name.
    std::string   name;
    /// Chroot description.
    std::string   description;
    /// Chroot prioroty.
    unsigned int  priority;
    /// Groups allowed to access the chroot.
    string_list   groups;
    /// Groups allowed to access the chroot as root.
    string_list   root_groups;
    /// Alternative names for the chroot.
    string_list   aliases;
    /// Location to mount chroot in the filesystem (if any).
    std::string   mount_location;
    /// Location inside the mount location root.
    std::string   location;
    /// Block device to mount (if any).
    std::string   mount_device;
    /// Chroot activity status.
    bool          active;
    /// Run chroot setup scripts?
    bool          run_setup_scripts;
    /// Run chroot exec scripts?
    bool          run_exec_scripts;
    /// Command prefix.
    string_list   command_prefix;
    /// Process execution domain (Linux only).
    personality   persona;
  };

}

#endif /* SBUILD_CHROOT_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
