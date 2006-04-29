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

#include <schroot/sbuild-chroot-file.h>

#include "test-helpers.h"
#include "test-sbuild-chroot.h"

using namespace CppUnit;

class chroot_file : public sbuild::chroot_file
{
public:
  chroot_file():
    sbuild::chroot_file()
  {}

  virtual ~chroot_file()
  {}
};

class test_chroot_file : public test_chroot_base<chroot_file>
{
  CPPUNIT_TEST_SUITE(test_chroot_file);
  CPPUNIT_TEST(test_file);
  CPPUNIT_TEST(test_chroot_type);
  CPPUNIT_TEST(test_setup_env);
  CPPUNIT_TEST(test_session_flags);
  CPPUNIT_TEST(test_print_details);
  CPPUNIT_TEST(test_print_config);
  CPPUNIT_TEST_SUITE_END();

public:
  test_chroot_file():
    test_chroot_base<chroot_file>()
  {}

  void setUp()
  {
    test_chroot_base<chroot_file>::setUp();
    sbuild::chroot_file *c = dynamic_cast<sbuild::chroot_file *>(chroot.get());
    c->set_file("/srv/chroot/example.tar.bz2");
  }

  void
  test_file()
  {
    sbuild::chroot_file *c = dynamic_cast<sbuild::chroot_file *>(chroot.get());
    CPPUNIT_ASSERT(c);
    c->set_file("/srv/chroot-images/unstable.tar.gz");
    CPPUNIT_ASSERT(c->get_file() == "/srv/chroot-images/unstable.tar.gz");
  }

  void test_chroot_type()
  {
    CPPUNIT_ASSERT(chroot->get_chroot_type() == "file");
  }

  void test_setup_env()
  {
    sbuild::environment expected;
    expected.add("CHROOT_TYPE",           "file");
    expected.add("CHROOT_NAME",           "test-name");
    expected.add("CHROOT_DESCRIPTION",    "test-description");
    expected.add("CHROOT_FILE",           "/srv/chroot/example.tar.bz2");
    expected.add("CHROOT_FILE_REPACK",    "false");
    expected.add("CHROOT_MOUNT_LOCATION", "/mnt/mount-location");
    expected.add("CHROOT_PATH",           "/mnt/mount-location");
    expected.add("CHROOT_MOUNT_DEVICE",   "/dev/device-to-mount");

    test_chroot_base<chroot_file>::test_setup_env(expected);
  }

  void test_session_flags()
  {
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

CPPUNIT_TEST_SUITE_REGISTRATION(test_chroot_file);

/*
 * Local Variables:
 * mode:C++
 * End:
 */
