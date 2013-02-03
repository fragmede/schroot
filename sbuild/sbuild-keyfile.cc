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

#include "sbuild-keyfile.h"

using namespace sbuild;

keyfile::keyfile():
  basic_keyfile<keyfile_traits>()
{}

keyfile::keyfile (std::string const& file):
  basic_keyfile<keyfile_traits>()
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

keyfile::keyfile (std::istream& stream):
  basic_keyfile<keyfile_traits>()
{
  stream >> *this;
}

keyfile::~keyfile()
{}
