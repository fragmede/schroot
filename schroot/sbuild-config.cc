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

using std::endl;
using boost::format;
using namespace sbuild;

Config::Config():
  chroots()
{
}

Config::Config(std::string const& file,
	       bool               active):
  chroots()
{
  struct stat statbuf;
  if (stat(file.c_str(), &statbuf) == 0 && S_ISDIR(statbuf.st_mode))
    add_config_directory(file, active);
  else
    add_config_file(file, active);
}

Config::~Config()
{
}

void
Config::add_config_file (std::string const& file,
			 bool               active)
{
  load(file, active);
}

void
Config::add_config_directory (std::string const& dir,
			      bool               active)
{
  if (dir.empty())
    return;

  DIR *d = opendir(dir.c_str());
  if (d == NULL)
    {
      format fmt(_("%1%: failed to open directory: %2%"));
      fmt % dir % strerror(errno);
      throw error(fmt);
    }

  struct dirent *de = NULL;
  while ((de = readdir(d)) != NULL)
    {
      std::string filename = dir + "/" + de->d_name;

      struct stat statbuf;
      if (stat(filename.c_str(), &statbuf) < 0)
	{
	  log_warning() << format(_("%1%: failed to stat file: %2%"))
	    % filename % strerror(errno)
		      << endl;
	  continue;
	}

      if (!S_ISREG(statbuf.st_mode))
	{
	  if (!(strcmp(de->d_name, ".") == 0 ||
		strcmp(de->d_name, "..") == 0))
	    log_warning() << format(_("%1%: not a regular file")) % filename
			  << endl;
	  continue;
	}

      load(filename, active);
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
Config::find_chroot (std::string const& name) const
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
Config::find_alias (std::string const& name) const
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
Config::print_chroot_info (string_list const& chroots,
			   std::ostream&      stream) const
{
  for (string_list::const_iterator pos = chroots.begin();
       pos != chroots.end();
       ++pos)
    {
      const Chroot::chroot_ptr chroot = find_alias(*pos);
      if (chroot)
	{
	  stream << chroot;
	  if (pos + 1 != chroots.end())
	    stream << '\n';
	}
      else
	log_error() << format(_("%1%: No such chroot")) % *pos
		    << endl;
    }
}

void
Config::print_chroot_config (string_list const& chroots,
			     std::ostream&      stream) const
{
  keyfile info;

  for (string_list::const_iterator pos = chroots.begin();
       pos != chroots.end();
       ++pos)
    {
      const Chroot::chroot_ptr chroot = find_alias(*pos);
      if (chroot)
	{
	  info << chroot;
	}
      else
	log_error() << format(_("%1%: No such chroot")) % *pos
		    << endl;
    }

  stream << info;
}

string_list
Config::validate_chroots(string_list const& chroots) const
{
  string_list bad_chroots;

  for (string_list::const_iterator pos = chroots.begin();
       pos != chroots.end();
       ++pos)
    {
      const Chroot::chroot_ptr chroot = find_alias(*pos);
      if (!chroot)
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

void
Config::load (std::string const& file,
	      bool               active)
{
  /* Use a UNIX fd, for security (no races) */
  int fd = open(file.c_str(), O_RDONLY|O_NOFOLLOW);
  if (fd < 0)
    {
      format fmt(_("%1%: failed to load configuration: %2%"));
      fmt % file % strerror(errno);
      throw error(fmt);
    }

  sbuild::FileLock lock(fd);
  try
    {
      lock.set_lock(Lock::LOCK_SHARED, 2);
    }
  catch (Lock::error const& e)
    {
      format fmt(_("%1%: lock acquisition failure: %2%"));
      fmt % file % e.what();
      throw error(fmt);
    }

  try
    {
      check_security(fd);
    }
  catch (error const& e)
    {
      format fmt(_("%1%: security failure: %2%"));
      fmt % file % e.what();
      throw error(fmt);
    }

  /* Now create an IO Channel and read in the data */
  __gnu_cxx::stdio_filebuf<char> fdbuf(fd, std::ios::in);
  std::istream input(&fdbuf);
  input.imbue(std::locale("C"));

  /* Create key file */
  keyfile kconfig(input);

  try
    {
      lock.unset_lock();
    }
  catch (Lock::error const& e)
    {
      format fmt(_("%1%: lock discard failure: %2%"));
      fmt % file % e.what();
      throw error(fmt);
    }

  /* Create Chroot objects from key file */
  string_list const& groups = kconfig.get_groups();
  for (string_list::const_iterator group = groups.begin();
       group != groups.end();
       ++group)
    {
      try
	{
	  // Set the active property for chroot creation, and create
	  // the chroot.
	  kconfig.set_value(*group, "active", active);
	  std::string type = "plain"; // "plain" is the default type.
	  kconfig.get_value(*group, "type", type);
	  Chroot::chroot_ptr chroot = Chroot::create(type);
	  chroot->set_name(*group);
	  kconfig >> chroot;

	  // Make sure insertion will succeed.
	  if (this->chroots.find(chroot->get_name()) == this->chroots.end() &&
	      this->aliases.find(chroot->get_name()) == this->aliases.end())
	    {
	      // Set up chroot.
	      this->chroots.insert(std::make_pair(chroot->get_name(), chroot));
	      this->aliases.insert(std::make_pair(chroot->get_name(),
						  chroot->get_name()));

	      // Set up aliases.
	      string_list const& aliases = chroot->get_aliases();
	      for (string_list::const_iterator pos = aliases.begin();
		   pos != aliases.end();
		   ++pos)
		{
		  if (this->aliases.insert
		      (std::make_pair(*pos, chroot->get_name()))
		      .second == false)
		    {
		      string_map::const_iterator dup = this->aliases.find(*pos);
		      if (dup != this->aliases.end())
			log_warning() <<
			  format(_("%1% chroot: "
				   "alias '%2%' already associated with "
				   "'%3%' chroot"))
			  % chroot->get_name() % dup->first % dup->second
				      << endl;
		      else
			log_warning() <<
			  format(_("%1% chroot: "
				   "alias '%2%' already associated with "
				   "another chroot"))
			  % chroot->get_name() % *pos
				      << endl;

		    }
		}
	    }
	  else
	    {
	      log_warning() << format(_("%1% chroot: a chroot or alias already exists by this name"))
		% chroot->get_name()
			    << endl;
	      log_warning() << format(_("%1% chroot: duplicate names are not allowed"))
		% chroot->get_name()
			    << endl;
	    }
	}
      catch (sbuild::runtime_error const& e)
	{
	  log_warning() << e.what() << endl;
	}
    }
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
