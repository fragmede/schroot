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

#include <gtest/gtest.h>

#include <schroot/keyfile.h>
#include <schroot/keyfile-reader.h>
#include <schroot/keyfile-writer.h>

#include <fstream>
#include <sstream>
#include <vector>

#include <config.h>

class Keyfile : public ::testing::Test
{
protected:
  schroot::keyfile *kf;

public:
  void SetUp()
  {
    std::istringstream is("# Comment\n"
                          "[group1]\n"
                          "name=Fred Walker\n"
                          "age=32\n"
                          "# Test item comment\n"
                          "#\n"
                          "# spanning multiple lines\n"
                          "numbers=1,2,3,4,5,6\n"
                          "\n"
                          "[group2]\n"
                          "name=Mary King\n"
                          "age=43\n"
                          "photo=mary.jpeg\n");
    kf = new schroot::keyfile;
    schroot::keyfile_reader kp(*kf);
    is >> kp;
  }

  void TearDown()
  {
    delete kf;
  }
};

TEST_F(Keyfile, ConstructFile)
{
  schroot::keyfile k;
  ASSERT_NO_THROW(schroot::keyfile_reader(k, TESTDATADIR "/keyfile.ex1"));
}

TEST_F(Keyfile, ConstructStream)
{
  std::ifstream strm(TESTDATADIR "/keyfile.ex1");
  ASSERT_TRUE(!!strm);
  schroot::keyfile k;
  ASSERT_NO_THROW(schroot::keyfile_reader(k, strm));
}

TEST_F(Keyfile, ConstructFail)
{
  schroot::keyfile k;
  ASSERT_THROW(schroot::keyfile_reader(k, TESTDATADIR "/nonexistent-keyfile-will-throw-exception"),
               schroot::keyfile::error);
}

TEST_F(Keyfile, GetGroups)
{
  schroot::string_list l = kf->get_groups();
  ASSERT_EQ(l.size(),2);
  ASSERT_EQ(l[0], "group1");
  ASSERT_EQ(l[1], "group2");

  ASSERT_TRUE(kf->has_group("group1"));
  ASSERT_FALSE(kf->has_group("nonexistent"));
}

TEST_F(Keyfile, GetKeys)
{
  schroot::string_list l = kf->get_keys("group2");
  ASSERT_EQ(l.size(), 3);
  ASSERT_EQ(l[0], "age");
  ASSERT_EQ(l[1], "name");
  ASSERT_EQ(l[2], "photo");

  ASSERT_TRUE(kf->has_key("group2", "name"));
  ASSERT_FALSE(kf->has_key("nonexistent", "name"));
  ASSERT_FALSE(kf->has_key("group1", "nonexistent"));
}

TEST_F(Keyfile, GetValue)
{
  std::string sval;
  int ival;

  ASSERT_TRUE(kf->get_value("group2", "name", sval));
  ASSERT_EQ(sval, "Mary King");
  ASSERT_TRUE(kf->get_value("group2", "age", ival));
  ASSERT_EQ(ival, 43);

  // Check failure does not alter value.
  ival = 11;
  ASSERT_FALSE(kf->get_value("group2", "nonexistent", ival));
  ASSERT_EQ(ival, 11);
}

TEST_F(Keyfile, GetValueFail)
{
  bool bval = false;

  // Expect a warning here.
  ASSERT_FALSE(kf->get_value("group2", "age", bval));
  ASSERT_FALSE(bval);
}

TEST_F(Keyfile, GetListValue)
{
  std::vector<int> expected;
  expected.push_back(1);
  expected.push_back(2);
  expected.push_back(3);
  expected.push_back(4);
  expected.push_back(5);
  expected.push_back(6);

  std::vector<int> found;
  ASSERT_TRUE(kf->get_list_value("group1", "numbers", found));
  ASSERT_EQ(found, expected);
}

TEST_F(Keyfile, GetListValueFail)
{
  std::vector<int> expected;
  expected.push_back(1);
  expected.push_back(2);
  expected.push_back(3);
  expected.push_back(4);
  expected.push_back(5);
  expected.push_back(6);

  std::vector<bool> found;

  // Expect a warning here.
  ASSERT_FALSE(kf->get_list_value("group1", "numbers", found));
  ASSERT_EQ(found.size(), 1); // 1 converts to bool.
}

// TODO: Test priority.
// TODO: Test comments, when available.

TEST_F(Keyfile, GetLine)
{
  ASSERT_EQ(kf->get_line("group2"), 10);
  ASSERT_EQ(kf->get_line("group1", "age"), 4);
  ASSERT_EQ(kf->get_line("group2", "name"), 11);
}

TEST_F(Keyfile, SetValue)
{
  kf->set_value("group1", "name", "Adam Smith");
  kf->set_value("group1", "age", 27);
  kf->set_value("group1", "interests", "Ice Hockey,GNU/Linux");
  kf->set_value("newgroup", "newitem", 89);

  std::string result;
  int number = 0;
  ASSERT_TRUE(kf->get_value("group1", "name", result));
  ASSERT_EQ(result, "Adam Smith");
  ASSERT_TRUE(kf->get_value("group1", "age", number));
  ASSERT_EQ(number, 27);
  ASSERT_TRUE(kf->get_value("group1", "interests", result));
  ASSERT_EQ(result, "Ice Hockey,GNU/Linux");
  ASSERT_TRUE(kf->get_value("newgroup", "newitem", number));
  ASSERT_EQ(number, 89);
}

TEST_F(Keyfile, SetListValue)
{
  std::vector<int> expected;
  expected.push_back(1);
  expected.push_back(2);
  expected.push_back(3);
  expected.push_back(4);
  expected.push_back(5);
  expected.push_back(6);

  std::vector<int> found;

  kf->set_list_value("listgroup", "numbers2",
                           expected.begin(), expected.end());
  ASSERT_TRUE(kf->get_list_value("listgroup", "numbers2", found));
  ASSERT_EQ(found, expected);
}

TEST_F(Keyfile, RemoveGroup)
{
  ASSERT_EQ(kf->get_groups().size(), 2);

  kf->set_value("newgroup", "newitem", 89);

  ASSERT_EQ(kf->get_groups().size(), 3);

  kf->remove_group("group1");

  schroot::string_list l = kf->get_groups();
  ASSERT_EQ(l.size(), 2);
  ASSERT_EQ(l[0], "group2");
  ASSERT_EQ(l[1], "newgroup");
}

TEST_F(Keyfile, RemoveKey)
{
  ASSERT_EQ(kf->get_keys("group2").size(), 3);

  kf->remove_key("group2", "photo");

  schroot::string_list l = kf->get_keys("group2");
  ASSERT_EQ(l.size(), 2);
  ASSERT_EQ(l[0], "age");
  ASSERT_EQ(l[1], "name");
}

TEST_F(Keyfile, StreamOutput)
{
  std::ostringstream os;
  os << schroot::keyfile_writer(*kf);

  ASSERT_EQ(os.str(),
            "# Comment\n"
            "[group1]\n"
            "age=32\n"
            "name=Fred Walker\n"
            "# Test item comment\n"
            "#\n"
            "# spanning multiple lines\n"
            "numbers=1,2,3,4,5,6\n"
            "\n"
            "[group2]\n"
            "age=43\n"
            "name=Mary King\n"
            "photo=mary.jpeg\n");
}
