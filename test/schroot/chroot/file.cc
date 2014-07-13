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

#include <schroot/chroot/facet/file.h>
#include <schroot/i18n.h>
#include <schroot/keyfile-writer.h>

#include <test/schroot/chroot/chroot.h>

#include <algorithm>
#include <set>

using schroot::_;

class ChrootFile : public ChrootBase
{
public:
  ChrootFile():
    ChrootBase("file")
  {}

  void SetUp()
  {
    ChrootBase::SetUp();
    ASSERT_NE(chroot, nullptr);
    ASSERT_NE(session, nullptr);
    ASSERT_NE(source, nullptr);
    ASSERT_NE(session_source, nullptr);
#ifdef SCHROOT_FEATURE_UNION
    ASSERT_EQ(chroot_union, nullptr);
    ASSERT_EQ(session_union, nullptr);
    ASSERT_EQ(source_union, nullptr);
    ASSERT_EQ(session_source_union, nullptr);
#endif // SCHROOT_FEATURE_UNION
  }

  virtual void setup_chroot_props (schroot::chroot::chroot::ptr& chroot)
  {
    ChrootBase::setup_chroot_props(chroot);

    schroot::chroot::facet::file::ptr filefac = chroot->get_facet_strict<schroot::chroot::facet::file>();
    filefac->set_filename("/srv/chroot/example.tar.bz2");
    filefac->set_location("/sid");
  }

  void setup_env_gen(schroot::environment &expected)
  {
    setup_env_chroot(expected);
    expected.add("CHROOT_TYPE",            "file");
    expected.add("CHROOT_FILE",            "/srv/chroot/example.tar.bz2");
    expected.add("CHROOT_LOCATION",        "/sid");
    expected.add("CHROOT_FILE_REPACK",     "false");
    expected.add("CHROOT_FILE_UNPACK_DIR", SCHROOT_FILE_UNPACK_DIR);
    expected.add("CHROOT_MOUNT_LOCATION",  "/mnt/mount-location");
    expected.add("CHROOT_PATH",            "/mnt/mount-location/sid");
    expected.add("CHROOT_SESSION_CLONE",   "true");
    expected.add("CHROOT_SESSION_CREATE",  "true");
    expected.add("CHROOT_SESSION_PURGE",   "false");
    expected.add("CHROOT_SESSION_SOURCE",  "false");

    ChrootBase::test_setup_env(chroot, expected);
  }

  void setup_keyfile_file(schroot::keyfile &expected, const std::string group)
  {
    expected.set_value(group, "type", "file");
    expected.set_value(group, "file", "/srv/chroot/example.tar.bz2");
    expected.set_value(group, "location", "/sid");
  }
};

TEST_F(ChrootFile, Filename)
{
  schroot::chroot::facet::file::ptr filefac = chroot->get_facet_strict<schroot::chroot::facet::file>();
  filefac->set_filename("/srv/chroot-images/unstable.tar.gz");
  ASSERT_EQ(filefac->get_filename(), "/srv/chroot-images/unstable.tar.gz");
}

TEST_F(ChrootFile, Type)
{
  ASSERT_EQ(chroot->get_chroot_type(), "file");
}

TEST_F(ChrootFile, Location)
{
  schroot::chroot::facet::file::ptr filefac = chroot->get_facet_strict<schroot::chroot::facet::file>();

  filefac->set_location("");
  ASSERT_EQ(filefac->get_location(), "");
  ASSERT_EQ(filefac->get_path(), chroot->get_mount_location());

  filefac->set_location("/test");
  ASSERT_EQ(filefac->get_location(), "/test");
  ASSERT_EQ(filefac->get_path(), "/mnt/mount-location/test");
}

TEST_F(ChrootFile, LocationInvalid)
{
  schroot::chroot::facet::file::ptr filefac = chroot->get_facet_strict<schroot::chroot::facet::file>();

  ASSERT_THROW(filefac->set_location("invalid"), schroot::chroot::chroot::error);
}

TEST_F(ChrootFile, Repack)
{
  schroot::chroot::facet::file::ptr filechrootfac = chroot->get_facet_strict<schroot::chroot::facet::file>();
  schroot::chroot::facet::file::ptr filesessionfac = session->get_facet_strict<schroot::chroot::facet::file>();
  schroot::chroot::facet::file::ptr filesourcefac = source->get_facet_strict<schroot::chroot::facet::file>();

  ASSERT_FALSE(filechrootfac->get_file_repack());
  ASSERT_FALSE(filesessionfac->get_file_repack());
  ASSERT_TRUE(filesourcefac->get_file_repack());
}

TEST_F(ChrootFile, SetupEnv)
{
  schroot::environment expected;
  setup_env_gen(expected);
  expected.add("CHROOT_FILE_REPACK",    "false");
  expected.add("CHROOT_SESSION_CLONE",  "true");
  expected.add("CHROOT_SESSION_CREATE", "true");
  expected.add("CHROOT_SESSION_PURGE",  "false");
  expected.add("CHROOT_SESSION_SOURCE", "false");

  ChrootBase::test_setup_env(chroot, expected);
}

