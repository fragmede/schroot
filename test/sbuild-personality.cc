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

#include <iostream>
#include <sstream>

#include <cppunit/extensions/HelperMacros.h>

#include <schroot/sbuild-personality.h>

using namespace CppUnit;

class test_personality : public TestCase
{
  CPPUNIT_TEST_SUITE(test_personality);
  CPPUNIT_TEST(test_construction);
  CPPUNIT_TEST_SUITE_END();

public:
  test_personality()
  {}

  virtual ~test_personality()
  {}

  void
  test_construction()
  {
    sbuild::personality p1();
    CPPUNIT_ASSERT(p.get_name() == "undefined");

    sbuild::personality p2(0xffffffff);
    CPPUNIT_ASSERT(p2.get_name() == "unknown");

    sbuild::personality p3("invalid_personality");
    CPPUNIT_ASSERT(p3.get_name() == "unknown");

    sbuild::personality p4("linux");
#ifdef __linux__
    CPPUNIT_ASSERT(p4.get_name() == "linux");
#else
    CPPUNIT_ASSERT(p4.get_name() == "unknown");
#endif
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(test_personality);

/*
 * Local Variables:
 * mode:C++
 * End:
 */
