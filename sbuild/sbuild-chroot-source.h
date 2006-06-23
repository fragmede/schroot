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
  class chroot_source : virtual public chroot
  {
  protected:
    /// The constructor.
    chroot_source ();

    friend class chroot;

  public:
    /// The destructor.
    virtual ~chroot_source ();

    virtual chroot::ptr
    clone_source () const = 0;

  protected:
    /**
     * Set the defaults in the cloned source chroot.
     *
     * @param clone the chroot to set up.
     */
    void
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
     * Mmebers of these users can switch to root without
     * authenticating themselves.
     *
     * @returns a list of users.
     */
    virtual string_list const&
    get_source_root_users () const;

    /**
     * Set the users allowed to access the source chroot as root.
     * Mmebers of these users can switch to root without
     * authenticating themselves.
     *
     * @param users a list of users.
     */
    virtual void
    set_source_root_users (string_list const& users);

    /**
     * Get the groups allowed to access the source chroot as root.
     * Mmebers of these groups can switch to root without
     * authenticating themselves.
     *
     * @returns a list of groups.
     */
    virtual string_list const&
    get_source_root_groups () const;

    /**
     * Set the groups allowed to access the source chroot as root.
     * Mmebers of these groups can switch to root without
     * authenticating themselves.
     *
     * @param groups a list of groups.
     */
    virtual void
    set_source_root_groups (string_list const& groups);

    void
    setup_env (environment& env);

  protected:
    void
    print_details (std::ostream& stream) const;

    void
    get_keyfile (keyfile& keyfile) const;

    void
    set_keyfile (keyfile const& keyfile);

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

#endif /* SBUILD_CHROOT_SOURCE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
