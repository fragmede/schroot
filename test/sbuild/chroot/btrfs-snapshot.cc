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

#include <sbuild/chroot/facet/btrfs-snapshot.h>
#include <sbuild/i18n.h>
#include <sbuild/keyfile-writer.h>
#include <sbuild/util.h>

#include <test/sbuild/chroot/chroot.h>

#include <algorithm>
#include <set>

using sbuild::_;

class BtrfsSnapshot : public ChrootBase
{
public:
  BtrfsSnapshot():
    ChrootBase("btrfs-snapshot")
  {}

  void SetUp()
  {
    ChrootBase::SetUp();
    ASSERT_NE(chroot, nullptr);
    ASSERT_NE(session, nullptr);
    ASSERT_NE(source, nullptr);
    ASSERT_NE(session_source, nullptr);
  }

  virtual void setup_chroot_props (sbuild::chroot::chroot::ptr& chroot)
  {
    ChrootBase::setup_chroot_props(chroot);

    sbuild::chroot::facet::btrfs_snapshot::ptr bfac = chroot->get_facet_strict<sbuild::chroot::facet::btrfs_snapshot>();
    bfac->set_source_subvolume("/srv/chroot/sid");
    bfac->set_snapshot_directory("/srv/chroot/snapshot");
  }

  void setup_env_gen(sbuild::environment &expected)
  {
    setup_env_chroot(expected);
    expected.add("CHROOT_MOUNT_LOCATION", "/mnt/mount-location");
    expected.add("CHROOT_PATH",           "/mnt/mount-location");
  }

  void setup_keyfile_btrfs(sbuild::keyfile &expected, std::string group)
  {
  }
};

TEST_F(BtrfsSnapshot, SourceSubvolume)
{
  sbuild::chroot::facet::btrfs_snapshot::ptr bfac = chroot->get_facet_strict<sbuild::chroot::facet::btrfs_snapshot>();
  bfac->set_source_subvolume("/srv/chroot/chroot");
  ASSERT_EQ(bfac->get_source_subvolume(), "/srv/chroot/chroot");
}

TEST_F(BtrfsSnapshot, SnapshotDirectory)
{
  sbuild::chroot::facet::btrfs_snapshot::ptr bfac = chroot->get_facet_strict<sbuild::chroot::facet::btrfs_snapshot>();
  bfac->set_snapshot_directory("/srv/chroot/snapshot2");
  ASSERT_EQ(bfac->get_snapshot_directory(), "/srv/chroot/snapshot2");
}

TEST_F(BtrfsSnapshot, SnapshotName)
{
  sbuild::chroot::facet::btrfs_snapshot::ptr bfac = chroot->get_facet_strict<sbuild::chroot::facet::btrfs_snapshot>();
  bfac->set_snapshot_directory("/srv/chroot/snapshot2/test-session-id");
  ASSERT_EQ(bfac->get_snapshot_directory(), "/srv/chroot/snapshot2/test-session-id");
}

TEST_F(BtrfsSnapshot, SourceSubvolumeFail)
{
  sbuild::chroot::facet::btrfs_snapshot::ptr bfac = chroot->get_facet_strict<sbuild::chroot::facet::btrfs_snapshot>();
  ASSERT_THROW(bfac->set_source_subvolume("chroot/invalid"), sbuild::chroot::chroot::error);
}

TEST_F(BtrfsSnapshot, SnapshotDirectoryFail)
{
  sbuild::chroot::facet::btrfs_snapshot::ptr bfac = chroot->get_facet_strict<sbuild::chroot::facet::btrfs_snapshot>();
  ASSERT_THROW(bfac->set_snapshot_directory("chroot/invalid"), sbuild::chroot::chroot::error);
}

TEST_F(BtrfsSnapshot, SnapshotNameFail)
{
  sbuild::chroot::facet::btrfs_snapshot::ptr bfac = chroot->get_facet_strict<sbuild::chroot::facet::btrfs_snapshot>();
  ASSERT_THROW(bfac->set_snapshot_name("invalid"), sbuild::chroot::chroot::error);
}

