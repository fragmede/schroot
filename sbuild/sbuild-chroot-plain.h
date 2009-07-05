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

#ifndef SBUILD_CHROOT_PLAIN_H
#define SBUILD_CHROOT_PLAIN_H

#include <sbuild/sbuild-chroot-directory-base.h>

namespace sbuild
{

  /**
   * A chroot located in the filesystem (mounts disabled).
   */
  class chroot_plain : virtual public chroot_directory_base
  {
  protected:
    /// The constructor.
    chroot_plain ();

    friend class chroot;

  public:
    /// The destructor.
    virtual ~chroot_plain ();

    virtual chroot::ptr
    clone () const;

    virtual std::string
    get_path () const;

    virtual std::string const&
    get_chroot_type () const;

    virtual session_flags
    get_session_flags () const;

  protected:
    virtual void
    setup_lock (chroot::setup_type type,
		bool               lock,
		int                status);
  };

}

#endif /* SBUILD_CHROOT_PLAIN_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
