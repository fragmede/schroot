/* Copyright © 2005-2009,2012  Roger Leigh <rleigh@debian.org>
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
#include "sbuild-chroot-facet-userdata.h"
#include "sbuild-regex.h"

#include <locale>

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

namespace
{
  typedef std::pair<chroot_facet_userdata::error_code,const char *> emap;

  /**
   * This is a list of the supported error codes.  It's used to
   * construct the real error codes map.
   */
  emap init_errors[] =
    {
      emap(chroot_facet_userdata::ENV_AMBIGUOUS,
	   N_("Environment variable ‘%1%’ is ambiguous")),
      emap(chroot_facet_userdata::KEY_AMBIGUOUS,
	   N_("Configuration key ‘%1%’ is ambiguous")),
      emap(chroot_facet_userdata::KEY_DISALLOWED,
	   N_("Configuration key ‘%1%’ is not permitted to be modified.")),
      emap(chroot_facet_userdata::KEYNAME_INVALID,
	   N_("Configuration key name ‘%1%’ is not a permitted name."))
    };

  bool
  validate_keyname(std::string const& key)
  {
    // Valid names consist of one (or more) namespaces which consist
    // of [a-z][a-z0-9] followed by a . (namespace separator).  The
    // key name following the separator is [a-z][a-z0-9-].  When
    // converted to an environment variable, all alphabet characters
    // are uppercased, and all periods and hyphens converted to
    // underscores.
    static sbuild::regex permitted("^([a-z][a-z0-9]*\\.)+[a-z][a-z0-9-]*$");

    // These would permit clashes with existing setup environment
    // variables, and potentially security issues if they were
    // user-settable.
    static sbuild::regex reserved("^(auth|chroot|host|libexec|mount|session|setup.data.dir|status|sysconf)\\.");

    return regex_search(key, permitted) && !regex_search(key, reserved);
  }

  std::string
  envname(std::string const& key)
  {
    std::string ret(key);

    static const std::ctype<char>& ct = std::use_facet<std::ctype<char> >(std::locale::classic());
    for (std::string::iterator pos = ret.begin(); pos != ret.end(); ++pos)
      {
	*pos = ct.toupper(*pos);
	if (*pos == '-' || *pos == '.')
	  *pos = '_';
      }
    return ret;
  }
}

template<>
error<chroot_facet_userdata::error_code>::map_type
error<chroot_facet_userdata::error_code>::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));

chroot_facet_userdata::chroot_facet_userdata ():
  chroot_facet(),
  userdata(),
  env(),
  user_modifiable_keys(),
  root_modifiable_keys()
{
}

chroot_facet_userdata::~chroot_facet_userdata ()
{
}

chroot_facet_userdata::ptr
chroot_facet_userdata::create ()
{
  return ptr(new chroot_facet_userdata());
}

chroot_facet::ptr
chroot_facet_userdata::clone () const
{
  return ptr(new chroot_facet_userdata(*this));
}

std::string const&
chroot_facet_userdata::get_name () const
{
  static const std::string name("userdata");

  return name;
}

void
chroot_facet_userdata::setup_env (chroot const& chroot,
				  environment&  env) const
{
  for (string_map::const_iterator pos = userdata.begin();
       pos != userdata.end();
       ++pos)
    {
      std::string name = envname(pos->first);
      std::string dummy;
      if (!env.get(name, dummy))
	env.add(name, pos->second);
      else
	{
	  error e(name, ENV_AMBIGUOUS);
	  format fmt(_("Configuration keys additional to ‘%1%’ would set this setup script environment variable"));
	  fmt % pos->first;
	  e.set_reason(fmt.str());
	  throw e;
	}
    }
}

sbuild::chroot::session_flags
chroot_facet_userdata::get_session_flags (chroot const& chroot) const
{
  return sbuild::chroot::SESSION_NOFLAGS;
}

void
chroot_facet_userdata::get_details (chroot const&  chroot,
				    format_detail& detail) const
{
  string_list userkeys(this->user_modifiable_keys.begin(),
		       this->user_modifiable_keys.end());
  std::sort(userkeys.begin(), userkeys.end());

  string_list rootkeys(this->root_modifiable_keys.begin(),
		       this->root_modifiable_keys.end());
  std::sort(rootkeys.begin(), rootkeys.end());

  detail.add(_("User Modifiable Keys"), userkeys);
  detail.add(_("Root Modifiable Keys"), rootkeys);
  detail.add(_("User Data"), "");

  string_list keys;
  for (string_map::const_iterator pos = userdata.begin();
       pos != userdata.end();
       ++pos)
    keys.push_back(pos->first);
  std::sort(keys.begin(), keys.end());

  for (string_list::const_iterator pos = keys.begin();
       pos != keys.end();
       ++pos)
    {
      string_map::const_iterator key = userdata.find(*pos);
      if (key != userdata.end())
	{
	  std::string name("  ");
	  name += key->first;
	  detail.add(name, key->second);
	}
    }
}

