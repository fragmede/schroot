/* Copyright © 2005-2007  Roger Leigh <rleigh@debian.org>
 *
 * schroot is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * schroot is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 *********************************************************************/

#include <config.h>

#include "sbuild-chroot.h"
#include "sbuild-chroot-source.h"
#include "sbuild-chroot-config.h"
#include "sbuild-dirstream.h"
#include "sbuild-lock.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <ext/stdio_filebuf.h>

#include <sys/types.h>
#include <sys/stat.h>
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
      // TRANSLATORS: %1% = chroot alias name
      // TRANSLATORS: %4% = chroot name
      emap(chroot_config::ALIAS_EXIST,     N_("Alias '%1%' already associated with '%4%' chroot")),
      emap(chroot_config::CHROOT_NOTFOUND, N_("No such chroot")),
      // TRANSLATORS: %1% = chroot name
      emap(chroot_config::CHROOT_EXIST,    N_("A chroot or alias '%1%' already exists with this name")),
      emap(chroot_config::FILE_NOTREG,     N_("File is not a regular file")),
      emap(chroot_config::FILE_OPEN,       N_("Failed to open file")),
      emap(chroot_config::FILE_OWNER,      N_("File is not owned by user root")),
      emap(chroot_config::FILE_PERMS,      N_("File has write permissions for others")),
      emap(chroot_config::FILE_STAT,       N_("Failed to stat file"))
    };

  bool chroot_alphasort (sbuild::chroot::ptr const& c1,
			 sbuild::chroot::ptr const& c2)
  {
    return c1->get_name() < c2->get_name();
  }

}

template<>
error<chroot_config::error_code>::map_type
error<chroot_config::error_code>::error_strings
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
  log_debug(DEBUG_NOTICE) << "Loading config file: " << file << endl;

  load_data(file, active);
}

void
chroot_config::add_config_directory (std::string const& dir,
				     bool               active)
{
  log_debug(DEBUG_NOTICE) << "Loading config directory: " << dir << endl;

  if (dir.empty())
    return;

  dirstream stream(dir);
  direntry de;
  while (stream >> de)
    {
      if (de.name() == "." || de.name() == "..")
	continue;

      std::string filename = dir + "/" + de.name();

      struct stat statbuf;
      if (stat(filename.c_str(), &statbuf) < 0)
	{
	  error e(filename, FILE_STAT, strerror(errno));
	  log_exception_warning(e);
	  continue;
	}

      if (!S_ISREG(statbuf.st_mode))
	{
	  error e (filename, FILE_NOTREG);
	  log_exception_warning(e);
	  continue;
	}

      load_data(filename, active);
    }
}

void
chroot_config::add (chroot::ptr&   chroot,
		    keyfile const& kconfig)
{
  std::string const& name = chroot->get_name();

  // Make sure insertion will succeed.
  if (this->chroots.find(name) == this->chroots.end() &&
      this->aliases.find(name) == this->aliases.end())
    {
      // Set up chroot.
      this->chroots.insert(std::make_pair(name, chroot));
      this->aliases.insert(std::make_pair(name, name));

      // Set up aliases.
      string_list const& aliases = chroot->get_aliases();
      for (string_list::const_iterator pos = aliases.begin();
	   pos != aliases.end();
	   ++pos)
	{
	  try
	    {
	      if (this->aliases.insert(std::make_pair(*pos, name))
		  .second == false)
		{
		  string_map::const_iterator dup = this->aliases.find(*pos);
		  // Don't warn if alias is for chroot of same name.
		  if (dup == this->aliases.end() ||
		      name != dup->first)
		    {
		      const char *const key("aliases");
		      unsigned int line = kconfig.get_line(name, key);

		      if (dup == this->aliases.end())
			{
			  error e(*pos, ALIAS_EXIST);
			  if (line)
			    throw keyfile::error(line, name, key,
						 keyfile::PASSTHROUGH_LGK, e);
			  else
			    throw keyfile::error(name, key,
						 keyfile::PASSTHROUGH_GK, e);
			}
		      else
			{
			  error e(dup->first, ALIAS_EXIST, dup->second);
			  if (line)
			    throw keyfile::error(line, name, key,
						 keyfile::PASSTHROUGH_LGK, e);
			  else
			    throw keyfile::error(name, key,
						 keyfile::PASSTHROUGH_GK, e);
			}
		    }
		}
	    }
	  catch (std::runtime_error const& e)
	    {
	      log_exception_warning(e);
	    }
	}
    }
  else
    {
      unsigned int line = kconfig.get_line(name);

      error e(name, CHROOT_EXIST);

      if (line)
	{
	  keyfile::error ke(line, name, keyfile::PASSTHROUGH_LG, e);
 	  ke.set_reason(_("Duplicate names are not allowed"));
	  throw ke;
	}
      else
	{
	  keyfile::error ke(name, keyfile::PASSTHROUGH_G, e);
 	  ke.set_reason(_("Duplicate names are not allowed"));
	  throw ke;
	}
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
	  error e(*pos, CHROOT_NOTFOUND);
	  log_exception_error(e);
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
	  error e(*pos, CHROOT_NOTFOUND);
	  log_exception_error(e);
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
	  error e(*pos, CHROOT_NOTFOUND);
	  log_exception_error(e);
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
  log_debug(DEBUG_NOTICE) << "Loading data file: " << file << endl;

  /* Use a UNIX fd, for security (no races) */
  int fd = open(file.c_str(), O_RDONLY|O_NOFOLLOW);
  if (fd < 0)
    throw error(file, FILE_OPEN, strerror(errno));

  // Create a stream buffer from the file descriptor.  The fd will
  // be closed when the buffer is destroyed.
#ifdef SCHROOT_FILEBUF_OLD
  __gnu_cxx::stdio_filebuf<char> fdbuf(fd, std::ios::in, true, BUFSIZ);
#else
  __gnu_cxx::stdio_filebuf<char> fdbuf(fd, std::ios::in);
#endif
  std::istream input(&fdbuf);
  input.imbue(std::locale::classic());

  sbuild::file_lock lock(fd);
  try
    {
      lock.set_lock(lock::LOCK_SHARED, 2);
    }
  catch (lock::error const& e)
    {
      throw error(file, e);
    }

  struct stat statbuf;
  if (fstat(fd, &statbuf) < 0)
    throw error(file, FILE_STAT, strerror(errno));

  if (statbuf.st_uid != 0)
    throw error(file, FILE_OWNER);
  if (statbuf.st_mode & S_IWOTH)
    throw error(file, FILE_PERMS);
  if (!S_ISREG(statbuf.st_mode))
    throw error(file, FILE_NOTREG);

  try
    {
      parse_data(input, active);
    }
  catch (std::runtime_error const& e)
    {
      throw error(file, e);
    }
  try
    {
      lock.unset_lock();
    }
  catch (lock::error const& e)
    {
      throw error(file, e);
    }
}

void
chroot_config::parse_data (std::istream& stream,
			   bool          active)
{
  /* Create key file */
  keyfile kconfig(stream);

  load_keyfile(kconfig, active);
}

void
chroot_config::load_keyfile (keyfile& kconfig,
			     bool     active)
{
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

      kconfig >> chroot;

      add(chroot, kconfig);

      {
	chroot_source *source = dynamic_cast<chroot_source *>(chroot.get());
	if (source != 0 && !chroot->get_active())
	  {
	    chroot::ptr source_chroot = source->clone_source();
	    if (source_chroot)
	      add(source_chroot, kconfig);
	  }
      }
    }
}
