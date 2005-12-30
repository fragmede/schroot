/* Copyright © 2005  Roger Leigh <rleigh@debian.org>
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

#include <iostream>

#include <ext/stdio_filebuf.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include <boost/format.hpp>

#include "sbuild-i18n.h"
#include "sbuild-config.h"
#include "sbuild-lock.h"
#include "sbuild-log.h"
#include "sbuild-chroot.h"
#include "sbuild-keyfile.h"
#include "sbuild-util.h"

using std::endl;
using boost::format;
using namespace sbuild;

Config::Config():
  chroots()
{
}

Config::Config(const std::string& file):
  chroots()
{
  struct stat statbuf;
  if (stat(file.c_str(), &statbuf) < 0)
    {
      format fmt(_("%1%: failed to stat file: %2%"));
      fmt % file % strerror(errno);
      throw error(fmt);
    }

  if (S_ISDIR(statbuf.st_mode))
    add_config_file(file);
  else
    add_config_directory(file);
}

Config::~Config()
{
  // TODO: Delete chroots.
}

void
Config::add_config_file (const std::string& file)
{
  load(file);
}

void
Config::add_config_directory (const std::string& dir)
{
  if (dir.empty())
    return;

  DIR *d = opendir(dir.c_str());
  if (d == NULL)
    {
      log_error() << format(_("%1%: failed to open directory: %2%"))
	% dir % strerror(errno)
		  << endl;
      exit (EXIT_FAILURE);
    }

  struct dirent *de = NULL;
  while ((de = readdir(d)) != NULL)
    {
      std::string filename = dir + "/" + de->d_name;

      struct stat statbuf;
      if (stat(filename.c_str(), &statbuf) < 0)
	{
	  log_error() << format(_("%1%: failed to stat file: %2%"))
	    % filename % strerror(errno)
		      << endl;
	  continue;
	}

      if (!S_ISREG(statbuf.st_mode))
	{
	  if (!(strcmp(de->d_name, ".") == 0 ||
		strcmp(de->d_name, "..") == 0))
	    log_error() << format(_("%1%: not a regular file")) % filename
			<< endl;
	  continue;
	}

      load(filename);
    }
}

static bool chroot_alphasort (Chroot::chroot_ptr const& c1,
			      Chroot::chroot_ptr const& c2)
{
  return c1->get_name() < c2->get_name();
}

Config::chroot_list
Config::get_chroots () const
{
  chroot_list ret;

  for (chroot_map::const_iterator pos = this->chroots.begin();
       pos != this->chroots.end();
       ++pos)
    ret.push_back(pos->second);

  std::sort(ret.begin(), ret.end(), chroot_alphasort);

  return ret;
}

const Chroot::chroot_ptr
Config::find_chroot (const std::string& name) const
{
  chroot_map::const_iterator pos = this->chroots.find(name);

  if (pos != this->chroots.end())
    return pos->second;
  else
    {
      Chroot *null_chroot = 0;
      return Chroot::chroot_ptr(null_chroot);
    }
}

const Chroot::chroot_ptr
Config::find_alias (const std::string& name) const
{
  string_map::const_iterator pos = this->aliases.find(name);

  if (pos != this->aliases.end())
    return find_chroot(pos->second);
  else
    {
      Chroot *null_chroot = 0;
      return Chroot::chroot_ptr(null_chroot);
    }
}

string_list
Config::get_chroot_list () const
{
  string_list ret;

  for (string_map::const_iterator pos = this->aliases.begin();
       pos != this->aliases.end();
       ++pos)
    ret.push_back(pos->first);

  std::sort(ret.begin(), ret.end());

  return ret;
}

void
Config::print_chroot_list (std::ostream& stream) const
{
  string_list chroots = get_chroot_list();

  for (string_list::const_iterator pos = chroots.begin();
       pos != chroots.end();
       ++pos)
    stream << *pos << "\n";
  stream << std::flush;
}

void
Config::print_chroot_info (const string_list& chroots,
			   std::ostream&      stream) const
{
  for (string_list::const_iterator pos = chroots.begin();
       pos != chroots.end();
       ++pos)
    {
      const Chroot::chroot_ptr chroot = find_alias(*pos);
      if (chroot)
	{
	  chroot->print_details(stream);
	  if (pos + 1 != chroots.end())
	    stream << '\n';
	}
      else
	log_error() << format(_("%1%: No such chroot")) % *pos
		    << endl;
    }
}

string_list
Config::validate_chroots(const string_list& chroots) const
{
  string_list bad_chroots;

  for (string_list::const_iterator pos = chroots.begin();
       pos != chroots.end();
       ++pos)
    {
      const Chroot::chroot_ptr chroot = find_alias(*pos);
      if (chroot)
	bad_chroots.push_back(*pos);
    }

  return bad_chroots;
}

void
Config::check_security(int fd) const
{
  struct stat statbuf;
  if (fstat(fd, &statbuf) < 0)
    {
      format fmt(_("failed to stat file: %1%"));
      fmt % strerror(errno);
      throw error(fmt);
    }

  if (statbuf.st_uid != 0)
    {
      throw error(_("not owned by user root"));
    }

  if (statbuf.st_mode & S_IWOTH)
    {
      throw error(_("others have write permission: "));
    }

  if (!S_ISREG(statbuf.st_mode))
    {
      throw error(_("not a regular file: "));
    }
}

/*
 * sbuild_config_load:
 * @file: the file to load.
 * @list: a list to append the #Chroot objects to.
 *
 * Load a configuration file.  If there are problems with the
 * configuration file, the program will be aborted immediately.
 */
