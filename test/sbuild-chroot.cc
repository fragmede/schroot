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

#include <sbuild/sbuild-chroot.h>

#include "test-helpers.h"
#include "test-sbuild-chroot.h"

#include <algorithm>
#include <set>

#include <cppunit/extensions/HelperMacros.h>

using namespace CppUnit;

class basic_chroot : public sbuild::chroot
{
public:
  basic_chroot ():
    sbuild::chroot()
  {}

  virtual ~basic_chroot()
  {}

  virtual ptr
  clone () const
  { return ptr(new basic_chroot(*this)); }

  virtual std::string const&
  get_chroot_type () const
  { static const std::string type("test"); return type; }

  virtual void
  setup_env (sbuild::environment& env)
  { this->sbuild::chroot::setup_env(env); }

  virtual void
  setup_lock (setup_type type,
	      bool       lock,
	      int        status)
  {}

  virtual sbuild::chroot::session_flags
  get_session_flags () const
  { return sbuild::chroot::SESSION_CREATE; }

  virtual void
  print_details (std::ostream& stream) const
  { sbuild::chroot::print_details(stream); }

};

class test_chroot : public test_chroot_base<basic_chroot>
{
  CPPUNIT_TEST_SUITE(test_chroot);
  CPPUNIT_TEST(test_name);
  CPPUNIT_TEST(test_description);
  CPPUNIT_TEST(test_mount_location);
  CPPUNIT_TEST(test_priority);
  CPPUNIT_TEST(test_groups);
  CPPUNIT_TEST(test_root_groups);
  CPPUNIT_TEST(test_aliases);
  CPPUNIT_TEST(test_environment_filter);
  CPPUNIT_TEST(test_active);
  CPPUNIT_TEST(test_run_setup_scripts);
  CPPUNIT_TEST(test_chroot_type);
  CPPUNIT_TEST(test_setup_env);
  CPPUNIT_TEST(test_session_flags);
  CPPUNIT_TEST(test_print_details);
  CPPUNIT_TEST(test_print_config);
  CPPUNIT_TEST_SUITE_END();

public:
  test_chroot():
    test_chroot_base<basic_chroot>()
  {}

  void test_name()
  {
    chroot->set_name("test-name-example");
    CPPUNIT_ASSERT(chroot->get_name() == "test-name-example");
  }

  void test_description()
  {
    chroot->set_description("test-description-example");
    CPPUNIT_ASSERT(chroot->get_description() == "test-description-example");
  }

  void test_mount_location()
  {
    chroot->set_mount_location("/mnt/mount-location/example");
    CPPUNIT_ASSERT(chroot->get_mount_location() ==
		   "/mnt/mount-location/example");
  }

  void test_priority()
  {
    chroot->set_priority(6);
    CPPUNIT_ASSERT(chroot->get_priority() == 6);
  }

  void test_groups()
  {
    sbuild::string_list groups;
    groups.push_back("schroot");
    groups.push_back("sbuild-users");
    groups.push_back("fred");
    groups.push_back("users");

    test_list(*chroot.get(),
	      groups,
	      &sbuild::chroot::get_groups,
	      &sbuild::chroot::set_groups);
  }

  void test_root_groups()
  {
    sbuild::string_list groups;
    groups.push_back("schroot");
    groups.push_back("trusted");
    groups.push_back("root");

    test_list(*chroot.get(),
	      groups,
	      &sbuild::chroot::get_root_groups,
	      &sbuild::chroot::set_root_groups);
  }

  void test_aliases()
  {
    sbuild::string_list aliases;
    aliases.push_back("alias1");
    aliases.push_back("alias2");

    test_list(*chroot.get(),
	      aliases,
	      &sbuild::chroot::get_aliases,
	      &sbuild::chroot::set_aliases);
  }

  void test_environment_filter()
  {
    sbuild::regex r("foo|bar|baz");

    chroot->set_environment_filter(r);

    CPPUNIT_ASSERT(chroot->get_environment_filter().compare(r) == 0);
  }

  void test_active()
  {
    CPPUNIT_ASSERT(chroot->get_active() == false);
    chroot->set_active(true);
    CPPUNIT_ASSERT(chroot->get_active() == true);
    chroot->set_active(false);
    CPPUNIT_ASSERT(chroot->get_active() == false);
  }

  void test_run_setup_scripts()
  {
    CPPUNIT_ASSERT(chroot->get_run_setup_scripts() == true);
    chroot->set_run_setup_scripts(false);
    CPPUNIT_ASSERT(chroot->get_run_setup_scripts() == false);
    chroot->set_run_setup_scripts(true);
    CPPUNIT_ASSERT(chroot->get_run_setup_scripts() == true);
  }

  void test_chroot_type()
  {
    CPPUNIT_ASSERT(chroot->get_chroot_type() == "test");
  }

  void test_setup_env()
  {
    sbuild::environment expected;
    expected.add("CHROOT_TYPE",           "test");
    expected.add("CHROOT_NAME",           "test-name");
    expected.add("CHROOT_DESCRIPTION",    "test-description");
    expected.add("CHROOT_MOUNT_LOCATION", "/mnt/mount-location");
    expected.add("CHROOT_PATH",           "/mnt/mount-location");
    expected.add("CHROOT_SCRIPT_CONFIG",  sbuild::normalname(std::string(PACKAGE_SYSCONF_DIR) + "/script-defaults"));
    expected.add("CHROOT_SESSION_CLONE",  "false");
    expected.add("CHROOT_SESSION_CREATE", "true");
    expected.add("CHROOT_SESSION_PURGE",  "false");

    test_chroot_base<basic_chroot>::test_setup_env(expected);
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

CPPUNIT_TEST_SUITE_REGISTRATION(test_chroot);
