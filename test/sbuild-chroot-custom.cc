/* Copyright © 2006-2012  Roger Leigh <rleigh@codelibre.net>
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
#include <sbuild/sbuild-chroot-custom.h>
#include <sbuild/sbuild-chroot-facet-session-clonable.h>
#include <sbuild/sbuild-chroot-facet-source-clonable.h>
#include <sbuild/sbuild-chroot-facet-userdata.h>

#include "test-helpers.h"
#include "test-sbuild-chroot.h"

#include <algorithm>
#include <set>

#include <cppunit/extensions/HelperMacros.h>

using namespace CppUnit;

class chroot_custom : public sbuild::chroot_custom
{
public:
  chroot_custom():
    sbuild::chroot_custom()
  {}

  virtual ~chroot_custom()
  {}
};

class test_chroot_custom : public test_chroot_base<chroot_custom>
{
  CPPUNIT_TEST_SUITE(test_chroot_custom);
  CPPUNIT_TEST(test_directory);
  CPPUNIT_TEST(test_chroot_type);
  CPPUNIT_TEST(test_setup_env);
  CPPUNIT_TEST(test_setup_keyfile);
  CPPUNIT_TEST(test_session_flags1);
  CPPUNIT_TEST(test_session_flags2);
  CPPUNIT_TEST(test_session_flags3);
  CPPUNIT_TEST(test_session_flags4);
  CPPUNIT_TEST(test_print_details);
  CPPUNIT_TEST(test_print_config);
  CPPUNIT_TEST(test_run_setup_scripts);
  CPPUNIT_TEST_SUITE_END();

public:
  test_chroot_custom():
    test_chroot_base<chroot_custom>()
  {}

  void setUp()
  {
    test_chroot_base<chroot_custom>::setUp();
    CPPUNIT_ASSERT(chroot);
    CPPUNIT_ASSERT(session);
    CPPUNIT_ASSERT(!source);
    CPPUNIT_ASSERT(!session_source);
  }

  virtual void setup_chroot_props (sbuild::chroot::ptr& chroot)
  {
    test_chroot_base<chroot_custom>::setup_chroot_props(chroot);

    std::shared_ptr<sbuild::chroot_custom> c = std::dynamic_pointer_cast<sbuild::chroot_custom>(chroot);

    sbuild::chroot_facet_userdata::ptr userdata = c->get_facet<sbuild::chroot_facet_userdata>();
    CPPUNIT_ASSERT(userdata);
    userdata->set_data("custom.directory", "/srv/chroots/sid");
    userdata->set_data("custom.options", "foobar");
  }

  void
  test_directory()
  {
    std::shared_ptr<sbuild::chroot_custom> c = std::dynamic_pointer_cast<sbuild::chroot_custom>(chroot);
    CPPUNIT_ASSERT(c);
    CPPUNIT_ASSERT(chroot->get_path() == "/mnt/mount-location");
    CPPUNIT_ASSERT(chroot->get_mount_location() == "/mnt/mount-location");
  }

  void test_chroot_type()
  {
    CPPUNIT_ASSERT(chroot->get_chroot_type() == "custom");
  }

  void setup_env_gen(sbuild::environment& expected)
  {
    setup_env_chroot(expected);

    expected.add("CHROOT_TYPE",           "custom");
    expected.add("CHROOT_MOUNT_LOCATION", "/mnt/mount-location");
    expected.add("CHROOT_PATH",           "/mnt/mount-location");
    expected.add("CHROOT_SESSION_CLONE",  "false");
    expected.add("CHROOT_SESSION_CREATE", "true");
    expected.add("CHROOT_SESSION_PURGE",  "false");
    expected.add("CHROOT_SESSION_SOURCE", "false");
    expected.add("CUSTOM_DIRECTORY",      "/srv/chroots/sid");
    expected.add("CUSTOM_OPTIONS",        "foobar");
  }

  void test_setup_env()
  {
    sbuild::environment expected;
    setup_env_gen(expected);

    test_chroot_base<chroot_custom>::test_setup_env(chroot, expected);
  }

  void test_setup_keyfile()
  {
    sbuild::keyfile expected;
    std::string group = chroot->get_name();
    setup_keyfile_chroot(expected, group);
    expected.set_value(group, "type", "custom");
    expected.set_value(group, "custom-session-purgeable", "false");
    expected.set_value(group, "custom.directory", "/srv/chroots/sid");
    expected.set_value(group, "custom.options", "foobar");

    test_chroot_base<chroot_custom>::test_setup_keyfile
      (chroot, expected, group);
  }

  void test_session_flags1()
  {
    std::shared_ptr<sbuild::chroot_custom> c = std::dynamic_pointer_cast<sbuild::chroot_custom>(chroot);
    CPPUNIT_ASSERT(c);

    CPPUNIT_ASSERT(chroot->get_session_flags() ==
                   sbuild::chroot::SESSION_CREATE);

    CPPUNIT_ASSERT(c->get_facet<sbuild::chroot_facet_session_clonable>());
    CPPUNIT_ASSERT(!c->get_facet<sbuild::chroot_facet_source_clonable>());
  }

  void test_session_flags2()
  {
    std::shared_ptr<sbuild::chroot_custom> c = std::dynamic_pointer_cast<sbuild::chroot_custom>(chroot);
    CPPUNIT_ASSERT(c);
    c->set_session_cloneable(false);

    CPPUNIT_ASSERT(chroot->get_session_flags() ==
                   sbuild::chroot::SESSION_NOFLAGS);

    CPPUNIT_ASSERT(!c->get_facet<sbuild::chroot_facet_session_clonable>());
    CPPUNIT_ASSERT(!c->get_facet<sbuild::chroot_facet_source_clonable>());
  }

  void test_session_flags3()
  {
    std::shared_ptr<sbuild::chroot_custom> c = std::dynamic_pointer_cast<sbuild::chroot_custom>(chroot);
    CPPUNIT_ASSERT(c);
    c->set_source_cloneable(true);

    CPPUNIT_ASSERT(chroot->get_session_flags() ==
                   (sbuild::chroot::SESSION_CREATE|sbuild::chroot::SESSION_CLONE));

    CPPUNIT_ASSERT(c->get_facet<sbuild::chroot_facet_session_clonable>());
    CPPUNIT_ASSERT(c->get_facet<sbuild::chroot_facet_source_clonable>());
  }

  void test_session_flags4()
  {
    std::shared_ptr<sbuild::chroot_custom> c = std::dynamic_pointer_cast<sbuild::chroot_custom>(chroot);
    CPPUNIT_ASSERT(c);
    c->set_session_cloneable(false);
    c->set_source_cloneable(true);

    CPPUNIT_ASSERT(chroot->get_session_flags() ==
                   sbuild::chroot::SESSION_CLONE);

    CPPUNIT_ASSERT(!c->get_facet<sbuild::chroot_facet_session_clonable>());
    CPPUNIT_ASSERT(c->get_facet<sbuild::chroot_facet_source_clonable>());
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

  void test_run_setup_scripts()
  {
    CPPUNIT_ASSERT(chroot->get_run_setup_scripts());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(test_chroot_custom);
