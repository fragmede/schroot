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

#include <config.h>

#include <sbuild/sbuild-chroot-directory.h>

#include "test-helpers.h"
#include "test-sbuild-chroot.h"

#include <algorithm>
#include <set>

#include <cppunit/extensions/HelperMacros.h>

using namespace CppUnit;

class chroot_directory : public sbuild::chroot_directory
{
public:
  chroot_directory():
    sbuild::chroot_directory()
  {}

  virtual ~chroot_directory()
  {}
};

class test_chroot_directory : public test_chroot_base<chroot_directory>
{
  CPPUNIT_TEST_SUITE(test_chroot_directory);
  CPPUNIT_TEST(test_directory);
  CPPUNIT_TEST(test_chroot_type);
  CPPUNIT_TEST(test_setup_env);
  CPPUNIT_TEST(test_setup_env2);
  CPPUNIT_TEST(test_session_flags);
  CPPUNIT_TEST(test_print_details);
  CPPUNIT_TEST(test_print_config);
  CPPUNIT_TEST_SUITE_END();

public:
  test_chroot_directory():
    test_chroot_base<chroot_directory>()
  {}

  void setUp()
  {
    test_chroot_base<chroot_directory>::setUp();
    sbuild::chroot_directory *c = dynamic_cast<sbuild::chroot_directory *>(chroot.get());
    c->set_directory("/srv/chroot/example-chroot");
  }

  void
  test_directory()
  {
    sbuild::chroot_directory *c = dynamic_cast<sbuild::chroot_directory *>(chroot.get());
    CPPUNIT_ASSERT(c);
    chroot->set_run_setup_scripts(true);
    CPPUNIT_ASSERT(chroot->get_mount_location() == "/mnt/mount-location");
    chroot->set_run_setup_scripts(false);
    c->set_directory("/mnt/mount-location/example");
    chroot->set_mount_location("");
    CPPUNIT_ASSERT(c->get_directory() == "/mnt/mount-location/example");
    CPPUNIT_ASSERT(chroot->get_path() == "/mnt/mount-location/example");
    CPPUNIT_ASSERT(chroot->get_mount_location() == "");
  }

  void test_chroot_type()
  {
    CPPUNIT_ASSERT(chroot->get_chroot_type() == "directory");
  }

  void test_setup_env()
  {
    sbuild::environment expected;
    expected.add("CHROOT_TYPE",           "directory");
    expected.add("CHROOT_NAME",           "test-name");
    expected.add("CHROOT_DESCRIPTION",    "test-description");
    expected.add("CHROOT_MOUNT_LOCATION", "/mnt/mount-location");
    expected.add("CHROOT_DIRECTORY",      "/srv/chroot/example-chroot");
    expected.add("CHROOT_PATH",           "/srv/chroot/example-chroot");
    expected.add("CHROOT_SCRIPT_CONFIG",  sbuild::normalname(std::string(PACKAGE_SYSCONF_DIR) + "/script-defaults"));
    expected.add("CHROOT_SESSION_CLONE",  "false");
    expected.add("CHROOT_SESSION_CREATE", "false");
    expected.add("CHROOT_SESSION_PURGE",  "false");

    test_chroot_base<chroot_directory>::test_setup_env(expected);
  }

  void test_setup_env2()
  {
    chroot->set_run_setup_scripts(true);

    sbuild::environment expected;
    expected.add("CHROOT_TYPE",           "directory");
    expected.add("CHROOT_NAME",           "test-name");
    expected.add("CHROOT_DESCRIPTION",    "test-description");
    expected.add("CHROOT_MOUNT_LOCATION", "/mnt/mount-location");
    expected.add("CHROOT_DIRECTORY",       "/srv/chroot/example-chroot");
    expected.add("CHROOT_PATH",           "/mnt/mount-location");
    expected.add("CHROOT_SCRIPT_CONFIG",  sbuild::normalname(std::string(PACKAGE_SYSCONF_DIR) + "/script-defaults"));
    expected.add("CHROOT_SESSION_CLONE",  "false");
    expected.add("CHROOT_SESSION_CREATE", "true");
    expected.add("CHROOT_SESSION_PURGE",  "false");

    test_chroot_base<chroot_directory>::test_setup_env(expected);
  }

  void test_session_flags()
  {
    CPPUNIT_ASSERT(chroot->get_session_flags() ==
		   sbuild::chroot::SESSION_NOFLAGS);

    chroot->set_run_setup_scripts(true);
    CPPUNIT_ASSERT(chroot->get_session_flags() ==
		   sbuild::chroot::SESSION_CREATE);
  }

  void test_print_details()
  {
    std::ostringstream os;
    os << chroot;
    // TODO: Compare output.
    CPPUNIT_ASSERT(!os.str().empty());
  }

  void test_print_config()
  {
    std::ostringstream os;
    sbuild::keyfile config;
    config << chroot;
    os << config;
    // TODO: Compare output.
    CPPUNIT_ASSERT(!os.str().empty());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(test_chroot_directory);
