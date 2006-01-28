/* Copyright © 2006  Roger Leigh <rleigh@debian.org>
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

#include <cppunit/extensions/HelperMacros.h>

#include <schroot/sbuild-parse-value.h>

using namespace CppUnit;

class test_parse_value : public TestCase
{
  CPPUNIT_TEST_SUITE(test_parse_value);
  CPPUNIT_TEST(test_bool);
  CPPUNIT_TEST(test_int);
  CPPUNIT_TEST(test_string);
  CPPUNIT_TEST_SUITE_END();

public:
  test_parse_value()
  {}

  void test_bool()
  {
    bool result;

    result = false;
    CPPUNIT_ASSERT(sbuild::parse_value("true", result) == true &&
		   result == true);
    result = false;
    CPPUNIT_ASSERT(sbuild::parse_value("yes", result) == true &&
		   result == true);
    result = false;
    CPPUNIT_ASSERT(sbuild::parse_value("1", result) == true &&
		   result == true);

    result = true;
    CPPUNIT_ASSERT(sbuild::parse_value("false", result) == true &&
		   result == false);
    result = true;
    CPPUNIT_ASSERT(sbuild::parse_value("no", result) == true &&
		   result == false);
    result = true;
    CPPUNIT_ASSERT(sbuild::parse_value("0", result) == true &&
		   result == false);

    result = true;
    CPPUNIT_ASSERT(sbuild::parse_value("invalud", result) == false &&
		   result == true);
  }

  void test_int()
  {
    int result = 0;

    CPPUNIT_ASSERT(sbuild::parse_value("23", result) == true &&
		   result == 23);
    CPPUNIT_ASSERT(sbuild::parse_value("invalid", result) == false);
    CPPUNIT_ASSERT(result == 23);
  }

  void test_string()
  {
    std::string result;

    CPPUNIT_ASSERT(sbuild::parse_value("test string", result) == true &&
		   result == "test string");
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(test_parse_value);

/*
 * Local Variables:
 * mode:C++
 * End:
 */
