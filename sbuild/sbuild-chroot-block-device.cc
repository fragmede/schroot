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

#include <config.h>

#include "sbuild-chroot-block-device.h"
#include "sbuild-format-detail.h"
#include "sbuild-lock.h"
#include "sbuild-util.h"

#include <cerrno>
#include <cstring>

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

chroot_block_device::chroot_block_device ():
  chroot_block_device_base()
{
}

chroot_block_device::~chroot_block_device ()
{
}

chroot_block_device::chroot_block_device (const chroot_block_device& rhs):
  chroot_block_device_base(rhs)
{
}

chroot_block_device::chroot_block_device (const chroot_lvm_snapshot& rhs):
  chroot_block_device_base(rhs)
{
}

sbuild::chroot::ptr
chroot_block_device::clone () const
{
  return ptr(new chroot_block_device(*this));
}
