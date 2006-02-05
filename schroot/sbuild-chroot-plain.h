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

#ifndef SBUILD_CHROOT_PLAIN_H
#define SBUILD_CHROOT_PLAIN_H

#include "sbuild-chroot.h"

namespace sbuild
{

  /**
   * A chroot located on a mounted filesystem.
   */
  class ChrootPlain : public chroot
  {
  protected:
    /// The constructor.
    ChrootPlain();

    /**
     * The constructor.  Initialise from an open keyfile.
     *
     * @param keyfile the configuration file
     * @param group the keyfile group (chroot name)
     */
    ChrootPlain (keyfile const&     keyfile,
		 std::string const& group);

    friend class chroot;

  public:
    /// The destructor.
    virtual ~ChrootPlain();

    virtual chroot::chroot_ptr
    clone () const;

    /**
     * Get the directory location of the chroot.
     *
     * @returns the location.
     */
    std::string const&
    get_location () const;

    /**
     * Set the directory location of the chroot.
     *
     * @param location the location.
     */
    void
    set_location (std::string const& location);

    virtual std::string const&
    get_mount_location () const;

    virtual std::string const&
    get_chroot_type () const;

    virtual void
    setup_env (environment& env);

    virtual void
    setup_lock (SetupType type,
		bool      lock);

    virtual SessionFlags
    get_session_flags () const;

  protected:
    virtual void
    print_details (std::ostream& stream) const;

    virtual void
    get_keyfile (keyfile& keyfile) const;

    virtual void
    set_keyfile (keyfile const& keyfile);

  private:
    /// The directory location of the chroot.
    std::string location;
  };

}

#endif /* SBUILD_CHROOT_PLAIN_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
