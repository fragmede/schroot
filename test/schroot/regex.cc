/* Copyright © 2006-2013  Roger Leigh <rleigh@codelibre.net>
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

#include <schroot/regex.h>

#include <iostream>
#include <sstream>

TEST(Regex, Construct)
{
  EXPECT_NO_THROW(schroot::regex r1);

  EXPECT_NO_THROW(schroot::regex r2("foo"));

  std::string p("foo|bar$");
  EXPECT_NO_THROW(schroot::regex r3(p));

  EXPECT_THROW(schroot::regex r("[foo"), std::regex_error);
}

TEST(Regex, StreamOutput)
{
  schroot::regex r("foo");
  std::ostringstream o;
  o << r;
  ASSERT_EQ(r.str(),"foo");
  ASSERT_EQ(o.str(),"foo");
}

TEST(Regex, StreamInput)
{
  schroot::regex r;
  std::istringstream i("foo");
  i >> r;
  ASSERT_EQ(r.str(),"foo");
}

TEST(Regex, Match)
{
  schroot::regex r("^[^:/,.][^:/,]*$");
  ASSERT_TRUE(schroot::regex_search("foobar", r));
  ASSERT_FALSE(schroot::regex_search(":fail:", r));
}

TEST(Regex, MatchBracket1)
{
  schroot::regex r("^[a-z0-9][a-z0-9-]*$");
  ASSERT_TRUE(schroot::regex_search("foobar", r));
  ASSERT_TRUE(schroot::regex_search("a-", r));
  ASSERT_FALSE(schroot::regex_search("-a", r));
}

TEST(Regex, MatchBracket2)
{
  schroot::regex r("^[a-z0-9][-a-z0-9]*$");
  ASSERT_TRUE(schroot::regex_search("foobar", r));
  ASSERT_TRUE(schroot::regex_search("a-", r));
  ASSERT_FALSE(schroot::regex_search("-a", r));
}

TEST(Regex, StreamInputFail)
{
  schroot::regex r;
  std::istringstream i("([[invalid_regex");
  EXPECT_THROW(i >> r, std::regex_error);
}
