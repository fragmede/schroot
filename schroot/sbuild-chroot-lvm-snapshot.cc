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

SbuildChrootLvmSnapshot::SbuildChrootLvmSnapshot():
  SbuildChrootBlockDevice(),
  snapshot_device(),
  snapshot_options()
{
}

SbuildChrootLvmSnapshot::SbuildChrootLvmSnapshot (GKeyFile   *keyfile,
						  const std::string& group):
  SbuildChrootBlockDevice(keyfile, group),
  snapshot_device(),
  snapshot_options()
{
  read_keyfile(keyfile, group);
}

SbuildChrootLvmSnapshot::~SbuildChrootLvmSnapshot()
{
}

SbuildChroot *
SbuildChrootLvmSnapshot::clone () const
{
  return new SbuildChrootLvmSnapshot(*this);
}

/**
 * sbuild_chroot_lvm_snapshot_get_snapshot_device:
 * @chroot: an #SbuildChrootLvmSnapshot
 *
 * Get the logical volume snapshot device name, used by lvcreate.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const std::string&
SbuildChrootLvmSnapshot::get_snapshot_device () const
{
  return this->snapshot_device;
}

/**
 * sbuild_chroot_lvm_snapshot_set_snapshot_device:
 * @chroot: an #SbuildChrootLvmSnapshot.
 * @snapshot_device: the snapshot device to set.
 *
 * Set the logical volume snapshot device name, used by lvcreate.
 */
void
SbuildChrootLvmSnapshot::set_snapshot_device (const std::string& snapshot_device)
{
  this->snapshot_device = snapshot_device;
}

const std::string&
SbuildChrootLvmSnapshot::get_mount_device () const
{
  return this->snapshot_device;
}

/**
 * sbuild_chroot_lvm_snapshot_get_snapshot_options:
 * @chroot: an #SbuildChrootLvmSnapshot
 *
 * Get the logical volume snapshot options, used by lvcreate.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const std::string&
SbuildChrootLvmSnapshot::get_snapshot_options () const
{
  return this->snapshot_options;
}

/**
 * sbuild_chroot_lvm_snapshot_set_snapshot_options:
 * @chroot: an #SbuildChrootLvmSnapshot.
 * @snapshot_options: the snapshot options to set.
 *
 * Set the logical volume snapshot options, used by lvcreate.
 */
void
SbuildChrootLvmSnapshot::set_snapshot_options (const std::string& snapshot_options)
{
  this->snapshot_options = snapshot_options;
}

const std::string&
SbuildChrootLvmSnapshot::get_chroot_type () const
{
  static const std::string type("lvm-snapshot");

  return type;
}

void
SbuildChrootLvmSnapshot::setup_env (env_list& env)
{
  this->SbuildChrootBlockDevice::setup_env(env);

  setup_env_var(env, "CHROOT_LVM_SNAPSHOT_NAME",
		Sbuild::basename(get_snapshot_device()));

  setup_env_var(env, "CHROOT_LVM_SNAPSHOT_DEVICE",
		get_snapshot_device());

  setup_env_var(env, "CHROOT_LVM_SNAPSHOT_OPTIONS",
		get_snapshot_options());
}

bool
SbuildChrootLvmSnapshot::setup_lock (SbuildChrootSetupType   type,
				     gboolean                lock,
				     GError                **error)
{
  std::string device;
  struct stat statbuf;

  /* Lock is removed by setup script on setup stop.  Unlocking here
     would fail: the LVM snapshot device no longer exists. */
  if (!(type == SBUILD_CHROOT_SETUP_STOP && lock == FALSE))
    {
      if (type == SBUILD_CHROOT_SETUP_START)
	device = get_device();
      else
	device = get_snapshot_device();

      if (device.empty())
	{
	  g_set_error(error,
		      SBUILD_CHROOT_ERROR, SBUILD_CHROOT_ERROR_LOCK,
		      _("%s chroot: device name not set\n"),
		      get_name().c_str());
	}
      else if (stat(device.c_str(), &statbuf) == -1)
	{
	  g_set_error(error,
		      SBUILD_CHROOT_ERROR, SBUILD_CHROOT_ERROR_LOCK,
		      _("%s chroot: failed to stat device %s: %s\n"),
		      get_name().c_str(),
		      device.c_str(), g_strerror(errno));
	}
      else if (!S_ISBLK(statbuf.st_mode))
	{
	  g_set_error(error,
		      SBUILD_CHROOT_ERROR, SBUILD_CHROOT_ERROR_LOCK,
		      _("%s chroot: %s is not a block device\n"),
		      get_name().c_str(),
		      device.c_str());
	}
      else
	{
	  /* Lock is preserved while running a command. */
	  if ((type == SBUILD_CHROOT_RUN_START && lock == FALSE) ||
	      (type == SBUILD_CHROOT_RUN_STOP && lock == TRUE))
	    return TRUE;

	  GError *tmp_error = NULL;
	  sbuild::DeviceLock dlock(device);
	  if (lock)
	    {
	      if (dlock.set_lock(SBUILD_LOCK_EXCLUSIVE,
			       15,
			       &tmp_error) == FALSE)
		{
		  g_set_error(error,
			      SBUILD_CHROOT_ERROR, SBUILD_CHROOT_ERROR_LOCK,
			      _("%s: failed to lock device: %s\n"),
			      device.c_str(), tmp_error->message);
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
			      device.c_str(), tmp_error->message);
		  g_error_free(tmp_error);
		}
	    }
	}
    }

  /* Create or unlink session information. */
  if ((type == SBUILD_CHROOT_SETUP_START && lock == TRUE) ||
      (type == SBUILD_CHROOT_SETUP_STOP && lock == FALSE))
    {
      gboolean start = (type == SBUILD_CHROOT_SETUP_START);
      GError *info_error = FALSE;
      if (setup_session_info(start, &info_error) == FALSE)
	{
	  if (error == NULL)
	    g_propagate_error(error, info_error);
	}
    }

  if (*error)
    return FALSE;
  else
    return TRUE;
}

