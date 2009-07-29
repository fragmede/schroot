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

#ifndef SBUILD_CHROOT_FACET_SESSION_H
#define SBUILD_CHROOT_FACET_SESSION_H

#include <sbuild/sbuild-chroot-facet.h>
#include <sbuild/sbuild-session.h>

namespace sbuild
{

  /**
   * Chroot support for kernel personalities (execution domains).
   */
  class chroot_facet_session : public chroot_facet
  {
  public:
    /// A shared_ptr to a chroot facet object.
    typedef std::tr1::shared_ptr<chroot_facet_session> ptr;

    /// A shared_ptr to a const chroot facet object.
    typedef std::tr1::shared_ptr<const chroot_facet_session> const_ptr;

  private:
    /// The constructor.
    chroot_facet_session ();

  public:
    /// The destructor.
    virtual ~chroot_facet_session ();

    /**
     * Create a chroot facet.
     *
     * @returns a shared_ptr to the new chroot facet.
     */
    static ptr
    create ();

    virtual chroot_facet::ptr
    clone () const;

    /**
     * Set the defaults in the cloned session chroot.
     *
     * @param clone the chroot to set up.
     * @param session_id the identifier for the new session.
     */
    virtual void
    clone_session_setup (chroot::ptr&       clone,
			 std::string const& session_id) const;

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
	       environment&  env) const;

    virtual chroot::session_flags
    get_session_flags (chroot const& chroot) const;

    virtual void
    get_details (chroot const&  chroot,
		 format_detail& detail) const;

    virtual void
    get_keyfile (chroot const& chroot,
		 keyfile&      keyfile) const;

    virtual void
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

#endif /* SBUILD_CHROOT_FACET_SESSION_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
