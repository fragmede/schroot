#include <cstdlib>

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

using namespace CppUnit;

int
main(int argc,
     char* argv[])
{
  TextUi::TestRunner runner;

  TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
  runner.addTest(registry.makeTest());

  bool ok = runner.run();

  return (ok) ? EXIT_SUCCESS : EXIT_FAILURE;
}
