/* sbuild-chroot - sbuild chroot object
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

#ifndef SBUILD_CHROOT_H
#define SBUILD_CHROOT_H

#include <string>
#include <sstream>
#include <vector>

#include <glib.h>
#include <glib/gprintf.h>
#include <glib-object.h>

typedef enum
{
  SBUILD_CHROOT_ERROR_LOCK
} SbuildChrootError;

#define SBUILD_CHROOT_ERROR sbuild_chroot_error_quark()

GQuark
sbuild_chroot_error_quark (void);

typedef enum
{
  SBUILD_CHROOT_SETUP_START,
  SBUILD_CHROOT_SETUP_RECOVER,
  SBUILD_CHROOT_SETUP_STOP,
  SBUILD_CHROOT_RUN_START,
  SBUILD_CHROOT_RUN_STOP
} SbuildChrootSetupType;

typedef enum
{
  SBUILD_CHROOT_SESSION_CREATE = 1 << 0
} SbuildChrootSessionFlags;

class SbuildChroot
{
public:
  typedef std::vector<std::string> string_list;
  typedef std::pair<std::string,std::string> env;
  typedef std::vector<env> env_list;

  SbuildChroot ();

  SbuildChroot (GKeyFile   *keyfile,
		const std::string& group);

  virtual ~SbuildChroot();

  virtual SbuildChroot *
  clone () const = 0;

  const std::string&
  get_name () const;

  void
  set_name (const std::string& name);

  const std::string&
  get_description () const;

  void
  set_description (const std::string& description);

  virtual const std::string&
  get_mount_location () const;

  void
  set_mount_location (const std::string& location);

  virtual const std::string&
  get_mount_device () const;

  void
  set_mount_device (const std::string& device);

  unsigned int
  get_priority () const;

  void
  set_priority (unsigned int priority);

  const string_list&
  get_groups () const;

  void
  set_groups (const string_list& groups);

  const string_list&
  get_root_groups () const;

  void
  set_root_groups (const string_list& groups);

  const string_list&
  get_aliases () const;

  void
  set_aliases (const string_list& aliases);

  bool
  get_active () const;

  void
  set_active (bool active);

  bool
  get_run_setup_scripts () const;

  void
  set_run_setup_scripts (bool run_setup_scripts);

  bool
  get_run_session_scripts () const;

  void
  set_run_session_scripts (bool run_session_scripts);

  virtual const std::string&
  get_chroot_type () const = 0;

  virtual void
  setup_env (env_list& env);

  /**
   * sbuild_chroot_setup_lock:
   * @chroot: an #SbuildChroot
   * @type: the type of setup being performed
   * @lock: TRUE to lock, FALSE to unlock
   * @error: a #GError.
   *
   * Lock a chroot during setup.  The locking technique (if any) may
   * vary depending upon the chroot type and setup stage.  For example,
   * during creation a block device might require locking, but
   * afterwards this will change to the new block device or directory.
   *
   * Returns TRUE on success, FALSE on failure.
   */
  virtual bool
  setup_lock (SbuildChrootSetupType   type,
	      gboolean                lock,
	      GError                **error) = 0;

  /**
   * sbuild_chroot_get_session_flags:
   * @chroot: an #SbuildChroot
   *
   * Get the session flags of the chroot.  These determine how the
   * #SbuildSession controlling the chroot will operate.
   *
   * Returns the #SbuildChrootSessionFlags.
   */
  virtual SbuildChrootSessionFlags
  get_session_flags () const = 0;

  virtual void
  print_details (FILE *file) const;

  virtual void
  print_config (FILE *file) const;

protected:
  void
  read_keyfile (GKeyFile   *keyfile,
		const std::string& group);

private:
  std::string   name;
  std::string   description;
  unsigned int  priority;
  string_list   groups;
  string_list   root_groups;
  string_list   aliases;
  std::string   mount_location;
  std::string   mount_device;
  bool          active;
  bool          run_setup_scripts;
  bool          run_session_scripts;
};

// TODO: move into separate environment class.
template<typename T>
void
setup_env_var(SbuildChroot::env_list& env,
	      const std::string& name,
	      const T& var)
{
  std::ostringstream varstring;
  varstring << var;
  env.push_back(std::make_pair(name, varstring.str()));
}

#endif /* SBUILD_CHROOT_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
