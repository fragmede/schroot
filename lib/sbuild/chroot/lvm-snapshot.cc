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

#include <sbuild/chroot/lvm-snapshot.h>
#include <sbuild/chroot/block-device.h>
#include <sbuild/chroot/facet/factory.h>
#include <sbuild/chroot/facet/session.h>
#include <sbuild/chroot/facet/session-clonable.h>
#include <sbuild/chroot/facet/source-clonable.h>
#include <sbuild/chroot/facet/mountable.h>
#include <sbuild/chroot/facet/storage.h>
#include "format-detail.h"

#include <cassert>
#include <cerrno>

#include <boost/format.hpp>

using std::endl;
using boost::format;
using namespace sbuild;

namespace sbuild
{
  namespace chroot
  {

    lvm_snapshot::lvm_snapshot ():
      chroot()
    {
      add_facet(std::dynamic_pointer_cast<facet::storage>(facet::factory::create("lvm-snapshot")));
    }

    lvm_snapshot::lvm_snapshot (const lvm_snapshot& rhs):
      chroot(rhs)
    {
    }

    lvm_snapshot::~lvm_snapshot ()
    {
    }

  }
}
