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

#include <sbuild/chroot/facet/directory.h>
#include <sbuild/i18n.h>
#include <sbuild/keyfile-writer.h>

#include <test/sbuild/chroot/chroot.h>

#include <algorithm>
#include <set>

using sbuild::_;

class ChrootDirectory : public ChrootBase
{
public:
  ChrootDirectory():
    ChrootBase("directory")
  {}

  void SetUp()
  {
    ChrootBase::SetUp();
    ASSERT_NE(chroot, nullptr);
    ASSERT_NE(session, nullptr);
    ASSERT_EQ(source, nullptr);
    ASSERT_EQ(session_source, nullptr);
#ifdef SBUILD_FEATURE_UNION
    ASSERT_NE(chroot_union, nullptr);
    ASSERT_NE(session_union, nullptr);
    ASSERT_NE(source_union, nullptr);
    ASSERT_NE(session_source_union, nullptr);
#endif
  }

  virtual void setup_chroot_props (sbuild::chroot::chroot::chroot::ptr& chroot)
  {
    ChrootBase::setup_chroot_props(chroot);

    sbuild::chroot::facet::directory::ptr dirfac = chroot->get_facet<sbuild::chroot::facet::directory>();
    ASSERT_NE(dirfac, nullptr);

    dirfac->set_directory("/srv/chroot/example-chroot");
  }

  void setup_env_gen(sbuild::environment& expected)
  {
    setup_env_chroot(expected);

    expected.add("CHROOT_TYPE",           "directory");
    expected.add("CHROOT_MOUNT_LOCATION", "/mnt/mount-location");
    expected.add("CHROOT_DIRECTORY",      "/srv/chroot/example-chroot");
    expected.add("CHROOT_PATH",           "/mnt/mount-location");
  }
};

TEST_F(ChrootDirectory, Directory)
{
  sbuild::chroot::facet::directory::ptr dirfac = chroot->get_facet<sbuild::chroot::facet::directory>();
  ASSERT_NE(dirfac, nullptr);
  dirfac->set_directory("/mnt/chroot/example");
  ASSERT_EQ(chroot->get_mount_location(), "/mnt/mount-location");
  ASSERT_EQ(dirfac->get_directory(), "/mnt/chroot/example");
  ASSERT_EQ(chroot->get_path(), "/mnt/mount-location");
}

TEST_F(ChrootDirectory, Type)
{
  ASSERT_EQ(chroot->get_chroot_type(), "directory");
}

TEST_F(ChrootDirectory, SetupEnv)
{
  sbuild::environment expected;
  setup_env_gen(expected);

  expected.add("CHROOT_SESSION_CLONE",  "false");
  expected.add("CHROOT_SESSION_CREATE", "true");
  expected.add("CHROOT_SESSION_PURGE",  "false");
#ifdef SBUILD_FEATURE_UNION
  expected.add("CHROOT_UNION_TYPE",     "none");
#endif // SBUILD_FEATURE_UNION

  ChrootBase::test_setup_env(chroot, expected);
}

TEST_F(ChrootDirectory, SetupEnvSession)
{
  sbuild::environment expected;
  setup_env_gen(expected);

  expected.add("SESSION_ID",            "test-session-name");
  expected.add("CHROOT_ALIAS",          "test-session-name");
  expected.add("CHROOT_DESCRIPTION",     chroot->get_description() + ' ' + _("(session chroot)"));
  expected.add("CHROOT_SESSION_CLONE",  "false");
  expected.add("CHROOT_SESSION_CREATE", "false");
  expected.add("CHROOT_SESSION_PURGE",  "false");
#ifdef SBUILD_FEATURE_UNION
  expected.add("CHROOT_UNION_TYPE",     "none");
#endif // SBUILD_FEATURE_UNION

  ChrootBase::test_setup_env(session, expected);
}

