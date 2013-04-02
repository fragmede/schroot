/* Copyright © 2006-2013  Roger Leigh <rleigh@debian.org>
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

#include <sbuild/chroot.h>
#include <sbuild/keyfile-writer.h>

#include <test/helpers.h>
#include <test/sbuild/chroot.h>

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

  virtual ptr
  clone_session (std::string const& session_id,
                 std::string const& alias,
                 std::string const& user,
                 bool               root) const
  { return ptr(); }


  chroot::ptr
  clone_source () const
  { return ptr(); }

  virtual std::string const&
  get_chroot_type () const
  { static const std::string type("test"); return type; }

  void
  set_run_setup_scripts (bool run_setup_scripts)
  {
    sbuild::chroot::set_run_setup_scripts(run_setup_scripts);
  }

  virtual std::string
  get_path () const
  { return get_mount_location(); }

  virtual void
  setup_env (sbuild::chroot const& chroot,
             sbuild::environment&  env) const
  { sbuild::chroot::setup_env(chroot, env); }

  virtual void
  get_details (sbuild::chroot const&  chroot,
               sbuild::format_detail& detail) const
  { sbuild::chroot::get_details(chroot, detail); }

  virtual void
  setup_lock (setup_type type,
              bool       lock,
              int        status)
  {}

  virtual sbuild::chroot::session_flags
  get_session_flags (sbuild::chroot const& chroot) const
  { return sbuild::chroot::SESSION_CREATE; }

  virtual void
  get_used_keys (sbuild::string_list& used_keys) const
  { sbuild::chroot::get_used_keys(used_keys); }

  virtual void
  get_keyfile (sbuild::chroot const& chroot,
               sbuild::keyfile&      keyfile) const
  { sbuild::chroot::get_keyfile(chroot, keyfile); }

  virtual void
  set_keyfile (sbuild::chroot&        chroot,
               sbuild::keyfile const& keyfile)
  { sbuild::chroot::set_keyfile(chroot, keyfile); }
};

class test_chroot : public test_chroot_base<basic_chroot>
{
  CPPUNIT_TEST_SUITE(test_chroot);
  CPPUNIT_TEST(test_name);
  CPPUNIT_TEST(test_description);
  CPPUNIT_TEST(test_mount_location);
  CPPUNIT_TEST(test_groups);
  CPPUNIT_TEST(test_root_groups);
  CPPUNIT_TEST(test_aliases);
  CPPUNIT_TEST(test_environment_filter);
  CPPUNIT_TEST(test_active);
  CPPUNIT_TEST(test_script_config);
  CPPUNIT_TEST(test_run_setup_scripts);
  CPPUNIT_TEST(test_verbose);
  CPPUNIT_TEST(test_preserve_environment);
  CPPUNIT_TEST_EXCEPTION(test_verbose_error, sbuild::chroot::error);
  CPPUNIT_TEST(test_chroot_type);
  CPPUNIT_TEST(test_setup_env);
  CPPUNIT_TEST(test_setup_keyfile);
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
    CPPUNIT_ASSERT(!chroot->get_facet<sbuild::chroot_facet_session>());
  }

  void test_script_config()
  {
    chroot->set_script_config("desktop/config");

    {
      sbuild::keyfile expected;
      const std::string group(chroot->get_name());
      setup_keyfile_chroot(expected, group);
      expected.remove_key(group, "setup.config");
      expected.remove_key(group, "setup.copyfiles");
      expected.remove_key(group, "setup.fstab");
      expected.remove_key(group, "setup.nssdatabases");
      expected.set_value(group, "type", "test");
      expected.set_value(group, "profile", "");
      expected.set_value(group, "script-config", "desktop/config");

      test_chroot_base<basic_chroot>::test_setup_keyfile
        (chroot, expected, group);
    }

    {
      sbuild::environment expected;
      setup_env_chroot(expected);
      expected.remove("CHROOT_PROFILE");
      expected.remove("CHROOT_PROFILE_DIR");
      expected.remove("SETUP_CONFIG");
      expected.remove("SETUP_COPYFILES");
      expected.remove("SETUP_FSTAB");
      expected.remove("SETUP_NSSDATABASES");
      expected.add("CHROOT_TYPE",           "test");
      expected.add("CHROOT_MOUNT_LOCATION", "/mnt/mount-location");
      expected.add("CHROOT_PATH",           "/mnt/mount-location");
      expected.add("CHROOT_SESSION_CLONE",  "false");
      expected.add("CHROOT_SESSION_CREATE", "true");
      expected.add("CHROOT_SESSION_PURGE",  "false");
      expected.add("CHROOT_SCRIPT_CONFIG",  sbuild::normalname(std::string(SCHROOT_SYSCONF_DIR) + "/desktop/config"));

      sbuild::environment observed;
      chroot->setup_env(observed);

      test_chroot_base<basic_chroot>::test_setup_env(observed, expected);
    }
  }

  void test_run_setup_scripts()
  {
    std::shared_ptr<basic_chroot> c = std::dynamic_pointer_cast<basic_chroot>(chroot);

    CPPUNIT_ASSERT(chroot->get_run_setup_scripts() == true);
    c->set_run_setup_scripts(false);
    CPPUNIT_ASSERT(chroot->get_run_setup_scripts() == false);
    c->set_run_setup_scripts(true);
    CPPUNIT_ASSERT(chroot->get_run_setup_scripts() == true);
  }

  void test_verbose()
  {
    std::shared_ptr<basic_chroot> c = std::dynamic_pointer_cast<basic_chroot>(chroot);

    CPPUNIT_ASSERT(chroot->get_verbosity() == sbuild::chroot::VERBOSITY_QUIET);
    c->set_verbosity(sbuild::chroot::VERBOSITY_VERBOSE);
    CPPUNIT_ASSERT(chroot->get_verbosity() == sbuild::chroot::VERBOSITY_VERBOSE);
    CPPUNIT_ASSERT(std::string(chroot->get_verbosity_string()) == "verbose");
    c->set_verbosity("normal");
    CPPUNIT_ASSERT(chroot->get_verbosity() == sbuild::chroot::VERBOSITY_NORMAL);
    CPPUNIT_ASSERT(std::string(chroot->get_verbosity_string()) == "normal");
  }

  void test_preserve_environment()
  {
    std::shared_ptr<basic_chroot> c = std::dynamic_pointer_cast<basic_chroot>(chroot);

    CPPUNIT_ASSERT(chroot->get_preserve_environment() == false);
    c->set_preserve_environment(true);
    CPPUNIT_ASSERT(chroot->get_preserve_environment() == true);
    c->set_preserve_environment(false);
    CPPUNIT_ASSERT(chroot->get_preserve_environment() == false);
  }

  void test_verbose_error()
  {
    std::shared_ptr<basic_chroot> c = std::dynamic_pointer_cast<basic_chroot>(chroot);
    c->set_verbosity("invalid");
  }

  void test_chroot_type()
  {
    CPPUNIT_ASSERT(chroot->get_chroot_type() == "test");
  }

  void test_setup_env()
  {
    sbuild::environment expected;
    setup_env_chroot(expected);
    expected.add("CHROOT_TYPE",           "test");
    expected.add("CHROOT_MOUNT_LOCATION", "/mnt/mount-location");
    expected.add("CHROOT_PATH",           "/mnt/mount-location");
    expected.add("CHROOT_SESSION_CLONE",  "false");
    expected.add("CHROOT_SESSION_CREATE", "true");
    expected.add("CHROOT_SESSION_PURGE",  "false");

    sbuild::environment observed;
    chroot->setup_env(observed);

    test_chroot_base<basic_chroot>::test_setup_env(observed, expected);
  }

  void test_setup_keyfile()
  {
    sbuild::keyfile expected;
    std::string group = chroot->get_name();
    setup_keyfile_chroot(expected, group);
    expected.set_value(group, "type", "test");

    test_chroot_base<basic_chroot>::test_setup_keyfile
      (chroot, expected, group);
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
    os << sbuild::keyfile_writer(config);
    // TODO: Compare output.
    CPPUNIT_ASSERT(!os.str().empty());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(test_chroot);
