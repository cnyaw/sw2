
//
//  ObjectPool unit test.
//
//  Copyright (c) 2008 Waync Cheng.
//  All Rights Reserved.
//
//  2008/05/25 Waync created.
//

#include <algorithm>
#include <vector>

#include "CppUnitLite/TestHarness.h"

#include "swObjectPool.h"
#include "swUtil.h"
using namespace sw2;

//
// Test fix size pool initialization with default size.
//

TEST(ObjectPool, init1)
{
  ObjectPool<int> op;
  CHECK(op.size() == 0);
  CHECK(op.capacity() == 16);
  CHECK(op.available() == 16);
  CHECK(op.first() == -1);
  CHECK(op.last() == -1);
}

//
// Test fix size pool initialization with specified size.
//

TEST(ObjectPool, init2)
{
  ObjectPool<int, 2> op1;
  CHECK(op1.size() == 0);
  CHECK(op1.capacity() == 2);
  CHECK(op1.available() == 2);
  CHECK(op1.first() == -1);
  CHECK(op1.last() == -1);

  ObjectPool<int, 4> op2;
  CHECK(op2.size() == 0);
  CHECK(op2.capacity() == 4);
  CHECK(op2.available() == 4);
  CHECK(op2.first() == -1);
  CHECK(op2.last() == -1);

  ObjectPool<int, 8> op3;
  CHECK(op3.size() == 0);
  CHECK(op3.capacity() == 8);
  CHECK(op3.available() == 8);
  CHECK(op3.first() == -1);
  CHECK(op3.last() == -1);

  ObjectPool<int, 32> op4;
  CHECK(op4.size() == 0);
  CHECK(op4.capacity() == 32);
  CHECK(op4.available() == 32);
  CHECK(op4.first() == -1);
  CHECK(op4.last() == -1);
}

//
// Test variable size pool initialization with specified size.
//

TEST(ObjectPool, init3)
{
  ObjectPool<int, 16, true> op1;
  CHECK(op1.size() == 0);
  CHECK(op1.capacity() == 16);
  CHECK(op1.available() == 16);
  CHECK(op1.first() == -1);
  CHECK(op1.last() == -1);

  ObjectPool<int, 32, true> op2;
  CHECK(op2.size() == 0);
  CHECK(op2.capacity() == 32);
  CHECK(op2.available() == 32);
  CHECK(op2.first() == -1);
  CHECK(op2.last() == -1);
}

//
// Test alloc/free of fix size pool.
//

TEST(ObjectPool, test1)
{
  int const mod[16] = {13, 2, 10, 1, 12, 3, 8, 4, 5, 16, 9, 6, 15, 14, 11, 7};

  ObjectPool<int,128> p;
  for (int i = 0; i < 65535; ++i) {

    int i0 = i % 16;
    if (0 == (i % mod[i0])) {

      int i1 = i % p.capacity();
      if (p.isUsed(i1)) {
        p.free(i1);
        continue;
      }
    }

    (void)p.alloc();
  }

  int const res[] = {
    1, 6, 7, 9, 11, 13, 17, 22, 23, 25, 27, 29, 33, 38, 39, 41, 43, 45, 49, 54, 55,
    57, 59, 61, 65, 70, 71, 73, 75, 77, 81, 86, 87, 89, 91, 93, 97, 102, 103, 105,
    107, 109, 113, 118, 119, 121, 123, 125, 44, 28, 64, 12, 46, 16, 94, 124, 74, 96,
    14, 90, 108, 47, 48, 62, 106, 31, 92, 110, 122, 0, 15, 24, 34, 104, 114, 127, 10,
    30, 56, 66, 76, 80, 111, 5, 8, 18, 20, 26, 53, 68, 78, 88, 95, 98, 101, 116, 21,
    32, 36, 40, 42, 50, 60, 69, 79, 84, 117, 120, 126, 2, 3, 4, 19, 35, 37, 51, 52,
    58, 63, 67, 72, 82, 83, 85, 99, 100, 112, 115
  };

  std::vector<int> v;
  for (int i = p.first(); -1 != i; i = p.next(i)) {
    v.push_back(i);
  }

  CHECK(v == std::vector<int>(res, res + sizeof(res)/sizeof(res[0])));
}

