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

#include <schroot/chroot/facet/directory.h>
#include <schroot/chroot/facet/userdata.h>

using schroot::_;

class Userdata : public ::testing::Test
{
public:
  schroot::chroot::chroot::ptr chroot;
  schroot::chroot::facet::userdata::ptr userdata;

  Userdata():
    chroot(),
    userdata()
  {}

  void SetUp()
  {
    chroot = schroot::chroot::chroot::create("directory");
    ASSERT_NE(chroot, nullptr);

    schroot::chroot::facet::directory::ptr dirfac = chroot->get_facet<schroot::chroot::facet::directory>();
    ASSERT_NE(dirfac, nullptr);
    dirfac->set_directory("/chroots/test");

    userdata = chroot->get_facet<schroot::chroot::facet::userdata>();
    ASSERT_NE(userdata, nullptr);

    schroot::string_set userkeys;
    userkeys.insert("sbuild.resolver");
    userkeys.insert("debian.dist");
    userkeys.insert("sbuild.purge");
    schroot::string_set rootkeys;
    rootkeys.insert("debian.apt-update");
    userdata->set_user_modifiable_keys(userkeys);
    userdata->set_root_modifiable_keys(rootkeys);
  }

  void TearDown()
  {
    this->chroot = schroot::chroot::chroot::ptr();
    this->userdata = schroot::chroot::facet::userdata::ptr();
  }
};

TEST_F(Userdata, Set)
{
  userdata->set_data("custom.test1", "testval");
  userdata->set_data("sbuild.resolver", "apt");
  userdata->set_data("setup.fstab", "custom/fstab");

  std::string t1;
  ASSERT_TRUE(userdata->get_data("custom.test1", t1));
  ASSERT_EQ(t1, "testval");

  std::string t2;
  ASSERT_TRUE(userdata->get_data("sbuild.resolver", t2));
  ASSERT_EQ(t2, "apt");

  std::string t3;
  ASSERT_TRUE(userdata->get_data("setup.fstab", t3));
  ASSERT_EQ(t3, "custom/fstab");

  std::string t4("invalid");
  ASSERT_TRUE(!userdata->get_data("invalidkey", t4));
  ASSERT_EQ(t4, "invalid");
}

TEST_F(Userdata, SetFail1)
{
  EXPECT_THROW(userdata->set_data("custom", "testval"),
               schroot::chroot::facet::userdata::error);
}

TEST_F(Userdata, SetFail2)
{
  EXPECT_NO_THROW(userdata->set_data("custom.key.set", "testval1"));
  EXPECT_THROW(userdata->set_data("custom.key_set", "testval2"),
               schroot::chroot::facet::userdata::error);
}

TEST_F(Userdata, SetFail3)
{
  EXPECT_THROW(userdata->set_data("setup-data-dir", "testval3"),
               schroot::chroot::facet::userdata::error);
}

TEST_F(Userdata, SetFail4)
{
  EXPECT_THROW(userdata->set_data("setup.data.dir", "testval4"),
               schroot::chroot::facet::userdata::error);
}

TEST_F(Userdata, UserSet)
{
  schroot::string_map d;
  d.insert(std::make_pair("sbuild.resolver", "aptitude"));
  userdata->set_user_data(d);

  std::string t1;
  ASSERT_TRUE(userdata->get_data("sbuild.resolver", t1));
  ASSERT_EQ(t1, "aptitude");
}

TEST_F(Userdata, UserSetFail1)
{
  schroot::string_map d;
  d.insert(std::make_pair("sbuild.apt-update", "true"));
  EXPECT_THROW(userdata->set_user_data(d),
               schroot::chroot::facet::userdata::error);
}

TEST_F(Userdata, UserSetFail2)
{
  // Use root key.
  schroot::string_map d;
  d.insert(std::make_pair("debian.apt-update", "false"));
  EXPECT_THROW(userdata->set_user_data(d),
               schroot::chroot::facet::userdata::error);
}

TEST_F(Userdata, RootSet)
{
  schroot::string_map d;
  d.insert(std::make_pair("sbuild.resolver", "aptitude"));
  d.insert(std::make_pair("debian.apt-update", "false"));
  userdata->set_root_data(d);

  std::string t1;
  ASSERT_TRUE(userdata->get_data("sbuild.resolver", t1));
  ASSERT_EQ(t1, "aptitude");

  std::string t2;
  ASSERT_TRUE(userdata->get_data("debian.apt-update", t2));
  ASSERT_EQ(t2, "false");
}

TEST_F(Userdata, RootSetFail)
{
  schroot::string_map d;
  d.insert(std::make_pair("invalid.key", "testv"));
  EXPECT_THROW(userdata->set_root_data(d),
               schroot::chroot::facet::userdata::error);
}
