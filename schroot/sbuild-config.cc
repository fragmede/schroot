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

/**
 * SECTION:sbuild-config
 * @short_description: config object
 * @title: SbuildConfig
 *
 * This class holds the configuration details from the configuration
 * file.  Conceptually, it's an opaque container of #SbuildChroot
 * objects.
 *
 * Methods are provided to query the available chroots and find
 * specific chroots.
 */

#include <config.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "sbuild-config.h"
#include "sbuild-lock.h"
#include "sbuild-chroot.h"
#include "sbuild-chroot-plain.h"
#include "sbuild-chroot-block-device.h"
#include "sbuild-chroot-lvm-snapshot.h"

/**
 * sbuild_config_file_error_quark:
 *
 * Get the SBUILD_CONFIG_FILE_ERROR domain number.
 *
 * Returns the domain.
 */
GQuark
sbuild_config_file_error_quark (void)
{
  static GQuark error_quark = 0;

  if (error_quark == 0)
    error_quark = g_quark_from_static_string ("sbuild-config-file-error-quark");

  return error_quark;
}


SbuildConfig::SbuildConfig():
  chroots()
{
}

SbuildConfig::SbuildConfig(const std::string& file):
  chroots()
{
  struct stat statbuf;
  if (stat(file.c_str(), &statbuf) < 0)
    {
      /* TODO: throw exception or print warning. */
      return;
    }

  if (S_ISDIR(statbuf.st_mode))
    add_config_file(file);
  else
    add_config_directory(file);
}

SbuildConfig::~SbuildConfig()
{
  // TODO: Delete chroots.
}

/**
 * sbuild_config_add_config_file:
 * @config: an #SbuildConfig.
 * @file: the filename to add.
 *
 * Add the configuration filename.  The configuration file specified
 * will be loaded.
 */
void
SbuildConfig::add_config_file (const std::string& file)
{
  load(file);
}

/**
 * sbuild_config_add_config_directory:
 * @config: an #SbuildConfig.
 * @dir: the directory to add.
 *
 * Add the configuration directory.  The configuration files in the
 * directory will be loaded.
 */
void
SbuildConfig::add_config_directory (const std::string& dir)
{
  if (dir.empty())
    return;

  DIR *d = opendir(dir.c_str());
  if (d == NULL)
    {
      g_printerr(_("%s: failed to open directory: %s\n"), dir.c_str(), g_strerror(errno));
      exit (EXIT_FAILURE);
    }

  struct dirent *de = NULL;
  while ((de = readdir(d)) != NULL)
    {
      char *filename = g_strconcat(dir.c_str(), "/", &de->d_name[0], NULL);

      struct stat statbuf;
      if (stat(filename, &statbuf) < 0)
	{
	  g_printerr(_("%s: failed to stat file: %s"), filename, g_strerror(errno));
	  g_free(filename);
	  continue;
	}

      if (!S_ISREG(statbuf.st_mode))
	{
	  if (!(strcmp(de->d_name, ".") == 0 ||
		strcmp(de->d_name, "..") == 0))
	    g_printerr(_("%s: failed to stat file: %s"), filename, g_strerror(errno));
	  g_free(filename);
	  continue;
	}

      load(filename);
      g_free(filename);
    }
}

static bool chroot_alphasort (SbuildChroot* const& c1,
			      SbuildChroot* const& c2)
{
  return c1->get_name() < c2->get_name();
}

/**
 * sbuild_config_get_chroots:
 * @config: a #SbuildConfig
 *
 * Get a list of available chroots.
 *
 * Returns a list of available chroots, or NULL if no chroots are
 * available.
 */
SbuildConfig::chroot_list
SbuildConfig::get_chroots () const
{
  chroot_list ret;

  for (chroot_map::const_iterator pos = this->chroots.begin();
       pos != this->chroots.end();
       ++pos)
    ret.push_back(pos->second);

  std::sort(ret.begin(), ret.end(), chroot_alphasort);

  return ret;
}

/**
 * sbuild_config_find_chroot:
 * @config: an #SbuildConfig
 * @name: the chroot name
 *
 * Find a chroot by its name.
 *
 * Returns the chroot if found, otherwise NULL.
 */
const SbuildChroot *
SbuildConfig::find_chroot (const std::string& name) const
{
  chroot_map::const_iterator pos = this->chroots.find(name);

  if (pos != this->chroots.end())
    return pos->second;
  else
    return 0;
}

