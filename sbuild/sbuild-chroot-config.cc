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
#include "sbuild-chroot-facet-session.h"
#include "sbuild-chroot-facet-session-clonable.h"
#include "sbuild-chroot-facet-source-clonable.h"
#include "sbuild-chroot-config.h"
#include "sbuild-lock.h"

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>

#include <ext/stdio_filebuf.h>

#include <boost/filesystem/operations.hpp>

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
      emap(chroot_config::ALIAS_EXIST,        N_("Alias '%1%' already associated with '%4%' chroot")),
      emap(chroot_config::CHROOT_NOTFOUND,    N_("No such chroot")),
      // TRANSLATORS: %1% = chroot name
      emap(chroot_config::CHROOT_EXIST,       N_("A chroot or alias '%1%' already exists with this name")),
      emap(chroot_config::FILE_NOTREG,        N_("File is not a regular file")),
      emap(chroot_config::FILE_OPEN,          N_("Failed to open file")),
      emap(chroot_config::FILE_OWNER,         N_("File is not owned by user root")),
      emap(chroot_config::FILE_PERMS,         N_("File has write permissions for others")),
      emap(chroot_config::NAME_INVALID,       N_("Invalid name")),
      emap(chroot_config::NAMESPACE_NOTFOUND, N_("No such namespace"))
    };

  bool
  chroot_alphasort (sbuild::chroot::ptr const& c1,
		    sbuild::chroot::ptr const& c2)
  {
    return c1->get_name() < c2->get_name();
  }

  void
  get_namespace(std::string const& name,
		std::string&       chroot_namespace,
		std::string&       chroot_name)
  {
    std::string::size_type pos =
      name.find_first_of(chroot_config::namespace_separator);

    if (pos != std::string::npos) // Found namespace
      {
	chroot_namespace = name.substr(0, pos);
	if (name.size() >= pos + 1)
	  chroot_name = name.substr(pos + 1);
      }
    else // No namespace
      {
	chroot_namespace.clear();
	chroot_name = name;
      }

  }

}

template<>
error<chroot_config::error_code>::map_type
error<chroot_config::error_code>::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));

const std::string chroot_config::namespace_separator(":");

chroot_config::chroot_config ():
  namespaces(),
  aliases()
{
  this->namespaces.insert(std::make_pair(std::string("chroot"), chroot_map()));
  this->namespaces.insert(std::make_pair(std::string("session"), chroot_map()));
  this->namespaces.insert(std::make_pair(std::string("source"), chroot_map()));
}

chroot_config::chroot_config (std::string const& chroot_namespace,
			      std::string const& file):
  namespaces(),
  aliases()
{
  this->namespaces.insert(std::make_pair(std::string("chroot"), chroot_map()));
  this->namespaces.insert(std::make_pair(std::string("session"), chroot_map()));
  this->namespaces.insert(std::make_pair(std::string("source"), chroot_map()));

  add(chroot_namespace, file);
}

chroot_config::~chroot_config ()
{
}

void
chroot_config::add (std::string const& chroot_namespace,
		    std::string const& location)
{
  if (stat(location).is_directory())
    add_config_directory(chroot_namespace, location);
  else
    add_config_file(chroot_namespace, location);
}

void
chroot_config::add_config_file (std::string const& chroot_namespace,
				std::string const& file)
{
  log_debug(DEBUG_NOTICE) << "Loading config file: " << file << endl;

  load_data(chroot_namespace, file);
}

void
chroot_config::add_config_directory (std::string const& chroot_namespace,
				     std::string const& dir)
{
  log_debug(DEBUG_NOTICE) << "Loading config directory: " << dir << endl;

  if (dir.empty())
    return;

  boost::filesystem::path dirpath(dir);
  boost::filesystem::directory_iterator end_iter;
  for (boost::filesystem::directory_iterator dirent(dirpath);
       dirent != end_iter;
       ++dirent)
    {
      std::string name(dirent->leaf());

      // Skip common directories.
      if (name == "." || name == "..")
	continue;

      // Skip backup files and dpkg configuration backup files.
      if (!is_valid_sessionname(name))
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

      load_data(chroot_namespace, filename);
    }
}

