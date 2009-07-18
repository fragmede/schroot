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

#ifndef SBUILD_CHROOT_FILE_H
#define SBUILD_CHROOT_FILE_H

#include <sbuild/sbuild-chroot.h>
#include <sbuild/sbuild-chroot-session.h>
#include <sbuild/sbuild-chroot-source.h>

namespace sbuild
{

  /**
   * A chroot stored in a file archive (tar or zip).  The archive will
   * be unpacked on demand.
   */
  class chroot_file : public chroot,
		      virtual public chroot_session,
		      public chroot_source
  {
  protected:
    /// The constructor.
    chroot_file ();

    /// The copy constructor.
    chroot_file (const chroot_file& rhs);

    friend class chroot;

  public:
    /// The destructor.
    virtual ~chroot_file ();

    virtual chroot::ptr
    clone () const;

    virtual chroot::ptr
    clone_session (std::string const& session_id) const;

    virtual chroot::ptr
    clone_source () const;

    /**
     * Get the file used by the chroot.
     *
     * @returns the file.
     */
    std::string const&
    get_file () const;

    /**
     * Set the file used by the chroot.
     *
     * @param file the file.
     */
    void
    set_file (std::string const& file);

    /**
     * Get the repack status.  This is true if the unpacked archive
     * file will be repacked.
     *
     * @returns the repack status.
     */
    bool
    get_file_repack () const;

    /**
     * Set the file repack status.  Set to true if the unpacked
     * archive file will be repacked on session cleanup, or false to
     * discard.
     *
     * @param repack the repack status.
     */
    void
    set_file_repack (bool repack);

    virtual std::string const&
    get_chroot_type () const;

    virtual void
    setup_env (environment& env);

    std::string
    get_path () const;

    virtual session_flags
    get_session_flags () const;

  protected:
    virtual void
    setup_lock (chroot::setup_type type,
		bool               lock,
		int                status);

    virtual void
    get_details (format_detail& detail) const;

    virtual void
    get_keyfile (keyfile& keyfile) const;

    virtual void
    set_keyfile (keyfile const& keyfile,
		 string_list&   used_keys);

  private:
    /// The file to use.
    std::string file;
    /// Should the chroot be repacked?
    bool repack;
  };

}

#endif /* SBUILD_CHROOT_FILE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
