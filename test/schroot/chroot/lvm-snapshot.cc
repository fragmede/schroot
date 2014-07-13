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

#include <schroot/chroot/facet/lvm-snapshot.h>
#include <schroot/chroot/facet/mountable.h>
#include <schroot/i18n.h>
#include <schroot/keyfile-writer.h>
#include <schroot/util.h>

#include <test/schroot/chroot/chroot.h>

#include <algorithm>
#include <set>

using schroot::_;

class ChrootLVMSnapshot : public ChrootBase
{
public:
  ChrootLVMSnapshot():
    ChrootBase("lvm-snapshot")
  {}

  void SetUp()
  {
    ChrootBase::SetUp();
    ASSERT_NE(chroot, nullptr);
    ASSERT_NE(session, nullptr);
    ASSERT_NE(source, nullptr);
    ASSERT_NE(session_source, nullptr);
  }

  virtual void setup_chroot_props (schroot::chroot::chroot::ptr& chroot)
  {
    ChrootBase::setup_chroot_props(chroot);

    schroot::chroot::facet::lvm_snapshot::ptr psnap(chroot->get_facet_strict<schroot::chroot::facet::lvm_snapshot>());

    psnap->set_device("/dev/volgroup/testdev");
    psnap->set_snapshot_options("--size 1G");

    schroot::chroot::facet::mountable::ptr pmnt(chroot->get_facet<schroot::chroot::facet::mountable>());
    ASSERT_NE(pmnt, nullptr);

    pmnt->set_mount_options("-t jfs -o quota,rw");
    pmnt->set_location("/squeeze");
    //c->set_snapshot_device("/dev/volgroup/snaptestdev");
  }

  void setup_env_gen(schroot::environment &expected)
  {
    setup_env_chroot(expected);
    expected.add("CHROOT_LOCATION",       "/squeeze");
    expected.add("CHROOT_MOUNT_LOCATION", "/mnt/mount-location");
    expected.add("CHROOT_PATH",           "/mnt/mount-location/squeeze");
    expected.add("CHROOT_DEVICE",         "/dev/volgroup/testdev");
    expected.add("CHROOT_MOUNT_OPTIONS",  "-t jfs -o quota,rw");
  }

  void setup_keyfile_lvm(schroot::keyfile &expected, std::string group)
  {
    expected.set_value(group, "device", "/dev/volgroup/testdev");
    expected.set_value(group, "location", "/squeeze");
    expected.set_value(group, "mount-options", "-t jfs -o quota,rw");
  }
};

TEST_F(ChrootLVMSnapshot, SnapshotDevice)
{
  schroot::chroot::facet::lvm_snapshot::ptr psnap(chroot->get_facet_strict<schroot::chroot::facet::lvm_snapshot>());
  psnap->set_snapshot_device("/dev/volgroup/some/snapshot/device");
  ASSERT_EQ(psnap->get_snapshot_device(), "/dev/volgroup/some/snapshot/device");
}

TEST_F(ChrootLVMSnapshot, SnapshotOptions)
{
  schroot::chroot::facet::lvm_snapshot::ptr psnap(chroot->get_facet_strict<schroot::chroot::facet::lvm_snapshot>());
  psnap->set_snapshot_options("-o opt1,opt2");
  ASSERT_EQ(psnap->get_snapshot_options(), "-o opt1,opt2");
}

TEST_F(ChrootLVMSnapshot, Type)
{
  ASSERT_EQ(chroot->get_chroot_type(), "lvm-snapshot");
}

TEST_F(ChrootLVMSnapshot, SetupEnv)
{
  schroot::environment expected;
  setup_env_gen(expected);
  expected.add("CHROOT_TYPE",           "lvm-snapshot");
  expected.add("CHROOT_LVM_SNAPSHOT_OPTIONS", "--size 1G");
  expected.add("CHROOT_SESSION_CLONE",  "true");
  expected.add("CHROOT_SESSION_CREATE", "true");
  expected.add("CHROOT_SESSION_PURGE",  "false");
  expected.add("CHROOT_SESSION_SOURCE", "false");

  ChrootBase::test_setup_env(chroot, expected);
}

TEST_F(ChrootLVMSnapshot, SetupEnvSession)
{
  schroot::environment expected;
  setup_env_gen(expected);
  expected.add("CHROOT_TYPE",           "lvm-snapshot");
  expected.add("SESSION_ID",            "test-session-name");
  expected.add("CHROOT_ALIAS",          "test-session-name");
  expected.add("CHROOT_DESCRIPTION",     chroot->get_description() + ' ' + _("(session chroot)"));
  expected.add("CHROOT_MOUNT_DEVICE",   "/dev/volgroup/test-session-name");
  expected.add("CHROOT_LVM_SNAPSHOT_NAME",    "test-session-name");
  expected.add("CHROOT_LVM_SNAPSHOT_DEVICE",  "/dev/volgroup/test-session-name");
  expected.add("CHROOT_LVM_SNAPSHOT_OPTIONS", "--size 1G");
  expected.add("CHROOT_SESSION_CLONE",  "false");
  expected.add("CHROOT_SESSION_CREATE", "false");
  expected.add("CHROOT_SESSION_PURGE",  "true");
  expected.add("CHROOT_SESSION_SOURCE", "false");

  ChrootBase::test_setup_env(session, expected);
}

