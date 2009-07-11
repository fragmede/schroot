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

#include <sbuild/sbuild-chroot-lvm-snapshot.h>
#include <sbuild/sbuild-util.h>

#include "test-helpers.h"
#include "test-sbuild-chroot.h"

#include <algorithm>
#include <set>

#include <cppunit/extensions/HelperMacros.h>

using namespace CppUnit;

class chroot_lvm_snapshot : public sbuild::chroot_lvm_snapshot
{
public:
  chroot_lvm_snapshot():
    sbuild::chroot_lvm_snapshot()
  {}

  virtual ~chroot_lvm_snapshot()
  {}
};

class test_chroot_lvm_snapshot : public test_chroot_base<chroot_lvm_snapshot>
{
  CPPUNIT_TEST_SUITE(test_chroot_lvm_snapshot);
  CPPUNIT_TEST(test_snapshot_device);
  CPPUNIT_TEST(test_snapshot_options);
  CPPUNIT_TEST(test_chroot_type);
  CPPUNIT_TEST(test_setup_env);
  CPPUNIT_TEST(test_setup_keyfile);
  CPPUNIT_TEST(test_setup_keyfile_source);
  CPPUNIT_TEST(test_session_flags);
  CPPUNIT_TEST(test_print_details);
  CPPUNIT_TEST(test_print_config);
  CPPUNIT_TEST_SUITE_END();

public:
  test_chroot_lvm_snapshot():
    test_chroot_base<chroot_lvm_snapshot>()
  {}

  void setUp()
  {
    test_chroot_base<chroot_lvm_snapshot>::setUp();
    std::tr1::shared_ptr<sbuild::chroot_lvm_snapshot> c = std::tr1::dynamic_pointer_cast<sbuild::chroot_lvm_snapshot>(chroot);
    c->set_device("/dev/testdev");
    c->set_mount_options("-t jfs -o quota,rw");
    c->set_location("/squeeze");
    c->set_snapshot_device("/dev/snaptestdev");
    c->set_snapshot_options("--size 1G");
    setup_source();
  }

  void
  test_snapshot_device()
  {
    std::tr1::shared_ptr<sbuild::chroot_lvm_snapshot> c = std::tr1::dynamic_pointer_cast<sbuild::chroot_lvm_snapshot>(chroot);
    CPPUNIT_ASSERT(c);
    c->set_snapshot_device("/dev/some/snapshot/device");
    CPPUNIT_ASSERT(c->get_snapshot_device() == "/dev/some/snapshot/device");
  }

  void
  test_snapshot_options()
  {
    std::tr1::shared_ptr<sbuild::chroot_lvm_snapshot> c = std::tr1::dynamic_pointer_cast<sbuild::chroot_lvm_snapshot>(chroot);
    CPPUNIT_ASSERT(c);
    c->set_snapshot_options("-o opt1,opt2");
    CPPUNIT_ASSERT(c->get_snapshot_options() == "-o opt1,opt2");
  }

  void test_chroot_type()
  {
    CPPUNIT_ASSERT(chroot->get_chroot_type() == "lvm-snapshot");
  }

  void test_setup_env()
  {
    std::tr1::shared_ptr<sbuild::chroot_lvm_snapshot> c = std::tr1::dynamic_pointer_cast<sbuild::chroot_lvm_snapshot>(chroot);
    CPPUNIT_ASSERT(c);

    sbuild::environment expected;
    setup_env_chroot(expected);
    expected.add("CHROOT_TYPE",           "lvm-snapshot");
    expected.add("CHROOT_LOCATION",       "/squeeze");
    expected.add("CHROOT_MOUNT_LOCATION", "/mnt/mount-location");
    expected.add("CHROOT_PATH",           "/mnt/mount-location/squeeze");
    expected.add("CHROOT_DEVICE",         "/dev/testdev");
    expected.add("CHROOT_MOUNT_DEVICE",   "/dev/snaptestdev");
    expected.add("CHROOT_MOUNT_OPTIONS",  "-t jfs -o quota,rw");
    expected.add("CHROOT_LVM_SNAPSHOT_NAME", sbuild::basename(c->get_snapshot_device()));
    expected.add("CHROOT_LVM_SNAPSHOT_DEVICE", "/dev/snaptestdev");
    expected.add("CHROOT_LVM_SNAPSHOT_OPTIONS", "--size 1G");
    expected.add("CHROOT_SESSION_CLONE",  "true");
    expected.add("CHROOT_SESSION_CREATE", "true");
    expected.add("CHROOT_SESSION_PURGE",  "false");

    test_chroot_base<chroot_lvm_snapshot>::test_setup_env(expected);
  }

  void test_setup_keyfile()
  {
    sbuild::keyfile expected;
    std::string group = chroot->get_name();
    setup_keyfile_chroot(expected, group);
    setup_keyfile_source(expected, group);
    expected.set_value(group, "active", "false");
    expected.set_value(group, "type", "lvm-snapshot");
    expected.set_value(group, "device", "/dev/testdev");
    expected.set_value(group, "location", "/squeeze");
    expected.set_value(group, "mount-options", "-t jfs -o quota,rw");
    expected.set_value(group, "lvm-snapshot-options", "--size 1G");

    test_chroot_base<chroot_lvm_snapshot>::test_setup_keyfile(expected, chroot->get_name());
  }

  void test_setup_keyfile_source()
  {
    std::tr1::shared_ptr<sbuild::chroot_source> c =
      std::tr1::dynamic_pointer_cast<sbuild::chroot_source>(chroot);

    CPPUNIT_ASSERT(c->get_source_clonable() == true);

    sbuild::chroot::ptr source = c->clone_source();
    std::tr1::shared_ptr<sbuild::chroot_source> s =
      std::tr1::dynamic_pointer_cast<sbuild::chroot_source>(source);

    CPPUNIT_ASSERT(s->get_source_clonable() == false);

    sbuild::keyfile expected;
    const std::string group(source->get_name());
    setup_keyfile_chroot(expected, group);
    setup_keyfile_union_unconfigured(expected, group);
    expected.set_value(group, "active", "false");
    expected.set_value(group, "type", "block-device");
    expected.set_value(group, "description", "test-description (source chroot)");
    expected.set_value(group, "aliases", "test-alias-1-source,test-alias-2-source");
    expected.set_value(group, "device", "/dev/testdev");
    expected.set_value(group, "location", "/squeeze");
    expected.set_value(group, "mount-options", "-t jfs -o quota,rw");
    // Block device union unconfigured.
    setup_keyfile_source_clone(expected, group);

    sbuild::keyfile observed;
    source->get_keyfile(observed);

    test_chroot_base<chroot_lvm_snapshot>::test_setup_keyfile
      (observed, group, expected, group);
  }

  void test_session_flags()
  {
    CPPUNIT_ASSERT(chroot->get_session_flags() ==
		   (sbuild::chroot::SESSION_CREATE |
		    sbuild::chroot::SESSION_CLONE));
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

CPPUNIT_TEST_SUITE_REGISTRATION(test_chroot_lvm_snapshot);
