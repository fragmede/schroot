/* Copyright © 2006-2007  Roger Leigh <rleigh@debian.org>
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

#include <sbuild/sbuild-regex.h>

#include <iostream>
#include <sstream>

#include <cppunit/extensions/HelperMacros.h>

using namespace CppUnit;

class test_regex : public TestCase
{
  CPPUNIT_TEST_SUITE(test_regex);
  CPPUNIT_TEST(test_construction);
  CPPUNIT_TEST_EXCEPTION(test_construction_fail, boost::regex_error);
  CPPUNIT_TEST(test_output);
  CPPUNIT_TEST(test_input);
  CPPUNIT_TEST_EXCEPTION(test_input_fail, boost::regex_error);
  CPPUNIT_TEST_SUITE_END();

public:
  test_regex()
  {}

  virtual ~test_regex()
  {}

  void
  test_construction()
  {
    sbuild::regex r1;

    sbuild::regex r2("foo");

    std::string p("foo|bar$");
    sbuild::regex r3(p);
  }

  void
  test_construction_fail()
  {
    sbuild::regex r("[foo");
  }

  void
  test_output()
  {
    sbuild::regex r("foo");
    std::ostringstream o;
    o << r;
    CPPUNIT_ASSERT(o.str() == "foo");
  }

  void
  test_input()
  {
    sbuild::regex r;
    std::istringstream i("foo");
    i >> r;
    CPPUNIT_ASSERT(r.str() == "foo");
  }

  void
  test_input_fail()
  {
    sbuild::regex r;
    std::istringstream i("([[invalid_regex");
    i >> r;
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(test_regex);