bool
SbuildChrootLvmSnapshot::setup_session_info (gboolean   start,
					     GError   **error)
{
  /* Create or unlink session information. */
  char *file = g_strconcat(SCHROOT_SESSION_DIR, "/",
			   get_name().c_str(),
			   NULL);
  FILE *sess_file = NULL;

  if (start)
    {
      int fd = open(file, O_CREAT|O_EXCL|O_WRONLY, 0664);
      if (fd < 0)
	{
	  g_set_error(error,
		      SBUILD_CHROOT_ERROR, SBUILD_CHROOT_ERROR_LOCK,
		      _("%s: failed to create session file: %s\n"),
		      file, g_strerror(errno));
	}
      else
	{
	  FILE *sess_file = fdopen(fd, "w");
	  if (sess_file == NULL)
	    {
	      g_set_error(error,
			  SBUILD_CHROOT_ERROR, SBUILD_CHROOT_ERROR_LOCK,
			  _("%s: failed to create FILE from fd: %s\n"),
			  file, g_strerror(errno));
	      if (close(fd) < 0) /* Can't set GError at this point. */
		    g_printerr("%s: failed to close session file: %s\n",
			       file, g_strerror(errno));
	    }
	  else
	    {
	      GError *lock_error = NULL;
	      sbuild::FileLock lock(fd);
	      lock.set_lock(SBUILD_LOCK_EXCLUSIVE, 2, &lock_error);
	      if (lock_error)
		{
		  g_set_error(error,
			      SBUILD_CHROOT_ERROR, SBUILD_CHROOT_ERROR_LOCK,
			      _("%s: lock acquisition failure: %s\n"),
			  file, lock_error->message);
		  g_error_free(lock_error);
		}
	      else
		{
		  print_config(sess_file);
		  /* 		  if (fflush(sess_file) != 0) */
		  /* 		    g_set_error(error, */
		  /* 				SBUILD_CHROOT_ERROR, SBUILD_CHROOT_ERROR_LOCK, */
		  /* 				_("%s: failed to flush session file: %s\n"), */
		  /* 				file, g_strerror(errno)); */

		  GError *unlock_error = NULL;
		  lock.unset_lock(&unlock_error);
		  if (unlock_error)
		    {
		      if (error == NULL)
			g_set_error(error,
				    SBUILD_CHROOT_ERROR, SBUILD_CHROOT_ERROR_LOCK,
				    _("%s: lock discard failure: %s\n"),
				    file, unlock_error->message);
		      g_error_free(unlock_error);
		    }
		}
	      if (sess_file && fclose(sess_file) != 0)
		{
		  if (error == NULL)
		    g_set_error(error,
				SBUILD_CHROOT_ERROR, SBUILD_CHROOT_ERROR_LOCK,
				_("%s: failed to close session file: %s\n"),
				file, g_strerror(errno));
		}
	    }
	}
    }
  else /* start == FALSE */
    {
      if (unlink(file) != 0)
	{
	  g_set_error(error,
		      SBUILD_CHROOT_ERROR, SBUILD_CHROOT_ERROR_LOCK,
		      _("%s: failed to unlink session file: %s\n"),
		      file, g_strerror(errno));
	}
    }

  g_free(file);

  if (*error)
    return FALSE;
  else
    return TRUE;
}

SbuildChrootSessionFlags
SbuildChrootLvmSnapshot::get_session_flags () const
{
  return SBUILD_CHROOT_SESSION_CREATE;
}

void
SbuildChrootLvmSnapshot::print_details (FILE *file) const
{
  this->SbuildChrootBlockDevice::print_details(file);

  if (!this->snapshot_device.empty())
    g_fprintf(file, "  %-22s%s\n", _("LVM Snapshot Device"),
	      this->snapshot_device.c_str());
  if (!this->snapshot_options.empty())
    g_fprintf(file, "  %-22s%s\n", _("LVM Snapshot Options"),
	      this->snapshot_options.c_str());
}

void
SbuildChrootLvmSnapshot::print_config (FILE *file) const
{
  this->SbuildChrootBlockDevice::print_config(file);

  if (!this->snapshot_device.empty())
    g_fprintf(file, _("lvm-snapshot-device=%s\n"), this->snapshot_device.c_str());
  if (!this->snapshot_options.empty())
    g_fprintf(file, _("lvm-snapshot-options=%s\n"), this->snapshot_options.c_str());
}

void
SbuildChrootLvmSnapshot::read_keyfile (GKeyFile   *keyfile,
				       const std::string& group)
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
