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

#include <sbuild/sbuild-config.h>
#include <sbuild/sbuild-chroot-plain.h>

#include "test-helpers.h"
#include "test-sbuild-chroot.h"

#include <algorithm>
#include <set>

#include <cppunit/extensions/HelperMacros.h>

using namespace CppUnit;

class chroot_plain : public sbuild::chroot_plain
{
public:
  chroot_plain():
    sbuild::chroot_plain()
  {}

  virtual ~chroot_plain()
  {}
};

class test_chroot_plain : public test_chroot_base<chroot_plain>
{
  CPPUNIT_TEST_SUITE(test_chroot_plain);
  CPPUNIT_TEST(test_directory);
  CPPUNIT_TEST(test_chroot_type);
  CPPUNIT_TEST(test_setup_env);
  CPPUNIT_TEST(test_setup_keyfile);
  CPPUNIT_TEST(test_print_details);
  CPPUNIT_TEST(test_print_config);
  CPPUNIT_TEST_SUITE_END();

public:
  test_chroot_plain():
    test_chroot_base<chroot_plain>()
  {}

  void setUp()
  {
    test_chroot_base<chroot_plain>::setUp();
    CPPUNIT_ASSERT(!session);
    CPPUNIT_ASSERT(!source);
    CPPUNIT_ASSERT(!chroot_union);
    CPPUNIT_ASSERT(!session_union);
    CPPUNIT_ASSERT(!source_union);
  }

  virtual void setup_chroot_props (sbuild::chroot::ptr& chroot)
  {
    test_chroot_base<chroot_plain>::setup_chroot_props(chroot);

    std::tr1::shared_ptr<sbuild::chroot_plain> c = std::tr1::dynamic_pointer_cast<sbuild::chroot_plain>(chroot);

    c->set_mount_location("");
    c->set_directory("/srv/chroot/example-chroot");
  }

  void
  test_directory()
  {
    std::tr1::shared_ptr<sbuild::chroot_plain> c = std::tr1::dynamic_pointer_cast<sbuild::chroot_plain>(chroot);
    CPPUNIT_ASSERT(c);
    c->set_directory("/mnt/mount-location/example");
    CPPUNIT_ASSERT(c->get_directory() == "/mnt/mount-location/example");
    CPPUNIT_ASSERT(chroot->get_path() == "/mnt/mount-location/example");
    CPPUNIT_ASSERT(chroot->get_mount_location() == "");
  }

  void test_chroot_type()
  {
    CPPUNIT_ASSERT(chroot->get_chroot_type() == "plain");
  }

  void test_setup_env()
  {
    sbuild::environment expected;
    setup_env_chroot(expected);
    expected.add("CHROOT_TYPE",           "plain");
    expected.add("CHROOT_DIRECTORY",      "/srv/chroot/example-chroot");
    expected.add("CHROOT_PATH",           "/srv/chroot/example-chroot");
    expected.add("CHROOT_SESSION_CLONE",  "false");
    expected.add("CHROOT_SESSION_CREATE", "false");
    expected.add("CHROOT_SESSION_PURGE",  "false");

    test_chroot_base<chroot_plain>::test_setup_env(chroot, expected);
  }

  void test_setup_keyfile()
  {
    sbuild::keyfile expected;
    std::string group = chroot->get_name();
    setup_keyfile_chroot(expected, group);
    expected.set_value(group, "active", "false");
    expected.set_value(group, "type", "plain");
    expected.set_value(group, "directory", "/srv/chroot/example-chroot");

    test_chroot_base<chroot_plain>::test_setup_keyfile
      (chroot, expected, group);
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

CPPUNIT_TEST_SUITE_REGISTRATION(test_chroot_plain);
