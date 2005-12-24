/* sbuild-chroot-lvm-snapshot - sbuild chroot lvm snapshot object
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
 * SECTION:sbuild-chroot-lvm-snapshot
 * @short_description: chroot lvm snapshot object
 * @title: SbuildChrootLvmSnapshot
 *
 * This object represents a chroot stored on an LVM logical volume
 * (LV).  A snapshot LV will be created and mounted on demand.
 */

#include <config.h>

#include <iostream>
#include <ext/stdio_filebuf.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

#include <boost/format.hpp>

#include "sbuild-i18n.h"
#include "sbuild-chroot-lvm-snapshot.h"
#include "sbuild-keyfile.h"
#include "sbuild-lock.h"
#include "sbuild-log.h"
#include "sbuild-util.h"

using std::endl;
using boost::format;
using namespace sbuild;

ChrootLvmSnapshot::ChrootLvmSnapshot():
  ChrootBlockDevice(),
  snapshot_device(),
  snapshot_options()
{
}

ChrootLvmSnapshot::ChrootLvmSnapshot (const keyfile&      keyfile,
				      const std::string&  group):
  ChrootBlockDevice(keyfile, group),
  snapshot_device(),
  snapshot_options()
{
  read_keyfile(keyfile, group);
}

ChrootLvmSnapshot::~ChrootLvmSnapshot()
{
}

Chroot *
ChrootLvmSnapshot::clone () const
{
  return new ChrootLvmSnapshot(*this);
}

/**
 * sbuild_chroot_lvm_snapshot_get_snapshot_device:
 * @chroot: an #ChrootLvmSnapshot
 *
 * Get the logical volume snapshot device name, used by lvcreate.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const std::string&
ChrootLvmSnapshot::get_snapshot_device () const
{
  return this->snapshot_device;
}

/**
 * sbuild_chroot_lvm_snapshot_set_snapshot_device:
 * @chroot: an #ChrootLvmSnapshot.
 * @snapshot_device: the snapshot device to set.
 *
 * Set the logical volume snapshot device name, used by lvcreate.
 */
void
ChrootLvmSnapshot::set_snapshot_device (const std::string& snapshot_device)
{
  this->snapshot_device = snapshot_device;
}

const std::string&
ChrootLvmSnapshot::get_mount_device () const
{
  return this->snapshot_device;
}

/**
 * sbuild_chroot_lvm_snapshot_get_snapshot_options:
 * @chroot: an #ChrootLvmSnapshot
 *
 * Get the logical volume snapshot options, used by lvcreate.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const std::string&
ChrootLvmSnapshot::get_snapshot_options () const
{
  return this->snapshot_options;
}

/**
 * sbuild_chroot_lvm_snapshot_set_snapshot_options:
 * @chroot: an #ChrootLvmSnapshot.
 * @snapshot_options: the snapshot options to set.
 *
 * Set the logical volume snapshot options, used by lvcreate.
 */
void
ChrootLvmSnapshot::set_snapshot_options (const std::string& snapshot_options)
{
  this->snapshot_options = snapshot_options;
}

const std::string&
ChrootLvmSnapshot::get_chroot_type () const
{
  static const std::string type("lvm-snapshot");

  return type;
}

void
ChrootLvmSnapshot::setup_env (env_list& env)
{
  this->ChrootBlockDevice::setup_env(env);

  setup_env_var(env, "CHROOT_LVM_SNAPSHOT_NAME",
		sbuild::basename(get_snapshot_device()));

  setup_env_var(env, "CHROOT_LVM_SNAPSHOT_DEVICE",
		get_snapshot_device());

  setup_env_var(env, "CHROOT_LVM_SNAPSHOT_OPTIONS",
		get_snapshot_options());
}

