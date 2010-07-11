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
#include "sbuild-chroot-lvm-snapshot.h"
#include "sbuild-chroot-facet-mountable.h"
#ifdef SBUILD_FEATURE_UNION
#include "sbuild-chroot-facet-union.h"
#endif // SBUILD_FEATURE_UNION
#include "sbuild-format-detail.h"
#include "sbuild-lock.h"
#include "sbuild-util.h"

#include <cerrno>
#include <cstring>

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

chroot_block_device_base::chroot_block_device_base ():
  chroot(),
  device()
{
  add_facet(chroot_facet_mountable::create());
}

chroot_block_device_base::chroot_block_device_base
(const chroot_block_device_base& rhs):
  chroot(rhs),
  device()
{
  /// @todo Required to set mount_device.  Remove once no longer
  /// needed.
  if (!rhs.device.empty())
    set_device(rhs.device);
}

chroot_block_device_base::~chroot_block_device_base ()
{
}

std::string const&
chroot_block_device_base::get_device () const
{
  return this->device;
}

void
chroot_block_device_base::set_device (std::string const& device)
{
  if (!is_absname(device))
    throw error(device, DEVICE_ABS);

  this->device = device;

  /// @todo: This may not be appropriate for derived classes such as
  /// lvm_snapshot, since re-setting the device could overwrite the
  /// mount device.
  chroot_facet_mountable::ptr pmnt
    (get_facet<chroot_facet_mountable>());
#ifdef SBUILD_FEATURE_LVMSNAP
  if (!dynamic_cast<chroot_lvm_snapshot *>(this))
#endif
    pmnt->set_mount_device(this->device);
}

std::string
chroot_block_device_base::get_path () const
{
  chroot_facet_mountable::const_ptr pmnt
    (get_facet<chroot_facet_mountable>());

  std::string path(get_mount_location());

  if (pmnt)
    path += pmnt->get_location();

  return path;
}

void
chroot_block_device_base::setup_env (chroot const& chroot,
				     environment& env) const
{
  chroot::setup_env(chroot, env);

  env.add("CHROOT_DEVICE", get_device());
}

sbuild::chroot::session_flags
chroot_block_device_base::get_session_flags (chroot const& chroot) const
{
  return chroot::SESSION_NOFLAGS;
}

void
chroot_block_device_base::get_details (chroot const& chroot,
				       format_detail& detail) const
{
  this->chroot::get_details(chroot, detail);

  if (!this->device.empty())
    detail.add(_("Device"), get_device());
}

void
chroot_block_device_base::get_keyfile (chroot const& chroot,
				       keyfile&      keyfile) const
{
  chroot::get_keyfile(chroot, keyfile);

  keyfile::set_object_value(*this, &chroot_block_device_base::get_device,
			    keyfile, get_name(), "device");
}

void
chroot_block_device_base::set_keyfile (chroot&        chroot,
				       keyfile const& keyfile,
				       string_list&   used_keys)
{
  chroot::set_keyfile(chroot, keyfile, used_keys);

  keyfile::get_object_value(*this, &chroot_block_device_base::set_device,
			    keyfile, get_name(), "device",
			    keyfile::PRIORITY_REQUIRED);
  used_keys.push_back("device");
}
