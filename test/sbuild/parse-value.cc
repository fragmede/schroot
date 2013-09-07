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

#include <sbuild/parse-value.h>

TEST(ParseValue, Bool)
{
  bool result;

  result = false;
  sbuild::parse_value("true", result);
  EXPECT_TRUE(result);
  result = false;
  sbuild::parse_value("yes", result);
  EXPECT_TRUE(result);
  result = false;
  sbuild::parse_value("1", result);
  EXPECT_TRUE(result);

  result = true;
  sbuild::parse_value("false", result);
  EXPECT_FALSE(result);
  result = true;
  sbuild::parse_value("no", result);
  EXPECT_FALSE(result);
  result = true;
  sbuild::parse_value("0", result);
  EXPECT_FALSE(result);
}

TEST(ParseValue, BoolFail)
{
  bool result = true;

  EXPECT_THROW(sbuild::parse_value("invalid", result), sbuild::parse_value_error);
  EXPECT_TRUE(result);
}

TEST(ParseValue, Int)
{
  int result = 0;
  sbuild::parse_value("23", result);
  EXPECT_EQ(result, 23);
}

TEST(ParseValue, IntFail)
{
  int result = 22;

  EXPECT_THROW(sbuild::parse_value("invalid", result), sbuild::parse_value_error);
  EXPECT_EQ(result, 22);
}

TEST(ParseValue, String)
{
  std::string result;

  sbuild::parse_value("test string", result);
  EXPECT_EQ(result, "test string");
}
