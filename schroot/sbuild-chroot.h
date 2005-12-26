/* sbuild-chroot - sbuild chroot object
 *
 * Copyright © 2005  Roger Leigh <rleigh@debian.org>
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
#include <sstream>
#include <vector>

#include "sbuild-error.h"
#include "sbuild-keyfile.h"
#include "sbuild-util.h"

namespace sbuild
{

  class Chroot
  {
  public:
    enum SetupType
      {
	SETUP_START,
	SETUP_RECOVER,
	SETUP_STOP,
	RUN_START,
	RUN_STOP
      };

    enum SessionFlags
      {
	SESSION_CREATE = 1 << 0
      };

    typedef runtime_error_custom<Chroot> error;

    Chroot ();

    Chroot (const keyfile&     keyfile,
	    const std::string& group);

    virtual ~Chroot();

    virtual Chroot *
    clone () const = 0;

    const std::string&
    get_name () const;

    void
    set_name (const std::string& name);

    const std::string&
    get_description () const;

    void
    set_description (const std::string& description);

    virtual const std::string&
    get_mount_location () const;

    void
    set_mount_location (const std::string& location);

    virtual const std::string&
    get_mount_device () const;

    void
    set_mount_device (const std::string& device);

    unsigned int
    get_priority () const;

    void
    set_priority (unsigned int priority);

    const string_list&
    get_groups () const;

    void
    set_groups (const string_list& groups);

    const string_list&
    get_root_groups () const;

    void
    set_root_groups (const string_list& groups);

    const string_list&
    get_aliases () const;

    void
    set_aliases (const string_list& aliases);

    bool
    get_active () const;

    void
    set_active (bool active);

    bool
    get_run_setup_scripts () const;

    void
    set_run_setup_scripts (bool run_setup_scripts);

    bool
    get_run_session_scripts () const;

    void
    set_run_session_scripts (bool run_session_scripts);

    virtual const std::string&
    get_chroot_type () const = 0;

    virtual void
    setup_env (env_list& env);

    /**
     * sbuild_chroot_setup_lock:
     * @chroot: an #Chroot
     * @type: the type of setup being performed
     * @lock: TRUE to lock, FALSE to unlock
     * @error: a #GError.
     *
     * Lock a chroot during setup.  The locking technique (if any) may
     * vary depending upon the chroot type and setup stage.  For example,
     * during creation a block device might require locking, but
     * afterwards this will change to the new block device or directory.
     *
     * Returns TRUE on success, FALSE on failure.
     */
    virtual void
    setup_lock (SetupType type,
		bool      lock) = 0;

    /**
     * sbuild_chroot_get_session_flags:
     * @chroot: an #Chroot
     *
     * Get the session flags of the chroot.  These determine how the
     * #SbuildSession controlling the chroot will operate.
     *
     * Returns the #SessionFlags.
     */
    virtual SessionFlags
    get_session_flags () const = 0;

    virtual void
    print_details (std::ostream& stream) const;

    virtual void
    print_config (std::ostream& stream) const;

  protected:
    void
    read_keyfile (const keyfile&     keyfile,
		  const std::string& group);

    template<typename T>
    class format_detail
    {
    public:
      format_detail(std::string const& name,
		    T const&           value):
	name(name),
	value(value)
      {}

      friend std::ostream& operator << (std::ostream& stream,
					const format_detail<T>& rhs)
      {
	return stream << "  " << std::setw(22) << rhs.name
		      << rhs.value << '\n';
      }

      friend std::ostream& operator << (std::ostream& stream,
					const format_detail<bool>& rhs)
      {
	const char *desc = 0;
	if (rhs.value)
	  desc =  _("true");
	else
	  desc = _("false");
	return stream << format_detail<std::string>(rhs.name, desc);
      }

      friend std::ostream& operator << (std::ostream& stream,
					const format_detail<string_list>& rhs)
      {
	return stream <<
	  format_detail<std::string>(rhs.name,
				     string_list_to_string(rhs.value, " "));
      }

    private:
      std::string const& name;
      T const&           value;
    };

    typedef format_detail<std::string> format_detail_string;
    typedef format_detail<int> format_detail_int;
    typedef format_detail<bool> format_detail_bool;
    typedef format_detail<string_list> format_detail_strv;

  private:
    std::string   name;
    std::string   description;
    unsigned int  priority;
    string_list   groups;
    string_list   root_groups;
    string_list   aliases;
    std::string   mount_location;
    std::string   mount_device;
    bool          active;
    bool          run_setup_scripts;
    bool          run_session_scripts;
  };

  // TODO: move into separate environment class, and drop sstream
  // header.
  template<typename T>
  void
  setup_env_var(env_list&          env,
		const std::string& name,
		const T&           var)
  {
    std::ostringstream varstring;
    varstring << var;
    env.push_back(std::make_pair(name, varstring.str()));
  }

}

#endif /* SBUILD_CHROOT_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
