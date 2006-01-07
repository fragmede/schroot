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

#include <schroot/sbuild-chroot.h>

using namespace CppUnit;

class basic_chroot : public sbuild::Chroot
{
public:
  basic_chroot ():
    sbuild::Chroot()
  {}

  virtual ~basic_chroot()
  {}

  virtual chroot_ptr
  clone () const
  { return chroot_ptr(new basic_chroot(*this)); }

  virtual std::string const&
  get_chroot_type () const
  { static const std::string type("test"); return type; }

  virtual void
  setup_env (sbuild::environment& env)
  { this->sbuild::Chroot::setup_env(env); }

  virtual void
  setup_lock (SetupType type,
	      bool      lock)
  {}

  virtual sbuild::Chroot::SessionFlags
  get_session_flags () const
  { return sbuild::Chroot::SESSION_CREATE; }

  virtual void
  print_details (std::ostream& stream) const
  { sbuild::Chroot::print_details(stream); }

  virtual void
  print_config (std::ostream& stream) const
  { sbuild::Chroot::print_config(stream); }
};

class test_chroot : public TestFixture
{
  CPPUNIT_TEST_SUITE(test_chroot);
  CPPUNIT_TEST(test_name);
  CPPUNIT_TEST(test_description);
  CPPUNIT_TEST(test_mount_device);
  CPPUNIT_TEST(test_mount_location);
  CPPUNIT_TEST(test_priority);
  CPPUNIT_TEST(test_groups);
  CPPUNIT_TEST(test_root_groups);
  CPPUNIT_TEST(test_aliases);
  CPPUNIT_TEST(test_active);
  CPPUNIT_TEST(test_run_setup_scripts);
  CPPUNIT_TEST(test_run_session_scripts);
  CPPUNIT_TEST(test_chroot_type);
  CPPUNIT_TEST(test_setup_env);
  CPPUNIT_TEST(test_session_flags);
  CPPUNIT_TEST(test_print_details);
  CPPUNIT_TEST(test_print_config);
  CPPUNIT_TEST_SUITE_END();

  sbuild::Chroot::chroot_ptr chroot;

public:
  test_chroot():
    chroot()
  {}

  void setUp()
  {
    this->chroot = sbuild::Chroot::chroot_ptr(new basic_chroot);
  }

  void tearDown()
  {
    this->chroot = sbuild::Chroot::chroot_ptr();
  }

  void test_name()
  {
    chroot->set_name("test-name");
    CPPUNIT_ASSERT(chroot->get_name() == "test-name");
  }

  void test_description()
  {
    chroot->set_description("test-description");
    CPPUNIT_ASSERT(chroot->get_description() == "test-description");
  }

  void test_mount_location()
  {
    chroot->set_mount_location("/mnt/mount-location");
    CPPUNIT_ASSERT(chroot->get_mount_location() == "/mnt/mount-location");
  }

  void test_mount_device()
  {
    chroot->set_mount_device("/dev/device-to-mount");
    CPPUNIT_ASSERT(chroot->get_mount_device() == "/dev/device-to-mount");
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

    chroot->set_groups(groups);

    // Check set groups exist, but make no assumptions about ordering.
    sbuild::string_list const& set_groups = chroot->get_groups();
    CPPUNIT_ASSERT(set_groups.size() == 4);
    CPPUNIT_ASSERT(std::find(set_groups.begin(), set_groups.end(),
			     std::string("schroot")) != set_groups.end());
    CPPUNIT_ASSERT(std::find(set_groups.begin(), set_groups.end(),
			     std::string("sbuild-users")) != set_groups.end());
    CPPUNIT_ASSERT(std::find(set_groups.begin(), set_groups.end(),
			     std::string("fred")) != set_groups.end());
    CPPUNIT_ASSERT(std::find(set_groups.begin(), set_groups.end(),
			     std::string("users")) != set_groups.end());
    // Ensure the test is working.
    CPPUNIT_ASSERT(std::find(set_groups.begin(), set_groups.end(),
			     std::string("invalid")) == set_groups.end());
  }

  void test_root_groups()
  {
    sbuild::string_list groups;
    groups.push_back("schroot");
    groups.push_back("trusted");
    groups.push_back("root");

    chroot->set_root_groups(groups);

    // Check set groups exist, but make no assumptions about ordering.
    sbuild::string_list const& set_groups = chroot->get_root_groups();
    CPPUNIT_ASSERT(set_groups.size() == 3);
    CPPUNIT_ASSERT(std::find(set_groups.begin(), set_groups.end(),
			     std::string("schroot")) != set_groups.end());
    CPPUNIT_ASSERT(std::find(set_groups.begin(), set_groups.end(),
			     std::string("trusted")) != set_groups.end());
    CPPUNIT_ASSERT(std::find(set_groups.begin(), set_groups.end(),
			     std::string("root")) != set_groups.end());
    // Ensure the test is working.
    CPPUNIT_ASSERT(std::find(set_groups.begin(), set_groups.end(),
			     std::string("invalid")) == set_groups.end());
  }

