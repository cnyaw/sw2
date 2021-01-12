
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
#include "swUtil.h"
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

class TestStageStack2
{
public:
  StageStack<TestStageStack2> m_stage;
  int m_count;                          // join,resume:+1, leave,suspend:-1.

  TestStageStack2() : m_count(0)
  {
    m_stage.initialize(this, &TestStageStack2::s1);
  }

  void defs(int s)
  {
    if (JOIN == s || RESUME == s) {
      m_count += 1;
    }
    if (LEAVE == s || SUSPEND == s) {
      m_count -= 1;
    }
  }

  void s1(int s, uint_ptr)
  {
    defs(s);
    if (TRIGGER == s) {
      m_stage.push(&TestStageStack2::s2);
    }
  }

  void s2(int s, uint_ptr)
  {
    defs(s);
    if (TRIGGER == s) {
      if (rand() % 2) {
        m_stage.pop();
      } else {
        m_stage.push(&TestStageStack2::s3);
      }
    }
  }

  void s3(int s, uint_ptr)
  {
    defs(s);
    if (TRIGGER == s) {
      if (rand() % 2) {
        m_stage.pop();
      } else {
        m_stage.push(&TestStageStack2::s4);
      }
    }
  }

  void s4(int s, uint_ptr)
  {
    defs(s);
    if (TRIGGER == s) {
      m_stage.pop();
    }
  }

  bool test(int ticks) const
  {
    return 1 == m_count;
  }

  void trigger()
  {
    m_stage.trigger();
  }
};

TEST(StageStack, test2)
{
  const int TICKS = Util::rangeRand(1500, 2000);

  TestStageStack2 s;
  for (int i = 0; i < TICKS; i++) {
    s.trigger();
  }
  CHECK(s.test(1000));
}

// end of TestStageStack.cpp
