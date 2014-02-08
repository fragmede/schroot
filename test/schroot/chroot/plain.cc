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
#include <schroot/chroot/facet/plain.h>
#include <schroot/keyfile-writer.h>

#include <test/schroot/chroot/chroot.h>

#include <algorithm>
#include <set>

class ChrootPlain : public ChrootBase
{

public:
  ChrootPlain():
    ChrootBase("plain")
  {}

  void SetUp()
  {
    ChrootBase::SetUp();
    ASSERT_NE(chroot, nullptr);
    ASSERT_EQ(session, nullptr);
    ASSERT_EQ(source, nullptr);
    ASSERT_EQ(session_source, nullptr);
  }

  virtual void setup_chroot_props (schroot::chroot::chroot::ptr& chroot)
  {
    ChrootBase::setup_chroot_props(chroot);

    chroot->set_mount_location("");

    schroot::chroot::facet::plain::ptr plfac = chroot->get_facet<schroot::chroot::facet::plain>();
    plfac->set_directory("/srv/chroot/example-chroot");
  }
};

TEST_F(ChrootPlain, Directory)
{
  schroot::chroot::facet::plain::ptr plfac = chroot->get_facet<schroot::chroot::facet::plain>();
  plfac->set_directory("/mnt/mount-location/example");
  ASSERT_EQ(plfac->get_directory(), "/mnt/mount-location/example");
  ASSERT_EQ(chroot->get_path(), "/mnt/mount-location/example");
  ASSERT_EQ(chroot->get_mount_location(), "");
}

TEST_F(ChrootPlain, Type)
{
  ASSERT_EQ(chroot->get_chroot_type(), "plain");
}

TEST_F(ChrootPlain, SetupEnv)
{
  schroot::environment expected;
  setup_env_chroot(expected);
  expected.add("CHROOT_TYPE",           "plain");
  expected.add("CHROOT_DIRECTORY",      "/srv/chroot/example-chroot");
  expected.add("CHROOT_PATH",           "/srv/chroot/example-chroot");
  expected.add("CHROOT_SESSION_CLONE",  "false");
  expected.add("CHROOT_SESSION_CREATE", "false");
  expected.add("CHROOT_SESSION_PURGE",  "false");

  ChrootBase::test_setup_env(chroot, expected);
}

TEST_F(ChrootPlain, SetupKeyfile)
{
  schroot::keyfile expected;
  std::string group = chroot->get_name();
  setup_keyfile_chroot(expected, group);
  expected.set_value(group, "type", "plain");
  expected.set_value(group, "directory", "/srv/chroot/example-chroot");

  ChrootBase::test_setup_keyfile
    (chroot, expected, group);
}

TEST_F(ChrootPlain, SessionFlags)
{
  ASSERT_EQ(chroot->get_session_flags(),
            schroot::chroot::facet::facet::SESSION_NOFLAGS);
}

TEST_F(ChrootPlain, PrintDetails)
{
  std::ostringstream os;
  os << chroot;
  // TODO: Compare output.
  ASSERT_FALSE(os.str().empty());
}

TEST_F(ChrootPlain, PrintConfig)
{
  std::ostringstream os;
  schroot::keyfile config;
  config << chroot;
  os << schroot::keyfile_writer(config);
  // TODO: Compare output.
  ASSERT_FALSE(os.str().empty());
}

TEST_F(ChrootPlain, RunSetupScripts)
{
  ASSERT_FALSE(chroot->get_run_setup_scripts());
}
