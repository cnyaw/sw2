
//
//  Cells unit test.
//
//  Copyright (c) 2008 Waync Cheng.
//  All Rights Reserved.
//
//  2008/10/12 Waync created.
//

#include <algorithm>

#include "CppUnitLite/TestHarness.h"

#include "swCells.h"
#include "swUtil.h"
using namespace sw2;

//
// Add object test.
//

TEST(Cells, alloc)
{
  Cells<int> grid;

  //
  // Add objects before initialization.
  //

  CHECK(-1 == grid.alloc(1, 10, 10));
  CHECK(-1 == grid.alloc(2, -10, 10));
  CHECK(-1 == grid.alloc(3, 10, -10));
  CHECK(-1 == grid.alloc(4, -10, -10));

  //
  // Add objects after initialization.
  //

  grid.init(-100, -100, 100, 100, 2, 2);

  CHECK(-1 != grid.alloc(1, 10, 10));
  CHECK(-1 != grid.alloc(2, -10, 10));
  CHECK(-1 != grid.alloc(3, 10, -10));
  CHECK(-1 != grid.alloc(4, -10, -10));
  CHECK(-1 != grid.alloc(5, 0, 0));

  //
  // Add objects on the boundary.
  //

  CHECK(-1 == grid.alloc(1, 100, 100)); // Boundary.
  CHECK(-1 == grid.alloc(2, -100, 100)); // Boundary.
  CHECK(-1 == grid.alloc(3, 100, -100)); // Boundary.
  CHECK(-1 != grid.alloc(4, -100, -100));

  //
  // Add objects outside the boundary.
  //

  CHECK(-1 == grid.alloc(1, 210, 10));
  CHECK(-1 == grid.alloc(2, -210, 10));
  CHECK(-1 == grid.alloc(3, 10, -210));
  CHECK(-1 == grid.alloc(4, -10, -210));
}

//
// Remove object test.
//

TEST(Cells, free)
{
  Cells<float> grid;
  grid.init(-100, -100, 100, 100, 2, 2);

  //
  // Add some test objects.
  //

  int a = grid.alloc(1, 10, 10);
  CHECK(-1 != a);

  int b = grid.alloc(2, -10, 10);
  CHECK(-1 != b);

  int c = grid.alloc(3, 10, -10);
  CHECK(-1 != c);

  int d = grid.alloc(4, -10, -10);
  CHECK(-1 != d);

  int e = grid.alloc(5, 0, 0);
  CHECK(-1 != e);

  int f = grid.alloc(6, -100, -100);
  CHECK(-1 != f);

  //
  // Remove objects.
  //

  CHECK(grid.free(a));
  CHECK(grid.free(b));
  CHECK(grid.free(c));
  CHECK(grid.free(d));
  CHECK(grid.free(e));
  CHECK(grid.free(f));

  CHECK(!grid.free(a));
  CHECK(!grid.free(b));
  CHECK(!grid.free(c));
  CHECK(!grid.free(d));
  CHECK(!grid.free(e));
  CHECK(!grid.free(f));

  CHECK(!grid.free(100));
  CHECK(!grid.free(101));
  CHECK(!grid.free(102));
  CHECK(!grid.free(-1));
  CHECK(!grid.free(-100));
  CHECK(!grid.free(-101));
  CHECK(!grid.free(-102));
}

//
// Move objects test.
//

TEST(Cells, move)
{
  Cells<int,float> grid;
  grid.init(-100, -100, 100, 100, 2, 2);

  int a = grid.alloc(1, 10, 10);
  CHECK(-1 != a);

  //
  // Move objects in the boundary.
  //

  CHECK(grid.move(a, -10, 10));
  CHECK(grid.move(a, 10, -10));
  CHECK(grid.move(a, -10, -10));
  CHECK(grid.move(a, 0, 0));

  //
  // Move objects on the boundary.
  //

  CHECK(grid.move(a, -100, 10));
  CHECK(grid.move(a, -10, -100));
  CHECK(grid.move(a, -100, -100));
  CHECK(!grid.move(a, -10, 100));
  CHECK(!grid.move(a, 100, 10));
  CHECK(!grid.move(a, 100, 100));

  //
  // Move objects outside the boundary..
  //

  CHECK(!grid.move(a, -200, 10));
  CHECK(!grid.move(a, 10, -200));
  CHECK(!grid.move(a, -10, 200));
  CHECK(!grid.move(a, -200, 2000));
}

//
// Search object in rectangle area.
//

struct TestCellsFilter
{
  std::vector<int> v;

  bool operator()(int i)
  {
    v.push_back(i);
    return true;
  }
};

