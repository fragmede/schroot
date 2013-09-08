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

#include <sbuild/log.h>

#include <iostream>
#include <ios>
#include <sstream>

class Log : public ::testing::Test
{
public:
  std::streambuf *saved;
  std::stringbuf *monitor;

  void SetUp()
  {
    monitor = new std::stringbuf();
    saved = std::cerr.std::ios::rdbuf(monitor);
  }

  void TearDown()
  {
    std::cerr.std::ios::rdbuf(saved);
    delete monitor;
  }

  std::string
  debug(sbuild::debug_level level,
        const std::string& msg)
  {
    monitor->str("");
    sbuild::log_debug(level) << msg;
    return monitor->str();
  }
};

TEST_F(Log, Info)
{
  sbuild::log_info() << "Discard me please";
  ASSERT_EQ(monitor->str(), "I: Discard me please");
}

TEST_F(Log, Warning)
{
  sbuild::log_warning() << "Discard me please";
  ASSERT_EQ(monitor->str(), "W: Discard me please");
}

TEST_F(Log, Error)
{
  sbuild::log_error() << "Discard me please";
  ASSERT_EQ(monitor->str(), "E: Discard me please");
}

TEST_F(Log, DebugNone)
{
  sbuild::debug_log_level = sbuild::DEBUG_NONE;
  ASSERT_EQ(debug(sbuild::DEBUG_NONE,     "Discard me"), "");
  ASSERT_EQ(debug(sbuild::DEBUG_NOTICE,   "Discard me"), "");
  ASSERT_EQ(debug(sbuild::DEBUG_INFO,     "Discard me"), "");
  ASSERT_EQ(debug(sbuild::DEBUG_WARNING,  "Discard me"), "");
  ASSERT_EQ(debug(sbuild::DEBUG_CRITICAL, "Discard me"), "");
}

TEST_F(Log, DebugNotice)
{
  sbuild::debug_log_level = sbuild::DEBUG_NOTICE;
  ASSERT_EQ(debug(sbuild::DEBUG_NONE,
                       "Discard me"), "");
  ASSERT_EQ(debug(sbuild::DEBUG_NOTICE,
                       "Discard me"), "D(1): Discard me");
  ASSERT_EQ(debug(sbuild::DEBUG_INFO,
                       "Discard me"), "D(2): Discard me");
  ASSERT_EQ(debug(sbuild::DEBUG_WARNING,
                       "Discard me"), "D(3): Discard me");
  ASSERT_EQ(debug(sbuild::DEBUG_CRITICAL,
                       "Discard me"), "D(4): Discard me");
}

TEST_F(Log, DebugInfo)
{
  sbuild::debug_log_level = sbuild::DEBUG_INFO;
  ASSERT_EQ(debug(sbuild::DEBUG_NONE,
                       "Discard me"), "");
  ASSERT_EQ(debug(sbuild::DEBUG_NOTICE,
                       "Discard me"), "");
  ASSERT_EQ(debug(sbuild::DEBUG_INFO,
                       "Discard me"), "D(2): Discard me");
  ASSERT_EQ(debug(sbuild::DEBUG_WARNING,
                       "Discard me"), "D(3): Discard me");
  ASSERT_EQ(debug(sbuild::DEBUG_CRITICAL,
                       "Discard me"), "D(4): Discard me");
  }

TEST_F(Log, DebugWarning)
{
  sbuild::debug_log_level = sbuild::DEBUG_WARNING;
  ASSERT_EQ(debug(sbuild::DEBUG_NONE,
                       "Discard me"), "");
  ASSERT_EQ(debug(sbuild::DEBUG_NOTICE,
                       "Discard me"), "");
  ASSERT_EQ(debug(sbuild::DEBUG_INFO,
                       "Discard me"), "");
  ASSERT_EQ(debug(sbuild::DEBUG_WARNING,
                       "Discard me"), "D(3): Discard me");
  ASSERT_EQ(debug(sbuild::DEBUG_CRITICAL,
                       "Discard me"), "D(4): Discard me");
  }

TEST_F(Log, DebugCritical)
{
  sbuild::debug_log_level = sbuild::DEBUG_CRITICAL;
  ASSERT_EQ(debug(sbuild::DEBUG_NONE,
                       "Discard me"), "");
  ASSERT_EQ(debug(sbuild::DEBUG_NOTICE,
                       "Discard me"), "");
  ASSERT_EQ(debug(sbuild::DEBUG_INFO,
                       "Discard me"), "");
  ASSERT_EQ(debug(sbuild::DEBUG_WARNING,
                       "Discard me"), "");
  ASSERT_EQ(debug(sbuild::DEBUG_CRITICAL,
                       "Discard me"), "D(4): Discard me");
}
