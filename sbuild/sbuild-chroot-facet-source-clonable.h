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

#ifndef SBUILD_CHROOT_FACET_SOURCE_CLONABLE_H
#define SBUILD_CHROOT_FACET_SOURCE_CLONABLE_H

#include <sbuild/sbuild-chroot-facet.h>

namespace sbuild
{

  /**
   * Chroot support for creation of source chroots.
   *
   * A chroot may offer a "source" chroot in addition to its typical
   * session clone, to allow for maintenence of the source data.  This
   * facet can be used by any chroot wishing to provide such
   * functionality.
   */
  class chroot_facet_source_clonable : public chroot_facet
  {
  public:
    /// A shared_ptr to a chroot facet object.
    typedef std::tr1::shared_ptr<chroot_facet_source_clonable> ptr;

    /// A shared_ptr to a const chroot facet object.
    typedef std::tr1::shared_ptr<const chroot_facet_source_clonable> const_ptr;

  private:
    /// The constructor.
    chroot_facet_source_clonable ();

  public:
    /// The destructor.
    virtual ~chroot_facet_source_clonable ();

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
     * Set the defaults in the cloned sourceg chroot.
     *
     * @param clone the chroot to set up.
     */
    virtual void
    clone_source_setup (chroot::ptr& clone) const;

    /**
     * Get the users allowed to access the source chroot.
     *
     * @returns a list of users.
     */
    virtual string_list const&
    get_source_users () const;

    /**
     * Set the users allowed to access the source chroot.
     *
     * @param users a list of users.
     */
    virtual void
    set_source_users (string_list const& users);

    /**
     * Get the groups allowed to access the source chroot.
     *
     * @returns a list of groups.
     */
    virtual string_list const&
    get_source_groups () const;

    /**
     * Set the groups allowed to access the source chroot.
     *
     * @param groups a list of groups.
     */
    virtual void
    set_source_groups (string_list const& groups);

    /**
     * Get the users allowed to access the source chroot as root.
     * Members of these users can switch to root without
     * authenticating themselves.
     *
     * @returns a list of users.
     */
    virtual string_list const&
    get_source_root_users () const;

    /**
     * Set the users allowed to access the source chroot as root.
     * Members of these users can switch to root without
     * authenticating themselves.
     *
     * @param users a list of users.
     */
    virtual void
    set_source_root_users (string_list const& users);

    /**
     * Get the groups allowed to access the source chroot as root.
     * Members of these groups can switch to root without
     * authenticating themselves.
     *
     * @returns a list of groups.
     */
    virtual string_list const&
    get_source_root_groups () const;

    /**
     * Set the groups allowed to access the source chroot as root.
     * Members of these groups can switch to root without
     * authenticating themselves.
     *
     * @param groups a list of groups.
     */
    virtual void
    set_source_root_groups (string_list const& groups);

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
    /// Users allowed to access the source chroot.
    string_list   source_users;
    /// Groups allowed to access the source chroot.
    string_list   source_groups;
    /// Users allowed to access the source chroot as root.
    string_list   source_root_users;
    /// Groups allowed to access the source chroot as root.
    string_list   source_root_groups;
  };

}

#endif /* SBUILD_CHROOT_FACET_SOURCE_CLONABLE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
