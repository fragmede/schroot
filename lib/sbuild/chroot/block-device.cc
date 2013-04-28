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

#include <sbuild/chroot/block-device.h>
#include <sbuild/chroot/facet/block-device.h>
#include <sbuild/chroot/facet/factory.h>
#include <sbuild/chroot/facet/lvm-snapshot.h>
#include <sbuild/chroot/facet/session.h>
#include <sbuild/chroot/facet/session-clonable.h>
#include <sbuild/chroot/facet/source-clonable.h>
#include <sbuild/chroot/facet/storage.h>
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

    block_device::block_device ():
      chroot()
    {
      add_facet(std::dynamic_pointer_cast<facet::storage>(facet::factory::create("block-device")));
    }

    block_device::~block_device ()
    {
    }

    block_device::block_device (const block_device& rhs):
      chroot(rhs)
    {
    }

#ifdef SBUILD_FEATURE_LVMSNAP
    block_device::block_device (const lvm_snapshot& rhs):
      chroot(rhs)
    {
      facet::storage::ptr bdev = facet::block_device::create(*get_facet_strict<facet::lvm_snapshot>());
      replace_facet<facet::storage>(bdev);
    }
#endif // SBUILD_FEATURE_LVMSNAP

    chroot::chroot::ptr
    block_device::clone () const
    {
      return ptr(new block_device(*this));
    }

    chroot::chroot::ptr
    block_device::clone_session (std::string const& session_id,
                                 std::string const& alias,
                                 std::string const& user,
                                 bool               root) const
    {
      facet::session_clonable::const_ptr psess
        (get_facet<facet::session_clonable>());
      assert(psess);

      ptr session(new block_device(*this));
      psess->clone_session_setup(*this, session, session_id, alias, user, root);

      return session;
    }

    chroot::chroot::ptr
    block_device::clone_source () const
    {
      ptr clone(new block_device(*this));

      facet::source_clonable::const_ptr psrc
        (get_facet<facet::source_clonable>());
      assert(psrc);

      psrc->clone_source_setup(*this, clone);

      return clone;
    }

  }
}
