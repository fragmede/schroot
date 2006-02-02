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

#include <iomanip>
#include <ostream>
#include <string>
#include <vector>
#include <tr1/memory>

#include "sbuild-error.h"
#include "sbuild-environment.h"
#include "sbuild-keyfile.h"
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
  class Chroot
  {
  public:
    /// Type of setup to perform.
    enum SetupType
      {
	SETUP_START,   ///< Activate a chroot.
	SETUP_RECOVER, ///< Reactivate a chroot.
	SETUP_STOP,    ///< Deactivate a chroot.
	RUN_START,     ///< Start running a command in an active chroot.
	RUN_STOP       ///< End running a command in an active chroot.
      };

    /// Chroot session properties
    enum SessionFlags
      {
	SESSION_CREATE = 1 << 0 ///< The chroot supports session creation.
      };

    /// Exception type.
    typedef runtime_error_custom<Chroot> error;

    /// A shared_ptr to an AuthConv object.
    typedef std::tr1::shared_ptr<Chroot> chroot_ptr;

  protected:
    /// The constructor.
    Chroot ();

    /**
     * The constructor.  Initialise from an open keyfile.
     *
     * @param keyfile the configuration file
     * @param group the keyfile group (chroot name)
     */
    Chroot (keyfile const&     keyfile,
	    std::string const& group);
  public:

    /// The destructor.
    virtual ~Chroot();

    /**
     * Create a chroot.  This is a factory function.
     *
     * @param type the type of chroot to create.
     * @returns a shared_ptr to the new chroot.
     */
    static chroot_ptr
    create (std::string const& type);

    /**
     * Copy the chroot.  This is a virtual copy constructor.
     *
     * @returns a shared_ptr to the new copy of the chroot.
     */
    virtual chroot_ptr
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
     * Check if chroot session scripts will be run.
     *
     * @returns true if session scripts will be run, otherwise false.
     */
    bool
    get_run_session_scripts () const;

    /**
     * Set whether chroot session scripts will be run.
     *
     * @param run_session_scripts true if session scripts will be run,
     * otherwise false.
     */
    void
    set_run_session_scripts (bool run_session_scripts);

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
     * @param lock true to lock, false to unlock
     */
    virtual void
    setup_lock (SetupType type,
		bool      lock) = 0;

  protected:
    /**
     * Set up persistent session information.
     *
     * @param start true if startion, or false if ending a session.
     */
    virtual void
    setup_session_info (bool start);

  public:
    /**
     * Get the session flags of the chroot.  These determine how the
     * Session controlling the chroot will operate.
     *
     * @returns the session flags.
     */
    virtual SessionFlags
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
    operator << (std::ostream&     stream,
		 chroot_ptr const& rhs)
    {
      rhs->print_details(stream);
      return stream;
    }

    /**
     * Chroot initialisation from a keyfile.
     */
    friend
    keyfile const&
    operator >> (keyfile const& keyfile, chroot_ptr& rhs)
    {
      rhs->set_keyfile(keyfile);
      return keyfile;
    }

    /**
     * Chroot serialisation to a keyfile.
     */
    friend
    keyfile&
    operator << (keyfile& keyfile, chroot_ptr const& rhs)
    {
      rhs->get_keyfile(keyfile);
      return keyfile;
    }


  protected:
    /**
     * Helper to perform formatting of chroot details.
     */
    template<typename T>
    class format_detail
    {
      /**
       * The constructor.
       *
       * @param name the name of the property to format.
       * @param value the value of the property to format.  The value
       * type must support output to an ostream.
       */
    public:
      format_detail(std::string const& name,
		    T const&           value):
	name(name),
	value(value)
      {}

      /**
       * Output the formatted detail to an ostream.
       *
       * @param stream the stream to output to.
       * @param rhs the formatted detail to output.
       * @returns the stream.
       */
      friend std::ostream&
      operator << (std::ostream&           stream,
		   format_detail<T> const& rhs)
      {
	return stream << "  "
		      << std::setw(22) << std::left << rhs.name
		      << rhs.value << '\n';
      }

      /**
       * Output the formatted detail to an ostream.  This is a special
       * case for boolean values.
       *
       * @param stream the stream to output to.
       * @param rhs the formatted detail to output.
       * @returns the stream.
       */
      friend std::ostream&
      operator << (std::ostream&              stream,
		   format_detail<bool> const& rhs)
      {
	const char *desc = 0;
	if (rhs.value)
	  desc =  _("true");
	else
	  desc = _("false");
	return stream << format_detail<std::string>(rhs.name, desc);
      }

      /**
       * Output the formatted detail to an ostream.  This is a special
       * case for string_list values.
       *
       * @param stream the stream to output to.
       * @param rhs the formatted detail to output.
       * @returns the stream.
       */
      friend std::ostream&
      operator << (std::ostream&                     stream,
		   format_detail<string_list> const& rhs)
      {
	return stream <<
	  format_detail<std::string>(rhs.name,
				     string_list_to_string(rhs.value, " "));
      }

    private:
      /// The name of the property.
      std::string const& name;
      /// The value of the property.
      T const&           value;
    };

    /**
     * Format a name-value pair for output.  This is a convenience
     * wrapper to construct a format_detail of the appropriate type.
     *
     * @param name the name to output.
     * @param value the value to output.
     * @returns a format_detail of the appropriate type.
     */
    template<typename T>
    format_detail<T>
    format_details(std::string const& name,
		   T const&           value) const
    {
      return format_detail<T>(name, value);
    }

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
    /// Block device to mount (if any).
    std::string   mount_device;
    /// Chroot activity status.
    bool          active;
    /// Run chroot setup scripts?
    bool          run_setup_scripts;
    /// Run session setup scripts?
    bool          run_session_scripts;
  };

}

#endif /* SBUILD_CHROOT_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
