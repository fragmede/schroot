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

#ifndef SBUILD_CHROOT_LVM_SNAPSHOT_H
#define SBUILD_CHROOT_LVM_SNAPSHOT_H

#include <glib.h>
#include <glib/gprintf.h>

#include "sbuild-chroot-block-device.h"


class SbuildChrootLvmSnapshot : public SbuildChrootBlockDevice
{
public:
  SbuildChrootLvmSnapshot();
  SbuildChrootLvmSnapshot (GKeyFile   *keyfile,
			   const std::string& group);
  virtual ~SbuildChrootLvmSnapshot();

  virtual SbuildChroot *
  clone () const;

  const std::string&
  get_snapshot_device () const;

  void
  set_snapshot_device (const std::string& snapshot_device);

  virtual const std::string&
  get_mount_device () const;

  const std::string&
  get_snapshot_options () const;

  void
  set_snapshot_options (const std::string& snapshot_options);

  virtual const std::string&
  get_chroot_type () const;

  virtual void
  setup_env (env_list& env);

  virtual bool
  setup_lock (SbuildChrootSetupType   type,
	      gboolean                lock,
	      GError                **error);

  virtual SbuildChrootSessionFlags
  get_session_flags () const;

  virtual void
  print_details (FILE *file) const;

  virtual void
  print_config (FILE *file) const;

private:
  bool
  setup_session_info (gboolean   start,
		      GError   **error);

  std::string snapshot_device;
  std::string snapshot_options;
};

#endif /* SBUILD_CHROOT_LVM_SNAPSHOT_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
