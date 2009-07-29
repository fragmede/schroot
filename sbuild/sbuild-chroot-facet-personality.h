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

#ifndef SBUILD_CHROOT_FACET_PERSONALITY_H
#define SBUILD_CHROOT_FACET_PERSONALITY_H

#include <sbuild/sbuild-chroot-facet.h>
#include <sbuild/sbuild-personality.h>

namespace sbuild
{

  /**
   * Chroot support for kernel personalities (execution domains).
   */
  class chroot_facet_personality : public chroot_facet
  {
  public:
    /// A shared_ptr to a chroot facet object.
    typedef std::tr1::shared_ptr<chroot_facet_personality> ptr;

    /// A shared_ptr to a const chroot facet object.
    typedef std::tr1::shared_ptr<const chroot_facet_personality> const_ptr;

  private:
    /// The constructor.
    chroot_facet_personality ();

  public:
    /// The destructor.
    virtual ~chroot_facet_personality ();

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
    /// Process execution domain (Linux only).
    personality   persona;
  };

}

#endif /* SBUILD_CHROOT_FACET_PERSONALITY_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
