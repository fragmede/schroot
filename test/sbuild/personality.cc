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

#include <sbuild/personality.h>

#include <iostream>
#include <sstream>

TEST(Personality, Construct1)
{
  sbuild::personality p;
  ASSERT_EQ(p.get_name(), "undefined");
}

TEST(Personality, Construct2)
{
  sbuild::personality p("linux");
  ASSERT_EQ(p.get_name(), "linux");
}

TEST(Personality, Construct3)
{
  ASSERT_THROW(sbuild::personality p("invalid_personality"), sbuild::personality::error);
}

TEST(Personality, StreamOutput)
{
  sbuild::personality p("linux");

  std::ostringstream os;
  os << p;

  ASSERT_EQ(os.str(), "linux");
}

TEST(Personality, StreamInput1)
{
  sbuild::personality p;
  std::istringstream is("undefined");
  ASSERT_NO_THROW(is >> p);
  ASSERT_EQ(p.get_name(), "undefined");
}

TEST(Personality, StreamInput2)
{
  sbuild::personality p;
  std::istringstream is("linux");
  ASSERT_NO_THROW(is >> p);
  ASSERT_EQ(p.get_name(), "linux");
}

TEST(Personality, StreamInput3)
{
  sbuild::personality p;
  std::istringstream is("invalid_personality");
  ASSERT_THROW(is >> p, sbuild::personality::error);
}

TEST(Personality, Set1)
{
  sbuild::personality p;
  p.set_name("undefined");
  ASSERT_EQ(p.get_name(), "undefined");
}

TEST(Personality, Set2)
{
  sbuild::personality p;
  p.set_name("linux");
  ASSERT_EQ(p.get_name(), "linux");
}

TEST(Personality, Set3)
{
  sbuild::personality p;
  ASSERT_THROW(p.set_name("invalid_personality"), sbuild::personality::error);
}