TEST_F(BtrfsSnapshot, Type)
{
  ASSERT_EQ(chroot->get_chroot_type(), "btrfs-snapshot");
}


TEST_F(BtrfsSnapshot, SetupEnv)
{
  sbuild::environment expected;
  setup_env_gen(expected);
  expected.add("CHROOT_TYPE",           "btrfs-snapshot");
  expected.add("CHROOT_BTRFS_SOURCE_SUBVOLUME",       "/srv/chroot/sid");
  expected.add("CHROOT_BTRFS_SNAPSHOT_DIRECTORY", "/srv/chroot/snapshot");
  expected.add("CHROOT_SESSION_CLONE",  "true");
  expected.add("CHROOT_SESSION_CREATE", "true");
  expected.add("CHROOT_SESSION_PURGE",  "false");

  ChrootBase::test_setup_env(chroot, expected);
}

TEST_F(BtrfsSnapshot, SetupEnvSession)
{
  sbuild::environment expected;
  setup_env_gen(expected);
  expected.add("CHROOT_TYPE",           "btrfs-snapshot");
  expected.add("SESSION_ID",            "test-session-name");
  expected.add("CHROOT_ALIAS",          "test-session-name");
  expected.add("CHROOT_DESCRIPTION",     chroot->get_description() + ' ' + _("(session chroot)"));
  expected.add("CHROOT_BTRFS_SOURCE_SUBVOLUME",       "/srv/chroot/sid");
  expected.add("CHROOT_BTRFS_SNAPSHOT_DIRECTORY", "/srv/chroot/snapshot");
  expected.add("CHROOT_BTRFS_SNAPSHOT_NAME", "/srv/chroot/snapshot/test-session-name");
  expected.add("CHROOT_SESSION_CLONE",  "false");
  expected.add("CHROOT_SESSION_CREATE", "false");
  expected.add("CHROOT_SESSION_PURGE",  "true");

  ChrootBase::test_setup_env(session, expected);
}

TEST_F(BtrfsSnapshot, SetupEnvSource)
{
  sbuild::environment expected;
  setup_env_gen(expected);
  expected.add("CHROOT_TYPE",           "directory");
  expected.add("CHROOT_NAME",           "test-name");
  expected.add("CHROOT_DESCRIPTION",     chroot->get_description() + ' ' + _("(source chroot)"));
  expected.add("CHROOT_DIRECTORY",       "/srv/chroot/sid");
  expected.add("CHROOT_SESSION_CLONE",  "false");
  expected.add("CHROOT_SESSION_CREATE", "true");
  expected.add("CHROOT_SESSION_PURGE",  "false");

  ChrootBase::test_setup_env(source, expected);
}

TEST_F(BtrfsSnapshot, SetupEnvSessionSource)
{
  sbuild::environment expected;
  setup_env_gen(expected);
  expected.add("CHROOT_TYPE",           "directory");
  expected.add("CHROOT_NAME",           "test-name");
  expected.add("CHROOT_DESCRIPTION",     chroot->get_description() + ' ' + _("(source chroot)"));
  expected.add("CHROOT_DIRECTORY",       "/srv/chroot/sid");
  expected.add("CHROOT_SESSION_CLONE",  "false");
  expected.add("CHROOT_SESSION_CREATE", "true");
  expected.add("CHROOT_SESSION_PURGE",  "false");

  ChrootBase::test_setup_env(source, expected);
}

