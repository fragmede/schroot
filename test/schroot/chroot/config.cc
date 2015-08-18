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

#include <schroot/chroot/config.h>
#include <schroot/nostream.h>

#include <fstream>
#include <sstream>
#include <vector>

#include <config.h>

class ChrootConfig : public ::testing::Test
{
public:
  schroot::chroot::config *cf;

  ChrootConfig():
    cf(nullptr)
  {}

  void SetUp()
  {
    cf = new schroot::chroot::config("chroot", TESTDATADIR "/config.ex1");
    ASSERT_NE(cf, nullptr);
  }

  void TearDown()
  {
    if (cf)
      delete cf;
    cf = nullptr;
  }
};

TEST_F(ChrootConfig, ConstructFile)
{
  ASSERT_NO_THROW(schroot::chroot::config c("chroot", TESTDATADIR "/config.ex1"));
}

TEST_F(ChrootConfig, ConstructDirectory)
{
  ASSERT_NO_THROW(schroot::chroot::config c("chroot", TESTDATADIR "/config.ex2"));
}

TEST_F(ChrootConfig, ConstructFailNonexistent)
{
  ASSERT_THROW(schroot::chroot::config c("chroot", TESTDATADIR "/config.nonexistent"), schroot::error_base);
}

TEST_F(ChrootConfig, ConstructFailInvalid)
{
  ASSERT_THROW(schroot::chroot::config c("chroot", TESTDATADIR "/config.ex3"), schroot::error_base);
}

TEST_F(ChrootConfig, AddFile)
{
  schroot::chroot::config c;
  c.add("chroot", TESTDATADIR "/config.ex1");
}

TEST_F(ChrootConfig, AddDirectory)
{
  schroot::chroot::config c;
  c.add("chroot", TESTDATADIR "/config.ex2");
}

TEST_F(ChrootConfig, AddFail)
{
  schroot::chroot::config c;
  ASSERT_THROW(c.add("chroot", TESTDATADIR "/config.nonexistent"), schroot::error_base);
}

TEST_F(ChrootConfig, GetChroots)
{
  EXPECT_EQ(cf->get_chroots("chroot").size(), 4);
}

TEST_F(ChrootConfig, FindChroot)
{
  schroot::chroot::chroot::ptr chroot;

  EXPECT_NO_THROW(chroot = cf->find_chroot("chroot", "sid"));
  EXPECT_NE(chroot, nullptr);
  EXPECT_EQ(chroot->get_name(), "sid");

  EXPECT_NO_THROW(chroot = cf->find_chroot("chroot", "stable"));
  EXPECT_EQ(chroot, nullptr);

  EXPECT_NO_THROW(chroot = cf->find_chroot("chroot", "invalid"));
  EXPECT_EQ(chroot, nullptr);
}

TEST_F(ChrootConfig, FindAlias)
{
  schroot::chroot::chroot::ptr chroot;

  chroot = cf->find_alias("chroot", "sid");
  EXPECT_NE(chroot, nullptr);
  EXPECT_EQ(chroot->get_name(), "sid");

  chroot = cf->find_alias("chroot", "stable");
  EXPECT_NE(chroot, nullptr);
  EXPECT_EQ(chroot->get_name(), "sarge");

  chroot = cf->find_alias("chroot", "invalid");
  EXPECT_EQ(chroot, nullptr);
}

TEST_F(ChrootConfig, GetChrootList)
{
  schroot::string_list chroots = cf->get_chroot_list("chroot");
  EXPECT_EQ(chroots.size(), 4);
  EXPECT_EQ(chroots[0], "chroot:experimental");
  EXPECT_EQ(chroots[1], "chroot:sarge");
  EXPECT_EQ(chroots[2], "chroot:sid");
  EXPECT_EQ(chroots[3], "chroot:sid-local");
}

TEST_F(ChrootConfig, GetAliasList)
{
  schroot::string_list chroots = cf->get_alias_list("chroot");
  EXPECT_EQ(chroots.size(), 7);
  EXPECT_EQ(chroots[0], "chroot:default");
  EXPECT_EQ(chroots[1], "chroot:experimental");
  EXPECT_EQ(chroots[2], "chroot:sarge");
  EXPECT_EQ(chroots[3], "chroot:sid");
  EXPECT_EQ(chroots[4], "chroot:sid-local");
  EXPECT_EQ(chroots[5], "chroot:stable");
  EXPECT_EQ(chroots[6], "chroot:unstable");
}

TEST_F(ChrootConfig, ValidateChroots)
{
  schroot::string_list chroots;
  chroots.push_back("default");
  chroots.push_back("sarge");
  chroots.push_back("unstable");

  schroot::chroot::config::chroot_map m = cf->validate_chroots("chroot", chroots);
  EXPECT_EQ(m.size(), 3);
}

TEST_F(ChrootConfig, ValidateChrootsFail)
{
  schroot::string_list chroots;
  chroots.push_back("default");
  chroots.push_back("invalid");
  chroots.push_back("invalid2");
  chroots.push_back("sarge");
  chroots.push_back("unstable");

  EXPECT_THROW(cf->validate_chroots("chroot", chroots), schroot::error_base);
}

TEST_F(ChrootConfig, ConfigFail)
{
  EXPECT_THROW(schroot::chroot::config c("chroot", TESTDATADIR "/config-directory-fail.ex"),
               schroot::error_base);
}

TEST_F(ChrootConfig, ConfigDeprecated)
{
  EXPECT_NO_THROW(schroot::chroot::config c("chroot", TESTDATADIR "/config-directory-deprecated.ex"));
}

TEST_F(ChrootConfig, ConfigValid)
{
  EXPECT_NO_THROW(schroot::chroot::config c("chroot", TESTDATADIR "/config-directory-valid.ex"));
}