TEST_F(ChrootLVMSnapshot, SetupEnvSource)
{
  schroot::environment expected;
  setup_env_gen(expected);
  expected.add("CHROOT_TYPE",           "block-device");
  expected.add("CHROOT_NAME",           "test-name");
  expected.add("CHROOT_DESCRIPTION",     chroot->get_description() + ' ' + _("(source chroot)"));
  expected.add("CHROOT_SESSION_CLONE",  "false");
  expected.add("CHROOT_SESSION_CREATE", "true");
  expected.add("CHROOT_SESSION_PURGE",  "false");
  expected.add("CHROOT_SESSION_SOURCE", "false");

  ChrootBase::test_setup_env(source, expected);
}

TEST_F(ChrootLVMSnapshot, SetupEnvSessionSource)
{
  schroot::environment expected;
  setup_env_gen(expected);
  expected.add("CHROOT_TYPE",           "block-device");
  expected.add("CHROOT_NAME",           "test-name");
  expected.add("SESSION_ID",            "test-session-name");
  expected.add("CHROOT_DESCRIPTION",     chroot->get_description() + ' ' + _("(source chroot) (session chroot)"));
  expected.add("CHROOT_ALIAS",          "test-session-name");
  expected.add("CHROOT_MOUNT_DEVICE",   "/dev/volgroup/testdev");
  expected.add("CHROOT_SESSION_CLONE",  "false");
  expected.add("CHROOT_SESSION_CREATE", "false");
  expected.add("CHROOT_SESSION_PURGE",  "false");
  expected.add("CHROOT_SESSION_SOURCE", "true");

  ChrootBase::test_setup_env(session_source, expected);
}

TEST_F(ChrootLVMSnapshot, SetupKeyfile)
{
  schroot::keyfile expected;
  std::string group = chroot->get_name();
  setup_keyfile_chroot(expected, group);
  setup_keyfile_source(expected, group);
  setup_keyfile_lvm(expected, group);
  expected.set_value(group, "type", "lvm-snapshot");
  expected.set_value(group, "lvm-snapshot-options", "--size 1G");

  ChrootBase::test_setup_keyfile
    (chroot,expected, chroot->get_name());
}

TEST_F(ChrootLVMSnapshot, SetupKeyfileSession)
{
  schroot::keyfile expected;
  const std::string group(session->get_name());
  setup_keyfile_session(expected, group);
  setup_keyfile_lvm(expected, group);
  expected.set_value(group, "type", "lvm-snapshot");
  expected.set_value(group, "name", "test-session-name");
  expected.set_value(group, "selected-name", "test-session-name");
  expected.set_value(group, "description", chroot->get_description() + ' ' + _("(session chroot)"));
  expected.set_value(group, "aliases", "");
  expected.set_value(group, "lvm-snapshot-device", "/dev/volgroup/test-session-name");
  expected.set_value(group, "mount-device", "/dev/volgroup/test-session-name");
  expected.set_value(group, "mount-location", "/mnt/mount-location");

  ChrootBase::test_setup_keyfile
    (session, expected, group);
}

TEST_F(ChrootLVMSnapshot, SetupKeyfileSource)
{
  schroot::keyfile expected;
  const std::string group(source->get_name());
  setup_keyfile_chroot(expected, group);
  setup_keyfile_lvm(expected, group);
  expected.set_value(group, "type", "block-device");
  expected.set_value(group, "description", chroot->get_description() + ' ' + _("(source chroot)"));
  expected.set_value(group, "aliases", "test-name-source,test-alias-1-source,test-alias-2-source");
  setup_keyfile_source_clone(expected, group);

  ChrootBase::test_setup_keyfile
    (source, expected, group);
}

TEST_F(ChrootLVMSnapshot, SetupKeyfileSessionSource)
{
  schroot::keyfile expected;
  const std::string group(source->get_name());
  setup_keyfile_chroot(expected, group);
  setup_keyfile_lvm(expected, group);
  expected.set_value(group, "type", "block-device");
  expected.set_value(group, "mount-device", "/dev/volgroup/testdev");
  expected.set_value(group, "mount-location", "/mnt/mount-location");
  setup_keyfile_session_source_clone(expected, group);

  ChrootBase::test_setup_keyfile
    (session_source, expected, group);
}

TEST_F(ChrootLVMSnapshot, SessionFlags)
{
  ASSERT_EQ(chroot->get_session_flags(),
            (schroot::chroot::facet::facet::SESSION_CREATE |
             schroot::chroot::facet::facet::SESSION_CLONE));

  ASSERT_EQ(session->get_session_flags(),
            (schroot::chroot::facet::facet::SESSION_PURGE));

  /// @todo: Should return NOFLAGS?  This depends upon if source
  /// chroots need transforming into sessions as well (which should
  /// probably happen and be tested for independently).
  ASSERT_EQ(source->get_session_flags(),
            (schroot::chroot::facet::facet::SESSION_CREATE));
}

TEST_F(ChrootLVMSnapshot, PrintDetails)
{
  std::ostringstream os;
  os << chroot;
  // TODO: Compare output.
  ASSERT_FALSE(os.str().empty());
}

TEST_F(ChrootLVMSnapshot, PrintConfig)
{
  std::ostringstream os;
  schroot::keyfile config;
  config << chroot;
  os << schroot::keyfile_writer(config);
  // TODO: Compare output.
  ASSERT_FALSE(os.str().empty());
}

TEST_F(ChrootLVMSnapshot, RunSetupScripts)
{
  ASSERT_TRUE(chroot->get_run_setup_scripts());
}
