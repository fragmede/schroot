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

#include <bin-common/option-action.h>
#include <bin-common/options.h>

#include <iostream>

using bin::common::option_action;


class OptionAction : public ::testing::Test
{
public:
  option_action *action;

  void SetUp()
  {
    action = new option_action;
  }

  void TearDown()
  {
    delete action;
  }

  static void
  add_examples(option_action& act)
  {
    act.add("help");
    act.add("version");
    act.add("list");
    act.add("info");
    act.add("config");
  }
};

TEST_F(OptionAction, Construct)
{
  ASSERT_NO_THROW(option_action act);
}

TEST_F(OptionAction, Default)
{
  add_examples(*action);
  EXPECT_EQ(action->get_default(), "");
  EXPECT_EQ(action->get(), "");

  action->set_default("info");
  EXPECT_EQ(action->get_default(), "info");
  EXPECT_EQ(action->get(), "info");
}

TEST_F(OptionAction, DefaultFail)
{
  add_examples(*action);

  ASSERT_THROW(action->set_default("invalid"), bin::common::options::error);
}

TEST_F(OptionAction, Current)
{
  add_examples(*action);

  EXPECT_EQ(action->get_default(), "");
  EXPECT_EQ(action->get(), "");
  action->set_default("list");

  EXPECT_EQ(action->get_default(), "list");
  EXPECT_EQ(action->get(), "list");

  action->set("config");
  EXPECT_EQ(action->get_default(), "list");
  EXPECT_EQ(action->get(), "config");
}

TEST_F(OptionAction, CurrentFail)
{
  add_examples(*action);

  ASSERT_THROW(action->set("invalid"), bin::common::options::error);
}

TEST_F(OptionAction, CurrentFailMultipleSet)
{
  add_examples(*action);

  ASSERT_NO_THROW(action->set("list"));
  ASSERT_THROW(action->set("info"), bin::common::options::error);
}

TEST_F(OptionAction, Operators)
{
  add_examples(*action);

  *action = "list";
  EXPECT_TRUE(*action == "list");
  EXPECT_FALSE(*action !="list");
  EXPECT_TRUE(*action != "invalid");
  EXPECT_FALSE(*action == "invalid");
}

TEST_F(OptionAction, OperatorsFailMultipleSet)
{
  add_examples(*action);

  ASSERT_NO_THROW(*action = "list");
  ASSERT_THROW(*action = "config", bin::common::options::error);
}
