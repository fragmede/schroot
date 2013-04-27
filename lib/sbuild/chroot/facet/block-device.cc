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

#include <config.h>

#include <sbuild/chroot/facet/block-device.h>
#include <sbuild/chroot/facet/lvm-snapshot.h>
#include <sbuild/chroot/facet/session.h>
#include <sbuild/chroot/facet/session-clonable.h>
#include <sbuild/chroot/facet/source-clonable.h>
#ifdef SBUILD_FEATURE_UNION
#include <sbuild/chroot/facet/fsunion.h>
#endif // SBUILD_FEATURE_UNION
#include "format-detail.h"
#include "util.h"

#include <cassert>
#include <cerrno>
#include <cstring>

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

namespace sbuild
{
  namespace chroot
  {
    namespace facet
    {

      block_device::block_device ():
        block_device_base()
      {
      }

      block_device::~block_device ()
      {
      }

      block_device::block_device (const block_device& rhs):
        block_device_base(rhs)
      {
      }

#ifdef SBUILD_FEATURE_LVMSNAP
      block_device::block_device (const lvm_snapshot& rhs):
        block_device_base(rhs)
      {
      }
#endif // SBUILD_FEATURE_LVMSNAP

      void
      block_device::set_chroot (chroot& chroot)
      {
        block_device_base::set_chroot(chroot);
#ifdef SBUILD_FEATURE_UNION
        if (!owner->get_facet<fsunion>())
          owner->add_facet(fsunion::create());
#endif // SBUILD_FEATURE_UNION
      }

      std::string const&
      block_device::get_name () const
      {
        static const std::string name("block-device");

        return name;
      }

      block_device::ptr
      block_device::create ()
      {
        return ptr(new block_device());
      }

#ifdef SBUILD_FEATURE_LVMSNAP
      block_device::ptr
      block_device::create (const lvm_snapshot& rhs)
      {
        return ptr(new block_device(rhs));
      }
#endif // SBUILD_FEATURE_LVMSNAP

      facet::ptr
      block_device::clone () const
      {
        return ptr(new block_device(*this));
      }

      void
      block_device::setup_lock (chroot::setup_type type,
                                bool               lock,
                                int                status)
      {
        /* Lock is preserved through the entire session. */
        if ((type == chroot::SETUP_START && lock == false) ||
            (type == chroot::SETUP_STOP && lock == true))
          return;

        try
          {
            if (!stat
#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
                (this->get_device()).is_character()
#else
                (this->get_device()).is_block()
#endif
                )
              {
                throw error(get_device(), chroot::DEVICE_NOTBLOCK);
              }
            else
              {
#ifdef SBUILD_FEATURE_UNION
                /* We don't lock the device if fsunion is configured. */
                fsunion::const_ptr puni
                  (owner->get_facet<fsunion>());
#endif // SBUILD_FEATURE_UNION
              }
          }
        catch (sbuild::stat::error const& e) // Failed to stat
          {
            // Don't throw if stopping a session and the device stat
            // failed.  This is because the setup scripts shouldn't fail
            // to be run if the block device no longer exists, which
            // would prevent the session from being ended.
            if (type != chroot::SETUP_STOP)
              throw;
          }

        /* Create or unlink session information. */
        if ((type == chroot::SETUP_START && lock == true) ||
            (type == chroot::SETUP_STOP && lock == false && status == 0))
          {
            bool start = (type == chroot::SETUP_START);
            owner->get_facet_strict<session>()->setup_session_info(start);
          }
      }

    }
  }
}