/**
 * sbuild_config_find_alias:
 * @config: an #SbuildConfig
 * @name: the chroot name
 *
 * Find a chroot by its name or an alias.
 *
 * Returns the chroot if found, otherwise NULL.
 */
const SbuildChroot *
SbuildConfig::find_alias (const std::string& name) const
{
  string_map::const_iterator pos = this->aliases.find(name);

  if (pos != this->aliases.end())
    return find_chroot(pos->second);
  else
    return 0;
}

/**
 * sbuild_config_get_chroot_list:
 * @config: an #SbuildConfig
 *
 * Get the names (including aliases) of all the available chroots.
 *
 * Returns the list, or NULL if no chroots are available.
 */
SbuildConfig::string_list
SbuildConfig::get_chroot_list () const
{
  string_list ret;

  for (string_map::const_iterator pos = this->aliases.begin();
       pos != this->aliases.end();
       ++pos)
    ret.push_back(pos->first);

  std::sort(ret.begin(), ret.end());

  return ret;
}

/**
 * sbuild_config_print_chroot_list:
 * @config: an #SbuildConfig
 * @file: the file to print to
 *
 * Print all the available chroots to the specified file.
 */
void
SbuildConfig::print_chroot_list (FILE *file) const
{
  string_list chroots = get_chroot_list();

  for (string_list::const_iterator pos = chroots.begin();
       pos != chroots.end();
       ++pos)
      g_print("%s\n", pos->c_str());
}

/**
 * sbuild_config_print_chroot_info:
 * @config: an #SbuildConfig
 * @chroots: the chroots to print
 * @file: the file to print to
 *
 * Print information about the specified chroots to the specified
 * file.
 */
void
SbuildConfig::print_chroot_info (const string_list& chroots,
				 FILE          *file) const
{
  for (string_list::const_iterator pos = chroots.begin();
       pos != chroots.end();
       ++pos)
    {
      const SbuildChroot *chroot = find_alias(*pos);
      if (chroot)
	{
	  chroot->print_details(file);
	  if (pos + 1 != chroots.end())
	    g_fprintf(stdout, "\n");
	}
      else
	g_printerr(_("%s: No such chroot\n"), pos->c_str());
    }
}

/**
 * sbuild_config_validate_chroots:
 * @config: an #SbuildConfig
 * @chroots: the chroots to validate
 *
 * Check that all the chroots specified by @chroots exist in @config.
 *
 * Returns NULL if all chroots are valid, or else a vector of invalid
 * chroots.
 */
SbuildConfig::string_list
SbuildConfig::validate_chroots(const string_list& chroots) const
{
  string_list bad_chroots;

  for (string_list::const_iterator pos = chroots.begin();
       pos != chroots.end();
       ++pos)
    {
      const SbuildChroot *chroot = find_alias(*pos);
      if (chroot == 0)
	bad_chroots.push_back(*pos);
    }
}

/**
 * sbuild_config_check_security:
 * @fd: the file descriptor to check.
 * @error: the #GError to report errors.
 *
 * Check the permissions and ownership of the configuration file.  The
 * file must be owned by root, not writable by other, and be a regular
 * file.
 *
 * Returns TRUE if the checks succeed, FALSE on failure.
 */
bool
SbuildConfig::check_security(int      fd,
			     GError **error) const
{
  struct stat statbuf;
  if (fstat(fd, &statbuf) < 0)
    {
      g_set_error(error,
		  SBUILD_CONFIG_FILE_ERROR, SBUILD_CONFIG_FILE_ERROR_STAT_FAIL,
		  _("failed to stat file: %s"), g_strerror(errno));
      return FALSE;
    }

  if (statbuf.st_uid != 0)
    {
      g_set_error(error,
		  SBUILD_CONFIG_FILE_ERROR, SBUILD_CONFIG_FILE_ERROR_OWNERSHIP,
		  _("not owned by user root"));
      return FALSE;
    }

  if (statbuf.st_mode & S_IWOTH)
    {
      g_set_error(error,
		  SBUILD_CONFIG_FILE_ERROR, SBUILD_CONFIG_FILE_ERROR_PERMISSIONS,
		  _("others have write permission"));
      return FALSE;
    }

  if (!S_ISREG(statbuf.st_mode))
    {
      g_set_error(error,
		  SBUILD_CONFIG_FILE_ERROR, SBUILD_CONFIG_FILE_ERROR_NOT_REGULAR,
		  _("not a regular file"));
      return FALSE;
    }

  return TRUE;
}

