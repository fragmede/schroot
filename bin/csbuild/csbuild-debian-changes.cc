/* Copyright © 2005-2007  Roger Leigh <rleigh@codelibre.net>
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

#include "csbuild-debian-changes.h"

#include <fstream>

#include <boost/format.hpp>

using sbuild::_;
using sbuild::N_;
using boost::format;
using namespace csbuild;

namespace
{

  typedef std::pair<debian_changes::error_code,const char *> emap;

  /**
   * This is a list of the supported error codes.  It's used to
   * construct the real error codes map.
   */
  emap init_errors[] =
    {
      // TRANSLATORS: %1% = file
      emap(debian_changes::BAD_FILE,          N_("Can't open file ‘%1%’")),
      // TRANSLATORS: %1% = line number in configuration file
      // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
      emap(debian_changes::DEPRECATED_KEY,    N_("line %1%: Deprecated key ‘%4%’ used")),
      // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
      emap(debian_changes::DEPRECATED_KEY_NL, N_("Deprecated key ‘%4%’ used")),
      // TRANSLATORS: %1% = line number in configuration file
      // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
      emap(debian_changes::DISALLOWED_KEY,    N_("line %1%: Disallowed key ‘%4%’ used")),
      // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
      emap(debian_changes::DISALLOWED_KEY_NL, N_("Disallowed key ‘%4%’ used")),
      // TRANSLATORS: %1% = line number in configuration file
      // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
      emap(debian_changes::DUPLICATE_KEY,     N_("line %1%: Duplicate key ‘%4%’")),
      // TRANSLATORS: %1% = line number in configuration file
      // TRANSLATORS: %4% = line contents as read from the configuration file
      emap(debian_changes::INVALID_LINE,      N_("line %1%: Invalid line: “%4%”")),
      // TRANSLATORS: %1% = line number in configuration file
      // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
      emap(debian_changes::MISSING_KEY,       N_("line %1%: Required key ‘%4%’ is missing")),
      // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
      emap(debian_changes::MISSING_KEY_NL,    N_("Required key ‘%4%’ is missing")),
      // TRANSLATORS: %1% = line number in configuration file
      // TRANSLATORS: %4% = line contents as read from the configuration file
      emap(debian_changes::NO_KEY,            N_("line %1%: No key specified: “%4%”")),
      // TRANSLATORS: %1% = line number in configuration file
      // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
      emap(debian_changes::OBSOLETE_KEY,      N_("line %1%: Obsolete key ‘%4%’ used")),
      // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
      emap(debian_changes::OBSOLETE_KEY_NL,   N_("Obsolete key ‘%4%’ used")),
      // TRANSLATORS: %2% = key name ("keyname=value" in configuration file)
      // TRANSLATORS: %4% = additional details
      emap(debian_changes::PASSTHROUGH_K,    N_("%2%: %4%")),
      // TRANSLATORS: %1% = line number in configuration file
      // TRANSLATORS: %3% = key name ("keyname=value" in configuration file)
      // TRANSLATORS: %4% = additional details
      emap(debian_changes::PASSTHROUGH_LK,   N_("line %1%: %3%: %4%"))
    };

}

template<>
sbuild::error<debian_changes::error_code>::map_type
sbuild::error<debian_changes::error_code>::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));

debian_changes::debian_changes ():
  items()
{
}

debian_changes::debian_changes (std::string const& file):
  items()
{
  std::ifstream fs(file.c_str());
  if (fs)
    {
      fs.imbue(std::locale::classic());
      fs >> *this;
    }
  else
    {
      throw error(file, BAD_FILE);
    }
}

debian_changes::debian_changes (std::istream& stream):
  items()
{
  stream >> *this;
}

debian_changes::~debian_changes()
{
}

sbuild::string_list
debian_changes::get_keys () const
{
  sbuild::string_list ret;

  for (item_map_type::const_iterator pos = items.begin();
       pos != items.end();
       ++pos)
    ret.push_back(pos->first);

  return ret;
}