string_map const&
chroot_facet_userdata::get_data () const
{
  return userdata;
}

bool
chroot_facet_userdata::get_data (std::string const& key,
				 std::string& value) const
{
  string_map::const_iterator pos = this->userdata.find(key);
  bool found = (pos != this->userdata.end());
  if (found)
    value = pos->second;
  return found;
}

void
chroot_facet_userdata::set_system_data (std::string const& key,
					std::string const& value)
{
  string_map::const_iterator inserted = userdata.find(key);
  if (inserted == userdata.end()) // Requires uniqueness checking.
    {
      std::string name = envname(key);
      string_set::const_iterator found = this->env.find(name);
      if (found != this->env.end())
	{
	  error e(key, KEY_AMBIGUOUS);
	  format fmt(_("More than one configuration key would set the ‘%1%’ environment variable"));
	  fmt % name;
	  e.set_reason(fmt.str());
	  throw e;
	}
      this->env.insert(name);
    }
  else
    this->userdata.erase(key);

  this->userdata.insert(std::make_pair(key, value));
}

void
chroot_facet_userdata::set_data (std::string const& key,
				 std::string const& value)
{
  if (!validate_keyname(key))
    throw error(key, KEYNAME_INVALID);

  set_system_data(key, value);
}

void
chroot_facet_userdata::set_data (string_map const& data)
{
  for (string_map::const_iterator pos = data.begin();
       pos != data.end();
       ++pos)
    set_data(pos->first, pos->second);
}

void
chroot_facet_userdata::set_user_data(string_map const&  data)
{
  set_data(data, this->user_modifiable_keys, false);
}

void
chroot_facet_userdata::set_root_data(string_map const&  data)
{
  // root can use both user and root keys, so combine the sets.
  string_set modifiable_keys;
  set_union(this->user_modifiable_keys.begin(),
	    this->user_modifiable_keys.end(),
	    this->root_modifiable_keys.begin(),
	    this->root_modifiable_keys.end(),
	    inserter(modifiable_keys, modifiable_keys.begin()));
  set_data(data, modifiable_keys, true);
}

void
chroot_facet_userdata::set_system_data(string_map const&  data)
{
  for (string_map::const_iterator pos = data.begin();
       pos != data.end();
       ++pos)
    set_system_data(pos->first, pos->second);
}

void
chroot_facet_userdata::set_data(string_map const&  data,
				string_set const&  allowed_keys,
				bool               root)
{
  // Require the key to be present in order to set it.  This ensures
  // that the key name has been pre-validated.
  for (string_map::const_iterator pos = data.begin();
       pos != data.end();
       ++pos)
    {
      string_set::const_iterator allowed = allowed_keys.find(pos->first);
      if (allowed == allowed_keys.end())
	{
	  error e(pos->first, KEY_DISALLOWED);
	  if (root)
	    e.set_reason(_("The key is not present in user-modifiable-keys or root-modifiable-keys"));
	  else
	    e.set_reason(_("The key is not present in user-modifiable-keys"));
	  throw e;
	}
      set_data(pos->first, pos->second);
    }
}

string_set const&
chroot_facet_userdata::get_user_modifiable_keys() const
{
  return this->user_modifiable_keys;
}

void
chroot_facet_userdata::set_user_modifiable_keys (string_set const& keys)
{
  this->user_modifiable_keys = keys;
}

string_set const&
chroot_facet_userdata::get_root_modifiable_keys() const
{
  return this->root_modifiable_keys;
}

void
chroot_facet_userdata::set_root_modifiable_keys (string_set const& keys)
{
  this->root_modifiable_keys = keys;
}

void
chroot_facet_userdata::get_keyfile (chroot const& chroot,
				    keyfile&      keyfile) const
{
  keyfile::set_object_set_value(*this,
				&chroot_facet_userdata::get_user_modifiable_keys,
				keyfile, chroot.get_name(),
				"user-modifiable-keys");

  keyfile::set_object_set_value(*this,
				&chroot_facet_userdata::get_root_modifiable_keys,
				keyfile, chroot.get_name(),
				"root-modifiable-keys");

  for (string_map::const_iterator pos = userdata.begin();
       pos != userdata.end();
       ++pos)
    {
      keyfile.set_value(chroot.get_name(),
			pos->first,
			pos->second);
    }
}

void
chroot_facet_userdata::set_keyfile (chroot&        chroot,
				    keyfile const& keyfile,
				    string_list&   used_keys)
{
  keyfile::get_object_set_value(*this,
				&chroot_facet_userdata::set_user_modifiable_keys,
				keyfile, chroot.get_name(),
				"user-modifiable-keys",
				keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("user-modifiable-keys");

  keyfile::get_object_set_value(*this,
				&chroot_facet_userdata::set_root_modifiable_keys,
				keyfile, chroot.get_name(),
				"root-modifiable-keys",
				keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("root-modifiable-keys");
}
