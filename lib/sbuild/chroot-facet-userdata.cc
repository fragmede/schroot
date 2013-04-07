/* Copyright © 2005-2013  Roger Leigh <rleigh@debian.org>
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

#include <sbuild/chroot/chroot.h>
#include "chroot-facet-userdata.h"
#include "regex.h"

#include <locale>

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

namespace
{

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
    static sbuild::regex reserved("^(((auth|chroot|host|libexec|mount|session|status|sysconf)\\..*)|setup.data.dir)$");

    return regex_search(key, permitted) && !regex_search(key, reserved);
  }

  std::string
  envname(std::string const& key)
  {
    std::string ret(key);

    static const std::ctype<char>& ct = std::use_facet<std::ctype<char> >(std::locale::classic());
    for (auto& chr : ret)
      {
        chr = ct.toupper(chr);
        if (chr == '-' || chr == '.')
          chr = '_';
      }
    return ret;
  }

}

template<>
error<chroot_facet_userdata::error_code>::map_type
error<chroot_facet_userdata::error_code>::error_strings =
  {
    {chroot_facet_userdata::ENV_AMBIGUOUS,
         N_("Environment variable ‘%1%’ is ambiguous")},
    {chroot_facet_userdata::KEY_AMBIGUOUS,
         N_("Configuration key ‘%1%’ is ambiguous")},
    {chroot_facet_userdata::KEY_DISALLOWED,
         N_("Configuration key ‘%1%’ is not permitted to be modified.")},
    {chroot_facet_userdata::KEYNAME_INVALID,
         N_("Configuration key name ‘%1%’ is not a permitted name.")},
    // TRANSLATORS: %1% = key name for which value parsing failed
    // TRANSLATORS: %4% = additional details of error
    {chroot_facet_userdata::PARSE_ERROR, N_("%1%: %4%")}
  };

chroot_facet_userdata::chroot_facet_userdata ():
  chroot::facet::facet(),
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

chroot::facet::facet::ptr
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
chroot_facet_userdata::setup_env (chroot::chroot const& chroot,
                                  environment&          env) const
{
  for (const auto& data : userdata)
    {
      std::string name = envname(data.first);
      std::string dummy;
      if (!env.get(name, dummy))
        env.add(name, data.second);
      else
        {
          error e(name, ENV_AMBIGUOUS);
          format fmt(_("Configuration keys additional to ‘%1%’ would set this setup script environment variable"));
          fmt % data.first;
          e.set_reason(fmt.str());
          throw e;
        }
    }
}

chroot::chroot::session_flags
chroot_facet_userdata::get_session_flags (chroot::chroot const& chroot) const
{
  return chroot::chroot::SESSION_NOFLAGS;
}

void
chroot_facet_userdata::get_details (chroot::chroot const& chroot,
                                    format_detail&        detail) const
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
  for (const auto& data : userdata)
    keys.push_back(data.first);
  std::sort(keys.begin(), keys.end());

  for (const auto& keyname : keys)
    {
      string_map::const_iterator key = userdata.find(keyname);
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
chroot_facet_userdata::remove_data (std::string const& key)
{
  this->userdata.erase(key);
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
  for (const auto& elem : data)
    set_data(elem.first, elem.second);
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
  for (const auto& elem : data)
    set_system_data(elem.first, elem.second);
}

void
chroot_facet_userdata::set_data(string_map const&  data,
                                string_set const&  allowed_keys,
                                bool               root)
{
  // Require the key to be present in order to set it.  This ensures
  // that the key name has been pre-validated.
  string_list used_keys = this->owner->get_used_keys();
  string_set used_set(used_keys.begin(), used_keys.end());

  sbuild::keyfile kf;
  // Ideally, we'd only set the changed keys, but currently the
  // set_keyfile validation requires all PRIORITY_REQUIRED keys to be
  // present.  This can be changed if the priority can be made
  // optional when only updating a subset of the total keys.
  this->owner->get_keyfile(kf);

  for (const auto& elem : data)
    {
      string_set::const_iterator allowed = allowed_keys.find(elem.first);
      if (allowed == allowed_keys.end())
        {
          error e(elem.first, KEY_DISALLOWED);
          if (root)
            e.set_reason(_("The key is not present in user-modifiable-keys or root-modifiable-keys"));
          else
            e.set_reason(_("The key is not present in user-modifiable-keys"));
          throw e;
        }
      string_set::const_iterator found = used_set.find(elem.first);
      if (found != used_set.end()) // Used in other facet
        kf.set_value(this->owner->get_name(), elem.first, elem.second);
      else
        set_data(elem.first, elem.second);
    }

  this->owner->set_keyfile(kf);
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
chroot_facet_userdata::get_used_keys (string_list& used_keys) const
{
  used_keys.push_back("user-modifiable-keys");
  used_keys.push_back("root-modifiable-keys");
}

void
chroot_facet_userdata::get_keyfile (chroot::chroot const& chroot,
                                    keyfile&              keyfile) const
{
  keyfile::set_object_set_value(*this,
                                &chroot_facet_userdata::get_user_modifiable_keys,
                                keyfile, chroot.get_name(),
                                "user-modifiable-keys");

  keyfile::set_object_set_value(*this,
                                &chroot_facet_userdata::get_root_modifiable_keys,
                                keyfile, chroot.get_name(),
                                "root-modifiable-keys");

  for (const auto& data : userdata)
    {
      keyfile.set_value(chroot.get_name(),
                        data.first,
                        data.second);
    }
}

void
chroot_facet_userdata::set_keyfile (chroot::chroot& chroot,
                                    keyfile const&  keyfile)
{
  keyfile::get_object_set_value(*this,
                                &chroot_facet_userdata::set_user_modifiable_keys,
                                keyfile, chroot.get_name(),
                                "user-modifiable-keys",
                                keyfile::PRIORITY_OPTIONAL);

  keyfile::get_object_set_value(*this,
                                &chroot_facet_userdata::set_root_modifiable_keys,
                                keyfile, chroot.get_name(),
                                "root-modifiable-keys",
                                keyfile::PRIORITY_OPTIONAL);

  // Check for keys which weren't set above.  These may be either
  // invalid keys or user-set keys.  The latter must have a namespace
  // separated with one or more periods.  These may be later
  // overridden by the user on the commandline.
  {
    string_list used_keys = chroot.get_used_keys();
    std::string const& group = chroot.get_name();
    const string_list total(keyfile.get_keys(group));

    const string_set a(total.begin(), total.end());
    const string_set b(used_keys.begin(), used_keys.end());

    string_set unused;

    set_difference(a.begin(), a.end(),
                   b.begin(), b.end(),
                   inserter(unused, unused.begin()));

    string_map userdata_keys;
    for (const auto& elem : unused)
      {
        // Skip language-specific key variants.
        static regex description_keys("\\[.*\\]$");
        if (regex_search(elem, description_keys))
          continue;

        try
          {
            std::string value;
            if (keyfile.get_value(get_name(), elem, value))
              set_data(elem, value);
          }
        catch (std::runtime_error const& e)
          {
            keyfile::size_type line = keyfile.get_line(group, elem);
            keyfile::error w(line, group, elem,
                             keyfile::PASSTHROUGH_LGK, e.what());

            try
              {
                sbuild::error_base const& r =
                  dynamic_cast<sbuild::error_base const&>(e);
                w.set_reason(r.get_reason());
              }
            catch (...)
              {
              }
            log_exception_warning(w);
          }
      }
  }

}