//
// Test fix size pool allocates object with specified id.
//

TEST(ObjectPool, alloc1)
{
  int const c[] = {26,17,18,29,31,30,7,6,20,23,0,1,2,3,4,5,8,9,10,11,12,13,14,15,16,19,21,22,24,25,27,28};

  ObjectPool<int,32> p;
  for (int i = 0; i < 10; ++i) {
    CHECK(c[i] == p.alloc(c[i]));
  }

  for (int i = 10; i < 32; ++i) {
    CHECK(-1 != p.alloc());
  }

  std::vector<int> v;
  for (int i = p.first(); -1 != i; i = p.next(i)) {
    v.push_back(i);
  }

  CHECK(std::vector<int>(c,c+32) == v);
}

TEST(ObjectPool, alloc1_2)
{
  std::vector<int> v1;
  for (int i = 0; i < 32; ++i) {
    v1.push_back(i);
  }

  std::random_shuffle(v1.begin(), v1.end());

  ObjectPool<int,32> p;
  for (size_t i = 0; i < v1.size(); ++i) {
    CHECK(v1[i] == p.alloc(v1[i]));
  }

  std::vector<int> v;
  for (int i = p.first(); -1 != i; i = p.next(i)) {
    v.push_back(i);
  }

  CHECK(v1 == v);
}

TEST(ObjectPool, alloc1_3)
{
  {
    ObjectPool<int,32> p;               // Fix size pool.
    CHECK(-1 == p.alloc(-2));
    CHECK(-1 == p.alloc(-1));
    CHECK(0 == p.alloc(0));
    CHECK(-1 == p.alloc(0));
    CHECK(31 == p.alloc(31));
    CHECK(-1 == p.alloc(31));
    CHECK(-1 == p.alloc(32));
  }

  {
    ObjectPool<int,32,true> p;          // Dynamic grow pool.
    CHECK(-1 == p.alloc(-2));
    CHECK(-1 == p.alloc(-1));
    CHECK(0 == p.alloc(0));
    CHECK(-1 == p.alloc(0));
    CHECK(31 == p.alloc(31));
    CHECK(-1 == p.alloc(31));
    CHECK(32 == p.alloc(32));
    CHECK(-1 == p.alloc(32));
    CHECK(128 == p.alloc(128));
    CHECK(-1 == p.alloc(128));
  }
}

//
// Test variable size pool allocates object with specified id.
//

TEST(ObjectPool, alloc2)
{
  int const c[] = {26,17,18,29,31,30,7,6,20,23,0,1,2,3,4,5,8,9,10,11,12,13,14,15,16,19,21,22,24,25,27,28};

  ObjectPool<int,1,true> p;
  for (int i = 0; i < 10; ++i) {
    CHECK(c[i] == p.alloc(c[i]));
  }

  for (int i = 10; i < 32; ++i) {
    CHECK(-1 != p.alloc());
  }

  std::vector<int> v;
  for (int i = p.first(); -1 != i; i = p.next(i)) {
    v.push_back(i);
  }

  CHECK(std::vector<int>(c,c+32) == v);
}

TEST(ObjectPool, alloc2_2)
{
  std::vector<int> v1;
  for (int i = 0; i < rangeRand<int>(32,48); ++i) {
    v1.push_back(i);
  }

  std::random_shuffle(v1.begin(), v1.end());

  ObjectPool<int,1,true> p;
  for (size_t i = 0; i < v1.size(); ++i) {
    CHECK(v1[i] == p.alloc(v1[i]));
  }

  std::vector<int> v;
  for (int i = p.first(); -1 != i; i = p.next(i)) {
    v.push_back(i);
  }

  CHECK(v1 == v);
}

//
// Test reset of fix size pool.
//

