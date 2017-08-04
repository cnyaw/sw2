
//
//  StageStack unit test.
//
//  Copyright (c) 2008 Waync Cheng.
//  All Rights Reserved.
//
//  2008/05/25 Waync created.
//

#include "CppUnitLite/TestHarness.h"

#include "swStageStack.h"
using namespace sw2;

class TestStageStack
{
public:

  StageStack<TestStageStack> mStack;

  TestStageStack()
  {
    mStack.initialize(this, &TestStageStack::s1);
  }

  void trigger()
  {
    mStack.trigger();
  }

  void s1(int s, uint_ptr)
  {
    if (TRIGGER == s) {
      mStack.popAndPush(&TestStageStack::s2);
    }
  }

  void s2(int s, uint_ptr)
  {
    if (TRIGGER == s) {
       mStack.push(&TestStageStack::s3);
    }
  }

  void s3(int s, uint_ptr)
  {
    if (TRIGGER == s) {
      mStack.popAndPush(&TestStageStack::s4);
    }
  }

  void s4(int s, uint_ptr)
  {
    if (TRIGGER == s) {
      mStack.popAndPush(&TestStageStack::s5, 2);
    }
  }

  void s5(int s, uint_ptr)
  {
    if (TRIGGER == s) {
      mStack.pop();
    }
  }
};

//
// Initialil value check.
//

TEST(StageStack, test)
{
  TestStageStack t1;
  CHECK(t1.mStack.top() == &TestStageStack::s1);

  t1.trigger();
  CHECK(t1.mStack.top() == &TestStageStack::s2);

  t1.trigger();
  CHECK(t1.mStack.top() == &TestStageStack::s3);

  t1.trigger();
  CHECK(t1.mStack.top() == &TestStageStack::s4);

  t1.trigger();
  CHECK(t1.mStack.top() == &TestStageStack::s5);

  t1.trigger();
  CHECK(t1.mStack.top() == 0);
}

// end of TestStageStack.cpp
