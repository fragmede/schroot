/* Copyright © 2005-2008  Roger Leigh <rleigh@codelibre.net>
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

#include <algorithm>
#include <fstream>
#include <set>

template <typename K, typename P>
sbuild::basic_keyfile<K, P>::basic_keyfile ():
  groups(),
  separator(",")
{
}

template <typename K, typename P>
sbuild::basic_keyfile<K, P>::basic_keyfile (std::string const& file):
  groups(),
  separator(",")
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

template <typename K, typename P>
sbuild::basic_keyfile<K, P>::basic_keyfile (std::istream& stream):
  groups(),
  separator(",")
{
  stream >> *this;
}

template <typename K, typename P>
sbuild::basic_keyfile<K, P>::~basic_keyfile()
{
}

template <typename K, typename P>
typename sbuild::basic_keyfile<K, P>::group_list
sbuild::basic_keyfile<K, P>::get_groups () const
{
  group_list ret;

  for (typename group_map_type::const_iterator pos = this->groups.begin();
       pos != this->groups.end();
       ++pos)
    ret.push_back(pos->first);

  return ret;
}

template <typename K, typename P>
typename sbuild::basic_keyfile<K, P>::key_list
sbuild::basic_keyfile<K, P>::get_keys (group_name_type const& group) const
{
  key_list ret;

  const group_type *found_group = find_group(group);
  if (found_group)
    {
      item_map_type const& items(std::get<1>(*found_group));
      for (typename item_map_type::const_iterator pos = items.begin();
           pos != items.end();
           ++pos)
        ret.push_back(pos->first);
    }

  return ret;
}

template <typename K, typename P>
void
sbuild::basic_keyfile<K, P>::check_keys (group_name_type const& group,
                                         key_list const&        keys) const
{
  const string_list total(get_keys(group));

  const string_set a(total.begin(), total.end());
  const string_set b(keys.begin(), keys.end());

  string_set unused;

  set_difference(a.begin(), a.end(),
                 b.begin(), b.end(),
                 inserter(unused, unused.begin()));

  for (string_set::const_iterator pos = unused.begin();
       pos != unused.end();
       ++pos)
    {
      size_type line = get_line(group, *pos);
      error e(line, group, UNKNOWN_KEY, *pos);
      e.set_reason(_("This option may be present in a newer version"));
      log_exception_warning(e);
    }
}

template <typename K, typename P>
bool
sbuild::basic_keyfile<K, P>::has_group (group_name_type const& group) const
{
  return (find_group(group) != 0);
}

template <typename K, typename P>
bool
sbuild::basic_keyfile<K, P>::has_key (group_name_type const& group,
                                      key_type const&        key) const
{
  return (find_item(group, key) != 0);
}

template <typename K, typename P>
void
sbuild::basic_keyfile<K, P>::set_group (group_name_type const& group,
                                        comment_type const&    comment)
{
  set_group(group, comment, 0);
}

template <typename K, typename P>
void
sbuild::basic_keyfile<K, P>::set_group (group_name_type const& group,
                                        comment_type const&    comment,
                                        size_type              line)
{
  if (!has_group(group))
    this->groups.insert
      (typename group_map_type::value_type(group,
                                           group_type(group,
                                                      item_map_type(),
                                                      comment,
                                                      line)));
}

template <typename K, typename P>
typename sbuild::basic_keyfile<K, P>::comment_type
sbuild::basic_keyfile<K, P>::get_comment (group_name_type const& group) const
{
  const group_type *found_group = find_group(group);
  if (found_group)
    return std::get<2>(*found_group);
  else
    return comment_type();
}

template <typename K, typename P>
typename sbuild::basic_keyfile<K, P>::comment_type
sbuild::basic_keyfile<K, P>::get_comment (group_name_type const& group,
                                          key_type const&        key) const
{
  const item_type *found_item = find_item(group, key);
  if (found_item)
      return std::get<2>(*found_item);
  else
    return comment_type();
}

template <typename K, typename P>
typename sbuild::basic_keyfile<K, P>::size_type
sbuild::basic_keyfile<K, P>::get_line (group_name_type const& group) const
{
  const group_type *found_group = find_group(group);
  if (found_group)
    return std::get<3>(*found_group);
  else
    return 0;
}

template <typename K, typename P>
typename sbuild::basic_keyfile<K, P>::size_type
sbuild::basic_keyfile<K, P>::get_line (group_name_type const& group,
                                       key_type const&        key) const
{
  const item_type *found_item = find_item(group, key);
  if (found_item)
      return std::get<3>(*found_item);
  else
    return 0;
}

template <typename K, typename P>
bool
sbuild::basic_keyfile<K, P>::get_locale_string (group_name_type const& group,
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
      localename = std::locale::classic();
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

template <typename K, typename P>
bool
sbuild::basic_keyfile<K, P>::get_locale_string (group_name_type const& group,
                                                key_type const&        key,
                                                priority               priority,
                                                value_type&            value) const
{
  bool status = get_locale_string(group, key, value);
  check_priority(group, key, priority, status);
  return status;
}

template <typename K, typename P>
bool
sbuild::basic_keyfile<K, P>::get_locale_string (group_name_type const& group,
                                                key_type const&        key,
                                                std::string const&     locale,
                                                value_type&            value) const
{
  std::string lkey = key + '[' + locale + ']';
  return get_value(group, lkey, value);
}

template <typename K, typename P>
bool
sbuild::basic_keyfile<K, P>::get_locale_string (group_name_type const& group,
                                                key_type const&        key,
                                                std::string const&     locale,
                                                priority               priority,
                                                value_type&            value) const
{
  bool status = get_locale_string(group, key, locale, value);
  check_priority(group, key, priority, status);
  return status;
}

template <typename K, typename P>
void
sbuild::basic_keyfile<K, P>::remove_group (group_name_type const& group)
{
  typename group_map_type::iterator pos = this->groups.find(group);
  if (pos != this->groups.end())
    this->groups.erase(pos);
}

template <typename K, typename P>
void
sbuild::basic_keyfile<K, P>::remove_key (group_name_type const& group,
                                         key_type const&        key)
{
  group_type *found_group = find_group(group);
  if (found_group)
    {
      item_map_type& items = std::get<1>(*found_group);
      typename item_map_type::iterator pos = items.find(key);
      if (pos != items.end())
        items.erase(pos);
    }
}

template <typename K, typename P>
sbuild::basic_keyfile<K, P>&
sbuild::basic_keyfile<K, P>::operator += (basic_keyfile const& rhs)
{
  for (typename group_map_type::const_iterator gp = rhs.groups.begin();
       gp != rhs.groups.end();
       ++gp)
    {
      group_type const& group = gp->second;
      group_name_type const& groupname = std::get<0>(group);
      comment_type const& comment = std::get<2>(group);
      size_type const& line = std::get<3>(group);
      set_group(groupname, comment, line);

      item_map_type const& items(std::get<1>(group));
      for (typename item_map_type::const_iterator it = items.begin();
           it != items.end();
           ++it)
        {
          item_type const& item = it->second;
          key_type const& key(std::get<0>(item));
          value_type const& value(std::get<1>(item));
          comment_type const& comment(std::get<2>(item));
          size_type const& line(std::get<3>(item));
          set_value(groupname, key, value, comment, line);
        }
    }
  return *this;
}

template <typename _K>
sbuild::basic_keyfile<_K>
operator + (sbuild::basic_keyfile<_K> const& lhs,
            sbuild::basic_keyfile<_K> const& rhs)
{
  sbuild::basic_keyfile<_K> ret(lhs);
  ret += rhs;
  return ret;
}

template <typename K, typename P>
const typename sbuild::basic_keyfile<K, P>::group_type *
sbuild::basic_keyfile<K, P>::find_group (group_name_type const& group) const
{
  typename group_map_type::const_iterator pos = this->groups.find(group);
  if (pos != this->groups.end())
    return &pos->second;

  return 0;
}

template <typename K, typename P>
typename sbuild::basic_keyfile<K, P>::group_type *
sbuild::basic_keyfile<K, P>::find_group (group_name_type const& group)
{
  typename group_map_type::iterator pos = this->groups.find(group);
  if (pos != this->groups.end())
    return &pos->second;

  return 0;
}

template <typename K, typename P>
const typename sbuild::basic_keyfile<K, P>::item_type *
sbuild::basic_keyfile<K, P>::find_item (group_name_type const& group,
                                        key_type const&        key) const
{
  const group_type *found_group = find_group(group);
  if (found_group)
    {
      item_map_type const& items = std::get<1>(*found_group);
      typename item_map_type::const_iterator pos = items.find(key);
      if (pos != items.end())
        return &pos->second;
    }

  return 0;
}

template <typename K, typename P>
typename sbuild::basic_keyfile<K, P>::item_type *
sbuild::basic_keyfile<K, P>::find_item (group_name_type const& group,
                                        key_type const&        key)
{
  group_type *found_group = find_group(group);
  if (found_group)
    {
      item_map_type& items = std::get<1>(*found_group);
      typename item_map_type::iterator pos = items.find(key);
      if (pos != items.end())
        return &pos->second;
    }

  return 0;
}

template <typename K, typename P>
void
sbuild::basic_keyfile<K, P>::print_comment (comment_type const& comment,
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

template <typename K, typename P>
void
sbuild::basic_keyfile<K, P>::check_priority (group_name_type const& group,
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
