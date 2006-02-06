/* Copyright © 2005-2006  Roger Leigh <rleigh@debian.org>
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

#include <config.h>

#include "sbuild.h"

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

chroot_block_device::chroot_block_device ():
  chroot(),
  device(),
  mount_options()
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
  this->device = device;
}

std::string const&
chroot_block_device::get_mount_device () const
{
  return this->device;
}

std::string const&
chroot_block_device::get_mount_options () const
{
  return this->mount_options;
}

void
chroot_block_device::set_mount_options (std::string const& mount_options)
{
  this->mount_options = mount_options;
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
  this->chroot::setup_env(env);

  env.add("CHROOT_DEVICE", get_device());
  env.add("CHROOT_MOUNT_OPTIONS", get_mount_options());
}

void
chroot_block_device::setup_lock (setup_type type,
				 bool       lock)
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
      sbuild::device_lock dlock(this->device);
      if (lock)
	{
	  try
	    {
	      dlock.set_lock(lock::LOCK_EXCLUSIVE, 15);
	    }
	  catch (sbuild::lock::error const& e)
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
	  catch (sbuild::lock::error const& e)
	    {
	      format fmt(_("%1%: failed to unlock device: %2%"));
	      fmt % get_device() % e.what();
	      throw error(fmt);
	    }
	}
    }
}

sbuild::chroot::session_flags
chroot_block_device::get_session_flags () const
{
  return static_cast<session_flags>(0);
}

void
chroot_block_device::print_details (std::ostream& stream) const
{
  this->chroot::print_details(stream);

  if (!this->device.empty())
    stream << format_details(_("Device"), get_device());
  if (!this->mount_options.empty())
    stream << format_details(_("Mount Options"), get_mount_options());
  stream << std::flush;
}

void
chroot_block_device::get_keyfile (keyfile& keyfile) const
{
  chroot::get_keyfile(keyfile);

  keyfile.set_value(get_name(), "device",
		    get_device());

  keyfile.set_value(get_name(), "mount-options",
		    get_mount_options());
}

void
chroot_block_device::set_keyfile (keyfile const& keyfile)
{
  chroot::set_keyfile(keyfile);

  std::string device;
  if (keyfile.get_value(get_name(), "device",
			keyfile::PRIORITY_REQUIRED, device))
    set_device(device);

  std::string mount_options;
  if (keyfile.get_value(get_name(), "mount-options",
			keyfile::PRIORITY_OPTIONAL, mount_options))
    set_mount_options(mount_options);
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