TEST_F(ChrootFile, SetupEnvSession)
{
  schroot::environment expected;
  setup_env_gen(expected);
  expected.add("SESSION_ID",           "test-session-name");
  expected.add("CHROOT_ALIAS",         "test-session-name");
  expected.add("CHROOT_DESCRIPTION",    chroot->get_description() + ' ' + _("(session chroot)"));
  expected.add("CHROOT_FILE_REPACK",    "false");
  expected.add("CHROOT_SESSION_CLONE",  "false");
  expected.add("CHROOT_SESSION_CREATE", "false");
  expected.add("CHROOT_SESSION_PURGE",  "true");
  expected.add("CHROOT_SESSION_SOURCE", "false");

  ChrootBase::test_setup_env(session, expected);
}

TEST_F(ChrootFile, SetupSource)
{
  schroot::environment expected;
  setup_env_gen(expected);
  expected.add("CHROOT_DESCRIPTION",    chroot->get_description() + ' ' + _("(source chroot)"));
  expected.add("CHROOT_FILE_REPACK",    "true");
  expected.add("CHROOT_SESSION_CLONE",  "false");
  expected.add("CHROOT_SESSION_CREATE", "true");
  expected.add("CHROOT_SESSION_PURGE",  "false");
  expected.add("CHROOT_SESSION_SOURCE", "false");

  ChrootBase::test_setup_env(source, expected);
}

TEST_F(ChrootFile, SetupEnvSessionSource)
{
  schroot::environment expected;
  setup_env_gen(expected);
  expected.add("SESSION_ID",           "test-session-name");
  expected.add("CHROOT_ALIAS",         "test-session-name");
  expected.add("CHROOT_DESCRIPTION",    chroot->get_description() + ' ' + _("(source chroot) (session chroot)"));
  expected.add("CHROOT_FILE_REPACK",    "true");
  expected.add("CHROOT_SESSION_CLONE",  "false");
  expected.add("CHROOT_SESSION_CREATE", "false");
  expected.add("CHROOT_SESSION_PURGE",  "true");
  expected.add("CHROOT_SESSION_SOURCE", "true");

  ChrootBase::test_setup_env(session_source, expected);
}

TEST_F(ChrootFile, SetupKeyfile)
{
  schroot::keyfile expected;
  const std::string group(chroot->get_name());
  setup_keyfile_chroot(expected, group);
  setup_keyfile_source(expected, group);
  setup_keyfile_file(expected, group);

  ChrootBase::test_setup_keyfile
    (chroot, expected, group);
}

TEST_F(ChrootFile, SetupKeyfileSession)
{
  schroot::keyfile expected;
  const std::string group(session->get_name());
  setup_keyfile_session(expected, group);
  setup_keyfile_file(expected, group);
  expected.set_value(group, "name", "test-session-name");
  expected.set_value(group, "selected-name", "test-session-name");
  expected.set_value(group, "file-repack", "false");
  expected.set_value(group, "mount-location", "/mnt/mount-location");
  setup_keyfile_session_clone(expected, group);

  ChrootBase::test_setup_keyfile
    (session, expected, group);
}

TEST_F(ChrootFile, SetupKeyfileSource)
{
  schroot::keyfile expected;
  const std::string group(source->get_name());
  setup_keyfile_chroot(expected, group);
  setup_keyfile_source_clone(expected, group);
  setup_keyfile_file(expected, group);
  expected.set_value(group, "description", chroot->get_description() + ' ' + _("(source chroot)"));

  ChrootBase::test_setup_keyfile
    (source, expected, group);
}

TEST_F(ChrootFile, SetupKeyfileSessionSource)
{
  schroot::keyfile expected;
  const std::string group(source->get_name());
  setup_keyfile_chroot(expected, group);
  setup_keyfile_file(expected, group);
  setup_keyfile_session_source_clone(expected, group);
  expected.set_value(group, "description", chroot->get_description() + ' ' + _("(source chroot) (session chroot)"));
  expected.set_value(group, "file-repack", "true");
  expected.set_value(group, "mount-location", "/mnt/mount-location");

  ChrootBase::test_setup_keyfile
    (session_source, expected, group);
}

TEST_F(ChrootFile, SessionFlags)
{
  ASSERT_EQ(chroot->get_session_flags(),
            (schroot::chroot::facet::facet::SESSION_CREATE |
             schroot::chroot::facet::facet::SESSION_CLONE));

  ASSERT_EQ(session->get_session_flags(),
            schroot::chroot::facet::facet::SESSION_PURGE);

  ASSERT_EQ(source->get_session_flags(),
            schroot::chroot::facet::facet::SESSION_CREATE);
}

TEST_F(ChrootFile, PrintDetails)
{
  std::ostringstream os;
  os << chroot;
  // TODO: Compare output.
  ASSERT_FALSE(os.str().empty());
}

TEST_F(ChrootFile, PrintConfig)
{
  std::ostringstream os;
  schroot::keyfile config;
  config << chroot;
  os << schroot::keyfile_writer(config);
  // TODO: Compare output.
  ASSERT_FALSE(os.str().empty());
}

TEST_F(ChrootFile, RunSetupScripts)
{
  ASSERT_TRUE(chroot->get_run_setup_scripts());
}
