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

#include <sbuild/chroot/facet/block-device-base.h>
#include <sbuild/chroot/facet/mountable.h>
#include "format-detail.h"
#include "lock.h"
#include "util.h"

#include <cerrno>
#include <cstring>

using namespace sbuild;

namespace sbuild
{
  namespace chroot
  {
    namespace facet
    {

      block_device_base::block_device_base ():
        storage(),
        device()
      {
      }

      block_device_base::block_device_base
      (const block_device_base& rhs):
        storage(rhs),
        device(rhs.device)
      {
      }

      block_device_base::~block_device_base ()
      {
      }

      void
      block_device_base::set_chroot (chroot& chroot)
      {
        facet::set_chroot(chroot);
        this->owner->add_facet(mountable::create());
      }

      std::string const&
      block_device_base::get_device () const
      {
        return this->device;
      }

      void
      block_device_base::set_device (std::string const& device)
      {
        if (!is_absname(device))
          throw error(device, chroot::DEVICE_ABS);

        this->device = device;
      }

      std::string
      block_device_base::get_path () const
      {
        mountable::const_ptr pmnt
          (this->owner->get_facet<mountable>());

        std::string path(owner->get_mount_location());

        if (pmnt)
          path += pmnt->get_location();

        return path;
      }

      void
      block_device_base::setup_env (chroot const& chroot,
                                    environment& env) const
      {
        env.add("CHROOT_DEVICE", get_device());
      }

      void
      block_device_base::get_details (chroot const& chroot,
                                      format_detail& detail) const
      {
        if (!this->device.empty())
          detail.add(_("Device"), get_device());
      }

      void
      block_device_base::get_used_keys (string_list& used_keys) const
      {
        used_keys.push_back("device");
      }

      void
      block_device_base::get_keyfile (chroot const& chroot,
                                      keyfile&      keyfile) const
      {
        keyfile::set_object_value(*this, &block_device_base::get_device,
                                  keyfile, get_name(), "device");
      }

      void
      block_device_base::set_keyfile (chroot&        chroot,
                                      keyfile const& keyfile)
      {
        keyfile::get_object_value(*this, &block_device_base::set_device,
                                  keyfile, get_name(), "device",
                                  keyfile::PRIORITY_REQUIRED);
      }

    }
  }
}
