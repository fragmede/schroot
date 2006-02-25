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

#include <algorithm>
#include <set>

#include <cppunit/extensions/HelperMacros.h>

#include <schroot/sbuild-chroot-block-device.h>

#include "test-helpers.h"
#include "test-sbuild-chroot.h"

using namespace CppUnit;

class chroot_block_device : public sbuild::chroot_block_device
{
public:
  chroot_block_device():
    sbuild::chroot_block_device()
  {}

  virtual ~chroot_block_device()
  {}
};

class test_chroot_block_device : public test_chroot_base<chroot_block_device>
{
  CPPUNIT_TEST_SUITE(test_chroot_block_device);
  CPPUNIT_TEST(test_device);
  CPPUNIT_TEST(test_mount_options);
  CPPUNIT_TEST(test_chroot_type);
  CPPUNIT_TEST(test_setup_env);
  CPPUNIT_TEST(test_session_flags);
  CPPUNIT_TEST(test_print_details);
  CPPUNIT_TEST(test_print_config);
  CPPUNIT_TEST_SUITE_END();

public:
  test_chroot_block_device():
    test_chroot_base<chroot_block_device>()
  {}

  void setUp()
  {
    test_chroot_base<chroot_block_device>::setUp();
    sbuild::chroot_block_device *c = dynamic_cast<sbuild::chroot_block_device *>(chroot.get());
    c->set_device("/dev/testdev");
    c->set_mount_options("-t jfs -o quota,rw");
  }

  void
  test_device()
  {
    sbuild::chroot_block_device *c = dynamic_cast<sbuild::chroot_block_device *>(chroot.get());
    CPPUNIT_ASSERT(c);
    c->set_device("/dev/some/device");
    CPPUNIT_ASSERT(c->get_device() == "/dev/some/device");
    CPPUNIT_ASSERT(chroot->get_mount_device() ==
		   "/dev/some/device");
  }

  void
  test_mount_options()
  {
    sbuild::chroot_block_device *c = dynamic_cast<sbuild::chroot_block_device *>(chroot.get());
    CPPUNIT_ASSERT(c);
    c->set_mount_options("-o opt1,opt2");
    CPPUNIT_ASSERT(c->get_mount_options() == "-o opt1,opt2");
  }

  void test_chroot_type()
  {
    CPPUNIT_ASSERT(chroot->get_chroot_type() == "block-device");
  }

  void test_setup_env()
  {
    sbuild::environment expected;
    expected.add("CHROOT_TYPE",           "block-device");
    expected.add("CHROOT_NAME",           "test-name");
    expected.add("CHROOT_DESCRIPTION",    "test-description");
    expected.add("CHROOT_MOUNT_LOCATION", "/mnt/mount-location");
    expected.add("CHROOT_PATH",           "/mnt/mount-location");
    expected.add("CHROOT_MOUNT_DEVICE",   "/dev/testdev");
    expected.add("CHROOT_DEVICE",         "/dev/testdev");
    expected.add("CHROOT_MOUNT_OPTIONS",  "-t jfs -o quota,rw");

    test_chroot_base<chroot_block_device>::test_setup_env(expected);
  }

  void test_session_flags()
  {
    CPPUNIT_ASSERT(chroot->get_session_flags() ==
		   static_cast<sbuild::chroot::session_flags>(0));
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

CPPUNIT_TEST_SUITE_REGISTRATION(test_chroot_block_device);

/*
 * Local Variables:
 * mode:C++
 * End:
 */
