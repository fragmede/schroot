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

SbuildChrootBlockDevice::SbuildChrootBlockDevice():
  SbuildChroot(),
  device(),
  mount_options()
{
}

SbuildChrootBlockDevice::SbuildChrootBlockDevice (GKeyFile   *keyfile,
						  const std::string& group):
  SbuildChroot(keyfile, group),
  device(),
  mount_options()
{
  read_keyfile(keyfile, group);
}

SbuildChrootBlockDevice::~SbuildChrootBlockDevice()
{
}

SbuildChroot *
SbuildChrootBlockDevice::clone () const
{
  return new SbuildChrootBlockDevice(*this);
}

/**
 * sbuild_chroot_block_device_get_device:
 * @chroot: an #SbuildChrootBlockDevice
 *
 * Get the block device of the chroot.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const std::string&
SbuildChrootBlockDevice::get_device () const
{
  return this->device;
}

/**
 * sbuild_chroot_block_device_set_device:
 * @chroot: an #SbuildChrootBlockDevice.
 * @device: the device to set.
 *
 * Set the block device of a chroot.  This is the "source" device.  It
 * may be the case that the real device is different (for example, an
 * LVM snapshot PV), but by default will be the device to mount.
 */
void
SbuildChrootBlockDevice::set_device (const std::string& device)
{
  this->device = device;
}

const std::string&
SbuildChrootBlockDevice::get_mount_device () const
{
  return this->device;
}

/**
 * sbuild_chroot_block_device_get_mount_options:
 * @chroot: an #SbuildChrootBlockDevice
 *
 * Get the filesystem mount_options of the chroot block device.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const std::string&
SbuildChrootBlockDevice::get_mount_options () const
{
  return this->mount_options;
}

/**
 * sbuild_chroot_block_device_set_mount_options:
 * @chroot: an #SbuildChrootBlockDevice.
 * @mount_options: the device to set.
 *
 * Set the filesystem mount_options of a chroot block device.  These
 * will be passed to mount(8) when mounting the device.
 */
void
SbuildChrootBlockDevice::set_mount_options (const std::string& mount_options)
{
  this->mount_options = mount_options;
}

const std::string&
SbuildChrootBlockDevice::get_chroot_type () const
{
  static const std::string type("block-device");

  return type;
}

void
SbuildChrootBlockDevice::setup_env (env_list& env)
{
  this->SbuildChroot::setup_env(env);

  setup_env_var(env, "CHROOT_DEVICE",
		get_device());
  setup_env_var(env, "CHROOT_MOUNT_OPTIONS",
		get_mount_options());
}

bool
SbuildChrootBlockDevice::setup_lock (SbuildChrootSetupType   type,
				     gboolean                lock,
				     GError                **error)
{
  struct stat statbuf;

  /* Only lock during setup, not run. */
  if (type == SBUILD_CHROOT_RUN_START || type == SBUILD_CHROOT_RUN_STOP)
    return TRUE;

  /* Lock is preserved through the entire session. */
  if ((type == SBUILD_CHROOT_SETUP_START && lock == FALSE) ||
      (type == SBUILD_CHROOT_SETUP_STOP && lock == TRUE))
    return TRUE;

  if (stat(this->device.c_str(), &statbuf) == -1)
    {
      g_set_error(error,
		  SBUILD_CHROOT_ERROR, SBUILD_CHROOT_ERROR_LOCK,
		  _("%s chroot: failed to stat device %s: %s\n"),
		  get_name().c_str(),
		  this->device.c_str(), g_strerror(errno));
    }
  else if (!S_ISBLK(statbuf.st_mode))
    {
      g_set_error(error,
		  SBUILD_CHROOT_ERROR, SBUILD_CHROOT_ERROR_LOCK,
		  _("%s chroot: %s is not a block device\n"),
		  get_name().c_str(),
		  this->device.c_str());
    }
  else
    {
      GError *tmp_error = NULL;
      sbuild::DeviceLock dlock(this->device);
      if (lock)
	{
	  if (dlock.set_lock(SBUILD_LOCK_EXCLUSIVE,
			    15,
			    &tmp_error) == FALSE)
	    {
	      g_set_error(error,
			  SBUILD_CHROOT_ERROR, SBUILD_CHROOT_ERROR_LOCK,
			  _("%s: failed to lock device: %s\n"),
			  this->device.c_str(), tmp_error->message);
	      g_error_free(tmp_error);
	    }
	}
      else
	{
	  if (dlock.unset_lock(&tmp_error) == FALSE)
	    {
	      g_set_error(error,
			  SBUILD_CHROOT_ERROR, SBUILD_CHROOT_ERROR_LOCK,
			  _("%s: failed to unlock device: %s\n"),
			  this->device.c_str(), tmp_error->message);
	      g_error_free(tmp_error);
	    }
	}
    }

  if (*error)
    return FALSE;
  else
    return TRUE;
}

SbuildChrootSessionFlags
SbuildChrootBlockDevice::get_session_flags () const
{
  return static_cast<SbuildChrootSessionFlags>(0);
}

void
SbuildChrootBlockDevice::print_details (FILE *file) const
{
  this->SbuildChroot::print_details(file);

  if (!this->device.empty())
    g_fprintf(file, _("  %-22s%s\n"), _("Device"), this->device.c_str());
  if (!this->mount_options.empty())
    g_fprintf(file, _("  %-22s%s\n"), _("Mount Options"), this->mount_options.c_str());
}

void
SbuildChrootBlockDevice::print_config (FILE *file) const
{
  this->SbuildChroot::print_config(file);

  g_fprintf(file, _("device=%s\n"), this->device.c_str());
  g_fprintf(file, _("mount-options=%s\n"), this->mount_options.c_str());
}

void
SbuildChrootBlockDevice::read_keyfile (GKeyFile   *keyfile,
				       const std::string& group)
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
