/* Copyright © 2005-2008  Roger Leigh <rleigh@debian.org>
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

#include <algorithm>
#include <set>

#include "sbuild-basic-keyfile.h"

#include <boost/format.hpp>

using boost::format;

using namespace sbuild;

template<>
error<basic_keyfile::error_code>::map_type
error<basic_keyfile::error_code>::error_strings =
  {
    // TRANSLATORS: %1% = file
    {basic_keyfile::BAD_FILE,
     N_("Can't open file ‘%1%’")},
    // TRANSLATORS: %1% = line number in configuration file
    // TRANSLATORS: %2% = group name ("[groupname]" in configuration file)
    // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
    {basic_keyfile::DEPRECATED_KEY,
     N_("line %1% [%2%]: Deprecated key ‘%4%’ used")},
    // TRANSLATORS: %1% = group name ("[groupname]" in configuration file)
    // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
    {basic_keyfile::DEPRECATED_KEY_NL,
     N_("[%1%]: Deprecated key ‘%4%’ used")},
    // TRANSLATORS: %1% = line number in configuration file
    // TRANSLATORS: %2% = group name ("[groupname]" in configuration file)
    // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
    {basic_keyfile::DISALLOWED_KEY,
     N_("line %1% [%2%]: Disallowed key ‘%4%’ used")},
    // TRANSLATORS: %1% = group name ("[groupname]" in configuration file)
    // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
    {basic_keyfile::DISALLOWED_KEY_NL,
     N_("[%1%]: Disallowed key ‘%4%’ used")},
    // TRANSLATORS: %1% = line number in configuration file
    // TRANSLATORS: %4% = group name ("[groupname]" in configuration file)
    {basic_keyfile::DUPLICATE_GROUP,
     N_("line %1%: Duplicate group ‘%4%’")},
    // TRANSLATORS: %1% = line number in configuration file
    // TRANSLATORS: %2% = group name ("[groupname]" in configuration file)
    // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
    {basic_keyfile::DUPLICATE_KEY,
     N_("line %1% [%2%]: Duplicate key ‘%4%’")},
    // TRANSLATORS: %1% = line number in configuration file
    // TRANSLATORS: %4% = line contents as read from the configuration file
    {basic_keyfile::INVALID_GROUP,
     N_("line %1%: Invalid group: “%4%”")},
    // TRANSLATORS: %1% = line number in configuration file
    // TRANSLATORS: %2% = group name ("[groupname]" in configuration file)
    // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
    {basic_keyfile::INVALID_KEY,
     N_("line %1% [%2%]: Invalid key ‘%4%’ used")},
    // TRANSLATORS: %1% = line number in configuration file
    // TRANSLATORS: %4% = line contents as read from the configuration file
    {basic_keyfile::INVALID_LINE,
     N_("line %1%: Invalid line: “%4%”")},
    // TRANSLATORS: %1% = line number in configuration file
    // TRANSLATORS: %2% = group name ("[groupname]" in configuration file)
    // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
    {basic_keyfile::MISSING_KEY,
     N_("line %1% [%2%]: Required key ‘%4%’ is missing")},
    // TRANSLATORS: %1% = group name ("[groupname]" in configuration file)
    // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
    {basic_keyfile::MISSING_KEY_NL,
     N_("[%1%]: Required key ‘%4%’ is missing")},
    // TRANSLATORS: %1% = line number in configuration file
    // TRANSLATORS: %4% = line contents as read from the configuration file
    {basic_keyfile::NO_GROUP,
     N_("line %1%: No group specified: “%4%”")},
    // TRANSLATORS: %1% = line number in configuration file
    // TRANSLATORS: %4% = line contents as read from the configuration file
    {basic_keyfile::NO_KEY,
     N_("line %1%: No key specified: “%4%”")},
    // TRANSLATORS: %1% = line number in configuration file
    // TRANSLATORS: %2% = group name ("[groupname]" in configuration file)
    // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
    {basic_keyfile::OBSOLETE_KEY,
     N_("line %1% [%2%]: Obsolete key ‘%4%’ used")},
    // TRANSLATORS: %1% = group name ("[groupname]" in configuration file)
    // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
    {basic_keyfile::OBSOLETE_KEY_NL,
     N_("[%1%]: Obsolete key ‘%4%’ used")},
    // TRANSLATORS: %1% = group name ("[groupname]" in configuration file)
    // TRANSLATORS: %4% = additional details
    {basic_keyfile::PASSTHROUGH_G,
     N_("[%1%]: %4%")},
    // TRANSLATORS: %1% = group name ("[groupname]" in configuration file)
    // TRANSLATORS: %2% = key name ("keyname=value" in configuration file)
    // TRANSLATORS: %4% = additional details
    {basic_keyfile::PASSTHROUGH_GK,
     N_("[%1%] %2%: %4%")},
    // TRANSLATORS: %1% = line number in configuration file
    // TRANSLATORS: %2% = group name ("[groupname]" in configuration file)
    // TRANSLATORS: %4% = additional details
    {basic_keyfile::PASSTHROUGH_LG,
     N_("line %1% [%2%]: %4%")},
    // TRANSLATORS: %1% = line number in configuration file
    // TRANSLATORS: %2% = group name ("[groupname]" in configuration file)
    // TRANSLATORS: %3% = key name ("keyname=value" in configuration file)
    // TRANSLATORS: %4% = additional details
    {basic_keyfile::PASSTHROUGH_LGK,
     N_("line %1% [%2%] %3%: %4%")},
    // TRANSLATORS: %1% = line number in configuration file
    // TRANSLATORS: %2% = group name ("[groupname]" in configuration file)
    // TRANSLATORS: %4% = key name ("keyname=value" in configuration file)
    {basic_keyfile::UNKNOWN_KEY,
     N_("line %1% [%2%]: Unknown key ‘%4%’ used")}
  };

sbuild::basic_keyfile::basic_keyfile ():
  groups(),
  separator(",")
{
}

sbuild::basic_keyfile::~basic_keyfile()
{
}

sbuild::basic_keyfile::group_list
sbuild::basic_keyfile::get_groups () const
{
  group_list ret;

  for (const auto& group : this->groups)
    ret.push_back(group.first);

  return ret;
}

sbuild::basic_keyfile::key_list
sbuild::basic_keyfile::get_keys (group_name_type const& group) const
{
  key_list ret;

  const group_type *found_group = find_group(group);
  if (found_group)
    {
      item_map_type const& items(std::get<1>(*found_group));
      for (const auto& item : items)
        ret.push_back(item.first);
    }

  return ret;
}

void
sbuild::basic_keyfile::check_keys (group_name_type const& group,
                                   key_list const&        keys) const
{
  const string_list total(get_keys(group));

  const string_set a(total.begin(), total.end());
  const string_set b(keys.begin(), keys.end());

  string_set unused;

  set_difference(a.begin(), a.end(),
                 b.begin(), b.end(),
                 inserter(unused, unused.begin()));

  for (const auto& item : unused)
    {
      size_type line = get_line(group, item);
      error e(line, group, UNKNOWN_KEY, item);
      e.set_reason(_("This option may be present in a newer version"));
      log_exception_warning(e);
    }
}

bool
sbuild::basic_keyfile::has_group (group_name_type const& group) const
{
  return (find_group(group) != 0);
}

bool
sbuild::basic_keyfile::has_key (group_name_type const& group,
                                key_type const&        key) const
{
  return (find_item(group, key) != 0);
}

void
sbuild::basic_keyfile::set_group (group_name_type const& group,
                                  comment_type const&    comment)
{
  set_group(group, comment, 0);
}

void
sbuild::basic_keyfile::set_group (group_name_type const& group,
                                  comment_type const&    comment,
                                  size_type              line)
{
  if (!has_group(group))
    this->groups.insert
      (group_map_type::value_type(group,
                                  group_type(group,
                                             item_map_type(),
                                             comment,
                                             line)));
}

sbuild::basic_keyfile::comment_type
sbuild::basic_keyfile::get_comment (group_name_type const& group) const
{
  const group_type *found_group = find_group(group);
  if (found_group)
    return std::get<2>(*found_group);
  else
    return comment_type();
}

sbuild::basic_keyfile::comment_type
sbuild::basic_keyfile::get_comment (group_name_type const& group,
                                    key_type const&        key) const
{
  const item_type *found_item = find_item(group, key);
  if (found_item)
    return std::get<2>(*found_item);
  else
    return comment_type();
}

sbuild::basic_keyfile::size_type
sbuild::basic_keyfile::get_line (group_name_type const& group) const
{
  const group_type *found_group = find_group(group);
  if (found_group)
    return std::get<3>(*found_group);
  else
    return 0;
}

sbuild::basic_keyfile::size_type
sbuild::basic_keyfile::get_line (group_name_type const& group,
                                 key_type const&        key) const
{
  const item_type *found_item = find_item(group, key);
  if (found_item)
    return std::get<3>(*found_item);
  else
    return 0;
}

bool
sbuild::basic_keyfile::get_locale_string (group_name_type const& group,
                                          key_type const&        key,
                                          value_type&            value) const
{
  std::string localename;
  try
    {
      localename = std::locale("").name();
    }
  catch (std::runtime_error const& e) // Invalid locale
    {
      localename = std::locale::classic().name();
    }
  std::string::size_type pos;
  bool status = false;

  // Strip off any charset.
  if ((pos = localename.find_first_of('.')) != std::string::npos)
    localename = localename.substr(0, pos);
  status = get_locale_string(group, key, localename, value);

  // Strip off territory.
  if (status == false &&
      (pos = localename.find_first_of('_')) != std::string::npos)
    {
      localename = localename.substr(0, pos);
      status = get_locale_string(group, key, localename, value);
    }

  // Fall back to non-localised version.
  if (status == false)
    status = get_value(group, key, value);

  return status;
}

bool
sbuild::basic_keyfile::get_locale_string (group_name_type const& group,
                                          key_type const&        key,
                                          priority               priority,
                                          value_type&            value) const
{
  bool status = get_locale_string(group, key, value);
  check_priority(group, key, priority, status);
  return status;
}

bool
sbuild::basic_keyfile::get_locale_string (group_name_type const& group,
                                          key_type const&        key,
                                          std::string const&     locale,
                                          value_type&            value) const
{
  std::string lkey = key + '[' + locale + ']';
  return get_value(group, lkey, value);
}

bool
sbuild::basic_keyfile::get_locale_string (group_name_type const& group,
                                          key_type const&        key,
                                          std::string const&     locale,
                                          priority               priority,
                                          value_type&            value) const
{
  bool status = get_locale_string(group, key, locale, value);
  check_priority(group, key, priority, status);
  return status;
}

void
sbuild::basic_keyfile::remove_group (group_name_type const& group)
{
  group_map_type::iterator pos = this->groups.find(group);
  if (pos != this->groups.end())
    this->groups.erase(pos);
}

void
sbuild::basic_keyfile::remove_key (group_name_type const& group,
                                   key_type const&        key)
{
  group_type *found_group = find_group(group);
  if (found_group)
    {
      item_map_type& items = std::get<1>(*found_group);
      item_map_type::iterator pos = items.find(key);
      if (pos != items.end())
        items.erase(pos);
    }
}

sbuild::basic_keyfile&
sbuild::basic_keyfile::operator += (basic_keyfile const& rhs)
{
  for (const auto& gp : rhs.groups)
    {
      group_type const& group = gp.second;
      group_name_type const& groupname = std::get<0>(group);
      comment_type const& comment = std::get<2>(group);
      size_type const& line = std::get<3>(group);
      set_group(groupname, comment, line);

      item_map_type const& items(std::get<1>(group));
      for (const auto& it : items)
        {
          item_type const& item = it.second;
          key_type const& key(std::get<0>(item));
          value_type const& value(std::get<1>(item));
          comment_type const& comment(std::get<2>(item));
          size_type const& line(std::get<3>(item));
          set_value(groupname, key, value, comment, line);
        }
    }
  return *this;
}

sbuild::basic_keyfile
operator + (sbuild::basic_keyfile const& lhs,
            sbuild::basic_keyfile const& rhs)
{
  sbuild::basic_keyfile ret(lhs);
  ret += rhs;
  return ret;
}

const sbuild::basic_keyfile::group_type *
sbuild::basic_keyfile::find_group (group_name_type const& group) const
{
  group_map_type::const_iterator pos = this->groups.find(group);
  if (pos != this->groups.end())
    return &pos->second;

  return 0;
}

sbuild::basic_keyfile::group_type *
sbuild::basic_keyfile::find_group (group_name_type const& group)
{
  group_map_type::iterator pos = this->groups.find(group);
  if (pos != this->groups.end())
    return &pos->second;

  return 0;
}

const sbuild::basic_keyfile::item_type *
sbuild::basic_keyfile::find_item (group_name_type const& group,
                                  key_type const&        key) const
{
  const group_type *found_group = find_group(group);
  if (found_group)
    {
      item_map_type const& items = std::get<1>(*found_group);
      item_map_type::const_iterator pos = items.find(key);
      if (pos != items.end())
        return &pos->second;
    }

  return 0;
}

sbuild::basic_keyfile::item_type *
sbuild::basic_keyfile::find_item (group_name_type const& group,
                                  key_type const&        key)
{
  group_type *found_group = find_group(group);
  if (found_group)
    {
      item_map_type& items = std::get<1>(*found_group);
      item_map_type::iterator pos = items.find(key);
      if (pos != items.end())
        return &pos->second;
    }

  return 0;
}

void
sbuild::basic_keyfile::print_comment (comment_type const& comment,
                                      std::ostream&       stream)
{
  std::string::size_type last_pos = 0;
  std::string::size_type pos = comment.find_first_of('\n', last_pos);

  while (1)
    {
      if (last_pos == pos)
        stream << "#\n";
      else
        stream << '#' << comment.substr(last_pos, pos - last_pos) << '\n';

      // Find next
      if (pos < comment.length() - 1)
        {
          last_pos = pos + 1;
          pos = comment.find_first_of('\n', last_pos);
        }
      else
        break;
    }
}

void
sbuild::basic_keyfile::check_priority (group_name_type const& group,
                                       key_type const&        key,
                                       priority               priority,
                                       bool                   valid) const
{
  if (valid == false)
    {
      size_type gline = get_line(group);

      switch (priority)
        {
        case PRIORITY_REQUIRED:
          {
            if (gline)
              throw error(gline, group, MISSING_KEY, key);
            else
              throw error(group, MISSING_KEY_NL, key);
          }
          break;
        default:
          break;
        }
    }
  else
    {
      size_type line = get_line(group, key);

      switch (priority)
        {
        case PRIORITY_DEPRECATED:
          {
            if (line)
              {
                error e(line, group, DEPRECATED_KEY, key);
                e.set_reason(_("This option will be removed in the future; please update your configuration"));
                log_exception_warning(e);
              }
            else
              {
                error e(group, DEPRECATED_KEY_NL, key);
                e.set_reason(_("This option will be removed in the future; please update your configuration"));
                log_exception_warning(e);
              }
          }
          break;
        case PRIORITY_OBSOLETE:
          {
            if (line)
              {
                error e(line, group, OBSOLETE_KEY, key);
                e.set_reason(_("This option has been removed, and no longer has any effect"));
                log_exception_warning(e);
              }
            else
              {
                error e(group, OBSOLETE_KEY_NL, key);
                e.set_reason(_("This option has been removed, and no longer has any effect"));
                log_exception_warning(e);
              }
          }
          break;
        case PRIORITY_DISALLOWED:
          {
            if (line)
              throw error(line, group, DISALLOWED_KEY, key);
            else
              throw error(group, DISALLOWED_KEY_NL, key);
          }
          break;
        default:
          break;
        }
    }
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
