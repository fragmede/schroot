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

#include <sbuild/sbuild-dirstream.h>
#include <sbuild/sbuild-util.h>

#include <iostream>
#include <sstream>

#include <cppunit/extensions/HelperMacros.h>

using namespace CppUnit;

class test_dirstream : public TestCase
{
  CPPUNIT_TEST_SUITE(test_dirstream);
  CPPUNIT_TEST(test_construction);
  CPPUNIT_TEST_EXCEPTION(test_construction_fail, sbuild::dirstream::error);
  CPPUNIT_TEST(test_read);
  CPPUNIT_TEST_SUITE_END();

public:
  test_dirstream()
  {}

  virtual ~test_dirstream()
  {}

  void
  test_construction()
  {
    sbuild::dirstream str(TESTDATADIR);
    CPPUNIT_ASSERT(str);
  }

  void
  test_construction_fail()
  {
    sbuild::dirstream str(TESTDATADIR "/invalid_dir");
    CPPUNIT_ASSERT(!str);
  }

  void test_read()
  {
    std::set<std::string> expected;
    std::set<std::string> actual;

    expected.insert(".");
    expected.insert("..");
    expected.insert("empty");
    expected.insert("experimental");
    expected.insert("file");
    expected.insert("sarge");
    expected.insert("sid");
    expected.insert("woody");

    sbuild::dirstream str(TESTDATADIR "/config.ex2");

    sbuild::direntry e;
    while (str >> e)
      actual.insert(e.name());

    CPPUNIT_ASSERT(expected == actual);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(test_dirstream);
