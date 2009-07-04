/* Copyright © 2008-2009  Jan-Marek Glogowski <glogow@fbihome.de>
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

#include "config.h"

#include <algorithm>
#include <set>

#include <sbuild/sbuild-chroot-loopback.h>

#include "test-helpers.h"
#include "test-sbuild-chroot.h"

#include <cppunit/extensions/HelperMacros.h>

#include <iostream>

using std::cout;
using std::endl;

using namespace CppUnit;

class chroot_loopback : public sbuild::chroot_loopback
{
public:
  chroot_loopback():
    sbuild::chroot_loopback()
  {}

  virtual ~chroot_loopback()
  {}
};

class test_chroot_loopback : public test_chroot_base<chroot_loopback>
{
  CPPUNIT_TEST_SUITE(test_chroot_loopback);
  CPPUNIT_TEST(test_file);
  CPPUNIT_TEST(test_mount_options);
  CPPUNIT_TEST(test_setup_env);
  CPPUNIT_TEST(test_setup_env_fsunion);
  CPPUNIT_TEST(test_session_flags);
  CPPUNIT_TEST(test_print_details);
  CPPUNIT_TEST(test_print_config);
  CPPUNIT_TEST_SUITE_END();

protected:
  std::string loopback_file;

public:
  test_chroot_loopback():
    test_chroot_base<chroot_loopback>(),
    loopback_file()
  {
    loopback_file = abs_testdata_dir;
    loopback_file.append("/loopback-file");
  }

  void setUp()
  {
    test_chroot_base<chroot_loopback>::setUp();
    std::tr1::shared_ptr<sbuild::chroot_loopback> c = std::tr1::dynamic_pointer_cast<sbuild::chroot_loopback>(chroot);
    c->set_mount_options("-t jfs -o quota,rw");
  }

  void
  test_file()
  {
    std::tr1::shared_ptr<sbuild::chroot_loopback> c = std::tr1::dynamic_pointer_cast<sbuild::chroot_loopback>(chroot);
    CPPUNIT_ASSERT(c);
    c->set_file("/dev/some/file");
    CPPUNIT_ASSERT(c->get_file() == "/dev/some/file");
  }

  void
  test_mount_options()
  {
    std::tr1::shared_ptr<sbuild::chroot_loopback> c = std::tr1::dynamic_pointer_cast<sbuild::chroot_loopback>(chroot);
    CPPUNIT_ASSERT(c);
    c->set_mount_options("-o opt1,opt2");
    CPPUNIT_ASSERT(c->get_mount_options() == "-o opt1,opt2");
  }

  void test_setup_env()
  {
    std::tr1::shared_ptr<sbuild::chroot_loopback> c = std::tr1::dynamic_pointer_cast<sbuild::chroot_loopback>(chroot);
    CPPUNIT_ASSERT(c);
    c->set_file(loopback_file);

    sbuild::environment expected;
    expected.add("CHROOT_TYPE",           "loopback");
    expected.add("CHROOT_NAME",           "test-name");
    expected.add("CHROOT_DESCRIPTION",    "test-description");
    expected.add("CHROOT_MOUNT_LOCATION", "/mnt/mount-location");
    expected.add("CHROOT_PATH",           "/mnt/mount-location");
    expected.add("CHROOT_FILE",           loopback_file);
    expected.add("CHROOT_MOUNT_DEVICE",   loopback_file);
    expected.add("CHROOT_MOUNT_OPTIONS",  "-t jfs -o quota,rw");
    expected.add("CHROOT_SCRIPT_CONFIG",  sbuild::normalname(std::string(PACKAGE_SYSCONF_DIR) + "/script-defaults"));
    expected.add("CHROOT_SESSION_CLONE",  "false");
    expected.add("CHROOT_SESSION_CREATE", "false");
    expected.add("CHROOT_SESSION_PURGE",  "false");
    expected.add("CHROOT_UNION_TYPE",     "none");

    test_chroot_base<chroot_loopback>::test_setup_env(expected);
  }

  void test_setup_env_fsunion()
  {
    std::tr1::shared_ptr<sbuild::chroot_loopback> c = std::tr1::dynamic_pointer_cast<sbuild::chroot_loopback>(chroot);
    CPPUNIT_ASSERT(c);
    c->set_file(loopback_file);
    c->set_union_type("aufs");
    c->set_union_overlay_directory("/overlay");
    c->set_union_underlay_directory("/underlay");

    sbuild::environment expected;
    expected.add("CHROOT_TYPE",           "loopback");
    expected.add("CHROOT_NAME",           "test-name");
    expected.add("CHROOT_DESCRIPTION",    "test-description");
    expected.add("CHROOT_MOUNT_LOCATION", "/mnt/mount-location");
    expected.add("CHROOT_PATH",           "/mnt/mount-location");
    expected.add("CHROOT_FILE",           loopback_file);
    expected.add("CHROOT_MOUNT_DEVICE",   loopback_file);
    expected.add("CHROOT_MOUNT_OPTIONS",  "-t jfs -o quota,rw");
    expected.add("CHROOT_SCRIPT_CONFIG",  sbuild::normalname(std::string(PACKAGE_SYSCONF_DIR) + "/script-defaults"));
    expected.add("CHROOT_SESSION_CLONE",  "true");
    expected.add("CHROOT_SESSION_CREATE", "true");
    expected.add("CHROOT_SESSION_PURGE",  "false");
    expected.add("CHROOT_UNION_TYPE",     "aufs");
    expected.add("CHROOT_UNION_OVERLAY_DIRECTORY",  "/overlay");
    expected.add("CHROOT_UNION_UNDERLAY_DIRECTORY",  "/underlay");

    test_chroot_base<chroot_loopback>::test_setup_env(expected);
  }

  void test_session_flags()
  {
    CPPUNIT_ASSERT(chroot->get_session_flags() ==
		   sbuild::chroot::SESSION_NOFLAGS);
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

CPPUNIT_TEST_SUITE_REGISTRATION(test_chroot_loopback);
