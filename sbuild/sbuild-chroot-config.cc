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
#include "sbuild-chroot-facet-source-clonable.h"
#include "sbuild-chroot-config.h"
#include "sbuild-dirstream.h"
#include "sbuild-lock.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>

#include <ext/stdio_filebuf.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
      emap(chroot_config::FILE_PERMS,      N_("File has write permissions for others"))
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
  if (stat(location).is_directory())
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
      std::string name(de.name());

      // Skip common directories.
      if (name == "." || name == "..")
	continue;

      // Skip backup files and dpkg configuration backup files.
      if (!is_valid_filename(name))
	continue;

      std::string filename = dir + "/" + name;

      try
	{
	  if (!stat(filename).is_regular())
	    throw error(filename, FILE_NOTREG);
	}
      catch (std::runtime_error const& e)
	{
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

  // stat filename (in case it's a pipe and open(2) blocks)
  stat file_status1(file);
  if (file_status1.uid() != 0)
    throw error(file, FILE_OWNER);
  if (file_status1.check_mode(stat::PERM_OTHER_WRITE))
    throw error(file, FILE_PERMS);
  if (!file_status1.is_regular())
    throw error(file, FILE_NOTREG);

  /* Use a UNIX fd, for security (no races) */
  int fd = open(file.c_str(), O_RDONLY);
  if (fd < 0)
    throw error(file, FILE_OPEN, strerror(errno));

  // stat fd following open
  stat file_status2(fd);
  if (file_status2.uid() != 0)
    throw error(file, FILE_OWNER);
  if (file_status2.check_mode(stat::PERM_OTHER_WRITE))
    throw error(file, FILE_PERMS);
  if (!file_status2.is_regular())
    throw error(file, FILE_NOTREG);

  // Create a stream buffer from the file descriptor.  The fd will
  // be closed when the buffer is destroyed.
  __gnu_cxx::stdio_filebuf<char> fdbuf(fd, std::ios::in);
  std::istream input(&fdbuf);
  input.imbue(std::locale::classic());

  try
    {
      sbuild::file_lock lock(fd);
      lock.set_lock(lock::LOCK_SHARED, 2);
      parse_data(input, active);
      lock.unset_lock();
    }
  catch (std::runtime_error const& e)
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
      std::string type = "plain"; // "plain" is the default type.
      kconfig.get_value(*group, "type", type);
      chroot::ptr chroot = chroot::create(type);

      // Set both; the keyfile load will correct them if needed.
      chroot->set_name(*group);
      chroot->set_session_id(*group);

      // If we are (re-)creating session objects, we need to re-clone
      // the session chroot object from its basic state, in order to
      // get the correct facets in place.  In the future, it would be
      // great if sessions could serialise their facet usage to allow
      // automatic reconstruction.
      if (active)
	chroot = chroot->clone_session("dummy-session-name");

      kconfig >> chroot;

      add(chroot, kconfig);

      {
	chroot_facet_source_clonable::const_ptr psrc
	  (chroot->get_facet<sbuild::chroot_facet_source_clonable>());

	if (psrc && !chroot->get_active())
	  {
	    chroot::ptr source_chroot = chroot->clone_source();
	    if (source_chroot)
	      add(source_chroot, kconfig);
	  }
      }
    }
}
