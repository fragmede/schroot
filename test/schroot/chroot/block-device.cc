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

#include <algorithm>
#include <set>

#include <schroot/chroot/facet/block-device.h>
#include <schroot/chroot/facet/mountable.h>
#include <schroot/i18n.h>
#include <schroot/keyfile-writer.h>

#include <test/schroot/chroot/chroot.h>

using schroot::_;

class ChrootBlockDevice : public ChrootBase
{
public:
  ChrootBlockDevice():
    ChrootBase("block-device")
  {}

  void SetUp()
  {
    ChrootBase::SetUp();
    ASSERT_NE(chroot, nullptr);
    ASSERT_NE(session, nullptr);
    ASSERT_EQ(source, nullptr);
    ASSERT_EQ(session_source, nullptr);
#ifdef SCHROOT_FEATURE_UNION
    ASSERT_NE(chroot_union, nullptr);
    ASSERT_NE(session_union, nullptr);
    ASSERT_NE(source_union, nullptr);
    ASSERT_NE(session_source_union, nullptr);
#endif // SCHROOT_FEATURE_UNION
  }

  virtual void setup_chroot_props (schroot::chroot::chroot::ptr& chroot)
  {
    ChrootBase::setup_chroot_props(chroot);

    chroot->get_facet_strict<schroot::chroot::facet::block_device>()->set_device("/dev/testdev");

    schroot::chroot::facet::mountable::ptr pmnt(chroot->get_facet<schroot::chroot::facet::mountable>());
    ASSERT_NE(pmnt, nullptr);

    pmnt->set_mount_options("-t jfs -o quota,rw");
    pmnt->set_location("/squeeze");
  }

  void setup_env_gen(schroot::environment& expected)
  {
    setup_env_chroot(expected);

    expected.add("CHROOT_TYPE",           "block-device");
    expected.add("CHROOT_LOCATION",       "/squeeze");
    expected.add("CHROOT_MOUNT_LOCATION", "/mnt/mount-location");
    expected.add("CHROOT_PATH",           "/mnt/mount-location/squeeze");
    expected.add("CHROOT_DEVICE",         "/dev/testdev");
    expected.add("CHROOT_MOUNT_OPTIONS",  "-t jfs -o quota,rw");
  }

  void setup_keyfile_block(schroot::keyfile &expected, std::string group)
  {
    expected.set_value(group, "type", "block-device");
    expected.set_value(group, "device", "/dev/testdev");
    expected.set_value(group, "location", "/squeeze");
    expected.set_value(group, "mount-options", "-t jfs -o quota,rw");
  }
};

TEST_F(ChrootBlockDevice, Device)
{
  chroot->get_facet_strict<schroot::chroot::facet::block_device>()->set_device("/dev/some/device");
  ASSERT_EQ(chroot->get_facet_strict<schroot::chroot::facet::block_device>()->get_device(),
            "/dev/some/device");
}

TEST_F(ChrootBlockDevice, MountOptions)
{
  schroot::chroot::facet::mountable::ptr pmnt(chroot->get_facet<schroot::chroot::facet::mountable>());
  ASSERT_NE(pmnt, nullptr);

  pmnt->set_mount_options("-o opt1,opt2");
  ASSERT_EQ(pmnt->get_mount_options(), "-o opt1,opt2");
  pmnt->set_location("/squeeze");
  ASSERT_EQ(pmnt->get_location(), "/squeeze");
}

TEST_F(ChrootBlockDevice, Type)
{
  ASSERT_EQ(chroot->get_chroot_type(), "block-device");
}

TEST_F(ChrootBlockDevice, SetupEnv)
{
  schroot::environment expected;
  setup_env_gen(expected);

  expected.add("CHROOT_SESSION_CLONE",  "false");
  expected.add("CHROOT_SESSION_CREATE", "true");
  expected.add("CHROOT_SESSION_PURGE",  "false");
  expected.add("CHROOT_SESSION_SOURCE", "false");
#ifdef SCHROOT_FEATURE_UNION
  expected.add("CHROOT_UNION_TYPE",     "none");
#endif // SCHROOT_FEATURE_UNION

  ChrootBase::test_setup_env(chroot, expected);
}

