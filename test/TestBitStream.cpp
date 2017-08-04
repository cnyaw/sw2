
//
//  BitStream unit test.
//
//  Copyright (c) 2008 Waync Cheng.
//  All Rights Reserved.
//
//  2008/05/25 Waync created.
//

#include <string>

#include "CppUnitLite/TestHarness.h"

#include "swBitStream.h"
#include "swUtil.h"
using namespace sw2;

//
// Initial state check.
//

TEST(BitStream, init)
{
  char s[256];
  BitStream bs(s, sizeof(s));

  CHECK(bs);
  CHECK(!bs.fail());
  CHECK(bs.getByteCount() == 0);
}

//
// Out of boundary check.
//

TEST(BitStream, outOfRange)
{
  //
  // Test write.
  //

  char s[1];
  BitStream bs(s, sizeof(s));

  for (int offset = 0; offset < 8; offset++) {
    bs.setPtr(0, offset);
    bs << 10;
    CHECK(bs.fail());
  }

  //
  // Test read.
  //

  char s2[1];
  BitStream bs2(s2, sizeof(s2));

  for (int offset = 0; offset < 8; offset++) {

    bs2.setPtr(0, offset);
    bs2 << setBitCount(8) << 10;

    int n = 0;
    bs2 >> n;

    CHECK(bs2.fail());
  }
}

//
// Read/write test.
//

TEST(BitStream, readwrite)
{
  char s[256];
  BitStream bs(s, sizeof(s));

  //
  // Test read and write.
  //

  for (int offset = 0; offset < 8; offset++) {

    //
    // Reset bit stream.
    //

    bs.setPtr(0, offset);

    //
    // Write uint.
    //

    for (int i = 1; i <= 32; i++) {
      sw2::uint u = 1 << (i - 1);
      bs << setBitCount(i) << u;
    }

    //
    // Write int.
    //

    for (int i = 2; i <= 32; i++) {
      int s = (i & 1) ? 1 : -1;
      bs << setBitCount(i) << s;
    }

    //
    // Write bool.
    //

    bool b1 = false, b2 = true;
    bs << b1 << b2;

    //
    // Write float.
    //

    float f1 = 3.1415926f, f2 = - 1.414f;
    bs << f1 << f2;

    //
    // Write std::string.
    //

    std::string s1("this is a string"), s2("yet another string");
    bs << setBitCount(24 + offset) << s1 << setBitCount(24 + offset) << s2;

    //
    // Reset bit stream.
    //

    bs.setPtr(0, offset);

    //
    // Note: The order of read operations must match the order of write, also
    // the data type and bit count should match.
    //

    //
    // Read uint.
    //

    for (int i = 1; i <= 32; i++) {
      sw2::uint u;
      bs >> setBitCount(i) >> u;
      CHECK(bs && u == (sw2::uint)(1 << (i - 1)));
    }

    //
    // Read int.
    //

    for (int i = 2; i <= 32; i++) {
      int s;
      bs >> setBitCount(i) >> s;
      CHECK(bs && s == (i & 1) ? 1 : -1);
    }

    //
    // Read bool.
    //

    bool b3, b4;
    bs >> b3;
    CHECK(bs && b1 == b3);
    bs >> b4;
    CHECK(bs && b2 == b4);

    //
    // Read float.
    //

    float f3, f4;
    bs >> f3;
    CHECK(bs && f1 == f3);
    bs >> f4;
    CHECK(bs && f2 == f4);

    //
    // Read std::string.
    //

    std::string s3, s4;
    bs >> setBitCount(24 + offset) >> s3;
    CHECK(bs && s1 == s3);
    bs >> setBitCount(24 + offset) >> s4;
    CHECK(bs && s2 == s4);
  }
}

TEST(BitStream, growbuff)
{
  std::string s;
  BitStream bs(s);

  const unsigned int COUNT = 5000;
  for (unsigned int i = 0; i < COUNT; i++) {
    bs << setBitCount(Util::getBitCount(i)) << i;
  }

  bs.setPtr(0, 0);

  for (unsigned int i = 0; i < COUNT; i++) {
    unsigned int u;
    bs >> setBitCount(Util::getBitCount(i)) >> u;
    CHECK(bs && u == i)
  }
}

// end of TestBitStream.cpp
