/* Copyright © 2006-2008  Roger Leigh <rleigh@codelibre.net>
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

#include <config.h>

#include <sbuild/sbuild-chroot-file.h>
#include <sbuild/sbuild-i18n.h>

#include "test-helpers.h"
#include "test-sbuild-chroot.h"

#include <algorithm>
#include <set>

#include <cppunit/extensions/HelperMacros.h>

using namespace CppUnit;

using sbuild::_;

class chroot_file : public sbuild::chroot_file
{
public:
  chroot_file():
    sbuild::chroot_file()
  {}

  virtual ~chroot_file()
  {}
};

class test_chroot_file : public test_chroot_base<chroot_file>
{
  CPPUNIT_TEST_SUITE(test_chroot_file);
  CPPUNIT_TEST(test_file);
  CPPUNIT_TEST(test_chroot_type);
  CPPUNIT_TEST(test_location);
  CPPUNIT_TEST_EXCEPTION(test_location_invalid, sbuild::chroot::error);
  CPPUNIT_TEST(test_repack);
  CPPUNIT_TEST(test_setup_env);
  CPPUNIT_TEST(test_setup_env_session);
  CPPUNIT_TEST(test_setup_env_source);
  CPPUNIT_TEST(test_setup_env_session_source);
  CPPUNIT_TEST(test_setup_keyfile);
  CPPUNIT_TEST(test_setup_keyfile_session);
  CPPUNIT_TEST(test_setup_keyfile_source);
  CPPUNIT_TEST(test_setup_keyfile_session_source);
  CPPUNIT_TEST(test_session_flags);
  CPPUNIT_TEST(test_print_details);
  CPPUNIT_TEST(test_print_config);
  CPPUNIT_TEST(test_run_setup_scripts);
  CPPUNIT_TEST_SUITE_END();

public:
  test_chroot_file():
    test_chroot_base<chroot_file>()
  {}

  void setUp()
  {
    test_chroot_base<chroot_file>::setUp();
    CPPUNIT_ASSERT(chroot);
    CPPUNIT_ASSERT(session);
    CPPUNIT_ASSERT(source);
    CPPUNIT_ASSERT(session_source);
#ifdef SBUILD_FEATURE_UNION
    CPPUNIT_ASSERT(!chroot_union);
    CPPUNIT_ASSERT(!session_union);
    CPPUNIT_ASSERT(!source_union);
    CPPUNIT_ASSERT(!session_source_union);
#endif // SBUILD_FEATURE_UNION
  }

  virtual void setup_chroot_props (sbuild::chroot::ptr& chroot)
  {
    test_chroot_base<chroot_file>::setup_chroot_props(chroot);

    std::shared_ptr<sbuild::chroot_file> c = std::dynamic_pointer_cast<sbuild::chroot_file>(chroot);

    c->set_file("/srv/chroot/example.tar.bz2");
    c->set_location("/sid");
  }

  void
  test_file()
  {
    std::shared_ptr<sbuild::chroot_file> c = std::dynamic_pointer_cast<sbuild::chroot_file>(chroot);
    CPPUNIT_ASSERT(c);
    c->set_file("/srv/chroot-images/unstable.tar.gz");
    CPPUNIT_ASSERT(c->get_file() == "/srv/chroot-images/unstable.tar.gz");
  }

  void test_chroot_type()
  {
    CPPUNIT_ASSERT(chroot->get_chroot_type() == "file");
  }

  void test_location()
  {
    std::shared_ptr<sbuild::chroot_file> fc = std::dynamic_pointer_cast<sbuild::chroot_file>(chroot);

    fc->set_location("");
    CPPUNIT_ASSERT(fc->get_location() == "");
    CPPUNIT_ASSERT(fc->get_path() == fc->get_mount_location());

    fc->set_location("/test");
    CPPUNIT_ASSERT(fc->get_location() == "/test");
    CPPUNIT_ASSERT(fc->get_path() == "/mnt/mount-location/test");
 }

  void test_location_invalid()
  {
    std::shared_ptr<sbuild::chroot_file> fc = std::dynamic_pointer_cast<sbuild::chroot_file>(chroot);

    fc->set_location("invalid");
  }

  void test_repack()
  {
    std::shared_ptr<sbuild::chroot_file> fc = std::dynamic_pointer_cast<sbuild::chroot_file>(chroot);
    std::shared_ptr<sbuild::chroot_file> fss = std::dynamic_pointer_cast<sbuild::chroot_file>(session);
    std::shared_ptr<sbuild::chroot_file> fs = std::dynamic_pointer_cast<sbuild::chroot_file>(source);


    CPPUNIT_ASSERT(fc->get_file_repack() == false);
    CPPUNIT_ASSERT(fss->get_file_repack() == false);
    CPPUNIT_ASSERT(fs->get_file_repack() == true);
  }

  void setup_env_gen(sbuild::environment &expected)
  {
    setup_env_chroot(expected);
    expected.add("CHROOT_TYPE",            "file");
    expected.add("CHROOT_FILE",            "/srv/chroot/example.tar.bz2");
    expected.add("CHROOT_LOCATION",        "/sid");
    expected.add("CHROOT_FILE_REPACK",     "false");
    expected.add("CHROOT_FILE_UNPACK_DIR", SCHROOT_FILE_UNPACK_DIR);
    expected.add("CHROOT_MOUNT_LOCATION",  "/mnt/mount-location");
    expected.add("CHROOT_PATH",            "/mnt/mount-location/sid");
    expected.add("CHROOT_SESSION_CLONE",   "true");
    expected.add("CHROOT_SESSION_CREATE",  "true");
    expected.add("CHROOT_SESSION_PURGE",   "false");
    expected.add("CHROOT_SESSION_SOURCE", "false");

    test_chroot_base<chroot_file>::test_setup_env(chroot, expected);
  }

  void test_setup_env()
  {
    sbuild::environment expected;
    setup_env_gen(expected);
    expected.add("CHROOT_FILE_REPACK",    "false");
    expected.add("CHROOT_SESSION_CLONE",  "true");
    expected.add("CHROOT_SESSION_CREATE", "true");
    expected.add("CHROOT_SESSION_PURGE",  "false");
    expected.add("CHROOT_SESSION_SOURCE", "false");

    test_chroot_base<chroot_file>::test_setup_env(chroot, expected);
  }

  void test_setup_env_session()
  {
    sbuild::environment expected;
    setup_env_gen(expected);
    expected.add("SESSION_ID",           "test-session-name");
    expected.add("CHROOT_ALIAS",         "test-session-name");
    expected.add("CHROOT_DESCRIPTION",    chroot->get_description() + ' ' + _("(session chroot)"));
    expected.add("CHROOT_FILE_REPACK",    "false");
    expected.add("CHROOT_SESSION_CLONE",  "false");
    expected.add("CHROOT_SESSION_CREATE", "false");
    expected.add("CHROOT_SESSION_PURGE",  "true");
    expected.add("CHROOT_SESSION_SOURCE", "false");

    test_chroot_base<chroot_file>::test_setup_env(session, expected);
  }

  void test_setup_env_source()
  {
    sbuild::environment expected;
    setup_env_gen(expected);
    expected.add("CHROOT_DESCRIPTION",    chroot->get_description() + ' ' + _("(source chroot)"));
    expected.add("CHROOT_FILE_REPACK",    "true");
    expected.add("CHROOT_SESSION_CLONE",  "false");
    expected.add("CHROOT_SESSION_CREATE", "true");
    expected.add("CHROOT_SESSION_PURGE",  "false");
    expected.add("CHROOT_SESSION_SOURCE", "false");

    test_chroot_base<chroot_file>::test_setup_env(source, expected);
  }

  void test_setup_env_session_source()
  {
    sbuild::environment expected;
    setup_env_gen(expected);
    expected.add("SESSION_ID",           "test-session-name");
    expected.add("CHROOT_ALIAS",         "test-session-name");
    expected.add("CHROOT_DESCRIPTION",    chroot->get_description() + ' ' + _("(source chroot) (session chroot)"));
    expected.add("CHROOT_FILE_REPACK",    "true");
    expected.add("CHROOT_SESSION_CLONE",  "false");
    expected.add("CHROOT_SESSION_CREATE", "false");
    expected.add("CHROOT_SESSION_PURGE",  "true");
    expected.add("CHROOT_SESSION_SOURCE", "true");

    test_chroot_base<chroot_file>::test_setup_env(session_source, expected);
  }

  void setup_keyfile_file(sbuild::keyfile &expected, const std::string group)
  {
    expected.set_value(group, "type", "file");
    expected.set_value(group, "file", "/srv/chroot/example.tar.bz2");
    expected.set_value(group, "location", "/sid");
  }

  void test_setup_keyfile()
  {
    sbuild::keyfile expected;
    const std::string group(chroot->get_name());
    setup_keyfile_chroot(expected, group);
    setup_keyfile_source(expected, group);
    setup_keyfile_file(expected, group);

    test_chroot_base<chroot_file>::test_setup_keyfile
      (chroot, expected, group);
  }

  void test_setup_keyfile_session()
  {
    sbuild::keyfile expected;
    const std::string group(session->get_name());
    setup_keyfile_session(expected, group);
    setup_keyfile_file(expected, group);
    expected.set_value(group, "name", "test-session-name");
    expected.set_value(group, "selected-name", "test-session-name");
    expected.set_value(group, "file-repack", "false");
    expected.set_value(group, "mount-location", "/mnt/mount-location");
    setup_keyfile_session_clone(expected, group);

    test_chroot_base<chroot_file>::test_setup_keyfile
      (session, expected, group);
  }

  void test_setup_keyfile_source()
  {
    sbuild::keyfile expected;
    const std::string group(source->get_name());
    setup_keyfile_chroot(expected, group);
    setup_keyfile_source_clone(expected, group);
    setup_keyfile_file(expected, group);
    expected.set_value(group, "description", chroot->get_description() + ' ' + _("(source chroot)"));

    test_chroot_base<chroot_file>::test_setup_keyfile
      (source, expected, group);
  }

  void test_setup_keyfile_session_source()
  {
    sbuild::keyfile expected;
    const std::string group(source->get_name());
    setup_keyfile_chroot(expected, group);
    setup_keyfile_file(expected, group);
    setup_keyfile_session_source_clone(expected, group);
    expected.set_value(group, "description", chroot->get_description() + ' ' + _("(source chroot) (session chroot)"));
    expected.set_value(group, "file-repack", "true");
    expected.set_value(group, "mount-location", "/mnt/mount-location");

    test_chroot_base<chroot_file>::test_setup_keyfile
      (session_source, expected, group);
  }

  void test_session_flags()
  {
    CPPUNIT_ASSERT(chroot->get_session_flags() ==
                   (sbuild::chroot::SESSION_CREATE |
                    sbuild::chroot::SESSION_CLONE));

    CPPUNIT_ASSERT(session->get_session_flags() ==
                   sbuild::chroot::SESSION_PURGE);

    CPPUNIT_ASSERT(source->get_session_flags() ==
                   sbuild::chroot::SESSION_CREATE);
  }

  void test_print_details()
  {
    std::ostringstream os;
    os << chroot;
    // TODO: Compare output.
    CPPUNIT_ASSERT(!os.str().empty());
  }

  void test_print_config()
  {
    std::ostringstream os;
    sbuild::keyfile config;
    config << chroot;
    os << config;
    // TODO: Compare output.
    CPPUNIT_ASSERT(!os.str().empty());
  }

  void test_run_setup_scripts()
  {
    CPPUNIT_ASSERT(chroot->get_run_setup_scripts());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(test_chroot_file);