#ifdef SBUILD_FEATURE_UNION
TEST_F(ChrootDirectory, SetupEnvUnion)
{
  sbuild::environment expected;
  setup_env_gen(expected);

  expected.add("SESSION_ID",            "test-union-session-name");
  expected.add("CHROOT_ALIAS",          "test-union-session-name");
  expected.add("CHROOT_DESCRIPTION",     chroot->get_description() + ' ' + _("(session chroot)"));
  expected.add("CHROOT_SESSION_CLONE",  "false");
  expected.add("CHROOT_SESSION_CREATE", "false");
  expected.add("CHROOT_SESSION_PURGE",  "true");
  expected.add("CHROOT_UNION_TYPE",     "aufs");
  expected.add("CHROOT_UNION_MOUNT_OPTIONS",      "union-mount-options");
  expected.add("CHROOT_UNION_OVERLAY_DIRECTORY",  "/overlay/test-union-session-name");
  expected.add("CHROOT_UNION_UNDERLAY_DIRECTORY", "/underlay/test-union-session-name");

  ChrootBase::test_setup_env(session_union, expected);
}

TEST_F(ChrootDirectory, SetupEnvSourceUnion)
{
  sbuild::environment expected;
  setup_env_gen(expected);

  expected.add("CHROOT_DESCRIPTION",     chroot->get_description() + ' ' + _("(source chroot)"));
  expected.add("CHROOT_SESSION_CLONE",  "false");
  expected.add("CHROOT_SESSION_CREATE", "true");
  expected.add("CHROOT_SESSION_PURGE",  "false");

  ChrootBase::test_setup_env(source_union, expected);
}

TEST_F(ChrootDirectory, SetupEnvSessionSourceUnion)
{
  sbuild::environment expected;
  setup_env_gen(expected);

  expected.add("SESSION_ID",            "test-session-name");
  expected.add("CHROOT_ALIAS",          "test-session-name");
  expected.add("CHROOT_DESCRIPTION",     chroot->get_description() + ' ' + _("(source chroot) (session chroot)"));
  expected.add("CHROOT_SESSION_CLONE",  "false");
  expected.add("CHROOT_SESSION_CREATE", "false");
  expected.add("CHROOT_SESSION_PURGE",  "false");

  ChrootBase::test_setup_env(session_source_union, expected);
}
#endif // SBUILD_FEATURE_UNION

TEST_F(ChrootDirectory, SetupKeyfile)
{
  sbuild::keyfile expected;
  const std::string group(chroot->get_name());
  setup_keyfile_chroot(expected, group);
  expected.set_value(group, "type", "directory");
  expected.set_value(group, "directory", "/srv/chroot/example-chroot");
#ifdef SBUILD_FEATURE_UNION
  setup_keyfile_union_unconfigured(expected, group);
#endif // SBUILD_FEATURE_UNION

  ChrootBase::test_setup_keyfile
    (chroot, expected, group);
}

TEST_F(ChrootDirectory, SetupKeyfileSession)
{
  sbuild::keyfile expected;
  const std::string group(session->get_name());
  setup_keyfile_session(expected, group);
  expected.set_value(group, "type", "directory");
  expected.set_value(group, "name", "test-session-name");
  expected.set_value(group, "selected-name", "test-session-name");
  expected.set_value(group, "directory", "/srv/chroot/example-chroot");
  expected.set_value(group, "mount-location", "/mnt/mount-location");
  setup_keyfile_session_clone(expected, group);
#ifdef SBUILD_FEATURE_UNION
  setup_keyfile_union_unconfigured(expected, group);
#endif // SBUILD_FEATURE_UNION

  ChrootBase::test_setup_keyfile
    (session, expected, group);
}

// Note, this test is unique to the directory chroot but is
// applicable to all.
TEST_F(ChrootDirectory, SetupKeyfileRootSession)
{
  // Create session owned by user in root-users.
  this->session = this->chroot->clone_session("test-session-name",
                                              "test-session-name",
                                              "user3",
                                              true);
  if (this->session)
    {
      ASSERT_NE(this->session->get_facet<sbuild::chroot::facet::session>(), nullptr);
    }

  sbuild::keyfile expected;
  const std::string group(session->get_name());
  setup_keyfile_session(expected, group);
  expected.set_value(group, "users", "");
  expected.set_value(group, "root-users", "user3");
  expected.set_value(group, "type", "directory");
  expected.set_value(group, "name", "test-session-name");
  expected.set_value(group, "selected-name", "test-session-name");
  expected.set_value(group, "directory", "/srv/chroot/example-chroot");
  expected.set_value(group, "mount-location", "/mnt/mount-location");
  setup_keyfile_session_clone(expected, group);
#ifdef SBUILD_FEATURE_UNION
  setup_keyfile_union_unconfigured(expected, group);
#endif // SBUILD_FEATURE_UNION

  ChrootBase::test_setup_keyfile
    (session, expected, group);
}

