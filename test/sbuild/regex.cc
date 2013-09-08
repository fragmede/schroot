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

#include <sbuild/regex.h>

#include <iostream>
#include <sstream>

TEST(Regex, Construct)
{
  EXPECT_NO_THROW(sbuild::regex r1);

  EXPECT_NO_THROW(sbuild::regex r2("foo"));

  std::string p("foo|bar$");
  EXPECT_NO_THROW(sbuild::regex r3(p));

  EXPECT_THROW(sbuild::regex r("[foo"), std::regex_error);
}

TEST(Regex, StreamOutput)
{
  sbuild::regex r("foo");
  std::ostringstream o;
  o << r;
  ASSERT_EQ(r.str(),"foo");
  ASSERT_EQ(o.str(),"foo");
}

TEST(Regex, StreamInput)
{
  sbuild::regex r;
  std::istringstream i("foo");
  i >> r;
  ASSERT_EQ(r.str(),"foo");
}

TEST(Regex, Match)
{
  sbuild::regex r("^[^:/,.][^:/,]*$");
  ASSERT_TRUE(sbuild::regex_search("foobar", r));
  ASSERT_FALSE(sbuild::regex_search(":fail:", r));
}

TEST(Regex, StreamInputFail)
{
  sbuild::regex r;
  std::istringstream i("([[invalid_regex");
  EXPECT_THROW(i >> r, std::regex_error);
}
