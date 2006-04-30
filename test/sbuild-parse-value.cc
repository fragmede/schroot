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
  CPPUNIT_TEST(test_bool_fail);
  CPPUNIT_TEST(test_int);
  CPPUNIT_TEST(test_int_fail);
  CPPUNIT_TEST(test_string);
  CPPUNIT_TEST_SUITE_END();

public:
  test_parse_value()
  {}

  void test_bool()
  {
    bool result;

    result = false;
    CPPUNIT_ASSERT((result = sbuild::parse_value("true")) == true);
    result = false;
    CPPUNIT_ASSERT((result = sbuild::parse_value("yes")) == true);
    result = false;
    CPPUNIT_ASSERT((result = sbuild::parse_value("1")) == true);

    result = true;
    CPPUNIT_ASSERT((result = sbuild::parse_value("false")) == false);
    result = true;
    CPPUNIT_ASSERT((result = sbuild::parse_value("no")) == false);
    result = true;
    CPPUNIT_ASSERT((result = sbuild::parse_value("0")) == false);
  }

  void test_bool_fail()
  {
    bool result = true;

    try
      {
	result = sbuild::parse_value("invalid");
      }
    catch (sbuild::parse_value::error const& e)
      {
	// Exception thown, and original value unmodified.
	CPPUNIT_ASSERT(result == true);
	return;
      }
    // Should never be reached
    CPPUNIT_ASSERT(false);
  }

  void test_int()
  {
    int result = 0;

    CPPUNIT_ASSERT((result = sbuild::parse_value("23")) == 23);
  }

  void test_int_fail()
  {
    int result = 22;

    try
      {
	result = sbuild::parse_value("invalid");
      }
    catch (sbuild::parse_value::error const& e)
      {
	// Exception thown, and original value unmodified.
	CPPUNIT_ASSERT(result == 22);
	return;
      }
    // Should never be reached
    CPPUNIT_ASSERT(false);
  }

  void test_string()
  {
    std::string result;

    CPPUNIT_ASSERT((result = static_cast<std::string const&>(sbuild::parse_value("test string"))) ==
      "test string");
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(test_parse_value);

/*
 * Local Variables:
 * mode:C++
 * End:
 */