TEST(ObjectPool, reset1)
{
  ObjectPool<int,128> p;
  for (int i = 0; i < 128; ++i) {
    CHECK(-1 != p.alloc());
  }

  std::vector<int> v1;
  for (int i = p.first(); -1 != i; i = p.next(i)) {
    v1.push_back(i);
  }

  p.reset();

  for (int i = 0; i < 128; ++i) {
    CHECK(-1 != p.alloc());
  }

  std::vector<int> v2;
  for (int i = p.first(); -1 != i; i = p.next(i)) {
    v2.push_back(i);
  }

  CHECK(v1 == v2);
}

//
// Test variable size pool reset.
//

TEST(ObjectPool, reset2)
{
  ObjectPool<int,4,true> p;
  for (int i = 0; i < 128; ++i) {
    CHECK(-1 != p.alloc());
  }

  std::vector<int> v1;
  for (int i = p.first(); -1 != i; i = p.next(i)) {
    v1.push_back(i);
  }

  p.reset();

  for (int i = 0; i < 128; ++i) {
    CHECK(-1 != p.alloc());
  }

  std::vector<int> v2;
  for (int i = p.first(); -1 != i; i = p.next(i)) {
    v2.push_back(i);
  }

  CHECK(v1 == v2);
}

//
// Test swap of fix size pool.
//

TEST(ObjectPool, swap1)
{
  ObjectPool<int,5> p;
  for (int i = 0; i < 5; ++i) {
    p.alloc();
  }

  CHECK(!p.swap(10, 20));
  CHECK(!p.swap(-10, -20));
  CHECK(!p.swap(1, 10));
  CHECK(!p.swap(-10, 1));

  CHECK(!p.swap(p.first(), p.first()));
  CHECK(!p.swap(1, 1));
  CHECK(!p.swap(p.last(), p.last()));

  CHECK(p.swap(3, 1));                  // 0,3,2,1,4
  CHECK(p.swap(p.first(), 1));          // 1,3,2,0,4
  CHECK(p.swap(p.last(), 0));           // 1,3,2,4,0
  CHECK(p.swap(p.first(), p.last()));   // 0,3,2,4,1
  CHECK(p.swap(3, 2));                  // 0,2,3,4,1
  CHECK(p.swap(2, p.first()));          // 2,0,3,4,1
  CHECK(p.swap(4, 3));                  // 2,0,4,3,1
  CHECK(p.swap(p.last(), 3));           // 2,0,4,1,3

  int const la[] = {2,0,4,1,3};
  for (int i = 0, id = p.first(); -1 != id; id = p.next(id), ++i) {
    CHECK(la[i] == id);
  }

  for (int i = 4, id = p.last(); -1 != id; id = p.prev(id), --i) {
    CHECK(la[i] == id);
  }
}

//
// Test swap of variable size pool.
//

TEST(ObjectPool, swap2)
{
  ObjectPool<int,5,true> p;
  for (int i = 0; i < 5; ++i) {
    p.alloc();
  }

  CHECK(!p.swap(10, 20));
  CHECK(!p.swap(-10, -20));
  CHECK(!p.swap(1, 10));
  CHECK(!p.swap(-10, 1));

  CHECK(!p.swap(p.first(), p.first()));
  CHECK(!p.swap(1, 1));
  CHECK(!p.swap(p.last(), p.last()));

  CHECK(p.swap(3, 1));                  // 0,3,2,1,4
  CHECK(p.swap(p.first(), 1));          // 1,3,2,0,4
  CHECK(p.swap(p.last(), 0));           // 1,3,2,4,0
  CHECK(p.swap(p.first(), p.last()));   // 0,3,2,4,1
  CHECK(p.swap(3, 2));                  // 0,2,3,4,1
  CHECK(p.swap(2, p.first()));          // 2,0,3,4,1
  CHECK(p.swap(4, 3));                  // 2,0,4,3,1
  CHECK(p.swap(p.last(), 3));           // 2,0,4,1,3

  int const la[] = {2,0,4,1,3};
  for (int i = 0, id = p.first(); -1 != id; id = p.next(id), ++i) {
    CHECK(la[i] == id);
  }

  for (int i = 4, id = p.last(); -1 != id; id = p.prev(id), --i) {
    CHECK(la[i] == id);
  }
}

