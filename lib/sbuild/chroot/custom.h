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

#ifndef SBUILD_CHROOT_CUSTOM_H
#define SBUILD_CHROOT_CUSTOM_H

#include <sbuild/chroot/chroot.h>

namespace sbuild
{
  namespace chroot
  {

    /**
     * A chroot stored with custom parameters.
     *
     * This chroot specifies no behaviour or policy.  It is entirely
     * configured using user options and setup scripts.  The intent is
     * to permit the prototyping of and experimentation with new chroot
     * types without requiring a "full" class definition and associated
     * infrastructural work.  It also makes schroot extensible without
     * requiring any C++ coding.
     */
    class custom : public chroot
    {
    protected:
      /// The constructor.
      custom ();

      /// The copy constructor.
      custom (const custom& rhs);

      friend class chroot;

    public:
      /// The destructor.
      virtual ~custom ();

      virtual chroot::ptr
      clone () const;

      virtual chroot::ptr
      clone_session (std::string const& session_id,
                     std::string const& alias,
                     std::string const& user,
                     bool               root) const;
    };

  }
}

#endif /* SBUILD_CHROOT_CUSTOM_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
