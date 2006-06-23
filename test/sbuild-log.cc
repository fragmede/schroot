/* Copyright © 2006  Roger Leigh <rleigh@debian.org>
 *
 * schroot is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * schroot is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA  02111-1307  USA
 *
 *********************************************************************/

#include <sbuild/sbuild-log.h>

#include <iostream>
#include <ios>
#include <sstream>

#include <cppunit/extensions/HelperMacros.h>

using namespace CppUnit;

class test_log : public TestFixture
{
  CPPUNIT_TEST_SUITE(test_log);
  CPPUNIT_TEST(test_info);
  CPPUNIT_TEST(test_warning);
  CPPUNIT_TEST(test_error);
  CPPUNIT_TEST(test_debug_none);
  CPPUNIT_TEST(test_debug_notice);
  CPPUNIT_TEST(test_debug_info);
  CPPUNIT_TEST(test_debug_warning);
  CPPUNIT_TEST(test_debug_critical);
  CPPUNIT_TEST_SUITE_END();

  std::streambuf *saved;
  std::stringbuf *monitor;

public:
  test_log()
  {}

  void setUp()
  {
    this->monitor = new std::stringbuf();
    this->saved = std::cerr.std::ios::rdbuf(this->monitor);
  }

  void tearDown()
  {
    std::cerr.std::ios::rdbuf(this->saved);
    delete this->monitor;
  }

  void test_info()
  {
    sbuild::log_info() << "Discard me please";
    CPPUNIT_ASSERT(this->monitor->str() == "I: Discard me please");
  }

  void test_warning()
  {
    sbuild::log_warning() << "Discard me please";
    CPPUNIT_ASSERT(this->monitor->str() == "W: Discard me please");
  }

  void test_error()
  {
    sbuild::log_error() << "Discard me please";
    CPPUNIT_ASSERT(this->monitor->str() == "E: Discard me please");
  }

  std::string
  debug(sbuild::DebugLevel level,
	     std::string const& msg)
  {
    this->monitor->str("");
    sbuild::log_debug(level) << msg;
    return this->monitor->str();
  }

  void test_debug_none()
  {
    sbuild::debug_level = sbuild::DEBUG_NONE;
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_NONE,     "Discard me") == "");
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_NOTICE,   "Discard me") == "");
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_INFO,     "Discard me") == "");
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_WARNING,  "Discard me") == "");
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_CRITICAL, "Discard me") == "");
  }

  void test_debug_notice()
  {
    sbuild::debug_level = sbuild::DEBUG_NOTICE;
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_NONE,
			 "Discard me") == "");
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_NOTICE,
			 "Discard me") == "D(1): Discard me");
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_INFO,
			 "Discard me") == "D(2): Discard me");
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_WARNING,
			 "Discard me") == "D(3): Discard me");
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_CRITICAL,
			 "Discard me") == "D(4): Discard me");
  }

  void test_debug_info()
  {
    sbuild::debug_level = sbuild::DEBUG_INFO;
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_NONE,
			 "Discard me") == "");
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_NOTICE,
			 "Discard me") == "");
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_INFO,
			 "Discard me") == "D(2): Discard me");
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_WARNING,
			 "Discard me") == "D(3): Discard me");
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_CRITICAL,
			 "Discard me") == "D(4): Discard me");
  }

  void test_debug_warning()
  {
    sbuild::debug_level = sbuild::DEBUG_WARNING;
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_NONE,
			 "Discard me") == "");
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_NOTICE,
			 "Discard me") == "");
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_INFO,
			 "Discard me") == "");
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_WARNING,
			 "Discard me") == "D(3): Discard me");
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_CRITICAL,
			 "Discard me") == "D(4): Discard me");
  }

  void test_debug_critical()
  {
    sbuild::debug_level = sbuild::DEBUG_CRITICAL;
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_NONE,
			 "Discard me") == "");
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_NOTICE,
			 "Discard me") == "");
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_INFO,
			 "Discard me") == "");
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_WARNING,
			 "Discard me") == "");
    CPPUNIT_ASSERT(debug(sbuild::DEBUG_CRITICAL,
			 "Discard me") == "D(4): Discard me");
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(test_log);

/*
 * Local Variables:
 * mode:C++
 * End:
 */
