
#include "TestResult.h"
#include "Failure.h"

#include <stdio.h>


TestResult::TestResult ()
    : failureCount (0)
{
}

void TestResult::testsStarted () 
{
}

void TestResult::addFailure (const Failure& failure) 
{
  fprintf (
    stdout,
    "%s%s%s%s%ld%s%s\n",
    "Failure: \"",
    failure.message.asCharString (),
    "\" " ,
    "line ",
    failure.lineNumber,
    " in ",
    failure.fileName.asCharString ());
      
  failureCount++;
}

void TestResult::testsEnded (int nTest) 
{
  if (failureCount > 0) {
    fprintf (stdout, "There were %d failures in %d test(s)\n", failureCount, nTest);
  } else {
    fprintf (stdout, "There were no test failures in %d test(s)\n", nTest);
  }
}
