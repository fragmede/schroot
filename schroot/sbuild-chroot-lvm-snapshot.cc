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

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "sbuild-chroot-lvm-snapshot.h"
#include "sbuild-keyfile.h"
#include "sbuild-lock.h"
#include "sbuild-util.h"

using namespace sbuild;

ChrootLvmSnapshot::ChrootLvmSnapshot():
  ChrootBlockDevice(),
  snapshot_device(),
  snapshot_options()
{
}

ChrootLvmSnapshot::ChrootLvmSnapshot (GKeyFile           *keyfile,
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
  if (!(type == SETUP_STOP && lock == FALSE))
    {
      if (type == SETUP_START)
	device = get_device();
      else
	device = get_snapshot_device();

      if (device.empty())
	{
	  throw error(format_string(_("%s chroot: device name not set"),
				    get_name().c_str()),
		      ERROR_LOCK);
	}
      else if (stat(device.c_str(), &statbuf) == -1)
	{
	  throw error(format_string(_("%s chroot: failed to stat device %s: %s"),
				    get_name().c_str(),
				    device.c_str(), g_strerror(errno)),
		      ERROR_LOCK);
	}
      else if (!S_ISBLK(statbuf.st_mode))
	{
	  throw error(format_string(_("%s chroot: %s is not a block device\n"),
				    get_name().c_str(),
				    device.c_str()),
		      ERROR_LOCK);
	}
      else
	{
	  /* Lock is preserved while running a command. */
	  if ((type == RUN_START && lock == FALSE) ||
	      (type == RUN_STOP && lock == TRUE))
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
		  throw error(format_string(_("%s: failed to lock device: %s"),
					    device.c_str(),
					    e.what()),
			      ERROR_LOCK);
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
		  throw error(format_string(_("%s: failed to unlock device: %s"),
					    device.c_str(),
					    e.what()),
					    ERROR_LOCK);
		}
	    }
	}
    }

  /* Create or unlink session information. */
  if ((type == SETUP_START && lock == TRUE) ||
      (type == SETUP_STOP && lock == FALSE))
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

  FILE *sess_file = NULL;

  if (start)
    {
      int fd = open(file.c_str(), O_CREAT|O_EXCL|O_WRONLY, 0664);
      if (fd < 0)
	{
	  throw error(format_string(_("%s: failed to create session file: %s\n"),
				    file.c_str(), g_strerror(errno)),
		      ERROR_LOCK);
	}

      FILE *sess_file = fdopen(fd, "w");
      if (sess_file == NULL)
	{
	  if (close(fd) < 0) /* Can't set GError at this point. */
	    g_printerr("%s: failed to close session file: %s\n",
		       file.c_str(), g_strerror(errno));
	  throw error(format_string(_("%s: failed to create FILE from fd: %s\n"),
				    file.c_str(), g_strerror(errno)),
		      ERROR_LOCK);
	}

      try
	{
	  sbuild::FileLock lock(fd);
	  try
	    {
	      lock.set_lock(Lock::LOCK_EXCLUSIVE, 2);
	    }
	  catch (const Lock::error& e)
	    {
	      throw error(format_string(_("%s: lock acquisition failure: %s\n"),
					file.c_str(), e.what()),
			  ERROR_LOCK);
	    }

	  print_config(sess_file);
	  /* 		  if (fflush(sess_file) != 0) */
	  /* 		    g_set_error(error, */
	  /* 				ERROR, ERROR_LOCK, */
	  /* 				_("%s: failed to flush session file: %s\n"), */
	  /* 				file, g_strerror(errno)); */

	  try
	    {
	      lock.unset_lock();
	    }
	  catch (const Lock::error& e)
	    {
	      throw error(format_string(_("%s: lock discard failure: %s\n"),
					file.c_str(), e.what()),
			  ERROR_LOCK);
	    }
	  if (fclose(sess_file) != 0)
	    {
	      throw error(format_string(_("%s: failed to close session file: %s\n"),
					file.c_str(), g_strerror(errno)),
			  ERROR_LOCK);
	    }
	}
      catch (const error& e)
	{
	  fclose(sess_file);
	  throw error(e);
	}
    }
  else /* start == FALSE */
    {
      if (unlink(file.c_str()) != 0)
	{
	  throw error(format_string(_("%s: failed to unlink session file: %s\n"),
				    file.c_str(), g_strerror(errno)),
		      ERROR_LOCK);
	}
    }
}

Chroot::SessionFlags
ChrootLvmSnapshot::get_session_flags () const
{
  return SESSION_CREATE;
}

void
ChrootLvmSnapshot::print_details (FILE *file) const
{
  this->ChrootBlockDevice::print_details(file);

  if (!this->snapshot_device.empty())
    g_fprintf(file, "  %-22s%s\n", _("LVM Snapshot Device"),
	      this->snapshot_device.c_str());
  if (!this->snapshot_options.empty())
    g_fprintf(file, "  %-22s%s\n", _("LVM Snapshot Options"),
	      this->snapshot_options.c_str());
}

void
ChrootLvmSnapshot::print_config (FILE *file) const
{
  this->ChrootBlockDevice::print_config(file);

  if (!this->snapshot_device.empty())
    g_fprintf(file, _("lvm-snapshot-device=%s\n"), this->snapshot_device.c_str());
  if (!this->snapshot_options.empty())
    g_fprintf(file, _("lvm-snapshot-options=%s\n"), this->snapshot_options.c_str());
}

void
ChrootLvmSnapshot::read_keyfile (GKeyFile           *keyfile,
				 const std::string&  group)
{
  std::string snapshot_device;
  if (keyfile_read_string(keyfile, group, "lvm-snapshot-device", snapshot_device))
    set_snapshot_device(snapshot_device);

  std::string snapshot_options;
  if (keyfile_read_string(keyfile, group, "lvm-snapshot-options", snapshot_options))
    set_snapshot_options(snapshot_options);
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
