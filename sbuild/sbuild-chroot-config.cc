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

#include "sbuild-chroot.h"
#include "sbuild-chroot-source.h"
#include "sbuild-chroot-config.h"
#include "sbuild-lock.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <ext/stdio_filebuf.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include <boost/format.hpp>

using std::endl;
using boost::format;
using namespace sbuild;

namespace
{

  typedef std::pair<chroot_config::error_code,const char *> emap;

  /**
   * This is a list of the supported error codes.  It's used to
   * construct the real error codes map.
   */
  emap init_errors[] =
    {
      emap(chroot_config::CHROOT,      N_("No such chroot")),
      emap(chroot_config::DIR_OPEN,    N_("Failed to open directory")),
      emap(chroot_config::FILE_NOTREG, N_("File is not a regular file")),
      emap(chroot_config::FILE_OPEN,   N_("Failed to open file")),
      emap(chroot_config::FILE_OWNER,  N_("File is not owned by user root")),
      emap(chroot_config::FILE_PERMS,  N_("File has write permissions for others")),
      emap(chroot_config::FILE_STAT,   N_("Failed to stat file"))
    };

  bool chroot_alphasort (sbuild::chroot::ptr const& c1,
			 sbuild::chroot::ptr const& c2)
  {
    return c1->get_name() < c2->get_name();
  }

}

template<>
custom_error<chroot_config::error_code>::map_type
custom_error<chroot_config::error_code>::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));

chroot_config::chroot_config ():
  chroots()
{
}

chroot_config::chroot_config (std::string const& file,
			      bool               active):
  chroots()
{
  add(file, active);
}

chroot_config::~chroot_config ()
{
}

void
chroot_config::add (std::string const& location,
		    bool               active)
{
  struct stat statbuf;
  if (stat(location.c_str(), &statbuf) == 0 && S_ISDIR(statbuf.st_mode))
    add_config_directory(location, active);
  else
    add_config_file(location, active);
}

void
chroot_config::add_config_file (std::string const& file,
				bool               active)
{
  load_data(file, active);
}

void
chroot_config::add_config_directory (std::string const& dir,
				     bool               active)
{
  if (dir.empty())
    return;

  DIR *d = opendir(dir.c_str());
  if (d == NULL)
    {
      throw error(dir, DIR_OPEN, errno);
    }

  struct dirent *de = NULL;
  while ((de = readdir(d)) != NULL)
    {
      std::string filename = dir + "/" + de->d_name;

      struct stat statbuf;
      if (stat(filename.c_str(), &statbuf) < 0)
	{
	  error e(filename, FILE_STAT, errno);
	  log_warning() << e.what() << endl;
	  continue;
	}

      if (!S_ISREG(statbuf.st_mode))
	{
	  if (!(strcmp(de->d_name, ".") == 0 ||
		strcmp(de->d_name, "..") == 0))
	    {
	      error e (filename, FILE_NOTREG);
	      log_warning() << e.what() << endl;
	    }
	  continue;
	}

      load_data(filename, active);
    }
}

void
chroot_config::add (chroot::ptr& chroot)
{
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
	      // Don't warn if alias is for chroot of same name.
	      if (dup != this->aliases.end())
		{
		  if (chroot->get_name() != dup->first)
		    log_warning() <<
		      format(_("%1% chroot: "
			       "Alias '%2%' already associated with "
			       "'%3%' chroot"))
		      % chroot->get_name() % dup->first % dup->second
				  << endl;
		}
	      else
		log_warning() <<
		  format(_("%1% chroot: "
			   "Alias '%2%' already associated with "
			   "another chroot"))
		  % chroot->get_name() % *pos
			      << endl;
	    }
	}
    }
  else
    {
      log_warning() << format(_("%1% chroot: A chroot or alias already exists by this name"))
	% chroot->get_name()
		    << endl;
      log_warning() << format(_("%1% chroot: Duplicate names are not allowed"))
	% chroot->get_name()
		    << endl;
    }
}

chroot_config::chroot_list
chroot_config::get_chroots () const
{
  chroot_list ret;

  for (chroot_map::const_iterator pos = this->chroots.begin();
       pos != this->chroots.end();
       ++pos)
    ret.push_back(pos->second);

  std::sort(ret.begin(), ret.end(), chroot_alphasort);

  return ret;
}

const sbuild::chroot::ptr
chroot_config::find_chroot (std::string const& name) const
{
  chroot_map::const_iterator pos = this->chroots.find(name);

  if (pos != this->chroots.end())
    return pos->second;
  else
    {
      chroot *null_chroot = 0;
      return chroot::ptr(null_chroot);
    }
}

const sbuild::chroot::ptr
chroot_config::find_alias (std::string const& name) const
{
  string_map::const_iterator pos = this->aliases.find(name);

  if (pos != this->aliases.end())
    return find_chroot(pos->second);
  else
    {
      chroot *null_chroot = 0;
      return chroot::ptr(null_chroot);
    }
}

string_list
chroot_config::get_chroot_list () const
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
chroot_config::print_chroot_list (std::ostream& stream) const
{
  string_list chroots = get_chroot_list();

  for (string_list::const_iterator pos = chroots.begin();
       pos != chroots.end();
       ++pos)
    stream << *pos << "\n";
  stream << std::flush;
}

