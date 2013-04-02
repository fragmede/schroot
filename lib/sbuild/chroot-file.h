/* Copyright © 2005-2013  Roger Leigh <rleigh@debian.org>
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

#include <sbuild/chroot.h>

namespace sbuild
{

  /**
   * A chroot stored in a file archive (tar with optional compression).
   *
   * The archive will be unpacked and repacked on demand.
   */
  class chroot_file : public chroot
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
    clone_session (std::string const& session_id,
                   std::string const& alias,
                   std::string const& user,
                   bool               root) const;

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
     * Get the location.  This is a path to the chroot directory
     * inside the archive (absolute path from the archive root).
     *
     * @returns the location.
     */
    virtual std::string const&
    get_location () const;

    /**
     * Set the location.  This is a path to the chroot directory
     * inside the archive (absolute path from the archive root).
     *
     * @param location the location.
     */
    virtual void
    set_location (std::string const& location);

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
    setup_env (chroot const& chroot,
               environment& env) const;

    std::string
    get_path () const;

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
    get_used_keys (string_list& used_keys) const;

    virtual void
    get_keyfile (chroot const& chroot,
                 keyfile&      keyfile) const;

    virtual void
    set_keyfile (chroot&        chroot,
                 keyfile const& keyfile);

  private:
    /// The file to use.
    std::string file;
    /// Location inside the mount location root.
    std::string location;
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
