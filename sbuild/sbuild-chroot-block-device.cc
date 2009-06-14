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
  chroot(),
  chroot_mountable(),
  device()
{
}

chroot_block_device::~chroot_block_device ()
{
}

sbuild::chroot::ptr
chroot_block_device::clone () const
{
  return ptr(new chroot_block_device(*this));
}

std::string const&
chroot_block_device::get_device () const
{
  return this->device;
}

void
chroot_block_device::set_device (std::string const& device)
{
  if (!is_absname(device))
    throw error(device, DEVICE_ABS);

  this->device = device;
  /** @todo When using LVM snapshots, this it is incorrect to call
   * set_mount_device here, since it is not the mount device.  This
   * currently works due to the order we call in, but could
   * potentially break.  Make chroot_mountable a pure virtual
   * interface to fix this.
   */
  chroot_mountable::set_mount_device(device);
}

std::string
chroot_block_device::get_path () const
{
  return chroot_mountable::get_path();
}

std::string const&
chroot_block_device::get_chroot_type () const
{
  static const std::string type("block-device");

  return type;
}

void
chroot_block_device::setup_env (environment& env)
{
  chroot::setup_env(env);
  chroot_mountable::setup_env(env);

  env.add("CHROOT_DEVICE", get_device());
}

void
chroot_block_device::setup_lock (chroot::setup_type type,
				 bool               lock,
				 int                status)
{
  /* Only lock during setup, not exec. */
  if (type == EXEC_START || type == EXEC_STOP)
    return;

  /* Lock is preserved through the entire session. */
  if ((type == SETUP_START && lock == false) ||
      (type == SETUP_STOP && lock == true))
    return;

  try
    {
      if (!stat(this->device).is_block())
	{
	  throw error(get_device(), DEVICE_NOTBLOCK);
	}
      else
	{
	  sbuild::device_lock dlock(this->device);
	  if (lock)
	    {
	      try
		{
		  dlock.set_lock(lock::LOCK_EXCLUSIVE, 15);
		}
	      catch (sbuild::lock::error const& e)
		{
		  throw error(get_device(), DEVICE_LOCK, e);
		}
	    }
	  else
	    {
	      try
		{
		  dlock.unset_lock();
		}
	      catch (sbuild::lock::error const& e)
		{
		  throw error(get_device(), DEVICE_UNLOCK, e);
		}
	    }
	}
    }
  catch (sbuild::stat::error const& e) // Failed to stat
    {
      // Don't throw if stopping a session and the device stat
      // failed.  This is because the setup scripts shouldn't fail
      // to be run if the block device no longer exists, which
      // would prevent the session from being ended.
      if (type != SETUP_STOP)
	throw;
    }
}

sbuild::chroot::session_flags
chroot_block_device::get_session_flags () const
{
  return SESSION_NOFLAGS | chroot_mountable::get_session_flags();
}

void
chroot_block_device::get_details (format_detail& detail) const
{
  this->chroot::get_details(detail);
  this->chroot_mountable::get_details(detail);

  if (!this->device.empty())
    detail.add(_("Device"), get_device());
}

void
chroot_block_device::get_keyfile (keyfile& keyfile) const
{
  chroot::get_keyfile(keyfile);
  chroot_mountable::get_keyfile(keyfile);

  keyfile::set_object_value(*this, &chroot_block_device::get_device,
			    keyfile, get_keyfile_name(), "device");
}

void
chroot_block_device::set_keyfile (keyfile const& keyfile,
				  string_list&   used_keys)
{
  chroot::set_keyfile(keyfile, used_keys);
  chroot_mountable::set_keyfile(keyfile, used_keys);

  keyfile::get_object_value(*this, &chroot_block_device::set_device,
			    keyfile, get_keyfile_name(), "device",
			    keyfile::PRIORITY_REQUIRED);
  used_keys.push_back("device");
}
