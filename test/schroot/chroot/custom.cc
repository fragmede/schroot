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

#include <schroot/config.h>
#include <schroot/chroot/facet/custom.h>
#include <schroot/chroot/facet/session-clonable.h>
#include <schroot/chroot/facet/source-clonable.h>
#include <schroot/chroot/facet/userdata.h>
#include <schroot/keyfile-writer.h>

#include <test/schroot/chroot/chroot.h>

#include <algorithm>
#include <set>

class ChrootCustom : public ChrootBase
{
public:
  ChrootCustom():
    ChrootBase("custom")
  {}

  void SetUp()
  {
    ChrootBase::SetUp();
    ASSERT_NE(chroot, nullptr);
    ASSERT_NE(session, nullptr);
    ASSERT_EQ(source, nullptr);
    ASSERT_EQ(session_source, nullptr);
  }

  virtual void setup_chroot_props (schroot::chroot::chroot::ptr& chroot)
  {
    ChrootBase::setup_chroot_props(chroot);

    schroot::chroot::facet::userdata::ptr userdata = chroot->get_facet<schroot::chroot::facet::userdata>();
    ASSERT_NE(userdata, nullptr);
    userdata->set_data("custom.directory", "/srv/chroots/sid");
    userdata->set_data("custom.options", "foobar");
  }

  void setup_env_gen(schroot::environment& expected)
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
};

TEST_F(ChrootCustom, Directory)
  {
    ASSERT_EQ(chroot->get_path(), "/mnt/mount-location");
    ASSERT_EQ(chroot->get_mount_location(), "/mnt/mount-location");
  }

TEST_F(ChrootCustom, Type)
{
  ASSERT_EQ(chroot->get_chroot_type(), "custom");
}


TEST_F(ChrootCustom, SetupEnv)
{
  schroot::environment expected;
  setup_env_gen(expected);

  ChrootBase::test_setup_env(chroot, expected);
}

TEST_F(ChrootCustom, SetupKeyfile)
{
  schroot::keyfile expected;
  std::string group = chroot->get_name();
  setup_keyfile_chroot(expected, group);
  expected.set_value(group, "type", "custom");
  expected.set_value(group, "custom-session-purgeable", "false");
  expected.set_value(group, "custom.directory", "/srv/chroots/sid");
  expected.set_value(group, "custom.options", "foobar");

  ChrootBase::test_setup_keyfile
    (chroot, expected, group);
}

TEST_F(ChrootCustom, SessionFlags1)
{
  ASSERT_EQ(chroot->get_session_flags(),
            schroot::chroot::facet::facet::SESSION_CREATE);

  ASSERT_NE(chroot->get_facet<schroot::chroot::facet::session_clonable>(), nullptr);
  ASSERT_EQ(chroot->get_facet<schroot::chroot::facet::source_clonable>(), nullptr);
}

TEST_F(ChrootCustom, SessionFlags2)
{
  schroot::chroot::facet::custom::ptr custp = chroot->get_facet_strict<schroot::chroot::facet::custom>();
  custp->set_session_cloneable(false);

  ASSERT_EQ(chroot->get_session_flags(),
            schroot::chroot::facet::facet::SESSION_NOFLAGS);

  ASSERT_EQ(chroot->get_facet<schroot::chroot::facet::session_clonable>(), nullptr);
  ASSERT_EQ(chroot->get_facet<schroot::chroot::facet::source_clonable>(), nullptr);
}

TEST_F(ChrootCustom, SessionFlags3)
{
  schroot::chroot::facet::custom::ptr custp = chroot->get_facet_strict<schroot::chroot::facet::custom>();
  custp->set_source_cloneable(true);

  ASSERT_EQ(chroot->get_session_flags(),
            (schroot::chroot::facet::facet::SESSION_CREATE|schroot::chroot::facet::facet::SESSION_CLONE));

  ASSERT_NE(chroot->get_facet<schroot::chroot::facet::session_clonable>(), nullptr);
  ASSERT_NE(chroot->get_facet<schroot::chroot::facet::source_clonable>(), nullptr);
}

TEST_F(ChrootCustom, SessionFlags4)
{
  schroot::chroot::facet::custom::ptr custp = chroot->get_facet_strict<schroot::chroot::facet::custom>();
  custp->set_session_cloneable(false);
  custp->set_source_cloneable(true);

  ASSERT_EQ(chroot->get_session_flags(),
            schroot::chroot::facet::facet::SESSION_CLONE);

  ASSERT_EQ(chroot->get_facet<schroot::chroot::facet::session_clonable>(), nullptr);
  ASSERT_NE(chroot->get_facet<schroot::chroot::facet::source_clonable>(), nullptr);
}

TEST_F(ChrootCustom, PrintDetails)
{
  std::ostringstream os;
  os << chroot;
  // TODO: Compare output.
  ASSERT_FALSE(os.str().empty());
}

TEST_F(ChrootCustom, PrintConfig)
{
  std::ostringstream os;
  schroot::keyfile config;
  config << chroot;
  os << schroot::keyfile_writer(config);
  // TODO: Compare output.
  ASSERT_FALSE(os.str().empty());
}

TEST_F(ChrootCustom, RunSetupScripts)
{
  ASSERT_TRUE(chroot->get_run_setup_scripts());
}
