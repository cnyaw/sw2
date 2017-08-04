
//
//  Unit tests.
//
//  Copyright (c) 2008 Waync Cheng.
//  All Rights Reserved.
//
//  NOTE: Set [Debugging/Working Directory] to '../../test'.
//
//  2008/05/25 Waync created.
//

#include <time.h>

#include "CppUnitLite/TestHarness.h"

#include "swUtil.h"

int main(int argc, char *argv[])
{
  if (2 != argc || strncmp("-d", (const char*)argv[1], 2)) {
    SW2_TRACE_RESET_TARGET();           // Disable trace output to make clean test msg.
  }

  srand((unsigned int)time(0));

  TestResult tr;
  TestRegistry::runAllTests(tr);

  return 0;
}

// end of main.cpp
