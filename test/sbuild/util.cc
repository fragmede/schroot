/* Copyright © 2006-2013  Roger Leigh <rleigh@debian.org>
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

#include <gtest/gtest.h>

#include <sbuild/util.h>

#include <cstdlib>

TEST(Util, Basename)
{
  ASSERT_EQ(sbuild::basename("/usr/bin/perl"), "perl");
  ASSERT_EQ(sbuild::basename("/usr/lib"), "lib");
  ASSERT_EQ(sbuild::basename("/usr/"), "usr");
  ASSERT_EQ(sbuild::basename("usr"), "usr");
  ASSERT_EQ(sbuild::basename("/"), "/");
  ASSERT_EQ(sbuild::basename("."), ".");
  ASSERT_EQ(sbuild::basename(".."), "..");
}

TEST(Util, Dirname)
{
  ASSERT_EQ(sbuild::dirname("/usr/bin/perl"), "/usr/bin");
  ASSERT_EQ(sbuild::dirname("/usr/lib"), "/usr");
  ASSERT_EQ(sbuild::dirname("/usr/"), "/");
  ASSERT_EQ(sbuild::dirname("usr"), ".");
  ASSERT_EQ(sbuild::dirname("/"), "/");
  ASSERT_EQ(sbuild::dirname("."), ".");
  ASSERT_EQ(sbuild::dirname(".."), ".");
}

TEST(Util, StringListToString)
{
  sbuild::string_list items;
  items.push_back("foo");
  items.push_back("bar");
  items.push_back("baz");

  ASSERT_EQ(sbuild::string_list_to_string(items, "--"), "foo--bar--baz");
}

TEST(Util, SplitString)
{
  sbuild::string_list items =
    sbuild::split_string("/usr/share/info", "/");

  ASSERT_EQ(items.size(), 3);
  ASSERT_EQ(items[0], "usr");
  ASSERT_EQ(items[1], "share");
  ASSERT_EQ(items[2], "info");
}

TEST(Util, FindProgramInPath)
{
  std::string path("/usr/local/bin:/usr/bin:/bin");
  ASSERT_EQ(sbuild::find_program_in_path("sh", path, ""), "/bin/sh");
  ASSERT_EQ(sbuild::find_program_in_path("sed", path, ""), "/bin/sed");
}
