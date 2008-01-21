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

#include "sbuild-environment.h"

using boost::format;
using namespace sbuild;

environment::environment ():
  std::map<std::string,std::string>(),
  filter()
{
}

environment::environment (char **environment):
  std::map<std::string,std::string>()
{
  add(environment);
}

environment::~environment ()
{
}

void
environment::set_filter (regex const& filter)
{
  this->filter = filter;
}

regex const&
environment::get_filter () const
{
  return this->filter;
}

void
environment::add (char **environment)
{
  if (environment)
    {
      for (char **ev = environment; ev != 0 && *ev != 0; ++ev)
	add(std::string(*ev));
    }
}

void
environment::add (environment const& environment)
{
  for (const_iterator pos = environment.begin();
       pos != environment.end();
       ++pos)
    add(*pos);
}

void
environment::add (std::string const& value)
{
  std::string::size_type pos = value.find('=');
  if (pos != std::string::npos && pos != 0)
    {
      std::string key = value.substr(0, pos);
      std::string val;
      if (pos < value.length())
	val = value.substr(pos + 1);
      add(std::make_pair(key, val));
    }
  else
    {
      add(std::make_pair(value, std::string()));
    }
}

void
environment::add (value_type const& value)
{
  remove(value);
  if (!value.first.empty() && !value.second.empty())
    {
      if (this->filter.str().empty() ||
	  !regex_search(value.first, this->filter))
	{
	  insert(value);
	  log_debug(DEBUG_NOTICE) << "Inserted into environment: "
				  << value.first << '=' << value.second
				  << std::endl;
	}
      else
	log_debug(DEBUG_INFO) << "Filtered from environment: " << value.first
			      << std::endl;
    }
}

void
environment::remove (char **environment)
{
  if (environment)
    {
      for (char **ev = environment; ev != 0 && *ev != 0; ++ev)
	remove(std::string(*ev));
    }
}

void
environment::remove (environment const& environment)
{
  for (const_iterator pos = environment.begin();
       pos != environment.end();
       ++pos)
    remove(*pos);
}

void
environment::remove (std::string const& value)
{
  std::string::size_type pos = value.find('=');
  if (pos != std::string::npos && pos != 0)
    {
      std::string key = value.substr(0, pos);
      std::string val;
      if (pos < value.length())
	val = value.substr(pos + 1);
      remove(std::make_pair(key, val));
    }
  else
    {
      remove(std::make_pair(value, std::string()));
    }
}

void
environment::remove (value_type const& value)
{
  iterator pos = find(value.first);
  if (pos != end())
    erase(pos);
}

char **
environment::get_strv () const
{
  char **ret = new char *[size() + 1];

  size_type idx = 0;
  for (const_iterator pos = begin(); pos != end(); ++pos, ++idx)
    {
      std::string envitem = pos->first + "=" + pos->second;
      ret[idx] = new char[envitem.length() + 1];
      std::strcpy(ret[idx], envitem.c_str());
    }
  ret[size()] = 0;

  return ret;
}
