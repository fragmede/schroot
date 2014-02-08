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

#include <schroot/environment.h>
#include <schroot/util.h>

#include <iostream>
#include <sstream>


class Environment : public ::testing::Test
{
public:
  schroot::environment *env;
  schroot::environment *half_env;

  void SetUp()
  {
    env = new schroot::environment;
    env->add(std::make_pair("TERM", "wy50"));
    env->add(std::make_pair("SHELL", "/bin/sh"));
    env->add(std::make_pair("USER", "root"));
    env->add(std::make_pair("COLUMNS", "80"));

    half_env = new schroot::environment;
    half_env->add(std::make_pair("TERM", "wy50"));
    half_env->add(std::make_pair("USER", "root"));
  }

  void TearDown()
  {
    delete env;
    delete half_env;
  }
};


TEST_F(Environment, Construction)
{
  const char *items[] = {"TERM=wy50", "SHELL=/bin/sh",
                           "USER=root", "COLUMNS=80", 0};
  schroot::environment e(const_cast<char **>(&items[0]));

  ASSERT_EQ(e.size(), 4);
  ASSERT_EQ(e, *env);
}

TEST_F(Environment, AddStrv)
{
  const char *items[] = {"TERM=wy50", "SHELL=/bin/sh",
                         "USER=root", "COLUMNS=80", 0};
  schroot::environment e;
  e.add(const_cast<char **>(&items[0]));

  ASSERT_EQ(e.size(),4);
  ASSERT_EQ(e, *env);
}

TEST_F(Environment, AddEnvironment)
{
  schroot::environment e;
  e.add(*env);

  ASSERT_EQ(e, *env);
}

TEST_F(Environment, AddValue)
{
  schroot::environment e;
  e.add(schroot::environment::value_type("TERM", "wy50"));
  e.add(schroot::environment::value_type("SHELL", "/bin/sh"));
  e.add(schroot::environment::value_type("USER", "root"));
  e.add(schroot::environment::value_type("COLUMNS", "80"));

  ASSERT_EQ(e, *env);
}

TEST_F(Environment, AddStringPair)
{
  schroot::environment e;
  e.add("TERM", "wy50");
  e.add("SHELL", "/bin/sh");
  e.add("USER", "root");
  e.add("COLUMNS", "80");

  ASSERT_EQ(e, *env);
}

TEST_F(Environment, AddTemplate)
{
  schroot::environment e;
  e.add("TERM", "wy50");
  e.add("SHELL", "/bin/sh");
  e.add("USER", std::string("root"));
  e.add("COLUMNS", 80);

  ASSERT_EQ(e, *env);
}

TEST_F(Environment, AddString)
{
  schroot::environment e;
  e.add("TERM=wy50");
  e.add("SHELL=/bin/sh");
  e.add("USER=root");
  e.add("COLUMNS=80");

  ASSERT_EQ(e, *env);
}

TEST_F(Environment, AddEmptyWithImplicitRemove)
{
  schroot::environment e;
  e.add("TERM=wy50");
  e.add("USER=root");

  env->add("COLUMNS=");
  env->add(schroot::environment::value_type("SHELL", ""));

  ASSERT_EQ(env->size(), 2);
  ASSERT_EQ(e, *env);
}

TEST_F(Environment, RemoveStrv)
{
  const char *items[] = {"SHELL=/bin/bash",
                         "COLUMNS=160", 0};
  env->remove(const_cast<char **>(&items[0]));

  ASSERT_EQ(env->size(), 2);
  ASSERT_EQ(*env, *half_env);
}

TEST_F(Environment, RemoveEnvironment)
{
  schroot::environment e;
  e.add("SHELL=/bin/bash");
  e.add("COLUMNS=160");

  env->remove(e);

  ASSERT_EQ(*env, *half_env);
}

TEST_F(Environment, RemoveValue)
{
  env->remove(schroot::environment::value_type("SHELL", "/bin/bash"));
  env->remove(schroot::environment::value_type("COLUMNS", "160"));

  ASSERT_EQ(*env, *half_env);
}

TEST_F(Environment, RemoveString)
{
  env->remove("SHELL=/bin/bash");
  env->remove("COLUMNS=160");

  ASSERT_EQ(*env, *half_env);
}

