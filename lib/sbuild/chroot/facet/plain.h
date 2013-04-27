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

#ifndef SBUILD_CHROOT_PLAIN_H
#define SBUILD_CHROOT_PLAIN_H

#include <sbuild/chroot/directory-base.h>

namespace sbuild
{
  namespace chroot
  {

    /**
     * A chroot located in the filesystem (scripts disabled).
     *
     * This doesn't run any setup scripts and doesn't provide any
     * session support.  If you need any of these functions, the
     * directory chroot type is more suited to your needs.
     */
    class plain : public directory_base
    {
    protected:
      /// The constructor.
      plain ();

      friend class chroot;

    public:
      /// The destructor.
      virtual ~plain ();

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

      virtual std::string const&
      get_chroot_type () const;

      virtual session_flags
      get_session_flags (chroot const& chroot) const;

    protected:
      virtual void
      setup_lock (chroot::setup_type type,
                  bool               lock,
                  int                status);
    };

  }
}

#endif /* SBUILD_CHROOT_PLAIN_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
