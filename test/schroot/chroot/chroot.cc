/* Copyright © 2006-2013  Roger Leigh <rleigh@codelibre.net>
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

#include <gtest/gtest.h>

#include <config.h>

#include <schroot/chroot/chroot.h>
#include <schroot/chroot/facet/facet.h>
#include <schroot/chroot/facet/factory.h>
#include <schroot/chroot/facet/session-clonable.h>
#include <schroot/chroot/facet/storage.h>
#include <schroot/keyfile-writer.h>

#include <test/schroot/chroot/chroot.h>
#include <test/schroot/chroot/chroot.h>

#include <algorithm>
#include <set>

class test_chroot_facet : public schroot::chroot::facet::facet,
                          public schroot::chroot::facet::storage
{
public:
  /// A shared_ptr to a chroot facet object.
  typedef std::shared_ptr<test_chroot_facet> ptr;

  /// A shared_ptr to a const chroot facet object.
  typedef std::shared_ptr<const test_chroot_facet> const_ptr;

protected:
  /// The constructor.
  test_chroot_facet ():
    facet(),
    storage()
  {
  }

  /// The copy constructor.
  test_chroot_facet (const test_chroot_facet& rhs):
    facet(rhs),
    storage(rhs)
  {
  }

  void
  set_chroot (schroot::chroot::chroot& chroot,
              bool                    copy)
  {
    facet::set_chroot(chroot, copy);

    if (!copy)
      owner->add_facet(schroot::chroot::facet::session_clonable::create());
  }

  friend class chroot;

public:
  /// The destructor.
  virtual ~test_chroot_facet ()
  {
  }

  virtual std::string const&
  get_name () const
  {
    static const std::string name("test");

    return name;
  }


  static ptr
  create ()
  {
    return ptr(new test_chroot_facet());
  }

  virtual facet::ptr
  clone () const
  {
    return ptr(new test_chroot_facet(*this));
  }

  virtual std::string
  get_path () const
  {
    return owner->get_mount_location();
  }
};

namespace
{

  const schroot::chroot::facet::factory::facet_info test_info =
    {
      "test",
      "Support for ‘test’ chroots",
      false,
      []() -> schroot::chroot::facet::facet::ptr { return test_chroot_facet::create(); }
    };

  schroot::chroot::facet::factory test_register(test_info);

}

class Chroot : public ChrootBase
{
public:
  Chroot():
    ChrootBase("test")
  {
  }
};

TEST_F(Chroot, Name)
{
  chroot->set_name("test-name-example");
  ASSERT_EQ(chroot->get_name(), "test-name-example");
}

TEST_F(Chroot, Description)
{
  chroot->set_description("test-description-example");
  ASSERT_EQ(chroot->get_description(), "test-description-example");
}

TEST_F(Chroot, MountLocation)
{
  chroot->set_mount_location("/mnt/mount-location/example");
  ASSERT_EQ(chroot->get_mount_location(),
                 "/mnt/mount-location/example");
}

TEST_F(Chroot, Groups)
{
  schroot::string_list groups;
  groups.push_back("schroot");
  groups.push_back("schroot-users");
  groups.push_back("fred");
  groups.push_back("users");

  test_list(*chroot.get(),
            groups,
            &schroot::chroot::chroot::get_groups,
            &schroot::chroot::chroot::set_groups);
}

TEST_F(Chroot, RootGroups)
{
  schroot::string_list groups;
  groups.push_back("schroot");
  groups.push_back("trusted");
  groups.push_back("root");

  test_list(*chroot.get(),
            groups,
            &schroot::chroot::chroot::get_root_groups,
            &schroot::chroot::chroot::set_root_groups);
}

TEST_F(Chroot, Aliases)
{
  schroot::string_list aliases;
  aliases.push_back("alias1");
  aliases.push_back("alias2");

  test_list(*chroot.get(),
            aliases,
            &schroot::chroot::chroot::get_aliases,
            &schroot::chroot::chroot::set_aliases);
}

TEST_F(Chroot, EnvironmentFilter)
{
  schroot::regex r("foo|bar|baz");

  chroot->set_environment_filter(r);

  ASSERT_EQ(chroot->get_environment_filter().compare(r), 0);
}

TEST_F(Chroot, Active)
{
  ASSERT_EQ(chroot->get_facet<schroot::chroot::facet::session>(), nullptr);
}

TEST_F(Chroot, ScriptConfig)
{
  chroot->set_script_config("desktop/config");

  {
    schroot::keyfile expected;
    const std::string group(chroot->get_name());
    setup_keyfile_chroot(expected, group);
    expected.remove_key(group, "setup.config");
    expected.remove_key(group, "setup.copyfiles");
    expected.remove_key(group, "setup.fstab");
    expected.remove_key(group, "setup.nssdatabases");
    expected.set_value(group, "type", "test");
    expected.set_value(group, "profile", "");
    expected.set_value(group, "script-config", "desktop/config");

    ChrootBase::test_setup_keyfile
      (chroot, expected, group);
  }

  {
    schroot::environment expected;
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
    expected.add("CHROOT_SESSION_SOURCE", "false");
    expected.add("CHROOT_SCRIPT_CONFIG",  schroot::normalname(std::string(SCHROOT_SYSCONF_DIR) + "/desktop/config"));

    schroot::environment observed;
    chroot->setup_env(observed);

    ChrootBase::test_setup_env(observed, expected);
  }
}

TEST_F(Chroot, RunSetupScripts)
{
  ASSERT_TRUE(chroot->get_run_setup_scripts());
}

TEST_F(Chroot, Verbose)
{
  ASSERT_EQ(chroot->get_verbosity(), schroot::chroot::chroot::VERBOSITY_QUIET);
  chroot->set_verbosity(schroot::chroot::chroot::VERBOSITY_VERBOSE);
  ASSERT_EQ(chroot->get_verbosity(), schroot::chroot::chroot::VERBOSITY_VERBOSE);
  ASSERT_EQ(std::string(chroot->get_verbosity_string()), "verbose");
  chroot->set_verbosity("normal");
  ASSERT_EQ(chroot->get_verbosity(), schroot::chroot::chroot::VERBOSITY_NORMAL);
  ASSERT_EQ(std::string(chroot->get_verbosity_string()), "normal");
}

TEST_F(Chroot, PreserveEnvironment)
{
  ASSERT_FALSE(chroot->get_preserve_environment());
  chroot->set_preserve_environment(true);
  ASSERT_TRUE(chroot->get_preserve_environment());
  chroot->set_preserve_environment(false);
  ASSERT_FALSE(chroot->get_preserve_environment());
}

TEST_F(Chroot, VerboseError)
{
  EXPECT_THROW(chroot->set_verbosity("invalid"), schroot::chroot::chroot::error);
}

TEST_F(Chroot, Type)
{
  ASSERT_EQ(chroot->get_chroot_type(), "test");
}

TEST_F(Chroot, SetupEnvironment)
{
  schroot::environment expected;
  setup_env_chroot(expected);
  expected.add("CHROOT_TYPE",           "test");
  expected.add("CHROOT_MOUNT_LOCATION", "/mnt/mount-location");
  expected.add("CHROOT_PATH",           "/mnt/mount-location");
  expected.add("CHROOT_SESSION_CLONE",  "false");
  expected.add("CHROOT_SESSION_CREATE", "true");
  expected.add("CHROOT_SESSION_PURGE",  "false");
  expected.add("CHROOT_SESSION_SOURCE", "false");

  schroot::environment observed;
  chroot->setup_env(observed);

  ChrootBase::test_setup_env(observed, expected);
}

TEST_F(Chroot, SetupKeyfile)
{
  schroot::keyfile expected;
  std::string group = chroot->get_name();
  setup_keyfile_chroot(expected, group);
  expected.set_value(group, "type", "test");

  ChrootBase::test_setup_keyfile
    (chroot, expected, group);
}

TEST_F(Chroot, SessionFlags)
{
  ASSERT_EQ(chroot->get_session_flags(),
            schroot::chroot::facet::facet::SESSION_CREATE);
}

TEST_F(Chroot, PrintDetails)
{
  std::ostringstream os;
  os << chroot;
  // TODO: Compare output.
  ASSERT_FALSE(os.str().empty());
}

TEST_F(Chroot, PrintConfig)
{
  std::ostringstream os;
  schroot::keyfile config;
  config << chroot;
  os << schroot::keyfile_writer(config);
  // TODO: Compare output.
  ASSERT_FALSE(os.str().empty());
}
