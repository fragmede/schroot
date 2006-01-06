#include <cstdlib>

#include <cppunit/extensions/HelperMacros.h>

#include <schroot/sbuild-util.h>

using namespace CppUnit;

class test_util : public TestCase
{
  CPPUNIT_TEST_SUITE(test_util);
  CPPUNIT_TEST(test_basename);
  CPPUNIT_TEST(test_dirname);
  CPPUNIT_TEST(test_string_list_to_string);
  CPPUNIT_TEST(test_split_string);
  CPPUNIT_TEST(test_find_program_in_path);
  CPPUNIT_TEST_SUITE_END();

public:
  test_util()
  {}

  void test_basename()
  {
    CPPUNIT_ASSERT(sbuild::basename("/usr/bin/perl") == "perl");
    CPPUNIT_ASSERT(sbuild::basename("/usr/lib") == "lib");
    CPPUNIT_ASSERT(sbuild::basename("/usr/") == "usr");
    CPPUNIT_ASSERT(sbuild::basename("usr") == "usr");
    CPPUNIT_ASSERT(sbuild::basename("/") == "/");
    CPPUNIT_ASSERT(sbuild::basename(".") == ".");
    CPPUNIT_ASSERT(sbuild::basename("..") == "..");
  }

  void test_dirname()
  {
    CPPUNIT_ASSERT(sbuild::dirname("/usr/bin/perl") == "/usr/bin");
    CPPUNIT_ASSERT(sbuild::dirname("/usr/lib") == "/usr");
    CPPUNIT_ASSERT(sbuild::dirname("/usr/") == "/");
    CPPUNIT_ASSERT(sbuild::dirname("usr") == ".");
    CPPUNIT_ASSERT(sbuild::dirname("/") == "/");
    CPPUNIT_ASSERT(sbuild::dirname(".") == ".");
    CPPUNIT_ASSERT(sbuild::dirname("..") == ".");
  }

  void test_string_list_to_string()
  {
    sbuild::string_list items;
    items.push_back("foo");
    items.push_back("bar");
    items.push_back("baz");

    CPPUNIT_ASSERT(sbuild::string_list_to_string(items, "--") ==
		   "foo--bar--baz");
  }

  void test_split_string()
  {
    sbuild::string_list items =
      sbuild::split_string("/usr/share/info", '/');

    CPPUNIT_ASSERT(items.size() == 3 &&
		   items[0] == "usr" &&
		   items[1] == "share" &&
		   items[2] == "info");
  }

  void test_find_program_in_path()
  {
    CPPUNIT_ASSERT(sbuild::find_program_in_path("sh") == "/bin/sh");
    CPPUNIT_ASSERT(sbuild::find_program_in_path("sed") == "/bin/sed");
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(test_util);
