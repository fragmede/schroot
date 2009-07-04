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
  CPPUNIT_TEST(test_chroot_strings);
  CPPUNIT_TEST(test_setup_env);
  CPPUNIT_TEST(test_setup_env2);
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
    sbuild::chroot_loopback *c = dynamic_cast<sbuild::chroot_loopback *>(chroot.get());
    c->set_mount_options("-t jfs -o quota,rw");
  }

  void
  test_file()
  {
    sbuild::chroot_loopback *c = dynamic_cast<sbuild::chroot_loopback *>(chroot.get());
    CPPUNIT_ASSERT(c);
    c->set_container("/dev/some/file");
    CPPUNIT_ASSERT(c->get_container() == "/dev/some/file");
  }

  void
  test_mount_options()
  {
    sbuild::chroot_loopback *c = dynamic_cast<sbuild::chroot_loopback *>(chroot.get());
    CPPUNIT_ASSERT(c);
    c->set_mount_options("-o opt1,opt2");
    CPPUNIT_ASSERT(c->get_mount_options() == "-o opt1,opt2");
  }

  void test_chroot_strings()
  {
    std::string type, key_name, detail_name;
    chroot->get_chroot_strings(&type, &key_name, &detail_name);
    
    CPPUNIT_ASSERT(type == "loopback");
    CPPUNIT_ASSERT(key_name == "file");
    CPPUNIT_ASSERT(detail_name == "File");
  }

  void setup_env_common(sbuild::environment& expected)
  {
    expected.add("CHROOT_TYPE",           "loopback");
    expected.add("CHROOT_NAME",           "test-name");
    expected.add("CHROOT_DESCRIPTION",    "test-description");
    expected.add("CHROOT_MOUNT_LOCATION", "/mnt/mount-location");
    expected.add("CHROOT_PATH",           "/mnt/mount-location");
    expected.add("CHROOT_MOUNT_OPTIONS",  "-t jfs -o quota,rw");
    expected.add("CHROOT_SCRIPT_CONFIG",  sbuild::normalname(std::string(PACKAGE_SYSCONF_DIR) + "/script-defaults"));
    expected.add("CHROOT_SESSION_CLONE",  "false");
    expected.add("CHROOT_SESSION_CREATE", "false");
    expected.add("CHROOT_SESSION_PURGE",  "false");
    expected.add("CHROOT_FS_UNION_TYPE",  "none");
  }

  void test_setup_env()
  {
    sbuild::environment expected;
    setup_env_common(expected);

    test_chroot_base<chroot_loopback>::test_setup_env(expected);
  }

  void test_setup_env2()
  {
    sbuild::chroot_loopback *c = dynamic_cast<sbuild::chroot_loopback *>(chroot.get());
    CPPUNIT_ASSERT(c);
    c->set_container(loopback_file);

    sbuild::environment expected;
    setup_env_common(expected);

    expected.add("CHROOT_FILE",           loopback_file);
    expected.add("CHROOT_CONTAINER",      loopback_file);
    expected.add("CHROOT_MOUNT_UUID",     FS_UUID);

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
