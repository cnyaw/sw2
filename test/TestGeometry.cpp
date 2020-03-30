
//
//  Geometry unit test.
//
//  Copyright (c) 2009 Waync Cheng.
//  All Rights Reserved.
//
//  2009/10/31 Waync created.
//

#include "CppUnitLite/TestHarness.h"

#include "swGeometry.h"
using namespace sw2;

//
// Test POINT_t initial value.
//

TEST(POINT_t, init)
{
  IntPoint pt1;
  CHECK(0 == pt1.x);
  CHECK(0 == pt1.y);

  FloatPoint pt2;
  CHECK(.0f == pt2.x);
  CHECK(.0f == pt2.y);
}

//
// Test RECT_t initial value.
//

TEST(RECT_t, init)
{
  IntRect rc1;
  CHECK(0 == rc1.left);
  CHECK(0 == rc1.top);
  CHECK(0 == rc1.right);
  CHECK(0 == rc1.bottom);
  CHECK(0 == rc1.width());
  CHECK(0 == rc1.height());

  FloatRect rc2;
  CHECK(.0f == rc2.left);
  CHECK(.0f == rc2.top);
  CHECK(.0f == rc2.right);
  CHECK(.0f == rc2.bottom);
  CHECK(.0f == rc2.width());
  CHECK(.0f == rc2.height());
}

//
// Test empty RECT_t.
//

TEST(RECT_t, empty)
{
  {
    IntRect rc1(0,0,0,0);
    IntRect rc2(1,0,0,0);
    IntRect rc3(0,2,0,0);
    IntRect rc4(0,0,3,0);
    IntRect rc5(0,0,0,4);
    IntRect rc6(1,1,0,0);
    IntRect rc7(2,0,2,2);
    IntRect rc8(3,4,5,6);
    CHECK(rc1.empty());
    CHECK(!rc2.empty());
    CHECK(!rc3.empty());
    CHECK(!rc4.empty());
    CHECK(!rc5.empty());
    CHECK(!rc6.empty());
    CHECK(!rc7.empty());
    CHECK(!rc8.empty());

    rc2.setEmpty();
    rc3.setEmpty();
    rc4.setEmpty();
    rc5.setEmpty();
    rc6.setEmpty();
    rc7.setEmpty();
    rc8.setEmpty();
    CHECK(rc2.empty());
    CHECK(rc3.empty());
    CHECK(rc4.empty());
    CHECK(rc5.empty());
    CHECK(rc6.empty());
    CHECK(rc7.empty());
    CHECK(rc8.empty());
  }

  {
    FloatRect rc1(.0f, .0f, .0f, .0f);
    FloatRect rc2(1.0f, .0f, .0f, .0f);
    FloatRect rc3(.0f, 2.0f, .0f, .0f);
    FloatRect rc4(.0f, .0f, 3.0f, .0f);
    FloatRect rc5(.0f, .0f, .0f, .40f);
    FloatRect rc6(.10f, 2.0f, .0f, .0f);
    FloatRect rc7(.0f, 2.0f, 3.0f, .0f);
    FloatRect rc8(4.0f, 1.0f, .0f, 13.0f);
    CHECK(rc1.empty());
    CHECK(!rc2.empty());
    CHECK(!rc3.empty());
    CHECK(!rc4.empty());
    CHECK(!rc5.empty());
    CHECK(!rc6.empty());
    CHECK(!rc7.empty());
    CHECK(!rc8.empty());

    rc2.setEmpty();
    rc3.setEmpty();
    rc4.setEmpty();
    rc5.setEmpty();
    rc6.setEmpty();
    rc7.setEmpty();
    rc8.setEmpty();
    CHECK(rc2.empty());
    CHECK(rc3.empty());
    CHECK(rc4.empty());
    CHECK(rc5.empty());
    CHECK(rc6.empty());
    CHECK(rc7.empty());
    CHECK(rc8.empty());
  }
}

//
// Test scale RECT_t.
//