void
ChrootLvmSnapshot::setup_lock (Chroot::SetupType type,
			       bool              lock)
{
  std::string device;
  struct stat statbuf;

  /* Lock is removed by setup script on setup stop.  Unlocking here
     would fail: the LVM snapshot device no longer exists. */
  if (!(type == SETUP_STOP && lock == false))
    {
      if (type == SETUP_START)
	device = get_device();
      else
	device = get_snapshot_device();

      if (device.empty())
	{
	  format fmt(_("%1% chroot: device name not set"));
	  fmt % get_name();
	  throw error(fmt, ERROR_LOCK);
	}
      else if (stat(device.c_str(), &statbuf) == -1)
	{
	  format fmt(_("%1% chroot: failed to stat device %2%: %3%"));
	  fmt % get_name() % device % strerror(errno);
	  throw error(fmt, ERROR_LOCK);
	}
      else if (!S_ISBLK(statbuf.st_mode))
	{
	  format fmt(_("%1% chroot: %2% is not a block device\n"));
	  fmt % get_name() % device;
	  throw error(fmt, ERROR_LOCK);
	}
      else
	{
	  /* Lock is preserved while running a command. */
	  if ((type == RUN_START && lock == false) ||
	      (type == RUN_STOP && lock == true))
	    return;

	  sbuild::DeviceLock dlock(device);
	  if (lock)
	    {
	      try
		{
		  dlock.set_lock(Lock::LOCK_EXCLUSIVE, 15);
		}
	      catch (const sbuild::Lock::error &e)
		{
		  format fmt(_("%1%: failed to lock device: %2%"));
		  fmt % device % e.what();
		  throw error(fmt, ERROR_LOCK);
		}
	    }
	  else
	    {
	      try
		{
		  dlock.unset_lock();
		}
	      catch (const sbuild::Lock::error &e)
		{
		  format fmt(_("%1%: failed to unlock device: %2%"));
		  fmt % device % e.what();
		  throw error(fmt, ERROR_LOCK);
		}
	    }
	}
    }

  /* Create or unlink session information. */
  if ((type == SETUP_START && lock == true) ||
      (type == SETUP_STOP && lock == false))
    {
      bool start = (type == SETUP_START);
      setup_session_info(start);
    }
}

void
ChrootLvmSnapshot::setup_session_info (bool start)
{
  /* Create or unlink session information. */
  std::string file = std::string(SCHROOT_SESSION_DIR) + "/" + get_name();

  if (start)
    {
      int fd = open(file.c_str(), O_CREAT|O_EXCL|O_WRONLY, 0664);
      if (fd < 0)
	{
	  format fmt(_("%1%: failed to create session file: %2%\n"));
	  fmt % file % strerror(errno);
	  throw error(fmt, ERROR_LOCK);
	}

      // Create a stream buffer from the file descriptor.  The fd will
      // be closed when the buffer is destroyed.
      __gnu_cxx::stdio_filebuf<char> fdbuf(fd, std::ios::out);
      std::ostream output(&fdbuf);

      sbuild::FileLock lock(fd);
      try
	{
	  lock.set_lock(Lock::LOCK_EXCLUSIVE, 2);
	}
      catch (const Lock::error& e)
	{
	  format fmt(_("%1%: lock acquisition failure: %2%\n"));
	  fmt % file % e.what();
	  throw error(fmt, ERROR_LOCK);
	}

      print_config(output);

      try
	{
	  lock.unset_lock();
	}
      catch (const Lock::error& e)
	{
	  format fmt(_("%1%: lock discard failure: %2%\n"));
	  fmt % file % e.what();
	  throw error(fmt, ERROR_LOCK);
	}
    }
  else /* start == false */
    {
      if (unlink(file.c_str()) != 0)
	{
	  format fmt(_("%1%: failed to unlink session file: %2%\n"));
	  fmt % file % strerror(errno);
	  throw error(fmt, ERROR_LOCK);
	}
    }
}

Chroot::SessionFlags
ChrootLvmSnapshot::get_session_flags () const
{
  return SESSION_CREATE;
}

void
ChrootLvmSnapshot::print_details (std::ostream& stream) const
{
  this->ChrootBlockDevice::print_details(stream);

  if (!this->snapshot_device.empty())
    stream << format_detail_string(_("LVM Snapshot Device"),
				   get_snapshot_device());
  if (!this->snapshot_options.empty())
    stream << format_detail_string(_("LVM Snapshot Options"),
				   get_snapshot_options());
  stream << std::flush;
}

void
ChrootLvmSnapshot::print_config (std::ostream& stream) const
{
  this->ChrootBlockDevice::print_config(stream);

  if (!this->snapshot_device.empty())
    stream << "lvm-snapshot-device=" << get_snapshot_device() << '\n';
  if (!this->snapshot_options.empty())
    stream << "lvm-snapshot-options=" << get_snapshot_options() << '\n';
  stream << std::flush;
}

void
ChrootLvmSnapshot::read_keyfile (const keyfile&      keyfile,
				 const std::string&  group)
{
  std::string snapshot_device;
  if (keyfile.get_value(group, "lvm-snapshot-device", snapshot_device))
    set_snapshot_device(snapshot_device);

  std::string snapshot_options;
  if (keyfile.get_value(group, "lvm-snapshot-options", snapshot_options))
    set_snapshot_options(snapshot_options);
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