  void test_aliases()
  {
    sbuild::string_list aliases;
    aliases.push_back("alias1");
    aliases.push_back("alias2");

    chroot->set_aliases(aliases);

    // Check aliases exist, but make no assumptions about ordering.
    sbuild::string_list const& set_aliases = chroot->get_aliases();
    CPPUNIT_ASSERT(set_aliases.size() == 2);
    CPPUNIT_ASSERT(std::find(set_aliases.begin(), set_aliases.end(),
			     std::string("alias1")) != set_aliases.end());
    CPPUNIT_ASSERT(std::find(set_aliases.begin(), set_aliases.end(),
			     std::string("alias2")) != set_aliases.end());
    // Ensure the test is working.
    CPPUNIT_ASSERT(std::find(set_aliases.begin(), set_aliases.end(),
			     std::string("invalid")) == set_aliases.end());
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
    CPPUNIT_ASSERT(chroot->get_run_setup_scripts() == false);
    chroot->set_run_setup_scripts(true);
    CPPUNIT_ASSERT(chroot->get_run_setup_scripts() == true);
    chroot->set_run_setup_scripts(false);
    CPPUNIT_ASSERT(chroot->get_run_setup_scripts() == false);
  }

  void test_run_session_scripts()
  {
    CPPUNIT_ASSERT(chroot->get_run_session_scripts() == false);
    chroot->set_run_session_scripts(true);
    CPPUNIT_ASSERT(chroot->get_run_session_scripts() == true);
    chroot->set_run_session_scripts(false);
    CPPUNIT_ASSERT(chroot->get_run_session_scripts() == false);
  }

  void test_chroot_type()
  {
    CPPUNIT_ASSERT(chroot->get_chroot_type() == "test");
  }

  void test_setup_env()
  {
    chroot->set_name("test-name");
    chroot->set_description("test-description");
    chroot->set_mount_location("/mnt/mount-location");
    chroot->set_mount_device("/dev/device-to-mount");
    sbuild::environment env;
    chroot->setup_env(env);

    CPPUNIT_ASSERT(env.size() != 0);

    std::set<std::string> expected;
    expected.insert("CHROOT_TYPE");
    expected.insert("CHROOT_NAME");
    expected.insert("CHROOT_DESCRIPTION");
    expected.insert("CHROOT_MOUNT_LOCATION");
    expected.insert("CHROOT_MOUNT_DEVICE");

    std::set<std::string> found;
    for (sbuild::environment::const_iterator pos = env.begin();
	 pos != env.end();
	 ++pos)
      found.insert(pos->first);

    sbuild::string_list missing;
    set_difference(expected.begin(), expected.end(),
		   found.begin(), found.end(),
		   std::back_inserter(missing));
    if (!missing.empty())
      {
	for (sbuild::string_list::const_iterator pos = missing.begin();
	     pos != missing.end();
	     ++pos)
	  std::cout << "Missing environment: " << *pos << std::endl;
      }
    CPPUNIT_ASSERT(missing.empty());

    sbuild::string_list extra;
    set_difference(found.begin(), found.end(),
		   expected.begin(), expected.end(),
		   std::back_inserter(extra));
    if (!extra.empty())
      {
	for (sbuild::string_list::const_iterator pos = extra.begin();
	     pos != extra.end();
	     ++pos)
	  std::cout << "Additional environment: " << *pos << std::endl;
      }
    CPPUNIT_ASSERT(extra.empty());

    std::string checkval;
    CPPUNIT_ASSERT(env.get("CHROOT_TYPE", checkval) == true &&
		   checkval == "test");
    CPPUNIT_ASSERT(env.get("CHROOT_NAME", checkval) == true &&
		   checkval == "test-name");
    CPPUNIT_ASSERT(env.get("CHROOT_DESCRIPTION", checkval) == true &&
		   checkval == "test-description");
    CPPUNIT_ASSERT(env.get("CHROOT_MOUNT_LOCATION", checkval) == true &&
		   checkval == "/mnt/mount-location");
    CPPUNIT_ASSERT(env.get("CHROOT_MOUNT_DEVICE", checkval) == true &&
		   checkval == "/dev/device-to-mount");
  }

  void test_session_flags()
  {
    CPPUNIT_ASSERT(chroot->get_session_flags() ==
		   sbuild::Chroot::SESSION_CREATE);
  }

  void test_print_details()
  {
    std::ostringstream os;
    chroot->print_details(os);
    // TODO: Compare output.
    CPPUNIT_ASSERT(!os.str().empty());
  }

  void test_print_config()
  {
    std::ostringstream os;
    chroot->print_config(os);
    // TODO: Compare output.
    CPPUNIT_ASSERT(!os.str().empty());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(test_chroot);
