/* Copyright © 2006-2007  Roger Leigh <rleigh@debian.org>
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

#include <schroot-base/schroot-base-option-action.h>

#include <boost/program_options.hpp>

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

using schroot_base::option_action;
namespace opt = boost::program_options;
using namespace CppUnit;

class test_option_action : public TestFixture
{
  CPPUNIT_TEST_SUITE(test_option_action);
  CPPUNIT_TEST(test_construction);
  CPPUNIT_TEST(test_default);
  CPPUNIT_TEST_EXCEPTION(test_default_fail, std::logic_error);
  CPPUNIT_TEST(test_current);
  CPPUNIT_TEST_EXCEPTION(test_current_fail, std::logic_error);
  CPPUNIT_TEST_EXCEPTION(test_current_fail_multipleset,
			 opt::validation_error);
  CPPUNIT_TEST(test_operators);
  CPPUNIT_TEST_EXCEPTION(test_operators_fail_multipleset,
			 opt::validation_error);
  CPPUNIT_TEST_SUITE_END();

protected:
  option_action *action;

public:
  test_option_action():
    TestFixture(),
    action()
  {}

  virtual ~test_option_action()
  {}

  void setUp()
  {
    this->action = new option_action;
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

  void tearDown()
  {
    delete this->action;
  }

  void
  test_construction()
  {
    option_action act;
  }

  void
  test_default()
  {
    add_examples(*this->action);
    CPPUNIT_ASSERT(this->action->get_default() == "");
    CPPUNIT_ASSERT(this->action->get() == "");

    this->action->set_default("info");
    CPPUNIT_ASSERT(this->action->get_default() == "info");
    CPPUNIT_ASSERT(this->action->get() == "info");
  }

  void
  test_default_fail()
  {
    add_examples(*this->action);

    this->action->set_default("invalid");
  }

  void
  test_current()
  {
    add_examples(*this->action);

    CPPUNIT_ASSERT(this->action->get_default() == "");
    CPPUNIT_ASSERT(this->action->get() == "");
    this->action->set_default("list");

    CPPUNIT_ASSERT(this->action->get_default() == "list");
    CPPUNIT_ASSERT(this->action->get() == "list");

    this->action->set("config");
    CPPUNIT_ASSERT(this->action->get_default() == "list");
    CPPUNIT_ASSERT(this->action->get() == "config");
  }

  void
  test_current_fail()
  {
    add_examples(*this->action);

    this->action->set("invalid");
  }

  void
  test_current_fail_multipleset()
  {
    add_examples(*this->action);

    this->action->set("list");
    this->action->set("info");
  }

  void
  test_operators()
  {
    add_examples(*this->action);

    *this->action = "list";
    CPPUNIT_ASSERT(*this->action == "list");
    CPPUNIT_ASSERT(!(*this->action != "list"));
    CPPUNIT_ASSERT(*this->action != "invalid");
    CPPUNIT_ASSERT(!(*this->action == "invalid"));
  }

  void
  test_operators_fail_multipleset()
  {
    add_examples(*this->action);

    *this->action = "list";
    *this->action = "config";
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(test_option_action);
