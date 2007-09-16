/* Copyright © 2006-2007  Roger Leigh <rleigh@debian.org>
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

#include <sbuild/sbuild-personality.h>

#include <iostream>
#include <sstream>

#include <cppunit/extensions/HelperMacros.h>

using namespace CppUnit;

class test_personality : public TestCase
{
  CPPUNIT_TEST_SUITE(test_personality);
  CPPUNIT_TEST(test_construction);
  CPPUNIT_TEST(test_output);
  CPPUNIT_TEST(test_input);
  CPPUNIT_TEST_EXCEPTION(test_input_fail, sbuild::personality::error);
  CPPUNIT_TEST_SUITE_END();

public:
  test_personality()
  {}

  virtual ~test_personality()
  {}

  void
  test_construction()
  {
    sbuild::personality p1;
#ifdef __linux__
    CPPUNIT_ASSERT(p1.get_name() == "linux" ||
		   p1.get_name() == "linux_32bit" ||
		   p1.get_name() == "linux32");
#else
    CPPUNIT_ASSERT(p1.get_name() == "undefined");
#endif

    sbuild::personality p2(0xffffffff);
    CPPUNIT_ASSERT(p2.get_name() == "undefined");

    sbuild::personality p3("invalid_personality");
    CPPUNIT_ASSERT(p3.get_name() == "undefined");

    sbuild::personality p4("linux");
#ifdef __linux__
    CPPUNIT_ASSERT(p4.get_name() == "linux");
#else
    CPPUNIT_ASSERT(p4.get_name() == "unknown");
#endif
  }

  void
  test_output()
  {
    sbuild::personality p2(0xffffffff);
    std::ostringstream ps2;
    ps2 << p2;
    CPPUNIT_ASSERT(ps2.str() == "undefined");

    sbuild::personality p3("invalid_personality");
    std::ostringstream ps3;
    ps3 << p3;
    CPPUNIT_ASSERT(ps3.str() == "undefined");

    sbuild::personality p4("linux");
    std::ostringstream ps4;
    ps4 << p4;
#ifdef __linux__
    CPPUNIT_ASSERT(ps4.str() == "linux");
#else
    CPPUNIT_ASSERT(ps4.str() == "unknown");
#endif
  }

  void
  test_input()
  {
    sbuild::personality p2;
    std::istringstream ps2("undefined");
    ps2 >> p2;
    CPPUNIT_ASSERT(p2.get_name() == "undefined");

    sbuild::personality p4;
#ifdef __linux__
    std::istringstream ps4("linux");
#else
    std::istringstream ps4("unknown");
#endif
    ps4 >> p4;
#ifdef __linux__
    CPPUNIT_ASSERT(p4.get_name() == "linux");
#else
    CPPUNIT_ASSERT(p4.get_name() == "unknown");
#endif
  }

  void
  test_input_fail()
  {
    sbuild::personality p3;
    std::istringstream ps3("invalid_personality");
    ps3 >> p3;
    CPPUNIT_ASSERT(p3.get_name() == "undefined");
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(test_personality);
