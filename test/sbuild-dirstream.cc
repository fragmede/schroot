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
    sbuild::dirstream str(SRCDIR);
    CPPUNIT_ASSERT(str);
  }

  void
  test_construction_fail()
  {
    sbuild::dirstream str(SRCDIR "/invalid_dir");
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

    sbuild::dirstream str(SRCDIR "/config.ex2");

    sbuild::direntry e;
    while (str >> e)
      {
	if (e.name() == ".svn") // To cater for running in SVN tree.
	  continue;
	actual.insert(e.name());
      }

    CPPUNIT_ASSERT(expected == actual);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(test_dirstream);

/*
 * Local Variables:
 * mode:C++
 * End:
 */