TEST_F(ChrootBlockDevice, SetupEnvSession)
{
  schroot::environment expected;
  setup_env_gen(expected);
  expected.add("SESSION_ID",            "test-session-name");
  expected.add("CHROOT_ALIAS",          "test-session-name");
  expected.add("CHROOT_DESCRIPTION",     chroot->get_description() + ' ' + _("(session chroot)"));
  expected.add("CHROOT_SESSION_CLONE",  "false");
  expected.add("CHROOT_SESSION_CREATE", "false");
  expected.add("CHROOT_SESSION_PURGE",  "false");
  expected.add("CHROOT_SESSION_SOURCE", "false");
  expected.add("CHROOT_MOUNT_DEVICE",   "/dev/testdev");
#ifdef SCHROOT_FEATURE_UNION
  expected.add("CHROOT_UNION_TYPE",     "none");
#endif // SCHROOT_FEATURE_UNION
  ChrootBase::test_setup_env(session, expected);
}

#ifdef SCHROOT_FEATURE_UNION
TEST_F(ChrootBlockDevice, SetupEnvUnion)
{
  schroot::environment expected;
  setup_env_gen(expected);
  expected.add("CHROOT_SESSION_CLONE",  "true");
  expected.add("CHROOT_SESSION_CREATE", "true");
  expected.add("CHROOT_SESSION_PURGE",  "false");
  expected.add("CHROOT_SESSION_SOURCE", "false");
  expected.add("CHROOT_UNION_TYPE",     "aufs");
  expected.add("CHROOT_UNION_MOUNT_OPTIONS",      "union-mount-options");
  expected.add("CHROOT_UNION_OVERLAY_DIRECTORY",  "/overlay");
  expected.add("CHROOT_UNION_UNDERLAY_DIRECTORY", "/underlay");

  ChrootBase::test_setup_env(chroot_union, expected);
}

TEST_F(ChrootBlockDevice, SetupEnvSessionUnion)
{
  schroot::environment expected;
  setup_env_gen(expected);
  expected.add("SESSION_ID",            "test-union-session-name");
  expected.add("CHROOT_ALIAS",          "test-union-session-name");
  expected.add("CHROOT_DESCRIPTION",     chroot->get_description() + ' ' + _("(session chroot)"));
  expected.add("CHROOT_SESSION_CLONE",  "false");
  expected.add("CHROOT_SESSION_CREATE", "false");
  expected.add("CHROOT_SESSION_PURGE",  "true");
  expected.add("CHROOT_SESSION_SOURCE", "false");
  expected.add("CHROOT_MOUNT_DEVICE",   "/dev/testdev");
  expected.add("CHROOT_UNION_TYPE",     "aufs");
  expected.add("CHROOT_UNION_MOUNT_OPTIONS",      "union-mount-options");
  expected.add("CHROOT_UNION_OVERLAY_DIRECTORY",  "/overlay/test-union-session-name");
  expected.add("CHROOT_UNION_UNDERLAY_DIRECTORY", "/underlay/test-union-session-name");
  ChrootBase::test_setup_env(session_union, expected);
}

TEST_F(ChrootBlockDevice, SetupEnvSourceUnion)
{
  schroot::environment expected;
  setup_env_gen(expected);

  expected.add("CHROOT_NAME",           "test-name");
  expected.add("CHROOT_DESCRIPTION",     chroot->get_description() + ' ' + _("(source chroot)"));
  expected.add("CHROOT_SESSION_CLONE",  "false");
  expected.add("CHROOT_SESSION_CREATE", "true");
  expected.add("CHROOT_SESSION_PURGE",  "false");
  expected.add("CHROOT_SESSION_SOURCE", "false");

  ChrootBase::test_setup_env(source_union, expected);
}

TEST_F(ChrootBlockDevice, SetupEnvSessionSourceUnion)
{
  schroot::environment expected;
  setup_env_gen(expected);

  expected.add("SESSION_ID",            "test-session-name");
  expected.add("CHROOT_ALIAS",          "test-session-name");
  expected.add("CHROOT_DESCRIPTION",     chroot->get_description() + ' ' + _("(source chroot) (session chroot)"));
  expected.add("CHROOT_SESSION_CLONE",  "false");
  expected.add("CHROOT_SESSION_CREATE", "false");
  expected.add("CHROOT_SESSION_PURGE",  "false");
  expected.add("CHROOT_SESSION_SOURCE", "true");
  expected.add("CHROOT_MOUNT_DEVICE",   "/dev/testdev");

  ChrootBase::test_setup_env(session_source_union, expected);
}
#endif // SCHROOT_FEATURE_UNION

