/* Copyright © 2005-2008  Roger Leigh <rleigh@debian.org>
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

#ifndef SBUILD_CHROOT_SOURCE_H
#define SBUILD_CHROOT_SOURCE_H

#include <sbuild/sbuild-chroot.h>

namespace sbuild
{

  /**
   * A chroot may offer a "source" chroot in addition to its normal
   * "session" copy, to allow for maintenence of the source data.
   * This interface may be implemented by any chroot wishing to
   * provide such functionality.
   *
   * While this is effectively an interface, in practice this derives
   * from sbuild::chroot, to allow setting and getting of data from a
   * keyfile, including storing the keyfile options.
   *
   * Chroot types implementing chroot_source should, at a minimum,
   * implement clone_source().  This should create and return a source
   * chroot, and must call clone_source_setup() to set up the source
   * chroot.
   */
  class chroot_source
  {
  protected:
    /// The constructor.
    chroot_source ();

    friend class chroot;

  public:
    /// The destructor.
    virtual ~chroot_source ();

    /**
     * Create a source chroot.
     *
     * @returns a source chroot.
     */
    virtual chroot::ptr
    clone_source () const = 0;

  protected:
    /**
     * Set the defaults in the cloned source chroot.
     *
     * @param clone the chroot to set up.
     */
    virtual void
    clone_source_setup (chroot::ptr& clone) const;

  public:
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

    /**
     * Get if the chroot is a clonable source chroot or not.
     *
     * @returns true if the chroot is a source chroot, otherwise false.
     */
    virtual bool
    get_source_clonable () const;

    /**
     * Set if the chroot is a clonable source chroot or not.
     *
     * @param source true if a source chroot, or false if not.
     */
    virtual void
    set_source_clonable (bool clonable);

    void
    setup_env (environment& env);

  protected:
    virtual chroot::session_flags
    get_session_flags () const;

    virtual void
    get_details (format_detail& detail) const;

    void
    get_keyfile (keyfile& keyfile) const;

    void
    set_keyfile (keyfile const& keyfile,
		 string_list&   used_keys);

  private:
    /// Is the chroot source or clone?
    bool          source_clonable;
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

#endif /* SBUILD_CHROOT_SOURCE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
