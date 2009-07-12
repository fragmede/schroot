/* Copyright © 2006-2008  Roger Leigh <rleigh@debian.org>
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

#ifndef TEST_SBUILD_CHROOT_H
#define TEST_SBUILD_CHROOT_H

#include <sbuild/sbuild-config.h>
#include <sbuild/sbuild-chroot.h>
#include <sbuild/sbuild-chroot-source.h>

#include <algorithm>
#include <iostream>
#include <set>

#include <cppunit/extensions/HelperMacros.h>

using namespace CppUnit;

template <class T>
class test_chroot_base : public TestFixture
{
protected:
  sbuild::chroot::ptr chroot;
  std::string abs_testdata_dir;

public:
  test_chroot_base():
    TestFixture(),
    chroot(),
    abs_testdata_dir()
  {
    char cwd[FILENAME_MAX];
    if (NULL != getcwd(cwd, FILENAME_MAX))
      {
        abs_testdata_dir = std::string(cwd);
        abs_testdata_dir.append("/" TESTDATADIR);
      }
  }

  virtual ~test_chroot_base()
  {}

  void setUp()
  {
    this->chroot = sbuild::chroot::ptr(new T);
    chroot->set_name("test-name");
    chroot->set_description("test-description");
    chroot->set_aliases(sbuild::split_string("test-alias-1,test-alias-2", ","));
    chroot->set_description("test-description");
    chroot->set_mount_location("/mnt/mount-location");
    chroot->set_environment_filter(SBUILD_DEFAULT_ENVIRONMENT_FILTER);
    chroot->set_users(sbuild::split_string("user1,user2", ","));
    chroot->set_root_users(sbuild::split_string("user3,user4", ","));
    chroot->set_groups(sbuild::split_string("group1,group2", ","));
    chroot->set_root_groups(sbuild::split_string("group3,group4", ","));
    chroot->set_persona(sbuild::personality("undefined"));
    chroot->set_priority(3);
  }

  void tearDown()
  {
    this->chroot = sbuild::chroot::ptr();
  }

  void setup_source()
  {
    std::tr1::shared_ptr<sbuild::chroot_source> c =
      std::tr1::dynamic_pointer_cast<sbuild::chroot_source>(chroot);
    if (c)
      {
	c->set_source_users(sbuild::split_string("suser1,suser2", ","));
	c->set_source_root_users(sbuild::split_string("suser3,suser4", ","));
	c->set_source_groups(sbuild::split_string("sgroup1,sgroup2", ","));
	c->set_source_root_groups(sbuild::split_string("sgroup3,sgroup4", ","));
      }
  }


  void setup_env_chroot (sbuild::environment& env)
  {
    env.add("CHROOT_NAME",           "test-name");
    env.add("CHROOT_DESCRIPTION",    "test-description");
    env.add("CHROOT_SCRIPT_CONFIG",  sbuild::normalname(std::string(PACKAGE_SYSCONF_DIR) + "/script-defaults"));
  }

  void setup_keyfile_chroot (sbuild::keyfile&   keyfile,
			     std::string const& group)
  {
    keyfile.set_value(group, "active", "false");
    keyfile.set_value(group, "description", "test-description");
    keyfile.set_value(group, "priority", "3");
    keyfile.set_value(group, "aliases", "test-alias-1,test-alias-2");
    keyfile.set_value(group, "users", "user1,user2");
    keyfile.set_value(group, "root-users", "user3,user4");
    keyfile.set_value(group, "groups", "group1,group2");
    keyfile.set_value(group, "root-groups", "group3,group4");
    keyfile.set_value(group, "environment-filter",
		      SBUILD_DEFAULT_ENVIRONMENT_FILTER);
    keyfile.set_value(group, "personality", "undefined");
    keyfile.set_value(group, "command-prefix", "");
    keyfile.set_value(group, "script-config", "script-defaults");
  }

  void setup_keyfile_chroot (sbuild::keyfile& keyfile)
  {
    setup_keyfile_chroot(keyfile, chroot->get_name());
  }

  void setup_keyfile_union_unconfigured (sbuild::keyfile&   keyfile,
					 std::string const& group)
  {
#ifdef SBUILD_FEATURE_UNION
    keyfile.set_value(group, "union-type", "none");
#endif // SBUILD_FEATURE_UNION
  }

  void setup_keyfile_union_unconfigured (sbuild::keyfile& keyfile)
  {
    setup_keyfile_union_unconfigured(keyfile, chroot->get_name());
  }

  void setup_keyfile_union_configured (sbuild::keyfile&   keyfile,
				       std::string const& group)
  {
#ifdef SBUILD_FEATURE_UNION
    keyfile.set_value(group, "union-type", "aufs");
    keyfile.set_value(group, "union-mount-options", "union-mount-options");
    keyfile.set_value(group, "union-overlay-directory", "/overlay");
    keyfile.set_value(group, "union-underlay-directory", "/underlay");
#endif // SBUILD_FEATURE_UNION
  }

  void setup_keyfile_union_configured (sbuild::keyfile& keyfile)
  {
    setup_keyfile_union_configured(keyfile, chroot->get_name());
  }

  void setup_keyfile_source (sbuild::keyfile&   keyfile,
			     std::string const& group)
  {
    keyfile.set_value(group, "source-users", "suser1,suser2");
    keyfile.set_value(group, "source-root-users", "suser3,suser4");
    keyfile.set_value(group, "source-groups", "sgroup1,sgroup2");
    keyfile.set_value(group, "source-root-groups", "sgroup3,sgroup4");
  }

  void setup_keyfile_source (sbuild::keyfile& keyfile)
  {
    setup_keyfile_source(keyfile, chroot->get_name());
  }

  void setup_keyfile_source_clone (sbuild::keyfile&   keyfile,
				   std::string const& group)
  {
    keyfile.set_value(group, "users", "suser1,suser2");
    keyfile.set_value(group, "root-users", "suser3,suser4");
    keyfile.set_value(group, "groups", "sgroup1,sgroup2");
    keyfile.set_value(group, "root-groups", "sgroup3,sgroup4");
  }

  void setup_keyfile_source_clone (sbuild::keyfile& keyfile)
  {
    setup_keyfile_source_clone(keyfile, chroot->get_name());
  }

  void test_setup_env(const sbuild::environment& observed_environment,
		      const sbuild::environment& expected_environment)
  {
    CPPUNIT_ASSERT(observed_environment.size() != 0);
    CPPUNIT_ASSERT(expected_environment.size() != 0);

    std::set<std::string> expected;
    for (sbuild::environment::const_iterator pos = expected_environment.begin();
	 pos != expected_environment.end();
	 ++pos)
      expected.insert(pos->first);

    std::set<std::string> found;
    for (sbuild::environment::const_iterator pos = observed_environment.begin();
	 pos != observed_environment.end();
	 ++pos)
      found.insert(pos->first);

    sbuild::string_list missing;
    set_difference(expected.begin(), expected.end(),
		   found.begin(), found.end(),
		   std::back_inserter(missing));
    if (!missing.empty())
      {
	for (sbuild::string_list::const_iterator pos = missing.begin();
	     pos != missing.end();
	     ++pos)
	  std::cout << "Missing environment: " << *pos << std::endl;
      }
    CPPUNIT_ASSERT(missing.empty());

    sbuild::string_list extra;
    set_difference(found.begin(), found.end(),
		   expected.begin(), expected.end(),
		   std::back_inserter(extra));
    if (!extra.empty())
      {
	for (sbuild::string_list::const_iterator pos = extra.begin();
	     pos != extra.end();
	     ++pos)
	  std::cout << "Additional environment: " << *pos << std::endl;
      }
    CPPUNIT_ASSERT(extra.empty());

    for (sbuild::environment::const_iterator pos = expected_environment.begin();
	 pos != expected_environment.end();
	 ++pos)
      {
	std::string checkval;
	CPPUNIT_ASSERT(observed_environment.get(pos->first, checkval) == true);

	if (checkval != pos->second)
	  std::cout << "Environment error (" << pos->first << "): "
		    << checkval << " [observed] != "
		    << pos->second << " [expected]"
		    << std::endl;
	CPPUNIT_ASSERT(checkval == pos->second);
      }
  }

  void test_setup_env(const sbuild::environment& expected_environment)
  {
    sbuild::environment observed_environment;
    chroot->setup_env(observed_environment);

    CPPUNIT_ASSERT(observed_environment.size() != 0);

    test_setup_env(observed_environment, expected_environment);
  }

  void test_setup_keyfile(const sbuild::keyfile& observed_keyfile,
			  const std::string&     observed_group,
			  const sbuild::keyfile& expected_keyfile,
			  const std::string&     expected_group)
  {
    CPPUNIT_ASSERT(observed_keyfile.get_keys(observed_group).size() != 0);
    CPPUNIT_ASSERT(expected_keyfile.get_keys(expected_group).size() != 0);


    sbuild::string_list expected_keys =
      expected_keyfile.get_keys(expected_group);
    std::set<std::string> expected(expected_keys.begin(), expected_keys.end());

    sbuild::string_list observed_keys =
      observed_keyfile.get_keys(observed_group);
    std::set<std::string> observed(observed_keys.begin(), observed_keys.end());

    sbuild::string_list missing;
    set_difference(expected.begin(), expected.end(),
		   observed.begin(), observed.end(),
		   std::back_inserter(missing));
    if (!missing.empty())
      {
	for (sbuild::string_list::const_iterator pos = missing.begin();
	     pos != missing.end();
	     ++pos)
	  std::cout << "Missing keys: " << *pos << std::endl;
      }
    CPPUNIT_ASSERT(missing.empty());

    sbuild::string_list extra;
    set_difference(observed.begin(), observed.end(),
		   expected.begin(), expected.end(),
		   std::back_inserter(extra));
    if (!extra.empty())
      {
	for (sbuild::string_list::const_iterator pos = extra.begin();
	     pos != extra.end();
	     ++pos)
	  std::cout << "Additional keys: " << *pos << std::endl;
      }
    CPPUNIT_ASSERT(extra.empty());

    for (sbuild::string_list::const_iterator pos = expected_keys.begin();
	 pos != expected_keys.end();
	 ++pos)
      {
	std::string expected_val;
	CPPUNIT_ASSERT(expected_keyfile.get_value(expected_group,
						  *pos, expected_val) == true);

	std::string observed_val;
	CPPUNIT_ASSERT(observed_keyfile.get_value(observed_group,
						  *pos, observed_val) == true);

	if (expected_val != observed_val)
	  std::cout << "Keyfile error (" << *pos << "): "
		    << observed_val << " [observed] != "
		    << expected_val << " [expected]"
		    << std::endl;
	CPPUNIT_ASSERT(expected_val == observed_val);
      }
  }

  void test_setup_keyfile(const sbuild::keyfile& expected_keyfile,
			  const std::string&     group)
  {
    sbuild::keyfile keys;
    chroot->get_keyfile(keys);

    CPPUNIT_ASSERT(keys.get_keys(group).size() != 0);

    test_setup_keyfile(keys, chroot->get_name(),
		       expected_keyfile, group);
  }

  // TODO: All chroot types should check text output matches.  If
  // possible, test chroot locking functions, but this is going to be
  // tricky without having root in many cases.

};

#endif /* TEST_SBUILD_CHROOT_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