TEST_F(BtrfsSnapshot, SetupKeyfile)
{
  sbuild::keyfile expected;
  std::string group = chroot->get_name();
  setup_keyfile_chroot(expected, group);
  setup_keyfile_source(expected, group);
  setup_keyfile_btrfs(expected, group);
  expected.set_value(group, "type", "btrfs-snapshot");
  expected.set_value(group, "btrfs-source-subvolume", "/srv/chroot/sid");
  expected.set_value(group, "btrfs-snapshot-directory", "/srv/chroot/snapshot");

  ChrootBase::test_setup_keyfile
    (chroot,expected, chroot->get_name());
}

TEST_F(BtrfsSnapshot, SetupKeyfileSession)
{
  sbuild::keyfile expected;
  const std::string group(session->get_name());
  setup_keyfile_session(expected, group);
  setup_keyfile_btrfs(expected, group);
  expected.set_value(group, "type", "btrfs-snapshot");
  expected.set_value(group, "name", "test-session-name");
  expected.set_value(group, "selected-name", "test-session-name");
  expected.set_value(group, "description", chroot->get_description() + ' ' + _("(session chroot)"));
  expected.set_value(group, "aliases", "");
  expected.set_value(group, "btrfs-snapshot-name", "/srv/chroot/snapshot/test-session-name");
  expected.set_value(group, "mount-location", "/mnt/mount-location");

  ChrootBase::test_setup_keyfile
    (session, expected, group);
}

TEST_F(BtrfsSnapshot, SetupKeyfileSource)
{
  sbuild::keyfile expected;
  const std::string group(source->get_name());
  setup_keyfile_chroot(expected, group);
  setup_keyfile_btrfs(expected, group);
  expected.set_value(group, "type", "directory");
  expected.set_value(group, "description", chroot->get_description() + ' ' + _("(source chroot)"));
  expected.set_value(group, "aliases", "test-alias-1-source,test-alias-2-source");
  expected.set_value(group, "directory", "/srv/chroot/sid");
  setup_keyfile_source_clone(expected, group);

  ChrootBase::test_setup_keyfile
    (source, expected, group);
}

TEST_F(BtrfsSnapshot, SetupKeyfileSessionSource)
{
  sbuild::keyfile expected;
  const std::string group(source->get_name());
  setup_keyfile_chroot(expected, group);
  setup_keyfile_btrfs(expected, group);
  expected.set_value(group, "type", "directory");
  expected.set_value(group, "description", chroot->get_description() + ' ' + _("(source chroot)"));
  expected.set_value(group, "aliases", "test-alias-1-source,test-alias-2-source");
  expected.set_value(group, "directory", "/srv/chroot/sid");
  expected.set_value(group, "mount-location", "/mnt/mount-location");
  setup_keyfile_session_source_clone(expected, group);

  ChrootBase::test_setup_keyfile
    (session_source, expected, group);
}

TEST_F(BtrfsSnapshot, SessionFlags)
{
  ASSERT_EQ(chroot->get_session_flags(),
            (sbuild::chroot::facet::facet::SESSION_CREATE |
             sbuild::chroot::facet::facet::SESSION_CLONE));

  ASSERT_EQ(session->get_session_flags(),
            (sbuild::chroot::facet::facet::SESSION_PURGE));

  /// @todo: Should return NOFLAGS?  This depends upon if source
  /// chroots need transforming into sessions as well (which should
  /// probably happen and be tested for independently).
  ASSERT_EQ(source->get_session_flags(),
            (sbuild::chroot::facet::facet::SESSION_CREATE));
}

TEST_F(BtrfsSnapshot, PrintDetails)
{
  std::ostringstream os;
  os << chroot;
  // TODO: Compare output.
  ASSERT_FALSE(os.str().empty());
}

TEST_F(BtrfsSnapshot, PrintConfig)
{
  std::ostringstream os;
  sbuild::keyfile config;
  config << chroot;
  os << sbuild::keyfile_writer(config);
  // TODO: Compare output.
  ASSERT_FALSE(os.str().empty());
}

TEST_F(BtrfsSnapshot, RunSetupScripts)
{
  ASSERT_TRUE(chroot->get_run_setup_scripts());
}