void
chroot_config::print_chroot_list_simple (std::ostream& stream) const
{
  stream << _("Available chroots: ");

  for (chroot_map::const_iterator pos = this->chroots.begin();
       pos != this->chroots.end();
       ++pos)
    {
      stream << pos->second->get_name();
      string_list const& aliases = pos->second->get_aliases();
      if (!aliases.empty())
	{
	  stream << " [";
	  for (string_list::const_iterator alias = aliases.begin();
	       alias != aliases.end();
	       ++alias)
	    {
		  stream << *alias;
		  if (alias + 1 != aliases.end())
		    stream << ", ";
	    }
	  stream << ']';
	}
      chroot_map::const_iterator is_end(pos);
      if ((++is_end) != chroots.end())
	stream << ", ";
    }

  stream << endl;
}

void
chroot_config::print_chroot_info (string_list const& chroots,
				  std::ostream&      stream) const
{
  for (string_list::const_iterator pos = chroots.begin();
       pos != chroots.end();
       ++pos)
    {
      const chroot::ptr chroot = find_alias(*pos);
      if (chroot)
	{
	  stream << chroot;
	  if (pos + 1 != chroots.end())
	    stream << '\n';
	}
      else
	{
	  error e(*pos, CHROOT);
	  log_error() << e.what() << endl;
	}
    }
}

void
chroot_config::print_chroot_location (string_list const& chroots,
				      std::ostream&      stream) const
{
  for (string_list::const_iterator pos = chroots.begin();
       pos != chroots.end();
       ++pos)
    {
      const chroot::ptr chroot = find_alias(*pos);
      if (chroot)
	{
	  stream << chroot->get_path() << '\n';
	}
      else
	{
	  error e(*pos, CHROOT);
	  log_error() << e.what() << endl;
	}
    }

  stream << std::flush;
}

void
chroot_config::print_chroot_config (string_list const& chroots,
				    std::ostream&      stream) const
{
  keyfile info;

  for (string_list::const_iterator pos = chroots.begin();
       pos != chroots.end();
       ++pos)
    {
      const chroot::ptr chroot = find_alias(*pos);

      // Generated chroots (e.g. source chroots) are not printed.
      if (chroot)
	{
	  if (chroot->get_original())
	    info << chroot;
	}
      else
	{
	  error e(*pos, CHROOT);
	  log_error() << e.what() << endl;
	}
    }

  stream << info;
}

string_list
chroot_config::validate_chroots (string_list const& chroots) const
{
  string_list bad_chroots;

  for (string_list::const_iterator pos = chroots.begin();
       pos != chroots.end();
       ++pos)
    {
      const chroot::ptr chroot = find_alias(*pos);
      if (!chroot)
	bad_chroots.push_back(*pos);
    }

  return bad_chroots;
}

void
chroot_config::load_data (std::string const& file,
			  bool               active)
{
  /* Use a UNIX fd, for security (no races) */
  int fd = open(file.c_str(), O_RDONLY|O_NOFOLLOW);
  if (fd < 0)
    throw error(file, FILE_OPEN, errno);

  sbuild::file_lock lock(fd);
  try
    {
      lock.set_lock(lock::LOCK_SHARED, 2);
    }
  catch (lock::error const& e)
    {
      throw error(file, e.what());
    }

  struct stat statbuf;
  if (fstat(fd, &statbuf) < 0)
    throw error(file, FILE_STAT, errno);

  if (statbuf.st_uid != 0)
    throw error(file, FILE_OWNER);
  if (statbuf.st_mode & S_IWOTH)
    throw error(file, FILE_PERMS);
  if (!S_ISREG(statbuf.st_mode))
    throw error(file, FILE_NOTREG);

  /* Now create an IO Channel and read in the data */
#ifdef SCHROOT_FILEBUF_OLD
  __gnu_cxx::stdio_filebuf<char> fdbuf(fd, std::ios::in, true, BUFSIZ);
#else
  __gnu_cxx::stdio_filebuf<char> fdbuf(fd, std::ios::in);
#endif
  std::istream input(&fdbuf);
  input.imbue(std::locale("C"));

  try
    {
      parse_data(input, active);
    }
  catch (runtime_error const& e)
    {
      throw error(file, e.what());
    }
  try
    {
      lock.unset_lock();
    }
  catch (lock::error const& e)
    {
      throw error(file, e.what());
    }
}

void
chroot_config::parse_data (std::istream& stream,
			   bool          active)
{
  /* Create key file */
  keyfile kconfig(stream);

  /* Create chroot objects from key file */
  string_list const& groups = kconfig.get_groups();
  for (string_list::const_iterator group = groups.begin();
       group != groups.end();
       ++group)
    {
      // Set the active property for chroot creation, and create
      // the chroot.
      kconfig.set_value(*group, "active", active);
      std::string type = "plain"; // "plain" is the default type.
      kconfig.get_value(*group, "type", type);
      chroot::ptr chroot = chroot::create(type);
      chroot->set_name(*group);

      try
	{
	  kconfig >> chroot;
	}
      catch (const runtime_error& e)
	{
	  format fmt(_("%1% chroot"));
	  fmt % *group;
	  throw error(fmt.str(), e.what());
	}

      add(chroot);

      {
	chroot_source *source = dynamic_cast<chroot_source *>(chroot.get());
	if (source != 0 && !chroot->get_active())
	  {
	    chroot::ptr source_chroot = source->clone_source();
	    if (source_chroot)
	      add(source_chroot);
	  }
      }
    }
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
