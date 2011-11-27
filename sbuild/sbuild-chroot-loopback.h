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

#ifndef SBUILD_CHROOT_LOOPBACK_H
#define SBUILD_CHROOT_LOOPBACK_H

#include <sbuild/sbuild-config.h>
#include <sbuild/sbuild-chroot.h>

namespace sbuild
{

  /**
   * A chroot stored in a file for loopback mounting.
   *
   * The file will be mounted on demand.
   */
  class chroot_loopback : public chroot
  {
  public:
    /// Exception type.
    typedef chroot::error error;

  protected:
    /// The constructor.
    chroot_loopback ();

    /// The copy constructor.
    chroot_loopback (const chroot_loopback& rhs);

    friend class chroot;

  public:
    /// The destructor.
    virtual ~chroot_loopback ();

    virtual chroot::ptr
    clone () const;

    virtual chroot::ptr
    clone_session (std::string const& session_id,
		   std::string const& alias,
		   std::string const& user,
		   bool               root) const;

    virtual chroot::ptr
    clone_source () const;

    /**
     * Get the file containing the chroot.
     *
     * @returns the file.
     */
    std::string const&
    get_file () const;

    /**
     * Set the file containing the chroot.
     *
     * @param file the file.
     */
    void
    set_file (std::string const& file);

    std::string const&
    get_chroot_type () const;

    virtual std::string
    get_path () const;

    virtual void
    setup_env (chroot const& chroot,
	       environment&  env) const;

    virtual session_flags
    get_session_flags (chroot const& chroot) const;

  protected:
    virtual void
    setup_lock (chroot::setup_type type,
		bool               lock,
		int                status);

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
    /// The file to use.
    std::string file;
  };

}

#endif /* SBUILD_CHROOT_LOOPBACK_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
