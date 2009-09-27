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
#include <sbuild/sbuild-chroot-facet-personality.h>
#include <sbuild/sbuild-chroot-facet-session.h>
#include <sbuild/sbuild-chroot-facet-session-clonable.h>
#include <sbuild/sbuild-chroot-facet-source.h>
#include <sbuild/sbuild-chroot-facet-source-clonable.h>
#include <sbuild/sbuild-chroot-facet-union.h>
#include <sbuild/sbuild-i18n.h>
#include <sbuild/sbuild-util.h>

#include <algorithm>
#include <iostream>
#include <set>

#include <cppunit/extensions/HelperMacros.h>

using namespace CppUnit;
using sbuild::_;

template <class T>
class test_chroot_base : public TestFixture
{
protected:
  sbuild::chroot::ptr chroot;
  sbuild::chroot::ptr session;
  sbuild::chroot::ptr source;
  sbuild::chroot::ptr chroot_union;
  sbuild::chroot::ptr session_union;
  sbuild::chroot::ptr source_union;
  std::string abs_testdata_dir;

public:
  test_chroot_base():
    TestFixture(),
    chroot(),
    abs_testdata_dir()
  {
    abs_testdata_dir = sbuild::getcwd();
    abs_testdata_dir.append("/" TESTDATADIR);
  }

  virtual ~test_chroot_base()
  {}

  void setUp()
  {
    // Create new chroot
    this->chroot = sbuild::chroot::ptr(new T);
    CPPUNIT_ASSERT(this->chroot);
    CPPUNIT_ASSERT(this->chroot->get_active() == false);

    setup_chroot_props(this->chroot);

    CPPUNIT_ASSERT(this->chroot->get_name().length());

    // Create new source chroot.
    sbuild::chroot_facet_session_clonable::const_ptr psess
      (this->chroot->template get_facet<sbuild::chroot_facet_session_clonable>());
    if (psess)
      {
	this->session = this->chroot->clone_session("test-session-name",
						    "user1",
						    false);
	if (this->session)
	  {
	    CPPUNIT_ASSERT(this->session->get_active() == true);
	  }
      }

    sbuild::chroot_facet_source_clonable::const_ptr psrc
      (this->chroot->
       template get_facet<sbuild::chroot_facet_source_clonable>());
    if (psrc)
      this->source = this->chroot->clone_source();
    if (this->source)
      {
	sbuild::chroot_facet_source_clonable::const_ptr pfsrcc
	  (this->source->
	   template get_facet<sbuild::chroot_facet_source_clonable>());
	CPPUNIT_ASSERT(!pfsrcc);
	sbuild::chroot_facet_source::const_ptr pfsrc
	  (this->source->
	   template get_facet<sbuild::chroot_facet_source>());
	CPPUNIT_ASSERT(pfsrc);
      }

    this->chroot_union = sbuild::chroot::ptr(new T);
    sbuild::chroot_facet_union::ptr un =
      this->chroot_union->template get_facet<sbuild::chroot_facet_union>();
    if (!un)
      {
	this->chroot_union.reset();
      }
    else
      {
	un->set_union_type("aufs");

	setup_chroot_props(this->chroot_union);
	CPPUNIT_ASSERT(this->chroot_union->get_active() == false);
	CPPUNIT_ASSERT(this->chroot_union->get_name().length());

	un->set_union_overlay_directory("/overlay");
	un->set_union_underlay_directory("/underlay");
	un->set_union_mount_options("union-mount-options");

	this->session_union =
	  this->chroot_union->clone_session("test-union-session-name",
					    "user1",
					    false);
	this->source_union = chroot_union->clone_source();

	CPPUNIT_ASSERT(this->session_union);
	CPPUNIT_ASSERT(this->session_union->get_active() == true);
	CPPUNIT_ASSERT(this->source_union);
      }
  }

  virtual void setup_chroot_props (sbuild::chroot::ptr& chroot)
  {
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

    sbuild::chroot_facet_personality::ptr pfac
      (chroot->get_facet<sbuild::chroot_facet_personality>());
    if (pfac)
      pfac->set_persona(sbuild::personality("undefined"));
    chroot->set_priority(3);

    sbuild::chroot_facet_source_clonable::ptr usrc
      (chroot->template get_facet<sbuild::chroot_facet_source_clonable>());
    if (usrc)
      {
	usrc->set_source_users(sbuild::split_string("suser1,suser2", ","));
	usrc->set_source_root_users(sbuild::split_string("suser3,suser4", ","));
	usrc->set_source_groups(sbuild::split_string("sgroup1,sgroup2", ","));
	usrc->set_source_root_groups(sbuild::split_string("sgroup3,sgroup4", ","));
      }
  }

