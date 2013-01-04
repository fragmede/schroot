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

#ifndef SBUILD_CHROOT_DIRECTORY_H
#define SBUILD_CHROOT_DIRECTORY_H

#include <sbuild/sbuild-config.h>
#include <sbuild/sbuild-chroot-directory-base.h>
#include <sbuild/sbuild-chroot-btrfs-snapshot.h>

namespace sbuild
{

  /**
   * A chroot located in the filesystem.
   *
   * It runs setup scripts and can provide multiple sessions
   * using the union facet.
   */
  class chroot_directory : public chroot_directory_base
  {
  protected:
    /// The constructor.
    chroot_directory ();

    /// The copy constructor.
    chroot_directory (const chroot_directory& rhs);

#ifdef SBUILD_FEATURE_BTRFSSNAP
    /// The copy constructor.
    chroot_directory (const chroot_btrfs_snapshot& rhs);
#endif

    friend class chroot;
#ifdef SBUILD_FEATURE_BTRFSSNAP
    friend class chroot_btrfs_snapshot;
#endif

  public:
    /// The destructor.
    virtual ~chroot_directory ();

    virtual chroot::ptr
    clone () const;

    virtual chroot::ptr
    clone_session (std::string const& session_id,
                   std::string const& alias,
                   std::string const& user,
                   bool               root) const;

    virtual chroot::ptr
    clone_source () const;

    virtual std::string
    get_path () const;

    virtual void
    setup_env (chroot const& chroot,
               environment& env) const;

    virtual std::string const&
    get_chroot_type () const;

    virtual session_flags
    get_session_flags (chroot const& chroot) const;

  protected:
    virtual void
    setup_lock (chroot::setup_type type,
                bool               lock,
                int                status);

    virtual void
    get_details (chroot const& chroot,
                 format_detail& detail) const;

    virtual void
    get_used_keys (string_list& used_keys) const;

    virtual void
    get_keyfile (chroot const& chroot,
                 keyfile& keyfile) const;

    virtual void
    set_keyfile (chroot&        chroot,
                 keyfile const& keyfile);
  };

}

#endif /* SBUILD_CHROOT_DIRECTORY_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
