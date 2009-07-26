/* Copyright © 2005-2009  Roger Leigh <rleigh@debian.org>
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

#ifndef SBUILD_CHROOT_SESSION_H
#define SBUILD_CHROOT_SESSION_H

#include <sbuild/sbuild-chroot.h>

namespace sbuild
{

  /**
   * A chroot may offer a "session" chroot in addition to its inactive
   * copy.  This interface may be implemented by any chroot wishing to
   * provide such functionality.
   *
   * While this is effectively an interface, in practice this derives
   * from sbuild::chroot, to allow setting and getting of data from a
   * keyfile, including storing the keyfile options.
   *
   * Chroot types implementing chroot_session should, at a minimum,
   * implement clone_session().  This should create and return a session
   * chroot, and must call clone_session_setup() to set up the session
   * chroot.
   */
  class chroot_session
  {
  protected:
    /// The constructor.
    chroot_session ();

    friend class chroot;

  public:
    /// The destructor.
    virtual ~chroot_session ();

    /**
     * Create a session chroot.
     *
     * @param session_id the identifier for the new session.
     * @returns a session chroot.
     */
    virtual chroot::ptr
    clone_session (std::string const& session_id) const = 0;

  protected:
    /**
     * Set the defaults in the cloned session chroot.
     *
     * @param clone the chroot to set up.
     * @param session_id the identifier for the new session.
     */
    virtual void
    clone_session_setup (chroot::ptr&       clone,
			 std::string const& session_id) const;

  public:
    /**
     * Get if the chroot is a session manageable chroot or not.
     *
     * @returns true if the chroot is a session manageable chroot,
     * otherwise false.
     */
    virtual bool
    get_session_manageable () const;

    /**
     * Set if the chroot is a session manageable chroot or not.
     *
     * @param manageable true if a session chroot, manageable or false if
     * not.
     */
    virtual void
    set_session_manageable (bool manageable);

    /**
     * Get if the chroot is an active session or not.
     *
     * @returns true if the chroot is an active session, otherwise false.
     */
    virtual bool
    get_session_active () const;

    /**
     * Set if the chroot is an active session or not.
     *
     * @param active true if an active session, or false if not.
     */
    virtual void
    set_session_active (bool active);

    virtual void
    setup_env (chroot const& chroot,
	       environment& env) const;

  protected:
    virtual chroot::session_flags
    get_session_flags (chroot const& chroot) const;

    virtual void
    get_details (chroot const& chroot,
		 format_detail& detail) const;

    void
    get_keyfile (chroot const& chroot,
		 keyfile& keyfile) const;

    void
    set_keyfile (chroot&        chroot,
		 keyfile const& keyfile,
		 string_list&   used_keys);

  private:
    /// Is the chroot session or clone?
    bool session_manageable;
    /// Is the session active?
    bool session_active;
  };

}

#endif /* SBUILD_CHROOT_SESSION_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
