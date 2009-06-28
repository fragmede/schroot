/* Copyright © 2008-2009  Jan-Marek Glogowski <glogow@fbihome.de>
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

#ifndef SBUILD_CHROOT_FS_UNION_H
#define SBUILD_CHROOT_FS_UNION_H

#include <sbuild/sbuild-chroot-source.h>

namespace sbuild
{
  /**
   * A chroot may offer session support using a union filesystem like
   * aufs or unionfs.
   *
   * This interface can be implemented by any chroot wishing to provide such
   * functionality.  It already includes sbuild::chroot_source support.
   *
   * At a minimum, the inheriting class has to implement the
   * chroot_source::clone_source() function, depending on the
   * setting of get_fs_union_configured().
   */
  class chroot_fs_union : public chroot_source
  {
  protected:
    /// The constructor.
    chroot_fs_union ();

    friend class chroot;

  public:
    /// The destructor.
    virtual ~chroot_fs_union ();

    /// Error codes.
    enum error_code
      {
	FS_TYPE_UNKNOWN      ///< Unknown union fs type
      };

    /// Exception type.
    typedef custom_error<error_code> error;

    /**
     * Get fs union configured state.
     *
     * @returns if fs union is configured
     */
    bool
    get_fs_union_configured () const;

    /**
     * Get the union filesystem type.
     *
     * @see set_fs_union_type
     * @returns the union filesytem type.
     */
    virtual std::string const&
    get_fs_union_type () const;

    /**
     * Get the union filesystem branch configuration.
     *
     * @see set_fs_union_branch_config
     * @returns the union filesystem branch configuration.
     */
    virtual std::string const&
    get_fs_union_branch_config () const;

    /**
     * Get the writeable overlay directory of the session.
     *
     * @see set_overlay_session_directory
     * @returns the writeable overlay directory.
     */
    virtual std::string const&
    get_overlay_session_directory () const;

    /**
     * Set the union filesystem type.
     *
     * Currently supported values are aufs, unionfs and none.
     *
     * @param fs_union_type the filesystem type.
     **/
    virtual void
    set_fs_union_type (std::string const& fs_union_type);

    /**
     * Set a complex branch configuration.
     *
     * Normally a temporary directory is used as the writeable branch, which 
     * is removed on session end. This allows to build a complex union chroot 
     * which can merge multiple branches. The string has to be constructed as 
     * expected by the union filesystem type and is directly used as the mount
     * '-o' option string.
     *
     * @param fs_union_branch_config a @fs_type specific branch description
     **/
    virtual void
    set_fs_union_branch_config (std::string const& fs_union_branch_config);

    /**
     * Allows schrooot to set the session dependant overlay directory.
     *
     * @param the absolute path of the overlay_session_directory.
     **/
    virtual void
    set_overlay_session_directory 
    (std::string const& overlay_session_directory);

    virtual std::string
    get_path () const;

    virtual void
    setup_env (environment& env);

    virtual session_flags
    get_session_flags () const;

  protected:
    virtual void
    get_details (format_detail& detail) const;

    virtual void
    get_keyfile (keyfile& keyfile) const;

    virtual void
    set_keyfile (keyfile const& keyfile,
                 string_list&   used_keys);

  private:
    /// Union filesystem type
    std::string fs_union_type;
    /// Complex branch configuration to pass directly as mount option
    std::string fs_union_branch_config;
    /// Writeable directory used as the overlay write branch
    std::string overlay_session_directory;
  };
}

#endif /* SBUILD_CHROOT_FS_UNION_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