#ifdef SBUILD_FEATURE_UNION
TEST_F(ChrootDirectory, SetupKeyfileUnion)
{
  sbuild::keyfile expected;
  const std::string group(chroot_union->get_name());
  setup_keyfile_chroot(expected, group);
  setup_keyfile_source(expected, group);
  expected.set_value(group, "type", "directory");
  expected.set_value(group, "directory", "/srv/chroot/example-chroot");
  setup_keyfile_union_configured(expected, group);

  ChrootBase::test_setup_keyfile
    (chroot_union, expected, group);
}

TEST_F(ChrootDirectory, SetupKeyfileSessionUnion)
{
  sbuild::keyfile expected;
  const std::string group(session_union->get_name());
  setup_keyfile_session(expected, group);
  expected.set_value(group, "type", "directory");
  expected.set_value(group, "name", "test-union-session-name");
  expected.set_value(group, "selected-name", "test-union-session-name");
  expected.set_value(group, "directory", "/srv/chroot/example-chroot");
  expected.set_value(group, "mount-location", "/mnt/mount-location");
  setup_keyfile_session_clone(expected, group);
  setup_keyfile_union_session(expected, group);

  ChrootBase::test_setup_keyfile
    (session_union, expected, group);
}

TEST_F(ChrootDirectory, SetupKeyfileSourceUnion)
{
  sbuild::keyfile expected;
  const std::string group(source_union->get_name());
  setup_keyfile_chroot(expected, group);
  setup_keyfile_source_clone(expected, group);
  expected.set_value(group, "type", "directory");
  expected.set_value(group, "description", chroot->get_description() + ' ' + _("(source chroot)"));
  expected.set_value(group, "directory", "/srv/chroot/example-chroot");

  ChrootBase::test_setup_keyfile
    (source_union, expected, group);
}

TEST_F(ChrootDirectory, SetupKeyfileSessionSourceUnion)
{
  sbuild::keyfile expected;
  const std::string group(source_union->get_name());
  setup_keyfile_chroot(expected, group);
  setup_keyfile_source_clone(expected, group);
  expected.set_value(group, "type", "directory");
  expected.set_value(group, "description", chroot->get_description() + ' ' + _("(source chroot)"));
  expected.set_value(group, "directory", "/srv/chroot/example-chroot");
  expected.set_value(group, "mount-location", "/mnt/mount-location");
  setup_keyfile_session_source_clone(expected, group);

  ChrootBase::test_setup_keyfile
    (session_source_union, expected, group);
}
#endif // SBUILD_FEATURE_UNION

TEST_F(ChrootDirectory, SessionFlags)
{
  ASSERT_EQ(chroot->get_session_flags(),
            sbuild::chroot::facet::facet::SESSION_CREATE);

  ASSERT_EQ(session->get_session_flags(),
            sbuild::chroot::facet::facet::SESSION_NOFLAGS);

#ifdef SBUILD_FEATURE_UNION
  ASSERT_EQ(chroot_union->get_session_flags(),
            (sbuild::chroot::facet::facet::SESSION_CREATE |
             sbuild::chroot::facet::facet::SESSION_CLONE));

  ASSERT_EQ(session_union->get_session_flags(),
            sbuild::chroot::facet::facet::SESSION_PURGE);

  ASSERT_EQ(source_union->get_session_flags(),
            sbuild::chroot::facet::facet::SESSION_CREATE);
#endif // SBUILD_FEATURE_UNION
}

TEST_F(ChrootDirectory, PrintDetails)
{
  std::ostringstream os;
  os << chroot;
  // TODO: Compare output.
  ASSERT_FALSE(os.str().empty());
}

TEST_F(ChrootDirectory, PrintConfig)
{
  std::ostringstream os;
  sbuild::keyfile config;
  config << chroot;
  os << sbuild::keyfile_writer(config);
  // TODO: Compare output.
  ASSERT_FALSE(os.str().empty());
}

TEST_F(ChrootDirectory, RunSetupScripts)
{
  ASSERT_TRUE(chroot->get_run_setup_scripts());
}