void
chroot_config::add (std::string const& chroot_namespace,
		    chroot::ptr&       chroot,
		    keyfile const&     kconfig)
{
  std::string const& name(chroot->get_name());
  std::string const& fullname(chroot_namespace + namespace_separator + chroot->get_name());

  chroot_map& chroots = find_namespace(chroot_namespace);

  // Make sure insertion will succeed.
  if (chroots.find(name) == chroots.end() &&
      this->aliases.find(fullname) == this->aliases.end())
    {
      // Set up chroot.
      chroots.insert(std::make_pair(name, chroot));
      this->aliases.insert(std::make_pair(fullname, fullname));

      // If a source chroot, add -source compatibility alias.
      if (chroot_namespace == "source")
	{
	  std::string source_alias = std::string("chroot") +
	    namespace_separator + chroot->get_name() + "-source";
	  if (this->aliases.find(source_alias) == this->aliases.end())
	    this->aliases.insert(std::make_pair(source_alias, fullname));
	}

      // If a session chroot, add compatibility alias.
      if (chroot_namespace == "session")
	{
	  std::string session_alias = std::string("chroot") +
	    namespace_separator + chroot->get_name();
	  if (this->aliases.find(session_alias) == this->aliases.end())
	    this->aliases.insert(std::make_pair(session_alias, fullname));
	}

      // Set up aliases.
      string_list const& aliases = chroot->get_aliases();
      for (string_list::const_iterator pos = aliases.begin();
	   pos != aliases.end();
	   ++pos)
	{
	  try
	    {
	      // TODO: Remove alias_namespace in 1.5.  Only needed for
	      // -source compatibility.
	      std::string alias_namespace(chroot_namespace);
	      if (this->aliases.insert(std::make_pair
				       (alias_namespace + namespace_separator + *pos,
					fullname))
		  .second == false)
		{
		  string_map::const_iterator dup = this->aliases.find(*pos);
		  // Don't warn if alias is for chroot of same name.
		  if (dup == this->aliases.end() ||
		      fullname != dup->first)
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
	  // If a source chroot, add -source compatibility alias.
	  if (chroot_namespace == "source")
	    {
	      std::string source_alias = std::string("chroot") +
		namespace_separator + *pos + "-source";
	      if (this->aliases.find(source_alias) == this->aliases.end())
		this->aliases.insert(std::make_pair(source_alias, fullname));
	    }
	}
    }
  else
    {
      unsigned int line = kconfig.get_line(name);

      error e(fullname, CHROOT_EXIST);

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
chroot_config::get_chroots (std::string const& chroot_namespace) const
{
  chroot_list ret;
  chroot_map const& chroots = find_namespace(chroot_namespace);

  for (chroot_map::const_iterator pos = chroots.begin();
       pos != chroots.end();
       ++pos)
    ret.push_back(pos->second);

  std::sort(ret.begin(), ret.end(), chroot_alphasort);

  return ret;
}

chroot_config::chroot_map&
chroot_config::find_namespace (std::string const& chroot_namespace)
{
  chroot_namespace_map::iterator pos = this->namespaces.find(chroot_namespace);

  if (pos == this->namespaces.end())
    throw error(chroot_namespace, NAMESPACE_NOTFOUND);

  return pos->second;
}

chroot_config::chroot_map const&
chroot_config::find_namespace (std::string const& chroot_namespace) const
{
  chroot_namespace_map::const_iterator pos = this->namespaces.find(chroot_namespace);

  if (pos == this->namespaces.end())
    throw error(chroot_namespace, NAMESPACE_NOTFOUND);

  return pos->second;
}

const sbuild::chroot::ptr
chroot_config::find_chroot (std::string const& name) const
{
  std::string chroot_namespace;
  std::string chroot_name;

  get_namespace(name, chroot_namespace, chroot_name);

  return find_chroot_in_namespace(chroot_namespace, chroot_name);
}

const sbuild::chroot::ptr
chroot_config::find_chroot (std::string const& namespace_hint,
			    std::string const& name) const
{
  std::string chroot_namespace(namespace_hint);
  std::string chroot_name(name);

  get_namespace(name, chroot_namespace, chroot_name);

  if (chroot_namespace.empty())
    chroot_namespace = "chroot";

  return find_chroot_in_namespace(chroot_namespace, chroot_name);
}

const sbuild::chroot::ptr
chroot_config::find_chroot_in_namespace (std::string const& chroot_namespace,
					 std::string const& name) const
{
  chroot_map const& chroots = find_namespace(chroot_namespace);

  log_debug(DEBUG_NOTICE) << "Looking for chroot " << name << " in namespace " << chroot_namespace << std::endl;

  chroot_map::const_iterator pos = chroots.find(name);

  if (pos != chroots.end())
    return pos->second;
  else
    {
      chroot *null_chroot = 0;
      return chroot::ptr(null_chroot);
    }
}

const sbuild::chroot::ptr
chroot_config::find_alias (std::string const& namespace_hint,
			   std::string const& name) const
{
  std::string chroot_namespace(namespace_hint);
  std::string alias_name(name);

  get_namespace(name, chroot_namespace, alias_name);

  if (chroot_namespace.empty())
    chroot_namespace = "chroot";

  string_map::const_iterator found = this->aliases.find(chroot_namespace + namespace_separator + alias_name);

  log_debug(DEBUG_NOTICE) << "Looking for alias " << name << " with hint " << namespace_hint << std::endl;
  log_debug(DEBUG_NOTICE) << "Alias " << (found != this->aliases.end() ? "found" : "not found") << std::endl;

  if (found != this->aliases.end())
    return find_chroot(namespace_hint, found->second);
  else
    return find_chroot(namespace_hint, name);
}

std::string
chroot_config::lookup_alias (std::string const& namespace_hint,
			     std::string const& name) const
{
  std::string chroot_namespace(namespace_hint);
  std::string alias_name(name);


  get_namespace(name, chroot_namespace, alias_name);

  if (chroot_namespace.empty())
    chroot_namespace = "chroot";

  string_map::const_iterator found = this->aliases.find(chroot_namespace + namespace_separator + alias_name);

  log_debug(DEBUG_NOTICE) << "Looking for alias " << name << " with hint " << namespace_hint << std::endl;
  log_debug(DEBUG_NOTICE) << "Alias " << (found != this->aliases.end() ? "found" : "not found") << std::endl;

  if (found != this->aliases.end())
    return found->second;
  else
    return std::string();
}

// TODO: Only printed aliases before...  Add variant which doesn't use
// namespaces to get all namespaces.
string_list
chroot_config::get_chroot_list (std::string const& chroot_namespace) const
{
  string_list ret;
  chroot_map const& chroots = find_namespace(chroot_namespace);

  for (chroot_map::const_iterator pos = chroots.begin();
       pos != chroots.end();
       ++pos)
    ret.push_back(chroot_namespace + namespace_separator + pos->first);

  std::sort(ret.begin(), ret.end());

  return ret;
}

string_list
chroot_config::get_alias_list (std::string const& chroot_namespace) const
{
  string_list ret;

  // To validate namespace.
  find_namespace(chroot_namespace);

  for (string_map::const_iterator pos = aliases.begin();
       pos != aliases.end();
       ++pos)
    {
      std::string::size_type seppos = pos->first.find_first_of(namespace_separator);
      if (seppos != std::string::npos)
	{
	  std::string alias_namespace = pos->first.substr(0, seppos);
	  if (alias_namespace == chroot_namespace)
	    ret.push_back(pos->first);
	}
    }

  std::sort(ret.begin(), ret.end());

  return ret;
}

void
chroot_config::print_chroot_list (string_list const& chroots,
				  std::ostream& stream) const
{
  for (string_list::const_iterator pos = chroots.begin();
       pos != chroots.end();
       ++pos)
    {
      log_debug(DEBUG_NOTICE) << "C: " << *pos << std::endl;

      const chroot::ptr chroot = find_alias("", *pos);
      if (chroot)
	{
	  stream << *pos << '\n';
	}
      else
	{
	  error e(*pos, CHROOT_NOTFOUND);
	  log_exception_error(e);
	}
    }
}

void
chroot_config::print_chroot_list_simple (std::ostream& stream) const
{
  stream << _("Available chroots: ");

  chroot_map const& chroots = find_namespace("chroot");

  for (chroot_map::const_iterator pos = chroots.begin();
       pos != chroots.end();
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
      const chroot::ptr chroot = find_alias("", *pos);
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
      const chroot::ptr chroot = find_alias("", *pos);
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
      const chroot::ptr chroot = find_alias("", *pos);

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
chroot_config::validate_chroots (std::string const& namespace_hint,
				 string_list&       chroots) const
{
  string_list bad_chroots;

  for (string_list::iterator pos = chroots.begin();
       pos != chroots.end();
       ++pos)
    {
      std::string chroot = lookup_alias(namespace_hint, *pos);
      if (chroot.empty())
	bad_chroots.push_back(*pos);
      else
	*pos = chroot;
    }

  return bad_chroots;
}

void
chroot_config::load_data (std::string const& chroot_namespace,
			  std::string const& file)
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
      file_lock lock(fd);
      lock.set_lock(lock::LOCK_SHARED, 2);
      parse_data(chroot_namespace, input);
      lock.unset_lock();
    }
  catch (std::runtime_error const& e)
    {
      throw error(file, e);
    }
}

void
chroot_config::parse_data (std::string const& chroot_namespace,
			   std::istream& stream)
{
  /* Create key file */
  keyfile kconfig(stream);

  load_keyfile(chroot_namespace, kconfig);
}

void
chroot_config::load_keyfile (std::string const& chroot_namespace,
			     keyfile& kconfig)
{
  /* Create chroot objects from key file */
  string_list const& groups = kconfig.get_groups();
  for (string_list::const_iterator group = groups.begin();
       group != groups.end();
       ++group)
    {
      std::string type = "plain"; // "plain" is the default type.
      kconfig.get_value(*group, "type", type);
      chroot::ptr chroot = chroot::create(type);

      // Set both; the keyfile load will correct them if needed.
      chroot->set_name(*group);

      // If we are (re-)creating session objects, we need to re-clone
      // the session chroot object from its basic state, in order to
      // get the correct facets in place.  In the future, it would be
      // great if sessions could serialise their facet usage to allow
      // automatic reconstruction.
      log_debug(DEBUG_INFO) << "Created template chroot (type=" << type
			    << "  name/session-id=" << *group
			    << "  namespace=" << chroot_namespace
			    << "  source-clonable="
			    << static_cast<bool>(chroot->get_facet<chroot_facet_session_clonable>())
			    << ")" << endl;

      // The "session" namespace is special.  We don't clone for other
      // types.  However, this special casing should probably be
      // removed.  Ideally, the chroot state should be stored in the
      // serialised session file (or chroot definition).
      if (chroot_namespace == "session" &&
	  chroot->get_facet<chroot_facet_session_clonable>())
	{
	  chroot = chroot->clone_session("dummy-session-name", "", false);
	  assert(chroot);
	  chroot_facet_session::const_ptr psess
	    (chroot->get_facet<chroot_facet_session>());
	  assert(psess);
	  chroot->set_name(*group);
	}
      else
	{
	  chroot_facet_session::const_ptr psess
	    (chroot->get_facet<chroot_facet_session>());
	  assert(!psess);
	}

      kconfig >> chroot;

      add(chroot_namespace, chroot, kconfig);

      {
	chroot_facet_source_clonable::const_ptr psrc
	  (chroot->get_facet<chroot_facet_source_clonable>());

	if (psrc && psrc->get_source_clone() &&
	    !chroot->get_facet<chroot_facet_session>())
	  {
	    chroot::ptr source_chroot = chroot->clone_source();
	    if (source_chroot)
	      add("source", source_chroot, kconfig);
	  }
      }
    }
}
