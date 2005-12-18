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

#include <glib.h>
#include <glib/gi18n.h>

#include "sbuild-chroot-block-device.h"
#include "sbuild-keyfile.h"
#include "sbuild-lock.h"

using namespace sbuild;

ChrootBlockDevice::ChrootBlockDevice():
  Chroot(),
  device(),
  mount_options()
{
}

ChrootBlockDevice::ChrootBlockDevice (GKeyFile           *keyfile,
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
  if ((type == SETUP_START && lock == FALSE) ||
      (type == SETUP_STOP && lock == TRUE))
    return;

  if (stat(this->device.c_str(), &statbuf) == -1)
    {
      char *gstr = g_strdup_printf(_("%s chroot: failed to stat device %s: %s"),
				    get_name().c_str(),
				    this->device.c_str(), g_strerror(errno));
      std::string err(gstr);
      g_free(gstr);
      throw error(err, ERROR_LOCK);
    }
  else if (!S_ISBLK(statbuf.st_mode))
    {
      char *gstr = g_strdup_printf(_("%s chroot: %s is not a block device"),
				   get_name().c_str(),
				   this->device.c_str());
      std::string err(gstr);
      g_free(gstr);
      throw error(err, ERROR_LOCK);
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
	      char *gstr = g_strdup_printf(_("%s: failed to lock device: %s"),
					     this->device.c_str(),
					     e.what());
	      std::string err(gstr);
	      g_free(gstr);
	      throw error(err, ERROR_LOCK);
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
	      char *gstr = g_strdup_printf(_("%s: failed to unlock device: %s"),
					     this->device.c_str(),
					     e.what());
	      std::string err(gstr);
	      g_free(gstr);
	      throw error(err, ERROR_LOCK);
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
ChrootBlockDevice::print_details (FILE *file) const
{
  this->Chroot::print_details(file);

  if (!this->device.empty())
    g_fprintf(file, _("  %-22s%s\n"), _("Device"), this->device.c_str());
  if (!this->mount_options.empty())
    g_fprintf(file, _("  %-22s%s\n"), _("Mount Options"), this->mount_options.c_str());
}

void
ChrootBlockDevice::print_config (FILE *file) const
{
  this->Chroot::print_config(file);

  g_fprintf(file, _("device=%s\n"), this->device.c_str());
  g_fprintf(file, _("mount-options=%s\n"), this->mount_options.c_str());
}

void
ChrootBlockDevice::read_keyfile (GKeyFile           *keyfile,
				 const std::string&  group)
{
  std::string device;
  if (keyfile_read_string(keyfile, group, "device", device))
    set_device(device);

  std::string mount_options;
  if (keyfile_read_string(keyfile, group, "mount-options", mount_options))
    set_mount_options(mount_options);
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