TEST(RECT_t, inflate)
{
  IntRect rc1(0,0,10,10);
  CHECK(10 == rc1.width());
  CHECK(10 == rc1.height());
  rc1.inflate(10, 0);
  CHECK(30 == rc1.width());
  CHECK(10 == rc1.height());
  rc1.inflate(0, 10);
  CHECK(30 == rc1.width());
  CHECK(30 == rc1.height());
  rc1.inflate(-10, 0);
  CHECK(10 == rc1.width());
  CHECK(30 == rc1.height());
  rc1.inflate(0, -10);
  CHECK(10 == rc1.width());
  CHECK(10 == rc1.height());

  FloatRect rc2(.0f, .0f, 10.0f, 10.0f);
  CHECK(10.0f == rc2.width());
  CHECK(10.0f == rc2.height());
  rc2.inflate(10.0f, .0f);
  CHECK(30.0f == rc2.width());
  CHECK(10.0f == rc2.height());
  rc2.inflate(.0f, 10.0f);
  CHECK(30.0f == rc2.width());
  CHECK(30.0f == rc2.height());
  rc2.inflate(-10.0f, .0f);
  CHECK(10.0f == rc2.width());
  CHECK(30.0f == rc2.height());
  rc2.inflate(.0f, -10.0f);
  CHECK(10.0f == rc2.width());
  CHECK(10.0f == rc2.height());
}

//
// Test offset RECT_t.
//

TEST(RECT_t, offset)
{
  IntRect rc1(0,0,10,10);
  CHECK(10 == rc1.width());
  CHECK(10 == rc1.height());
  rc1.offset(10, 0);
  CHECK(10 == rc1.width());
  CHECK(10 == rc1.height());
  rc1.offset(0, 10);
  CHECK(10 == rc1.width());
  CHECK(10 == rc1.height());
  rc1.offset(-10, 0);
  CHECK(10 == rc1.width());
  CHECK(10 == rc1.height());
  rc1.offset(0, -10);
  CHECK(10 == rc1.width());
  CHECK(10 == rc1.height());

  FloatRect rc2(.0f, .0f, 10.0f, 10.0f);
  CHECK(10.0f == rc2.width());
  CHECK(10.0f == rc2.height());
  rc2.offset(10.0f, 0);
  CHECK(10.0f == rc2.width());
  CHECK(10.0f == rc2.height());
  rc2.offset(0, 10.0f);
  CHECK(10.0f == rc2.width());
  CHECK(10.0f == rc2.height());
  rc2.offset(-10.0f, 0);
  CHECK(10.0f == rc2.width());
  CHECK(10.0f == rc2.height());
  rc2.offset(0, -10.0f);
  CHECK(10.0f == rc2.width());
  CHECK(10.0f == rc2.height());
}

//
// Test RECT_t equality.
//

TEST(RECT_t, equal)
{
  CHECK(IntRect() == IntRect());
  CHECK(IntRect(1,2,3,4) == IntRect(1,2,3,4));
  CHECK(IntRect(-1,2,-3,4) == IntRect(-1,2,-3,4));

  CHECK(FloatRect() == FloatRect());
  CHECK(FloatRect(1.0f, 2.0f, 3.0f, 4.0f) == FloatRect(1.0f, 2.0f, 3.0f, 4.0f));
  CHECK(FloatRect(-1.0f, 2.0f, -3.0f, 4.0f) == FloatRect(-1.0f, 2.0f, -3.0f, 4.0f));
}

//
// Test intersect POINT_t with RECT_t.
//