void
Config::load (const std::string& file)
{
  // TODO: Move error handling out to top level.
  /* Use a UNIX fd, for security (no races) */
  int fd = open(file.c_str(), O_RDONLY|O_NOFOLLOW);
  if (fd < 0)
    {
      log_error() << format(_("%1%: failed to load configuration: %2%"))
	% file % strerror(errno)
		  << endl;
      exit (EXIT_FAILURE);
    }

  sbuild::FileLock lock(fd);
  try
    {
      lock.set_lock(Lock::LOCK_SHARED, 2);
    }
  catch (const Lock::error& e)
    {
      log_error() << format(_("%1%: lock acquisition failure: %2%"))
	% file % e.what()
		  << endl;
      exit (EXIT_FAILURE);
    }

  try
    {
      check_security(fd);
    }
  catch (const error &e)
    {
      log_error() << format(_("%1%: security failure: %2%"))
	% file % e.what()
		  << endl;
      exit (EXIT_FAILURE);
    }

  /* Now create an IO Channel and read in the data */
  __gnu_cxx::stdio_filebuf<char> fdbuf(fd, std::ios::in);
  std::istream input(&fdbuf);

  /* Create key file */
  keyfile kconfig(input);
  // TODO: Where should parse error exception be handled?

  try
    {
      lock.unset_lock();
    }
  catch (const Lock::error& e)
    {
      log_error() << format(_("%1%: lock discard failure: %2%"))
	% file % e.what()
		  << endl;
      exit (EXIT_FAILURE);
    }

  /* Create Chroot objects from key file */
  string_list const& groups = kconfig.get_groups();
  for (string_list::const_iterator group = groups.begin();
       group != groups.end();
       ++group)
    {
      // TODO: Check if creation fails, and refuse to insert.
      Chroot::chroot_ptr chroot = Chroot::create(kconfig, *group);

      // TODO: Catch exception rather than null ptr check.
      if (chroot)
	{
	  // TODO: error checking (did insertion work? was the alias a
	  // duplicate?
	  this->chroots.insert(std::make_pair(chroot->get_name(), chroot));
	  this->aliases.insert(std::make_pair(chroot->get_name(), chroot->get_name()));
	  const string_list& aliases = chroot->get_aliases();
	  for (string_list::const_iterator pos = aliases.begin();
	       pos != aliases.end();
	       ++pos)
	    this->aliases.insert(std::make_pair(*pos, chroot->get_name()));
	}
      else
	{
	  log_warning()
	    << format(_("%1% chroot: Could not create chroot")) % *group
	    << endl;
	}
    }
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