TEST(Cells, search1)
{
  Cells<int,float> grid;
  grid.init(-100, -100, 10, 10, 20, 20);

  float const pl[] = {
    -99.7497f, 12.7171f, -61.3392f, 61.7481f, 17.0019f, -4.02539f, -29.9417f, 79.1925f,
    64.568f, 49.321f, -65.1784f, 71.7887f, 42.1003f, 2.70699f, -39.201f, -97.0031f,
    -81.7194f, -27.1096f, -70.5374f, -66.8203f, 97.705f, -10.8615f, -76.1834f, -99.0661f,
    -98.2177f, -24.424f, 6.33259f, 14.2369f, 20.3528f, 21.4331f, -66.7531f, 32.609f,
    -9.84222f, -29.5755f, -88.5922f, 21.5369f, 56.6637f, 60.5213f, 3.97656f, -39.61f,
    75.1946f, 45.3352f, 91.1802f, 85.1436f, 7.87072f, -71.5323f, -7.58385f, -52.9344f,
    72.4479f, -58.0798f, 55.9313f, 68.7307f, 99.3591f, 99.939f, 22.2999f, -21.5125f,
    -46.7574f, -40.5438f, 68.0288f, -95.2513f, -24.8268f, -81.4753f, 35.4411f, -88.757f
  };

  for (int i = 0; i < 32; ++i) {
    CHECK(-1 != grid.alloc(i, pl[2 * i], pl[2 * i + 1]));
  }

  {
    TestCellsFilter f;
    grid.search(-50, -50, 50, 50, 32, f);

    int const res[] = {2, 6, 13, 14, 16, 19, 27, 28};
    std::vector<int> v(res, res + sizeof(res)/sizeof(res[0]));

    std::sort(f.v.begin(), f.v.end());
    CHECK(v == f.v);
  }

  {
    TestCellsFilter f;
    grid.search(-83, -54, 124, 112, 32, f);

    int const res[] = {1, 2, 3, 4, 5, 6, 8, 10, 13, 14, 15, 16, 18, 19, 20, 21, 23, 25, 26, 27, 28};
    std::vector<int> v(res, res + sizeof(res)/sizeof(res[0]));

    std::sort(f.v.begin(), f.v.end());
    CHECK(v == f.v);
  }
}

TEST(Cells, search1_2)
{
  Cells<int> grid;
  grid.init(-100, -100, 10, 10, 20, 20);

  std::vector<IntPoint> pt;

  for (int i = 0; i < 128; ++i) {
    pt.push_back(IntPoint(rangeRand<int>(-100,100-1),rangeRand<int>(-100,100-1)));
    CHECK(-1 != grid.alloc(i, pt[i].x, pt[i].y));
  }

  for (int i = 0; i < 1024; ++i) {

    int x = rangeRand<int>(-100,100-1), y = rangeRand<int>(-100,100-1);
    int w = rangeRand<int>(30,60), h = rangeRand<int>(30,60);

    TestCellsFilter f;
    grid.search(x, y, x+w, y+h, 32, f);

    IntRect bound(x, y, x+w, y+h);

    for (size_t j = 0; j < f.v.size(); ++j) {
      CHECK(bound.ptInRect(pt[f.v[j]]));
    }
  }
}

//
// Search objects in circle area.
//

TEST(Cells, search2)
{
  Cells<int> grid;
  grid.init(-100, -100, 10, 10, 20, 20);

  int const pl[] = {
    -100, 12, -62, 61, 17, -5, -30, 79, 64, 49, -66, 71, 42, 2, -40, -98,
    -82, -28, -71, -67, 97, -11, -77, -100, -99, -25, 6, 14, 20, 21, -67, 32,
    -10, -30, -89, 21, 56, 60, 3, -40, 75, 45, 91, 85, 7, -72, -8, -53, 72, -59,
    55, 68, 99, 99, 22, -22, -47, -41, 68, -96, -25, -82, 35, -89
  };

  for (int i = 0; i < 32; ++i) {
    CHECK(-1 != grid.alloc(i, pl[2 * i], pl[2 * i + 1]));
  }

  {
    TestCellsFilter f;
    grid.search(-10, 10, 72, 32, f);

    int const res[] = {2, 3, 6, 13, 14, 15, 16, 19, 23, 27, 28};
    std::vector<int> v(res, res + sizeof(res)/sizeof(res[0]));

    std::sort(f.v.begin(), f.v.end());
    CHECK(v == f.v);
  }

  {
    TestCellsFilter f;
    grid.search(-55, -63, 69, 32, f);

    int const res[] = {7, 8, 9, 11, 12, 16, 19, 22, 23, 28, 30};
    std::vector<int> v(res, res + sizeof(res)/sizeof(res[0]));

    std::sort(f.v.begin(), f.v.end());
    CHECK(v == f.v);
  }
}

TEST(Cells, search2_2)
{
  Cells<int> grid;
  grid.init(-100, -100, 10, 10, 20, 20);

  std::vector<IntPoint> pt;

  for (int i = 0; i < 128; ++i) {
    pt.push_back(IntPoint(rangeRand<int>(-100, 100 - 1), rangeRand<int>(-100, 100 - 1)));
    CHECK(-1 != grid.alloc(i, pt[i].x, pt[i].y));
  }

  for (int i = 0; i < 1024; ++i) {

    int x = rangeRand<int>(-100, 100 - 1), y = rangeRand<int>(-100, 100 - 1);
    int r = rangeRand<int>(30,80);

    TestCellsFilter f;
    grid.search(x, y, r, 50, f);

    int r2 = r * r;

    for (size_t j = 0; j < f.v.size(); ++j) {
      IntPoint const& p = pt[f.v[j]];
      CHECK(r2 >= (p.x - x) * (p.x - x) + (p.y - y) * (p.y - y));
    }
  }
}

// end of TestCells.cpp