//
// Test insert of variable size pool.
//

TEST(ObjectPool, insert1)
{
  const int n = 512;

  std::vector<int> v;
  for (int i = 0; i < n; ++i) {
    v.push_back(i);
  }

  std::random_shuffle(v.begin(), v.end());

  ObjectPool<int,4,true> p;
  for (int i = 0; i < n; ++i) {
    CHECK(v[i] == p.alloc(v[i]));
  }

  for (int i = n - 1, x = p.last(); i >= 0; --i, x = p.prev(x)) {
    CHECK(v[i] == x);
  }

  for (int i = 0; i < n; ++i) {
    int pos = rand() % n;
    while (pos == i) {
      pos = rand() % n;
    }
    for (int j = 0; j < n; ++j) {
      if (v[j] == i) {
        v.erase(v.begin() + j);
        break;
      }
    }
    for (int j = 0; j < n - 1; ++j) {
      if (pos == v[j]) {
        v.insert(v.begin() + j, i);
        break;
      }
    }
    p.insert(pos, i);
  }

  for (int i = 0, x = p.first(); i < n; ++i, x = p.next(x)) {
    CHECK(v[i] == x);
  }

  for (int i = 0; i < n; ++i) {
    p.insert(p.first(), i);
  }

  for (int i = 0, x = p.last(); i < n; ++i, x = p.prev(x)) {
    CHECK(i == x);
  }

  for (int i = 0; i < n; ++i) {
    p.insert(-1, i);
  }

  for (int i = 0, x = p.last(); i < n; ++i, x = p.prev(x)) {
    CHECK(n - i - 1 == x);
  }
}

//
// Test insert of fix size pool.
//

TEST(ObjectPool, insert2)
{
  const int n = 512;

  std::vector<int> v;
  for (int i = 0; i < n; ++i) {
    v.push_back(i);
  }

  std::random_shuffle(v.begin(), v.end());

  ObjectPool<int,n> p;
  for (int i = 0; i < n; ++i) {
    CHECK(v[i] == p.alloc(v[i]));
  }

  for (int i = 0, x = p.first(); i < n; ++i, x = p.next(x)) {
    CHECK(v[i] == x);
  }

  for (int i = 0; i < n; ++i) {
    int pos = rand() % n;
    while (pos == i) {
      pos = rand() % n;
    }
    for (int j = 0; j < n; ++j) {
      if (v[j] == i) {
        v.erase(v.begin() + j);
        break;
      }
    }
    for (int j = 0; j < n - 1; ++j) {
      if (pos == v[j]) {
        v.insert(v.begin() + j, i);
        break;
      }
    }
    p.insert(pos, i);
  }

  for (int i = n - 1, x = p.last(); i >= 0; --i, x = p.prev(x)) {
    CHECK(v[i] == x);
  }

  for (int i = 0; i < n; ++i) {
    p.insert(p.first(), i);
  }

  for (int i = 0, x = p.first(); i < n; ++i, x = p.next(x)) {
    CHECK(n - i - 1 == x);
  }

  for (int i = 0; i < n; ++i) {
    p.insert(-1, i);
  }

  for (int i = 0, x = p.first(); i < n; ++i, x = p.next(x)) {
    CHECK(i == x);
  }
}

//
// Test ObjectPool::clear.
//

TEST(ObjectPool, clear)
{
  ObjectPool<int, 16> p1;

  CHECK(0 == p1.size());
  for (int i = 0; i < 20; i++) {
    (void)p1.alloc();
  }
  CHECK(p1.capacity() == p1.size());

  p1.clear();
  CHECK(0 == p1.size());

  ObjectPool<int, 16, true> p2;

  CHECK(0 == p2.size());
  for (int i = 0; i < 20; i++) {
    (void)p2.alloc();
  }
  CHECK(20 == p2.size());

  p2.clear();
  CHECK(0 == p2.size());
}

// end of TestObjectPool.cpp
