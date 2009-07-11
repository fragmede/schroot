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
#ifdef SBUILD_FEATURE_UNION
  CPPUNIT_TEST(test_setup_env_fsunion);
#endif // SBUILD_FEATURE_UNION
  CPPUNIT_TEST(test_setup_keyfile);
#ifdef SBUILD_FEATURE_UNION
  CPPUNIT_TEST(test_setup_keyfile_fsunion);
#endif // SBUILD_FEATURE_UNION
  CPPUNIT_TEST(test_session_flags);
#ifdef SBUILD_FEATURE_UNION
  CPPUNIT_TEST(test_session_flags_fsunion);
#endif // SBUILD_FEATURE_UNION
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
    std::tr1::shared_ptr<sbuild::chroot_directory> c = std::tr1::dynamic_pointer_cast<sbuild::chroot_directory>(chroot);
    c->set_directory("/srv/chroot/example-chroot");
    setup_source();
  }

  void
  test_directory()
  {
    std::tr1::shared_ptr<sbuild::chroot_directory> c = std::tr1::dynamic_pointer_cast<sbuild::chroot_directory>(chroot);
    CPPUNIT_ASSERT(c);
    c->set_directory("/mnt/chroot/example");
    CPPUNIT_ASSERT(chroot->get_mount_location() == "/mnt/mount-location");
    CPPUNIT_ASSERT(c->get_directory() == "/mnt/chroot/example");
    CPPUNIT_ASSERT(chroot->get_path() == "/mnt/mount-location");
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
    expected.add("CHROOT_PATH",           "/mnt/mount-location");
    expected.add("CHROOT_SCRIPT_CONFIG",  sbuild::normalname(std::string(PACKAGE_SYSCONF_DIR) + "/script-defaults"));
    expected.add("CHROOT_SESSION_CLONE",  "false");
    expected.add("CHROOT_SESSION_CREATE", "true");
    expected.add("CHROOT_SESSION_PURGE",  "false");
#ifdef SBUILD_FEATURE_UNION
    expected.add("CHROOT_UNION_TYPE",  "none");
#endif // SBUILD_FEATURE_UNION

    test_chroot_base<chroot_directory>::test_setup_env(expected);
  }

#ifdef SBUILD_FEATURE_UNION
  void test_setup_env_fsunion()
  {
    std::tr1::shared_ptr<sbuild::chroot_directory> c = std::tr1::dynamic_pointer_cast<sbuild::chroot_directory>(chroot);
    c->set_union_type("aufs");
    c->set_union_overlay_directory("/overlay");
    c->set_union_underlay_directory("/underlay");

    sbuild::environment expected;
    expected.add("CHROOT_TYPE",           "directory");
    expected.add("CHROOT_NAME",           "test-name");
    expected.add("CHROOT_DESCRIPTION",    "test-description");
    expected.add("CHROOT_MOUNT_LOCATION", "/mnt/mount-location");
    expected.add("CHROOT_DIRECTORY",      "/srv/chroot/example-chroot");
    expected.add("CHROOT_PATH",           "/mnt/mount-location");
    expected.add("CHROOT_SCRIPT_CONFIG",  sbuild::normalname(std::string(PACKAGE_SYSCONF_DIR) + "/script-defaults"));
    expected.add("CHROOT_SESSION_CLONE",  "true");
    expected.add("CHROOT_SESSION_CREATE", "true");
    expected.add("CHROOT_SESSION_PURGE",  "false");
    expected.add("CHROOT_UNION_TYPE",     "aufs");
    expected.add("CHROOT_UNION_OVERLAY_DIRECTORY",  "/overlay");
    expected.add("CHROOT_UNION_UNDERLAY_DIRECTORY",  "/underlay");

    test_chroot_base<chroot_directory>::test_setup_env(expected);
  }
#endif // SBUILD_FEATURE_UNION

  void test_setup_keyfile()
  {
    sbuild::keyfile expected;
    setup_keyfile_chroot(expected);
    expected.set_value(chroot->get_name(), "active", "false");
    expected.set_value(chroot->get_name(), "type", "directory");
    expected.set_value(chroot->get_name(), "directory", "/srv/chroot/example-chroot");
    setup_keyfile_union_unconfigured(expected);

    test_chroot_base<chroot_directory>::test_setup_keyfile(expected, chroot->get_name());
  }

#ifdef SBUILD_FEATURE_UNION
  void test_setup_keyfile_fsunion()
  {
    std::tr1::shared_ptr<sbuild::chroot_directory> c = std::tr1::dynamic_pointer_cast<sbuild::chroot_directory>(chroot);
    c->set_union_type("aufs");
    c->set_union_overlay_directory("/overlay");
    c->set_union_underlay_directory("/underlay");
    c->set_union_mount_options("union-mount-options");

    sbuild::keyfile expected;
    setup_keyfile_chroot(expected);
    setup_keyfile_source(expected);
    expected.set_value(chroot->get_name(), "active", "false");
    expected.set_value(chroot->get_name(), "type", "directory");
    expected.set_value(chroot->get_name(), "directory", "/srv/chroot/example-chroot");
    setup_keyfile_union_configured(expected);

    test_chroot_base<chroot_directory>::test_setup_keyfile(expected, chroot->get_name());
  }
#endif // SBUILD_FEATURE_UNION

void test_session_flags()
  {
    CPPUNIT_ASSERT(chroot->get_session_flags() ==
		   sbuild::chroot::SESSION_CREATE);
  }

#ifdef SBUILD_FEATURE_UNION
  void test_session_flags_fsunion()
  {
    std::tr1::shared_ptr<sbuild::chroot_directory> c = std::tr1::dynamic_pointer_cast<sbuild::chroot_directory>(chroot);
    c->set_union_type("aufs");
    c->set_union_overlay_directory("/overlay");
    c->set_union_underlay_directory("/underlay");

    CPPUNIT_ASSERT(chroot->get_session_flags() ==
		   (sbuild::chroot::SESSION_CREATE |
		    sbuild::chroot::SESSION_CLONE));
  }
#endif // SBUILD_FEATURE_UNION

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
