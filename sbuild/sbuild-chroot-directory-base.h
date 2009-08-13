/* Copyright © 2005-2007  Roger Leigh <rleigh@debian.org>
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

#ifndef SBUILD_CHROOT_DIRECTORY_BASE_H
#define SBUILD_CHROOT_DIRECTORY_BASE_H

#include <sbuild/sbuild-chroot.h>

namespace sbuild
{

  /**
   * A base class for chroots located in a local directory.
   *
   * This class doesn't implement a chroot (get_chroot_type is not
   * implemented).  plain and directory chroots inherit from this
   * class.
   *
   * Originally plain inherited from the directory chroot, but this
   * had to be changed when union support was introduced.  As plain
   * chroots don't run any setup scripts and basically just call
   * 'chroot' on a directory, they can't support union based sessions.
   */
  class chroot_directory_base : public chroot
  {
  protected:
    /// The constructor.
    chroot_directory_base ();

    friend class chroot;

  public:
    /// The destructor.
    virtual ~chroot_directory_base ();

    /**
     * Get the directory containing the chroot.
     *
     * @returns the location.
     */
    std::string const&
    get_directory () const;

    /**
     * Set the directory containing the chroot.
     *
     * @param directory the directory.
     */
    void
    set_directory (std::string const& directory);

    virtual void
    setup_env (chroot const& chroot,
	       environment& env) const;

  protected:
    virtual void
    get_details (chroot const& chroot,
		 format_detail& detail) const;

    virtual void
    get_keyfile (chroot const& chroot,
		 keyfile& keyfile) const;

    virtual void
    set_keyfile (chroot&        chroot,
		 keyfile const& keyfile,
		 string_list&   used_keys);

  private:
    /// The directory to use.
    std::string directory;
  };

}

#endif /* SBUILD_CHROOT_DIRECTORY_BASE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
