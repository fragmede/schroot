/* sbuild-config - sbuild config object
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

#ifndef SBUILD_CONFIG_H
#define SBUILD_CONFIG_H

#include <map>
#include <vector>
#include <string>

#include <glib.h>
#include <glib/gprintf.h>
#include <glib-object.h>

#include "sbuild-chroot.h"

typedef enum
{
  SBUILD_CONFIG_FILE_ERROR_STAT_FAIL,
  SBUILD_CONFIG_FILE_ERROR_OWNERSHIP,
  SBUILD_CONFIG_FILE_ERROR_PERMISSIONS,
  SBUILD_CONFIG_FILE_ERROR_NOT_REGULAR
} SbuildConfigFileError;

#define SBUILD_CONFIG_FILE_ERROR sbuild_config_file_error_quark()

GQuark
sbuild_config_file_error_quark (void);

class SbuildConfig
{
public:
  typedef std::vector<std::string> string_list;
  typedef std::vector<SbuildChroot *> chroot_list;
  typedef std::map<std::string, std::string> string_map;
  typedef std::map<std::string, SbuildChroot *> chroot_map;

  SbuildConfig();
  SbuildConfig(const std::string& file);
  virtual ~SbuildConfig();

  void
  add_config_file (const std::string& file);

  void
  add_config_directory (const std::string& dir);

  chroot_list
  get_chroots () const;

  const SbuildChroot *
  find_chroot (const std::string& name) const;

  const SbuildChroot *
  find_alias (const std::string& name) const;

  string_list
  get_chroot_list () const;

  void
  print_chroot_list (FILE *file) const;

  void
  print_chroot_info (const string_list& chroots,
		     FILE          *file) const;

  string_list
  validate_chroots(const string_list& chroots) const;

private:
  bool
  check_security(int      fd,
		 GError **error) const;

  void
  load (const std::string& file);

  chroot_map chroots;
  string_map aliases;
};

#endif /* SBUILD_CONFIG_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
