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

#ifndef SBUILD_CHROOT_BLOCK_DEVICE_H
#define SBUILD_CHROOT_BLOCK_DEVICE_H

#include <sbuild/config.h>
#include <sbuild/chroot/chroot.h>
#include <sbuild/chroot/lvm-snapshot.h>

namespace sbuild
{
  namespace chroot
  {

    /**
     * A chroot stored on an unmounted block device.
     *
     * The device will be mounted on demand.
     */
    class block_device : public chroot
    {
    public:
      /// Exception type.
      typedef chroot::error error;

    protected:
      /// The constructor.
      block_device ();

      /// The copy constructor.
      block_device (const block_device& rhs);

#ifdef SBUILD_FEATURE_LVMSNAP
      /// The copy constructor.
      block_device (const lvm_snapshot& rhs);
#endif

      friend class chroot;
#ifdef SBUILD_FEATURE_LVMSNAP
      friend class lvm_snapshot;
#endif

    public:
      /// The destructor.
      virtual ~block_device ();
    };

  }
}

#endif /* SBUILD_CHROOT_BLOCK_DEVICE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
