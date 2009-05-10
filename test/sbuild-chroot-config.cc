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

#include <sbuild/sbuild-chroot-config.h>
#include <sbuild/sbuild-nostream.h>

#include <fstream>
#include <sstream>
#include <vector>

#include <cppunit/extensions/HelperMacros.h>

using namespace CppUnit;

class test_config : public TestFixture
{
  CPPUNIT_TEST_SUITE(test_config);
  CPPUNIT_TEST(test_construction_file);
  CPPUNIT_TEST(test_construction_dir);
  CPPUNIT_TEST_EXCEPTION(test_construction_fail, sbuild::error_base);
  CPPUNIT_TEST(test_add_file);
  CPPUNIT_TEST(test_add_dir);
  CPPUNIT_TEST_EXCEPTION(test_add_fail, sbuild::error_base);
  CPPUNIT_TEST(test_get_chroots);
  CPPUNIT_TEST(test_find_chroot);
  CPPUNIT_TEST(test_find_alias);
  CPPUNIT_TEST(test_get_chroot_list);
  CPPUNIT_TEST(test_print_chroot_list);
  CPPUNIT_TEST(test_print_chroot_info);
  CPPUNIT_TEST(test_validate_chroots);
  CPPUNIT_TEST_EXCEPTION(test_config_fail, sbuild::error_base);
  CPPUNIT_TEST(test_config_deprecated);
  CPPUNIT_TEST(test_config_valid);
  CPPUNIT_TEST_SUITE_END();

protected:
  sbuild::chroot_config *cf;

public:
  test_config():
    TestFixture(),
    cf()
  {}

  virtual ~test_config()
  {}

  void setUp()
  {
    this->cf = new sbuild::chroot_config(TESTDATADIR "/config.ex1", false);
  }

  void tearDown()
  {
    delete this->cf;
  }

  void test_construction_file()
  {
    sbuild::chroot_config c(TESTDATADIR "/config.ex1", false);
  }

  void test_construction_dir()
  {
    sbuild::chroot_config c(TESTDATADIR "/config.ex2", false);
  }

  void test_construction_fail()
  {
    sbuild::chroot_config c(TESTDATADIR "/config.nonexistent", false);
  }

  void test_construction_fail_wrong()
  {
    sbuild::chroot_config c(TESTDATADIR "/config.ex3", false);
  }

  void test_add_file()
  {
    sbuild::chroot_config c;
    c.add(TESTDATADIR "/config.ex1", false);
  }

  void test_add_dir()
  {
    sbuild::chroot_config c;
    c.add(TESTDATADIR "/config.ex2", false);
  }

  void test_add_fail()
  {
    sbuild::chroot_config c;
    c.add(TESTDATADIR "/config.nonexistent", false);
  }

  void test_get_chroots()
  {
    CPPUNIT_ASSERT(this->cf->get_chroots().size() == 4);
  }

  void test_find_chroot()
  {
    sbuild::chroot::ptr chroot;

    chroot = this->cf->find_chroot("sid");
    CPPUNIT_ASSERT((chroot));
    CPPUNIT_ASSERT(chroot->get_name() == "sid");

    chroot = this->cf->find_chroot("stable");
    CPPUNIT_ASSERT((!chroot));

    chroot = this->cf->find_chroot("invalid");
    CPPUNIT_ASSERT((!chroot));
  }

  void test_find_alias()
  {
    sbuild::chroot::ptr chroot;

    chroot = this->cf->find_alias("sid");
    CPPUNIT_ASSERT((chroot));
    CPPUNIT_ASSERT(chroot->get_name() == "sid");

    chroot = this->cf->find_alias("stable");
    CPPUNIT_ASSERT((chroot));
    CPPUNIT_ASSERT(chroot->get_name() == "sarge");

    chroot = this->cf->find_alias("invalid");
    CPPUNIT_ASSERT((!chroot));
  }

  void test_get_chroot_list()
  {
    sbuild::string_list chroots = this->cf->get_chroot_list();
    CPPUNIT_ASSERT(chroots.size() == 7); // Includes aliases
    CPPUNIT_ASSERT(chroots[0] == "default");
    CPPUNIT_ASSERT(chroots[1] == "experimental");
    CPPUNIT_ASSERT(chroots[2] == "sarge");
    CPPUNIT_ASSERT(chroots[3] == "sid");
    CPPUNIT_ASSERT(chroots[4] == "sid-local");
    CPPUNIT_ASSERT(chroots[5] == "stable");
    CPPUNIT_ASSERT(chroots[6] == "unstable");
  }

  void test_print_chroot_list()
  {
    this->cf->print_chroot_list(sbuild::cnull);
  }

  void test_print_chroot_info()
  {
    this->cf->print_chroot_info(this->cf->get_chroot_list(), sbuild::cnull);
  }

  void test_validate_chroots()
  {
    sbuild::string_list chroots;
    chroots.push_back("default");
    chroots.push_back("invalid");
    chroots.push_back("invalid2");
    chroots.push_back("sarge");
    chroots.push_back("unstable");

    sbuild::string_list invalid = this->cf->validate_chroots(chroots);
    CPPUNIT_ASSERT(invalid.size() == 2);
    CPPUNIT_ASSERT(invalid[0] == "invalid");
    CPPUNIT_ASSERT(invalid[1] == "invalid2");
  }

  void test_config_fail()
  {
    sbuild::chroot_config c(TESTDATADIR "/config-directory-fail.ex", false);
  }

  void test_config_deprecated()
  {
    sbuild::chroot_config c(TESTDATADIR "/config-directory-deprecated.ex", false);
  }

  void test_config_valid()
  {
    sbuild::chroot_config c(TESTDATADIR "/config-directory-valid.ex", false);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(test_config);