TEST_F(ChrootBlockDevice, SetupKeyfile)
{
  schroot::keyfile expected;
  std::string group = chroot->get_name();
  setup_keyfile_chroot(expected, group);
  setup_keyfile_block(expected, group);
#ifdef SCHROOT_FEATURE_UNION
  setup_keyfile_union_unconfigured(expected, group);
#endif
  ChrootBase::test_setup_keyfile
    (chroot, expected, group);
}

TEST_F(ChrootBlockDevice, SetupKeyfileSession)
{
  schroot::keyfile expected;
  const std::string group(session->get_name());
  setup_keyfile_session(expected, group);
  setup_keyfile_block(expected, group);
  expected.set_value(group, "name", "test-session-name");
  expected.set_value(group, "selected-name", "test-session-name");
  expected.set_value(group, "mount-device", "/dev/testdev");
  expected.set_value(group, "mount-location", "/mnt/mount-location");
  setup_keyfile_session_clone(expected, group);
#ifdef SCHROOT_FEATURE_UNION
  setup_keyfile_union_unconfigured(expected, group);
#endif

  ChrootBase::test_setup_keyfile
    (session, expected, group);
}

#ifdef SCHROOT_FEATURE_UNION
TEST_F(ChrootBlockDevice, SetupKeyfileUnion)
{
  schroot::keyfile expected;
  const std::string group(chroot_union->get_name());
  setup_keyfile_union_configured(expected, group);
  setup_keyfile_chroot(expected, group);
  setup_keyfile_source(expected, group);
  setup_keyfile_block(expected, group);

  ChrootBase::test_setup_keyfile
    (chroot_union, expected, group);
}

TEST_F(ChrootBlockDevice, SetupKeyfileSessionUnion)
{
  schroot::keyfile expected;
  const std::string group(session_union->get_name());
  setup_keyfile_session(expected, group);
  setup_keyfile_block(expected, group);
  expected.set_value(group, "name", "test-union-session-name");
  expected.set_value(group, "selected-name", "test-union-session-name");
  expected.set_value(group, "mount-device", "/dev/testdev");
  expected.set_value(group, "mount-location", "/mnt/mount-location");
  setup_keyfile_session_clone(expected, group);
  setup_keyfile_union_session(expected, group);

  ChrootBase::test_setup_keyfile
    (session_union, expected, group);
}

TEST_F(ChrootBlockDevice, SetupKeyfileSourceUnion)
{
  schroot::keyfile expected;
  const std::string group(source_union->get_name());
  setup_keyfile_chroot(expected, group);
  setup_keyfile_source_clone(expected, group);
  setup_keyfile_block(expected, group);
  expected.set_value(group, "description", chroot->get_description() + ' ' + _("(source chroot)"));

  ChrootBase::test_setup_keyfile
    (source_union, expected, group);
}
#endif // SCHROOT_FEATURE_UNION

TEST_F(ChrootBlockDevice, SessionFlags)
{
  ASSERT_EQ(chroot->get_session_flags(),
            schroot::chroot::facet::facet::SESSION_CREATE);

  ASSERT_EQ(session->get_session_flags(),
            schroot::chroot::facet::facet::SESSION_NOFLAGS);

#ifdef SCHROOT_FEATURE_UNION
  ASSERT_EQ(chroot_union->get_session_flags(),
            (schroot::chroot::facet::facet::SESSION_CREATE |
             schroot::chroot::facet::facet::SESSION_CLONE));

  ASSERT_EQ(session_union->get_session_flags(),
            schroot::chroot::facet::facet::SESSION_PURGE);

  ASSERT_EQ(source_union->get_session_flags(),
            schroot::chroot::facet::facet::SESSION_CREATE);
#endif // SCHROOT_FEATURE_UNION
}

TEST_F(ChrootBlockDevice, PrintDetails)
{
  std::ostringstream os;
  os << chroot;
  // TODO: Compare output.
  ASSERT_FALSE(os.str().empty());
}

TEST_F(ChrootBlockDevice, PrintConfig)
{
  std::ostringstream os;
  schroot::keyfile config;
  config << chroot;
  os << schroot::keyfile_writer(config);
  // TODO: Compare output.
  ASSERT_FALSE(os.str().empty());
}

TEST_F(ChrootBlockDevice, RunSetupScripts)
{
  ASSERT_TRUE(chroot->get_run_setup_scripts());
}
