/* sbuild-keyfile - sbuild GKeyFile wrapper
 *
 * Copyright © 2005  Roger Leigh <rleigh@debian.org>
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

inline bool keyfile_read_bool(GKeyFile   *keyfile,
				const std::string& group,
				const std::string& key,
				bool&       value)
{
  GError *error = 0;
  bool b = g_key_file_get_boolean(keyfile, group.c_str(), key.c_str(), &error);
  if (!error)
    {
      value = b;
      return true;
    }
  else
    return false;
}

inline bool keyfile_read_uint(GKeyFile   *keyfile,
			      const std::string& group,
			      const std::string& key,
			      unsigned int&       value)
{
  GError *error = 0;
  int num = g_key_file_get_integer(keyfile, group.c_str(), key.c_str(), &error);
  if (!error)
    {
      value = num;
      return true;
    }
  else
    return false;
}

inline bool keyfile_read_string(GKeyFile   *keyfile,
				const std::string& group,
				const std::string& key,
				std::string&       value)
{
  GError *error = 0;
  char *str = g_key_file_get_string(keyfile, group.c_str(), key.c_str(), &error);
  if (!error && str)
    {
      value = str;
      g_free(str);
      return true;
    }
  else
    return false;
}

inline bool keyfile_read_string_list(GKeyFile   *keyfile,
				     const std::string& group,
				     const std::string& key,
				     SbuildChroot::string_list& value)
{
  GError *error = 0;
  char **strv = g_key_file_get_string_list(keyfile, group.c_str(), key.c_str(), 0, &error);
  if (!error && strv)
    {
      SbuildChroot::string_list newlist;
      for (char *pos = strv[0]; pos != 0; ++pos)
	newlist.push_back(pos);
      value = newlist;
      g_strfreev(strv);
      return true;
    }
  else
    return false;
}