  void tearDown()
  {
    this->chroot = sbuild::chroot::ptr();
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

  void setup_keyfile_session (sbuild::keyfile&   keyfile,
			      std::string const& group)
  {
    setup_keyfile_chroot(keyfile, group);
    keyfile.set_value(group, "users", "user1");
    keyfile.set_value(group, "root-users", "");
    keyfile.set_value(group, "groups", "");
    keyfile.set_value(group, "root-groups", "");
  }

  void setup_keyfile_union_unconfigured (sbuild::keyfile&   keyfile,
					 std::string const& group)
  {
#ifdef SBUILD_FEATURE_UNION
    keyfile.set_value(group, "union-type", "none");
#endif // SBUILD_FEATURE_UNION
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

  void setup_keyfile_union_session (sbuild::keyfile&   keyfile,
				    std::string const& group)
  {
#ifdef SBUILD_FEATURE_UNION
    keyfile.set_value(group, "union-type", "aufs");
    keyfile.set_value(group, "union-mount-options", "union-mount-options");
    keyfile.set_value(group, "union-overlay-directory", "/overlay/test-union-session-name");
    keyfile.set_value(group, "union-underlay-directory", "/underlay/test-union-session-name");
#endif // SBUILD_FEATURE_UNION
  }

  void setup_keyfile_session_clone (sbuild::keyfile&   keyfile,
				    std::string const& group)
  {
    keyfile.set_value(group, "description", chroot->get_description() + ' ' + _("(session chroot)"));
    keyfile.set_value(group, "aliases", "");
  }

  void setup_keyfile_source (sbuild::keyfile&   keyfile,
			     std::string const& group)
  {
    keyfile.set_value(group, "source-users", "suser1,suser2");
    keyfile.set_value(group, "source-root-users", "suser3,suser4");
    keyfile.set_value(group, "source-groups", "sgroup1,sgroup2");
    keyfile.set_value(group, "source-root-groups", "sgroup3,sgroup4");
  }

  void setup_keyfile_source_clone (sbuild::keyfile&   keyfile,
				   std::string const& group)
  {
    keyfile.set_value(group, "description", chroot->get_description() + ' ' + _("(source chroot)"));
    keyfile.set_value(group, "users", "suser1,suser2");
    keyfile.set_value(group, "root-users", "suser3,suser4");
    keyfile.set_value(group, "groups", "sgroup1,sgroup2");
    keyfile.set_value(group, "root-groups", "sgroup3,sgroup4");
    keyfile.set_value(group, "aliases", "test-alias-1-source,test-alias-2-source");
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
	std::string value;
	for (sbuild::string_list::const_iterator pos = missing.begin();
	     pos != missing.end();
	     ++pos)
	  {
	    expected_environment.get(*pos, value);
	    std::cout << "Missing environment: "
		      << *pos << "=" << value << std::endl;
	  }
      }
    CPPUNIT_ASSERT(missing.empty());

    sbuild::string_list extra;
    set_difference(found.begin(), found.end(),
		   expected.begin(), expected.end(),
		   std::back_inserter(extra));
    if (!extra.empty())
      {
	std::string value;
	for (sbuild::string_list::const_iterator pos = extra.begin();
	     pos != extra.end();
	     ++pos)
	  {
	    observed_environment.get(*pos, value);
	    std::cout << "Additional environment: "
		      << *pos << "=" << value << std::endl;
	  }
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

  void test_setup_env(sbuild::chroot::ptr&       chroot,
		      const sbuild::environment& expected_environment)
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
	std::string value;
	for (sbuild::string_list::const_iterator pos = missing.begin();
	     pos != missing.end();
	     ++pos)
	  {
	    expected_keyfile.get_value(expected_group, *pos, value);
	    std::cout << "Missing keys: "
		      << *pos << "=" << value << std::endl;
	  }
      }
    CPPUNIT_ASSERT(missing.empty());

    sbuild::string_list extra;
    set_difference(observed.begin(), observed.end(),
		   expected.begin(), expected.end(),
		   std::back_inserter(extra));
    if (!extra.empty())
      {
	std::string value;
	for (sbuild::string_list::const_iterator pos = extra.begin();
	     pos != extra.end();
	     ++pos)
	  {
	    observed_keyfile.get_value(observed_group, *pos, value);
	    std::cout << "Additional keys: " 
		      << *pos << "=" << value << std::endl;
	  }
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

  void test_setup_keyfile(sbuild::chroot::ptr&   chroot,
			  const sbuild::keyfile& expected_keyfile,
			  const std::string&     group)
  {
    sbuild::keyfile keys;
    chroot->get_keyfile(keys);

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
