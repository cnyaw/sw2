
//
//  ThreadPool unit test.
//
//  Copyright (c) 2008 Waync Cheng.
//  All Rights Reserved.
//
//  2008/06/04 Waync created.
//

#include <time.h>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "CppUnitLite/TestHarness.h"

#include "swThreadPool.h"
#include "swUtil.h"
using namespace sw2;

class TestThreadPool : public ThreadTask
{
public:

  enum { SIZE = 512 };

  int* mArray;                          // int[]

  virtual void threadTask()
  {
    std::sort(mArray, mArray + SIZE);
  }
};

//
// Test init/uninit.
//

TEST(ThreadPool, init)
{
  CHECK(InitializeThreadPool(4));
  UninitializeThreadPool();
}

//
// Check state of uninitial ThreadPool.
//

TEST(ThreadPool, state1)
{
  int const THREADS = 128;

  TestThreadPool t1[THREADS];

  for (int i = 0; i < THREADS; ++i) {
    CHECK(!t1[i].isRunning());
  }
}

//
// Check state 2.
//

TEST(ThreadPool, state2)
{
  CHECK(InitializeThreadPool(4));

  { // guard
    int const THREADS = 128;

    TestThreadPool t1[THREADS];

    for (int i = 0; i < THREADS; ++i) {
      CHECK(!t1[i].isRunning());
    }
  }

  UninitializeThreadPool();
}

//
// Run task with uninitial ThreadPool.
//

TEST(ThreadPool, runTask1)
{
  int const THREADS = 128;

  TestThreadPool t1[THREADS];
  for (int i = 0; i < THREADS; ++i) {
    CHECK(!t1[i].isRunning());
    CHECK(!t1[i].runTask());
  }
}

//
// Run test 2.
//

TEST(ThreadPool, runTask2)
{
  CHECK(InitializeThreadPool(4));

  { // guard
    int const SIZE = TestThreadPool::SIZE;
    int const THREADS = 256;

    //
    // Initial test data.
    //

    int data[THREADS][SIZE];
    for (int i = 0; i < THREADS; ++i) {
      for (int j = 0; j < SIZE; ++j) {
        data[i][j] = j;
      }
      std::random_shuffle(data[i], data[i] + SIZE);
    }

    //
    // Sort above 8 arrays with 8 threads.
    //

    TestThreadPool t1[THREADS];
    for (int i = 0; i < THREADS; ++i) {
      t1[i].mArray = data[i];           // Specify object to sort.
      CHECK(!t1[i].isRunning());
      CHECK(t1[i].runTask());
    }

    //
    // Wait.
    //

continue_1:

    for (int i = 0; i < THREADS; ++i) {
      if (t1[i].isRunning()) {
        Util::sleep(1);
        goto continue_1;
      }
    }

    //
    // Wait all tasks done and validate result.
    //

    int iv[SIZE];
    for (int i = 0; i < SIZE; ++i) {
      iv[i] = i;
    }

    std::vector<int> const v(iv, iv + SIZE);
    for (int i = 0; i < THREADS; ++i) {
      CHECK(v == std::vector<int>(data[i], data[i] + SIZE));
    }
  }

  UninitializeThreadPool();
}

//
// Word count.
//

class WordCountTask : public ThreadTask
{
public:

  std::vector<std::string> v;
  std::map<std::string, int> m;

  virtual void threadTask()
  {
    for (size_t i = 0; i < v.size(); i++) {
      std::map<std::string, int>::iterator it = m.find(v[i]);
      if (m.end() == it) {
        m[v[i]] = 1;
      } else {
        it->second += 1;
      }
    }
  }

  void merge(std::map<std::string, int>& res)
  {
    std::map<std::string, int>::iterator it;
    for (it = m.begin(); m.end() != it; ++it) {
      std::map<std::string, int>::iterator it2 = res.find(it->first);
      if (res.end() == it2) {
        res[it->first] = it->second;
      } else {
        it2->second += it->second;
      }
    }
  }
};

TEST(ThreadPool, wordcount)
{
  CHECK(InitializeThreadPool(20));

  srand((unsigned int)time(0));

  for (int test = 0; test < 10; test++) {

    //
    // Init test data.
    //

    const int NW = 2500;

    std::vector<std::string> v;
    for (int i = 0; i < NW; i++) {
      char s[6] = {0};
      int len = 1 + (rand() % 5);
      for (int j = 0; j < len; j++) {
        s[j] = 'a' + (rand() % 5);
      }
      v.push_back(s);
    }

    std::map<std::string, int> result;
    for (size_t i = 0; i < v.size(); i++) {
      std::map<std::string, int>::iterator it = result.find(v[i]);
      if (result.end() == it) {
        result[v[i]] = 1;
      } else {
        it->second += 1;
      }
    }

    //
    // Assign tasks.
    //

    std::map<std::string, int> m2;

    WordCountTask wc[10];

    const int TEST_COUNT = 100;

    int n = 0;
    while (NW > n) {
      for (int i = 0; i < 10; i++) {
        if (wc[i].isRunning()) {
          continue;
        }
        wc[i].merge(m2);
        wc[i].m.clear();
        wc[i].v.assign(v.begin() + n, v.begin() + n + TEST_COUNT);
        CHECK(wc[i].runTask());
        n += TEST_COUNT;
        if (NW <= n) {
          break;
        }
      }
    }

    //
    // Wait tasks done.
    //

wait:
    for (int i = 0; i < 10; i++) {
      if (wc[i].isRunning()) {
        Util::sleep(1);
        goto wait;
      }
    }

    for (int i = 0; i < 10; i++) {
      wc[i].merge(m2);
    }

    //
    // Verify.
    //

    CHECK(result == m2);
  }

  UninitializeThreadPool();
}

// end of TestThreadPool.cpp
