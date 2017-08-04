
#include "Test.h"
#include "TestResult.h"
#include "TestRegistry.h"

#include <stdio.h>

void TestRegistry::addTest (Test *test)
{
  instance ().add (test);
}

void TestRegistry::runAllTests (TestResult& result)
{
  instance ().run (result);
}

TestRegistry& TestRegistry::instance ()
{
  static TestRegistry registry;
  return registry;
}

void TestRegistry::add (Test *test)
{
  if (tests == 0) {
    tests = test;
    return;
  }

  test->setNext (tests);
  tests = test;
}

void TestRegistry::run (TestResult& result)
{
  result.testsStarted ();

  int tCount = 0;
  for (Test *test = tests; test != 0; test = test->getNext ()) {
    printf("run... %s\n", test->theName().asCharString());
    test->run (result);
    tCount ++;
  }

  result.testsEnded (tCount);
}
