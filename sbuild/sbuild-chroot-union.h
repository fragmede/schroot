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

#ifndef SBUILD_CHROOT_UNION_H
#define SBUILD_CHROOT_UNION_H

#include <sbuild/sbuild-chroot-source.h>

namespace sbuild
{
  /**
   * A chroot may offer session support using a filesystem union like
   * aufs or unionfs.  This is an extension interface class, not a
   * chroot type in its own right.
   *
   * At a minimum, the inheriting class has to implement the
   * chroot_source::clone_source() function, depending upon the
   * setting of get_union_configured().
   */
  class chroot_union : public chroot_source
  {
  public:
    /// Error codes.
    enum error_code
      {
	UNION_TYPE_UNKNOWN      ///< Unknown filesystem union type
      };

    /// Exception type.
    typedef custom_error<error_code> error;

  protected:
    /// The constructor.
    chroot_union ();

    friend class chroot;

  public:
    /// The destructor.
    virtual ~chroot_union ();

  protected:
    virtual void
    clone_source_setup (chroot::ptr& clone) const;

  public:
    /**
     * Get fs union configured state.
     *
     * @returns if fs union is configured
     */
    bool
    get_union_configured () const;

    /**
     * Get the filesystem union type.
     *
     * @see set_union_type
     * @returns the union filesytem type.
     */
    virtual std::string const&
    get_union_type () const;

    /**
     * Set the filesystem union type.
     *
     * Currently supported values are aufs, unionfs and none.
     *
     * @param union_type the filesystem type.
     **/
    virtual void
    set_union_type (std::string const& union_type);

    /**
     * Get the filesystem union mount options (branch configuration).
     *
     * @see set_union_mount_options
     * @returns the filesystem union branch configuration.
     */
    virtual std::string const&
    get_union_mount_options () const;

    /**
     * Set the filesystem union mount options (branch configuration).
     *
     * Normally a temporary directory is used as the writeable branch,
     * which is removed on session end. This allows the building of a
     * complex union which can merge multiple branches. The string has
     * to be constructed as expected by the filesystem union type and
     * is directly used as the mount '-o' option string.
     *
     * @param union_mount_options a @fs_type specific branch description
     **/
    virtual void
    set_union_mount_options (std::string const& union_mount_options);

    /**
     * Get the union overlay directory.
     *
     * @returns the writeable overlay directory.
     */
    virtual std::string const&
    get_union_overlay_directory () const;

    /**
     * Set the union overlay directory.
     *
     * @returns the writeable overlay directory.
     */
    virtual void
    set_union_overlay_directory (std::string const& directory);

    /**
     * Get the union underlay directory.
     *
     * @returns the writeable underlay directory.
     */
    virtual std::string const&
    get_union_underlay_directory () const;

    /**
     * Set the union underlay directory.
     *
     * @returns the writeable underlay directory.
     */
    virtual void
    set_union_underlay_directory (std::string const& directory);

    virtual void
    setup_env (environment& env);

    virtual chroot::session_flags
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
    /// filesystem union type.
    std::string union_type;
    /// Union mount options (branch configuration).
    std::string union_mount_options;
    /// Union read-write overlay directory.
    std::string union_overlay_directory;
    /// Union read-only underlay directory.
    std::string union_underlay_directory;
  };
}

#endif /* SBUILD_CHROOT_UNION_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
