/* Copyright © 2006-2008  Roger Leigh <rleigh@debian.org>
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

#ifndef TEST_SBUILD_CHROOT_H
#define TEST_SBUILD_CHROOT_H

#include <sbuild/sbuild-chroot.h>

#include <algorithm>
#include <iostream>
#include <set>

#include <cppunit/extensions/HelperMacros.h>

using namespace CppUnit;

template <class T>
class test_chroot_base : public TestFixture
{
protected:
  sbuild::chroot::ptr chroot;
  std::string abs_testdata_dir;

public:
  test_chroot_base():
    TestFixture(),
    chroot(),
    abs_testdata_dir()
  {
    char cwd[FILENAME_MAX];
    if (NULL != getcwd(cwd, FILENAME_MAX))
      {
        abs_testdata_dir = std::string(cwd);
        abs_testdata_dir.append("/" TESTDATADIR);
      }
  }

  virtual ~test_chroot_base()
  {}

  void setUp()
  {
    this->chroot = sbuild::chroot::ptr(new T);
    chroot->set_name("test-name");
    chroot->set_description("test-description");
    chroot->set_mount_location("/mnt/mount-location");
  }

  void tearDown()
  {
    this->chroot = sbuild::chroot::ptr();
  }

  void test_setup_env(const sbuild::environment& test_environment)
  {
    sbuild::environment env;
    chroot->setup_env(env);

    CPPUNIT_ASSERT(env.size() != 0);

    std::set<std::string> expected;
    for (sbuild::environment::const_iterator pos = test_environment.begin();
	 pos != test_environment.end();
	 ++pos)
      expected.insert(pos->first);

    std::set<std::string> found;
    for (sbuild::environment::const_iterator pos = env.begin();
	 pos != env.end();
	 ++pos)
      found.insert(pos->first);

    sbuild::string_list missing;
    set_difference(expected.begin(), expected.end(),
		   found.begin(), found.end(),
		   std::back_inserter(missing));
    if (!missing.empty())
      {
	for (sbuild::string_list::const_iterator pos = missing.begin();
	     pos != missing.end();
	     ++pos)
	  std::cout << "Missing environment: " << *pos << std::endl;
      }
    CPPUNIT_ASSERT(missing.empty());

    sbuild::string_list extra;
    set_difference(found.begin(), found.end(),
		   expected.begin(), expected.end(),
		   std::back_inserter(extra));
    if (!extra.empty())
      {
	for (sbuild::string_list::const_iterator pos = extra.begin();
	     pos != extra.end();
	     ++pos)
	  std::cout << "Additional environment: " << *pos << std::endl;
      }
    CPPUNIT_ASSERT(extra.empty());

    for (sbuild::environment::const_iterator pos = test_environment.begin();
	 pos != test_environment.end();
	 ++pos)
      {
	std::string checkval;
	CPPUNIT_ASSERT(env.get(pos->first, checkval) == true);

	if (checkval != pos->second)
	  std::cout << "Environment error (" << pos->first << "): "
		    << checkval << " [observed] != "
		    << pos->second << " [expected]"
		    << std::endl;
	CPPUNIT_ASSERT(checkval == pos->second);
      }
  }

  // TODO: All chroot types should check text output matches.  If
  // possible, test chroot locking functions, but this is going to be
  // tricky without having root in many cases.

};

#endif /* TEST_SBUILD_CHROOT_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