bool
debian_changes::has_key (key_type const& key) const
{
  return (find_item(key) != 0);
}

debian_changes::size_type
debian_changes::get_line (key_type const& key) const
{
  const item_type *found_item = find_item(key);
  if (found_item)
      return std::get<2>(*found_item);
  else
    return 0;
}

bool
debian_changes::get_value (key_type const& key,
                           value_type&     value) const
{
  sbuild::log_debug(sbuild::DEBUG_INFO)
    << "Getting debian_changes key=" << key << std::endl;
  const item_type *found_item = find_item(key);
  if (found_item)
    {
      value_type const& val(std::get<1>(*found_item));
      value = val;
      return true;
    }
  sbuild::log_debug(sbuild::DEBUG_NOTICE)
    << "key not found" << std::endl;
  return false;
}

bool
debian_changes::get_value (key_type const& key,
                           priority        priority,
                           value_type&     value) const
{
  bool status = get_value(key, value);
  check_priority(key, priority, status);
  return status;
}

void
debian_changes::set_value (key_type const&   key,
                           value_type const& value,
                           size_type         line)
{
  item_map_type::iterator pos = items.find(key);
  if (pos != items.end())
    items.erase(pos);
  items.insert
    (item_map_type::value_type(key,
                               item_type(key, value, line)));
}

void
debian_changes::remove_key (key_type const& key)
{
  item_map_type::iterator pos = items.find(key);
  if (pos != items.end())
    items.erase(pos);
}

debian_changes&
debian_changes::operator += (debian_changes const& rhs)
{
  for (item_map_type::const_iterator it = rhs.items.begin();
       it != rhs.items.end();
       ++it)
    {
      item_type const& item = it->second;
      key_type const& key(std::get<0>(item));
      value_type const& value(std::get<1>(item));
      size_type const& line(std::get<2>(item));
      set_value(key, value, line);
    }

  return *this;
}

debian_changes
operator + (debian_changes const& lhs,
            debian_changes const& rhs)
{
  debian_changes ret(lhs);
  ret += rhs;
  return ret;
}

const debian_changes::item_type *
debian_changes::find_item (key_type const& key) const
{
  item_map_type::const_iterator pos = items.find(key);
  if (pos != items.end())
    return &pos->second;

  return 0;
}

debian_changes::item_type *
debian_changes::find_item (key_type const& key)
{
  item_map_type::iterator pos = items.find(key);
  if (pos != items.end())
    return &pos->second;

  return 0;
}

void
debian_changes::check_priority (key_type const& key,
                                priority        priority,
                                bool            valid) const
{
  if (valid == false)
    {
      size_type line = get_line(key);

      switch (priority)
        {
        case PRIORITY_REQUIRED:
          {
            if (line)
              throw error(line, MISSING_KEY, key);
            else
              throw error(MISSING_KEY_NL, key);
          }
          break;
        default:
          break;
        }
    }
  else
    {
      size_type line = get_line(key);

      switch (priority)
        {
        case PRIORITY_DEPRECATED:
          {
            if (line)
              {
                error e(line, DEPRECATED_KEY, key);
                e.set_reason(_("This option will be removed in the future"));
                log_exception_warning(e);
              }
            else
              {
                error e(DEPRECATED_KEY_NL, key);
                e.set_reason(_("This option will be removed in the future"));
                log_exception_warning(e);
              }
          }
          break;
        case PRIORITY_OBSOLETE:
          {
            if (line)
              {
                error e(line, OBSOLETE_KEY, key);
                e.set_reason(_("This option has been removed, and no longer has any effect"));
                log_exception_warning(e);
              }
            else
              {
                error e(OBSOLETE_KEY_NL, key);
                e.set_reason(_("This option has been removed, and no longer has any effect"));
                log_exception_warning(e);
              }
          }
          break;
        case PRIORITY_DISALLOWED:
          {
            if (line)
              throw error(line, DISALLOWED_KEY, key);
            else
              throw error(DISALLOWED_KEY_NL, key);
          }
          break;
        default:
          break;
        }
    }
}