TEST(RECT_t, intersectPoint)
{
  IntRect rc1(0,0,10,10);
  CHECK(rc1.ptInRect(IntPoint(0,0)));
  CHECK(!rc1.ptInRect(IntPoint(0,10)));
  CHECK(!rc1.ptInRect(IntPoint(10,0)));
  CHECK(!rc1.ptInRect(IntPoint(10,10)));
  CHECK(!rc1.ptInRect(IntPoint(0,-1)));
  CHECK(!rc1.ptInRect(IntPoint(-1,0)));
  CHECK(!rc1.ptInRect(IntPoint(10,-1)));
  CHECK(!rc1.ptInRect(IntPoint(11,0)));
  CHECK(!rc1.ptInRect(IntPoint(0,11)));
  CHECK(!rc1.ptInRect(IntPoint(-1,10)));
  CHECK(!rc1.ptInRect(IntPoint(10,11)));
  CHECK(!rc1.ptInRect(IntPoint(11,10)));

  FloatRect rc2(.0f, .0f, 10.0f, 10.0f);
  CHECK(rc2.ptInRect(FloatPoint(.0f, .0f)));
  CHECK(!rc2.ptInRect(FloatPoint(.0f, 10.0f)));
  CHECK(!rc2.ptInRect(FloatPoint(10.0f, .0f)));
  CHECK(!rc2.ptInRect(FloatPoint(10.0f, 10.0f)));
  CHECK(!rc2.ptInRect(FloatPoint(.0f, -1.0f)));
  CHECK(!rc2.ptInRect(FloatPoint(-1.0f, .0f)));
  CHECK(!rc2.ptInRect(FloatPoint(10.0f, -1.0f)));
  CHECK(!rc2.ptInRect(FloatPoint(11.0f, .0f)));
  CHECK(!rc2.ptInRect(FloatPoint(.0f, 11.0f)));
  CHECK(!rc2.ptInRect(FloatPoint(-1.0f, 10.0f)));
  CHECK(!rc2.ptInRect(FloatPoint(10.0f, 11.0f)));
  CHECK(!rc2.ptInRect(FloatPoint(11.0f, 10.0f)));
}

//
// Test RECT_t intersect.
//

TEST(RECT_t, intersectRect)
{
  IntRect rc1(0,0,10,10);
  CHECK(rc1.intersect(IntRect(0,0,10,10)));
  CHECK(rc1.intersect(IntRect(5,0,10,10)));
  CHECK(rc1.intersect(IntRect(0,5,10,10)));
  CHECK(rc1.intersect(IntRect(0,0,5,10)));
  CHECK(rc1.intersect(IntRect(0,0,10,5)));
  CHECK(!rc1.intersect(IntRect(5,-10,15,-5)));
  CHECK(!rc1.intersect(IntRect(-10,-5,-5,5)));
  CHECK(!rc1.intersect(IntRect(-5,15,5,20)));
  CHECK(!rc1.intersect(IntRect(15,5,20,15)));

  FloatRect rc2(.0f, .0f, 10.0f, 10.0f);
  CHECK(rc2.intersect(FloatRect(.0f, .0f, 10.0f, 10.0f)));
  CHECK(rc2.intersect(FloatRect(5.0f, .0f, 10.0f, 10.0f)));
  CHECK(rc2.intersect(FloatRect(.0f, 5.0f, 10.0f, 10.0f)));
  CHECK(rc2.intersect(FloatRect(.0f, .0f, 5.0f, 10.0f)));
  CHECK(rc2.intersect(FloatRect(.0f, .0f, 10.0f, 5.0f)));
  CHECK(!rc2.intersect(FloatRect(5.0f, -10.0f, 15.0f, -5.0f)));
  CHECK(!rc2.intersect(FloatRect(-10.0f, -5.0f, -5.0f, 5.0f)));
  CHECK(!rc2.intersect(FloatRect(-5.0f, 15.0f, 5.0f, 20.0f)));
  CHECK(!rc2.intersect(FloatRect(15.0f, 5.0f, 20.0f, 15.0f)));
}

TEST(RECT_t, contain)
{
  IntRect r(0, 0, 100, 100);
  
  CHECK(!r.contain(IntRect(0, 110, 100, 200)));
  CHECK(!r.contain(IntRect(110, 0, 200, 100)));
  CHECK(!r.contain(IntRect(-110, 0, -110, 200)));
  CHECK(!r.contain(IntRect(0, -110, 100, -10)));

  CHECK(!r.contain(IntRect(0, 50, 100, 150)));
  CHECK(!r.contain(IntRect(50, 50, 150, 150)));
  CHECK(!r.contain(IntRect(-50, 0, 50, 100)));
  CHECK(!r.contain(IntRect(0, -50, 100, 50)));

  CHECK(r.contain(IntRect(10, 10, 90, 90)));
  CHECK(r.contain(r));
}

// end of TestGeometry.cpp