TEST_F(Environment, GetValue)
{
  std::string value;
  ASSERT_TRUE(env->get("TERM", value));
  ASSERT_EQ(value,"wy50");
  ASSERT_TRUE(env->get("SHELL", value));
  ASSERT_EQ(value, "/bin/sh");
  ASSERT_TRUE(env->get("USER", value));
  ASSERT_EQ(value,"root");
  ASSERT_TRUE(env->get("COLUMNS", value));
  ASSERT_EQ(value, "80");
  // Check failure doesn't overwrite value.
  ASSERT_FALSE(env->get("MUSTFAIL", value));
  ASSERT_EQ(value, "80");

  // Check getting templated types.
  int tval;
  ASSERT_TRUE(env->get("COLUMNS", tval));
  ASSERT_EQ(tval, 80);
}

TEST_F(Environment, GetStrv)
{
  char **strv = env->get_strv();

  int size = 0;
  for (char **ev = strv; ev != 0 && *ev != 0; ++ev, ++size);

  ASSERT_EQ(size, 4);
  ASSERT_EQ(std::string(strv[0]), "COLUMNS=80");
  ASSERT_EQ(std::string(strv[1]), "SHELL=/bin/sh");
  ASSERT_EQ(std::string(strv[2]), "TERM=wy50");
  ASSERT_EQ(std::string(strv[3]), "USER=root");

  schroot::strv_delete(strv);
}

TEST_F(Environment, OperatorPlus)
{
  schroot::environment e;
  e.add("SHELL=/bin/sh");
  e.add("COLUMNS=80");

  schroot::environment result;
  result = *half_env + e;
  ASSERT_EQ(result, *env);

  schroot::environment e2;
  e2 = *half_env + "SHELL=/bin/sh";
  e2 = e2 + schroot::environment::value_type("COLUMNS", "80");
  ASSERT_EQ(e2, *env);
}

TEST_F(Environment, OperatorPlusEquals)
{
  schroot::environment e;
  e.add("SHELL=/bin/sh");
  e.add("COLUMNS=80");

  schroot::environment result(*half_env);
  result += e;
  ASSERT_EQ(result, *env);

  schroot::environment e2(*half_env);
  e2 += "SHELL=/bin/sh";
  // TODO: Why does calling direct fail?
  schroot::environment::value_type val("COLUMNS", "80");
  e2 += val;
  ASSERT_EQ(e2, *env);
}

TEST_F(Environment, OperatorMinus)
{
  schroot::environment e;
  e.add("SHELL=/bin/sh");
  e.add("COLUMNS=80");

  schroot::environment result;
  result = *env - e;
  ASSERT_EQ(result, *half_env);

  schroot::environment e2;
  e2 = *env - "SHELL=/bin/sh";
  e2 = e2 - schroot::environment::value_type("COLUMNS", "80");
  ASSERT_EQ(e2, *half_env);
}

TEST_F(Environment, OperatorMinusEquals)
{
  schroot::environment e;
  e.add("SHELL=/bin/sh");
  e.add("COLUMNS=80");

  schroot::environment result(*env);
  result -= e;
  ASSERT_EQ(result, *half_env);

  schroot::environment e2(*env);
  e2 -= "SHELL=/bin/sh";
  // TODO: Why does calling direct fail?
  schroot::environment::value_type val("COLUMNS", "80");
  e2 -= val;
  ASSERT_EQ(e2, *half_env);
}

TEST_F(Environment, AddFilter)
{
  schroot::regex f("^FOO|BAR$");

  schroot::environment e;
  e.set_filter(f);

  ASSERT_EQ(f.compare(e.get_filter()), 0);
}

TEST_F(Environment, Filter)
{
  schroot::regex f("^FOO|BAR$");

  schroot::environment e;
  e.set_filter(f);

  e.add("FOO=bar");
  e.add("BAR=baz");
  e.add("BAZ=bat");
  e.add("BAT=bah");

  std::string value;
  ASSERT_FALSE(e.get("FOO", value));
  ASSERT_FALSE(e.get("BAR", value));
  ASSERT_TRUE(e.get("BAZ", value));
  ASSERT_EQ(value, "bat");
  ASSERT_TRUE(e.get("BAT", value));
  ASSERT_EQ(value, "bah");
}

TEST_F(Environment, StreamOutput)
{
  std::ostringstream os;
  os << *env;

  ASSERT_EQ(os.str(),
            "COLUMNS=80\n"
            "SHELL=/bin/sh\n"
            "TERM=wy50\n"
            "USER=root\n");
}
