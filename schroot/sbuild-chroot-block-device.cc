/* sbuild-chroot-block-device - sbuild chroot block device object
 *
 * Copyright © 2005  Roger Leigh <rleigh@debian.org>
 *
 * schroot is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * schroot is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA  02111-1307  USA
 *
 *********************************************************************/

/**
 * SECTION:sbuild-chroot-block-device
 * @short_description: chroot block device object
 * @title: SbuildChrootBlockDevice
 *
 * This object represents a chroot stored on an unmounted block
 * device.  The device will be mounted on demand.
 */

#include <config.h>

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

#include <boost/format.hpp>

#include "sbuild-i18n.h"
#include "sbuild-chroot-block-device.h"
#include "sbuild-keyfile.h"
#include "sbuild-lock.h"
#include "sbuild-util.h"

using boost::format;
using namespace sbuild;

ChrootBlockDevice::ChrootBlockDevice():
  Chroot(),
  device(),
  mount_options()
{
}

ChrootBlockDevice::ChrootBlockDevice (const keyfile&      keyfile,
				      const std::string&  group):
  Chroot(keyfile, group),
  device(),
  mount_options()
{
  read_keyfile(keyfile, group);
}

ChrootBlockDevice::~ChrootBlockDevice()
{
}

Chroot *
ChrootBlockDevice::clone () const
{
  return new ChrootBlockDevice(*this);
}

/**
 * sbuild_chroot_block_device_get_device:
 * @chroot: an #ChrootBlockDevice
 *
 * Get the block device of the chroot.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const std::string&
ChrootBlockDevice::get_device () const
{
  return this->device;
}

/**
 * sbuild_chroot_block_device_set_device:
 * @chroot: an #ChrootBlockDevice.
 * @device: the device to set.
 *
 * Set the block device of a chroot.  This is the "source" device.  It
 * may be the case that the real device is different (for example, an
 * LVM snapshot PV), but by default will be the device to mount.
 */
void
ChrootBlockDevice::set_device (const std::string& device)
{
  this->device = device;
}

const std::string&
ChrootBlockDevice::get_mount_device () const
{
  return this->device;
}

/**
 * sbuild_chroot_block_device_get_mount_options:
 * @chroot: an #ChrootBlockDevice
 *
 * Get the filesystem mount_options of the chroot block device.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const std::string&
ChrootBlockDevice::get_mount_options () const
{
  return this->mount_options;
}

/**
 * sbuild_chroot_block_device_set_mount_options:
 * @chroot: an #ChrootBlockDevice.
 * @mount_options: the device to set.
 *
 * Set the filesystem mount_options of a chroot block device.  These
 * will be passed to mount(8) when mounting the device.
 */
void
ChrootBlockDevice::set_mount_options (const std::string& mount_options)
{
  this->mount_options = mount_options;
}

const std::string&
ChrootBlockDevice::get_chroot_type () const
{
  static const std::string type("block-device");

  return type;
}

void
ChrootBlockDevice::setup_env (env_list& env)
{
  this->Chroot::setup_env(env);

  setup_env_var(env, "CHROOT_DEVICE",
		get_device());
  setup_env_var(env, "CHROOT_MOUNT_OPTIONS",
		get_mount_options());
}

void
ChrootBlockDevice::setup_lock (Chroot::SetupType type,
			       bool              lock)
{
  struct stat statbuf;

  /* Only lock during setup, not run. */
  if (type == RUN_START || type == RUN_STOP)
    return;

  /* Lock is preserved through the entire session. */
  if ((type == SETUP_START && lock == false) ||
      (type == SETUP_STOP && lock == true))
    return;

  if (stat(this->device.c_str(), &statbuf) == -1)
    {
      format fmt(_("%1% chroot: failed to stat device %2%: %3%"));
      fmt % get_name() % get_device() % strerror(errno);
      throw error(fmt);
    }
  else if (!S_ISBLK(statbuf.st_mode))
    {
      format fmt(_("%1% chroot: %2% is not a block device"));
      fmt % get_name() % get_device();
      throw error(fmt);
    }
  else
    {
      sbuild::DeviceLock dlock(this->device);
      if (lock)
	{
	  try
	    {
	      dlock.set_lock(Lock::LOCK_EXCLUSIVE, 15);
	    }
	  catch (const sbuild::Lock::error& e)
	    {
	      format fmt(_("%1%: failed to lock device: %2%"));
	      fmt % get_device() % e.what();
	      throw error(fmt);
	    }
	}
      else
	{
	  try
	    {
	      dlock.unset_lock();
	    }
	  catch (const sbuild::Lock::error& e)
	    {
	      format fmt(_("%1%: failed to unlock device: %2%"));
	      fmt % get_device() % e.what();
	      throw error(fmt);
	    }
	}
    }
}

Chroot::SessionFlags
ChrootBlockDevice::get_session_flags () const
{
  return static_cast<SessionFlags>(0);
}

void
ChrootBlockDevice::print_details (std::ostream& stream) const
{
  this->Chroot::print_details(stream);

  if (!this->device.empty())
    stream << format_detail_string(_("Device"), get_device());
  if (!this->mount_options.empty())
    stream << format_detail_string(_("Mount Options"), get_mount_options());
  stream << std::flush;
}

void
ChrootBlockDevice::print_config (std::ostream& stream) const
{
  this->Chroot::print_config(stream);

  stream << "device=" << get_device() << '\n'
	 << "mount-options=" << get_mount_options() << '\n';
  stream << std::flush;
}

void
ChrootBlockDevice::read_keyfile (const keyfile&      keyfile,
				 const std::string&  group)
{
  std::string device;
  if (keyfile.get_value(group, "device", device))
    set_device(device);

  std::string mount_options;
  if (keyfile.get_value(group, "mount-options", mount_options))
    set_mount_options(mount_options);
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
