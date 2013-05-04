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

#include <sbuild/chroot/chroot.h>

namespace sbuild
{
  namespace chroot
  {

    /**
     * A chroot stored in a file archive (tar with optional compression).
     *
     * The archive will be unpacked and repacked on demand.
     */
    class file : public chroot
    {
    protected:
      /// The constructor.
      file ();

      /// The copy constructor.
      file (const file& rhs);

      friend class chroot;

    public:
      /// The destructor.
      virtual ~file ();

      virtual chroot::chroot::ptr
      clone () const;
    };

  }
}

#endif /* SBUILD_CHROOT_FILE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