/**
 * sbuild_config_load:
 * @file: the file to load.
 * @list: a list to append the #SbuildChroot objects to.
 *
 * Load a configuration file.  If there are problems with the
 * configuration file, the program will be aborted immediately.
 */
void
SbuildConfig::load (const std::string& file)
{

  /* Use a UNIX fd, for security (no races) */
  int fd = open(file.c_str(), O_RDONLY|O_NOFOLLOW);
  if (fd < 0)
    {
      g_printerr(_("%s: failed to load configuration: %s\n"), file.c_str(), g_strerror(errno));
      exit (EXIT_FAILURE);
    }

  GError *lock_error = NULL;
  sbuild::FileLock lock(fd);
  lock.set_lock(SBUILD_LOCK_SHARED, 2, &lock_error);
  if (lock_error)
    {
      g_printerr(_("%s: lock acquisition failure: %s\n"), file.c_str(), lock_error->message);
      exit (EXIT_FAILURE);
    }

  GError *security_error = NULL;
  check_security(fd, &security_error);
  if (security_error)
    {
      g_printerr(_("%s: security failure: %s\n"), file.c_str(), security_error->message);
      exit (EXIT_FAILURE);
    }

  /* Now create an IO Channel and read in the data */
  GIOChannel *channel = g_io_channel_unix_new(fd);
  gchar *data = NULL;
  gsize size = 0;
  GError *read_error = NULL;

  g_io_channel_set_encoding(channel, NULL, NULL);
  g_io_channel_read_to_end(channel, &data, &size, &read_error);
  if (read_error)
    {
      g_printerr(_("%s: read failure: %s\n"), file.c_str(), read_error->message);
      exit (EXIT_FAILURE);
    }

  GError *unlock_error = NULL;
  lock.unset_lock(&unlock_error);
  if (unlock_error)
    {
      g_printerr(_("%s: lock discard failure: %s\n"), file.c_str(), unlock_error->message);
      exit (EXIT_FAILURE);
    }

  GError *close_error = NULL;
  g_io_channel_shutdown(channel, FALSE, &close_error);
  if (close_error)
    {
      g_printerr(_("%s: close failure: %s\n"), file.c_str(), close_error->message);
      exit (EXIT_FAILURE);
    }
  g_io_channel_unref(channel);

  /* Create key file */
  GKeyFile *keyfile = g_key_file_new();
  g_key_file_set_list_separator(keyfile, ',');
  GError *parse_error = NULL;
  g_key_file_load_from_data(keyfile, data, size, G_KEY_FILE_NONE, &parse_error);
  g_free(data);
  data = NULL;

  if (parse_error)
    {
      g_printerr(_("%s: parse failure: %s\n"), file.c_str(), parse_error->message);
      exit (EXIT_FAILURE);
    }

  /* Create SbuildChroot objects from key file */
  char **groups = g_key_file_get_groups(keyfile, NULL);
  for (guint i=0; groups[i] != NULL; ++i)
    {
      /* TODO: Can't cope with error.  Replace ASAP. */
      std::string type =
	g_key_file_get_string(keyfile, groups[i], "type", 0);
      SbuildChroot *chroot = 0;
      if (type == "plain")
	chroot = new SbuildChrootPlain(keyfile, groups[i]);
      else if (type == "block-device")
	chroot = new SbuildChrootBlockDevice(keyfile, groups[i]);
      else if (type == "lvm-snapshot")
	chroot = new SbuildChrootLvmSnapshot(keyfile, groups[i]);
      if (chroot)
	{
	  // TODO: error checking (did insertion work? was the alias a
	  // duplicate?
	  this->chroots.insert(std::make_pair(chroot->get_name(), chroot));
	  const SbuildChroot::string_list& aliases = chroot->get_aliases();
	  for (SbuildChroot::string_list::const_iterator pos = aliases.begin();
	       pos != aliases.end();
	       ++pos)
	    this->aliases.insert(std::make_pair(*pos, chroot->get_name()));
	}
    }
  g_strfreev(groups);
  g_key_file_free(keyfile);
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
