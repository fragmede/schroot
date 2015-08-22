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

#include <schroot/nostream.h>
#include <schroot/run-parts.h>
#include <schroot/util.h>

#include <iostream>
#include <sstream>

#include <boost/filesystem/operations.hpp>

#include <config.h>

extern char **environ;

class RunParts : public ::testing::Test
{
public:
  std::streambuf            *saved;
  schroot::basic_nbuf<char> *monitor;

  void SetUp()
  {
    monitor = new schroot::basic_nbuf<char>();
    saved = std::cerr.std::ios::rdbuf(monitor);
  }

  void TearDown()
  {
    std::cerr.std::ios::rdbuf(saved);
    delete monitor;
  }
};

TEST_F(RunParts, Construct)
{
  ASSERT_NO_THROW(schroot::run_parts rp(TESTDATADIR "/run-parts.ex1"));
}

TEST_F(RunParts, ConstructFail)
{
  ASSERT_THROW(schroot::run_parts rp(TESTDATADIR "/invalid_dir"),
               boost::filesystem::filesystem_error);
}

TEST_F(RunParts, Run1)
{
  schroot::run_parts rp(TESTDATADIR "/run-parts.ex1");

  int status;

  schroot::string_list command;
  schroot::environment env(environ);

  command.push_back("ok");
  status = rp.run(command, env);
  ASSERT_EQ(status, EXIT_SUCCESS);

  command.clear();
  command.push_back("fail");
  status = rp.run(command, env);
  ASSERT_EQ(status, EXIT_FAILURE);

  command.clear();
  command.push_back("fail2");
  status = rp.run(command, env);
  ASSERT_EQ(status, EXIT_FAILURE);
}

TEST_F(RunParts, Run2)
{
  schroot::run_parts rp(TESTDATADIR "/run-parts.ex2");

  int status;

  schroot::string_list command;
  schroot::environment env(environ);

  command.push_back("ok");
  status = rp.run(command, env);
  ASSERT_EQ(status, EXIT_SUCCESS);
}

TEST_F(RunParts, Run3)
{
  schroot::run_parts rp(TESTDATADIR "/run-parts.ex3");

  int status;

  schroot::string_list command;
  schroot::environment env(environ);

  command.push_back("ok");
  status = rp.run(command, env);
  ASSERT_EQ(status, EXIT_FAILURE);
}
