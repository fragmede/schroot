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

#ifndef SBUILD_CHROOT_FACET_UNSHARE_H
#define SBUILD_CHROOT_FACET_UNSHARE_H

#include <sbuild/sbuild-chroot-facet.h>

namespace sbuild
{

  /**
   * Chroot support for unsharing process execution context
   */
  class chroot_facet_unshare : public chroot_facet
  {
  public:
    /// A shared_ptr to a chroot facet object.
    typedef std::shared_ptr<chroot_facet_unshare> ptr;

    /// A shared_ptr to a const chroot facet object.
    typedef std::shared_ptr<const chroot_facet_unshare> const_ptr;

    /// Error codes.
    enum error_code
      {
	UNSHARE ///< Could not unshare process execution context
      };

    /// Exception type.
    typedef custom_error<error_code> error;

  private:
    /// The constructor.
    chroot_facet_unshare ();

  public:
    /// The destructor.
    virtual ~chroot_facet_unshare ();

    /**
     * Create a chroot facet.
     *
     * @returns a shared_ptr to the new chroot facet.
     */
    static ptr
    create ();

    virtual chroot_facet::ptr
    clone () const;

    virtual std::string const&
    get_name () const;

    /**
     * Is networking unshared?
     *
     * @returns true if unsharing networking, otherwise false.
     */
    bool
    get_unshare_net () const;

    /**
     * Set network unsharing.
     *
     * @param unshare unshare networking?
     */
    void
    set_unshare_net (bool unshare);

    /**
     * Unshare process execution context.
     */
    void
    unshare () const;

    virtual void
    setup_env (chroot const& chroot,
	       environment&  env) const;

    virtual chroot::session_flags
    get_session_flags (chroot const& chroot) const;

    virtual void
    get_details (chroot const&  chroot,
		 format_detail& detail) const;

    virtual void
    get_used_keys (string_list& used_keys) const;

    virtual void
    get_keyfile (chroot const& chroot,
		 keyfile&      keyfile) const;

    virtual void
    set_keyfile (chroot&        chroot,
		 keyfile const& keyfile);

  private:
    /// Unshare networking.
    bool unshare_net;
  };

}

#endif /* SBUILD_CHROOT_FACET_UNSHARE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
