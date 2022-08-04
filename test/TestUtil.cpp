
//
//  Util unit test.
//
//  Copyright (c) 2008 Waync Cheng.
//  All Rights Reserved.
//
//  2008/05/26 Waync created.
//

#include <fstream>
#include <iterator>
#include <sstream>

#include "CppUnitLite/TestHarness.h"

#include "swArchive.h"
#include "swGeometry.h"
#include "swIni.h"
#include "swUtil.h"
using namespace sw2;

static std::string const sSampleText("Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.");

//
// Test clamp.
//

TEST(Util, clamp)
{
  CHECK(10 == Util::clamp<int>(10, 5, 20));
  CHECK(5 == Util::clamp<int>(5, 5, 20));
  CHECK(20 == Util::clamp<int>(20, 5, 20));
  CHECK(5 == Util::clamp<int>(3, 5, 20));
  CHECK(20 == Util::clamp<int>(26, 5, 20));

  CHECK(10.0f == Util::clamp<float>(10.0f, 5.0f, 20.0f));
  CHECK(5.0f == Util::clamp<float>(5.0f, 5.0f, 20.0f));
  CHECK(20.0f == Util::clamp<float>(20.0f, 5.0f, 20.0f));
  CHECK(5.0f == Util::clamp<float>(3.0f, 5.0f, 20.0f));
  CHECK(20.0f == Util::clamp<float>(26.0f, 5.0f, 20.0f));

  CHECK(10.0 == Util::clamp<double>(10.0, 5.0, 20.0));
  CHECK(5.0 == Util::clamp<double>(5.0, 5.0, 20.0));
  CHECK(20.0 == Util::clamp<double>(20.0, 5.0, 20.0));
  CHECK(5.0 == Util::clamp<double>(3.0, 5.0, 20.0));
  CHECK(20.0 == Util::clamp<double>(26.0, 5.0, 20.0));
}

//
// Test BITCOUNT.
//

TEST(Util, BITCOUNT)
{
  CHECK(1 == BITCOUNT<0>::value);
  CHECK(1 == BITCOUNT<1>::value);
  CHECK(2 == BITCOUNT<2>::value);
  CHECK(2 == BITCOUNT<3>::value);
  CHECK(3 == BITCOUNT<4>::value);
  CHECK(3 == BITCOUNT<6>::value);
  CHECK(4 == BITCOUNT<8>::value);
  CHECK(4 == BITCOUNT<12>::value);
  CHECK(5 == BITCOUNT<16>::value);
  CHECK(6 == BITCOUNT<32>::value);
  CHECK(7 == BITCOUNT<64>::value);
  CHECK(8 == BITCOUNT<128>::value);
  CHECK(8 == BITCOUNT<255>::value);
  CHECK(11 == BITCOUNT<1024>::value);
  CHECK(16 == BITCOUNT<65535>::value);
  CHECK(32 == BITCOUNT<4294967295U>::value);
}

//
// Test bitCount.
//

TEST(Util, bitCount)
{
  CHECK(1 == Util::getBitCount(0));
  CHECK(1 == Util::getBitCount(1));
  CHECK(2 == Util::getBitCount(2));
  CHECK(2 == Util::getBitCount(3));
  CHECK(3 == Util::getBitCount(4));
  CHECK(3 == Util::getBitCount(6));
  CHECK(4 == Util::getBitCount(8));
  CHECK(4 == Util::getBitCount(12));
  CHECK(5 == Util::getBitCount(16));
  CHECK(6 == Util::getBitCount(32));
  CHECK(7 == Util::getBitCount(64));
  CHECK(8 == Util::getBitCount(128));
  CHECK(8 == Util::getBitCount(255));
  CHECK(11 == Util::getBitCount(1024));
  CHECK(16 == Util::getBitCount(65535));
  CHECK(32 == Util::getBitCount(4294967295U));
}

//
// Test base64.
//

TEST(Util, base64)
{
  std::string const sText2("TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=");

  std::string os1;
  if (!Util::base64(sSampleText, os1)) {
    FAIL("base64");
  }

  CHECK(os1 == sText2);

  std::string os2;
  if (!Util::unbase64(os1, os2)) {
    FAIL("unbase64");
  }

  CHECK(os2 == sSampleText);
}

//
// Test zip/unzip.
//

TEST(Util, zip_unzip)
{
  std::string str;

  { // get test data
    std::ifstream ifs("./data/widget.txt");
    std::stringstream ss;
    ss << ifs.rdbuf();
    str = ss.str();
    ifs.close();
  }

  for (int i = -1; i < 10; ++i) {
    std::stringstream ss(str), os, os2;
    CHECK(Util::zip(ss, os, i));
    CHECK(Util::unzip(os, os2));
    CHECK(os2.str() == str);
  }
}

//
// Test zipArchive.
//

#define INIT_TEST_UTIL_ZIP_ARCHIVE()\
  Ini ini1, ini2;\
  CHECK(ini1.load("./data/test.ini"));\
  CHECK(ini2.load("./data/widget.txt"));\
  std::string str1;\
  {\
    std::ifstream ifs("./data/ThePoolOfTears.txt", std::ios_base::binary);\
    std::stringstream ss;\
    ss << ifs.rdbuf();\
    ifs.close();\
    str1 = ss.str();\
  }\
  std::string str2;\
  {\
    std::ifstream ifs("./data/test.txt", std::ios_base::binary);\
    std::stringstream ss;\
    ss << ifs.rdbuf();\
    ifs.close();\
    str2 = ss.str();\
  }\
  std::stringstream out1, out2;\
  CHECK(ini1.store(out1));\
  CHECK(ini2.store(out2));

TEST(Util, zipArchive)
{
  INIT_TEST_UTIL_ZIP_ARCHIVE();

  //
  // Test 1.
  //

  const std::string TEST_ADD_ZIP_FILE_NAME = "./data/testAddZip.zip";

  std::vector<std::string> v;
  v.push_back("test.ini");

  CHECK(Util::zipArchive(true, TEST_ADD_ZIP_FILE_NAME, v)); // Create new.

  {
    Archive* ar = Archive::alloc();
    CHECK(ar->addFileSystem(TEST_ADD_ZIP_FILE_NAME));

    std::stringstream ss1;
    CHECK(ar->loadFile("test.ini", ss1));

    Ini ini1_;
    CHECK(ini1_.load(ss1));

    std::stringstream out1_;
    CHECK(ini1_.store(out1_));
    CHECK(out1_.str() == out1.str());

    Archive::free(ar);
  }

  //
  // Test 2.
  //

  v.clear();
  v.push_back("ThePoolOfTears.txt");

  CHECK(Util::zipArchive(false, TEST_ADD_ZIP_FILE_NAME, v, "smallworld2")); // Append with password.

  {
    Archive* ar = Archive::alloc();
    CHECK(ar->addFileSystem(TEST_ADD_ZIP_FILE_NAME));

    std::stringstream ss1;
    CHECK(ar->loadFile("test.ini", ss1)); // Existing item.

    std::stringstream ss2;
    CHECK(ar->loadFile("ThePoolOfTears.txt", ss2, "smallworld2"));

    std::string str1_ = ss2.str();
    CHECK(str1_ == str1);

    Archive::free(ar);
  }

  //
  // Test 3.
  //

  v.clear();
  v.push_back("widget.txt");

  CHECK(Util::zipArchive(false, TEST_ADD_ZIP_FILE_NAME, v)); // Append.

  {
    Archive* ar = Archive::alloc();
    CHECK(ar->addFileSystem(TEST_ADD_ZIP_FILE_NAME));

    std::stringstream ss2;
    CHECK(ar->loadFile("widget.txt", ss2));

    Ini ini2_;
    CHECK(ini2_.load(ss2));

    std::stringstream out2_;
    CHECK(ini2_.store(out2_));
    CHECK(out2_.str() == out2.str());

    Archive::free(ar);
  }

  //
  // Test 4.
  //

  v.clear();
  v.push_back("test.txt");

  CHECK(Util::zipArchive(false, TEST_ADD_ZIP_FILE_NAME, v, "smallworld2")); // Append with password.

  {
    Archive* ar = Archive::alloc();
    CHECK(ar->addFileSystem(TEST_ADD_ZIP_FILE_NAME));

    std::stringstream ss2;
    CHECK(ar->loadFile("test.txt", ss2, "smallworld2"));

    std::string str2_ = ss2.str();
    CHECK(str2_ == str2);

    Archive::free(ar);
  }

  remove(TEST_ADD_ZIP_FILE_NAME.c_str());       // Delete test file.
}

TEST(Util, zipArchive2)
{
  INIT_TEST_UTIL_ZIP_ARCHIVE();

  //
  // Test 1.
  //

  std::vector<std::string> v;
  v.push_back("test.ini");

  std::stringstream dummy, stream;
  CHECK(Util::zipStream("./data/", dummy, stream, v)); // Create new.

  {
    Archive* ar = Archive::alloc();
    CHECK(ar->addFileSystem(stream));

    std::stringstream ss1;
    CHECK(ar->loadFile("test.ini", ss1));

    Ini ini1_;
    CHECK(ini1_.load(ss1));

    std::stringstream out1_;
    CHECK(ini1_.store(out1_));
    CHECK(out1_.str() == out1.str());

    Archive::free(ar);
  }

  //
  // Test 2.
  //

  v.clear();
  v.push_back("ThePoolOfTears.txt");

  std::stringstream stream2;
  CHECK(Util::zipStream("./data/", stream, stream2, v, "smallworld2")); // Append with password.

  {
    Archive* ar = Archive::alloc();
    CHECK(ar->addFileSystem(stream2));

    std::stringstream ss2;
    CHECK(ar->loadFile("ThePoolOfTears.txt", ss2, "smallworld2"));

    std::string str1_ = ss2.str();
    CHECK(str1_ == str1);

    Archive::free(ar);
  }

  //
  // Test 3.
  //

  v.clear();
  v.push_back("widget.txt");

  std::stringstream stream3;
  CHECK(Util::zipStream("./data/", stream2, stream3, v)); // Append.

  {
    Archive* ar = Archive::alloc();
    CHECK(ar->addFileSystem(stream3));

    std::stringstream ss2;
    CHECK(ar->loadFile("widget.txt", ss2));

    Ini ini2_;
    CHECK(ini2_.load(ss2));

    std::stringstream out2_;
    CHECK(ini2_.store(out2_));
    CHECK(out2_.str() == out2.str());

    Archive::free(ar);
  }

  //
  // Test 4.
  //

  v.clear();
  v.push_back("test.txt");

  std::stringstream stream4;
  CHECK(Util::zipStream("./data/", stream3, stream4, v, "smallworld2")); // Append with password.

  {
    Archive* ar = Archive::alloc();
    CHECK(ar->addFileSystem(stream4));

    std::stringstream ss2;
    CHECK(ar->loadFile("test.txt", ss2, "smallworld2"));

    std::string str2_ = ss2.str();
    CHECK(str2_ == str2);

    Archive::free(ar);
  }
}

TEST(Util, TimeoutTimer)
{
  TimeoutTimer tt(1000);
  CHECK(!tt.isExpired());

  Util::sleep(250);
  CHECK(!tt.isExpired());

  Util::sleep(250);
  CHECK(!tt.isExpired());

  Util::sleep(800);
  CHECK(tt.isExpired());
}

TEST(Util, TraceTool)
{
  const char *FILE_NAME = "tmltrace.txt";

  FILE* out = fopen(FILE_NAME, "wt");
  CHECK(out);

  SW2_TRACE_ADD_TARGET(out);

  SW2_TRACE_MESSAGE("TRACE1 messAge test %d", 123);
  SW2_TRACE_WARNING("TRACE2 wArning test %d", 456);
  SW2_TRACE_ERROR("TRACE3 errOr test %d", 789);

  SW2_TRACE_RESET_TARGET();
  fclose(out);

  std::ifstream ifs(FILE_NAME);
  std::stringstream ss;

  ss << ifs.rdbuf();
  ifs.close();

  ::remove(FILE_NAME);                  // Delete temp file.

  std::string s = ss.str();
  CHECK(s.npos != s.find("TRACE1 messAge test 123"));
  CHECK(s.npos != s.find("TRACE2 wArning test 456"));
  CHECK(s.npos != s.find("TRACE3 errOr test 789"));
}

TEST(Util, TraceTool_level)
{
  const char *FILE_NAME = "tmltrace.txt";

  FILE* out = fopen(FILE_NAME, "wt");
  CHECK(out);

  SW2_TRACE_ADD_TARGET(out, 1);

  SW2_TRACE_MESSAGE_LEVEL(1, "TRACE1 messAge test %d", 123);
  SW2_TRACE_WARNING_LEVEL(1, "TRACE1 wArning test %d", 456);
  SW2_TRACE_ERROR_LEVEL(1, "TRACE1 errOr test %d", 789);
  SW2_TRACE_MESSAGE_LEVEL(2, "TRACE2 messAge test %d", 123);
  SW2_TRACE_WARNING_LEVEL(2, "TRACE2 wArning test %d", 456);
  SW2_TRACE_ERROR_LEVEL(2, "TRACE2 errOr test %d", 789);

  SW2_TRACE_RESET_TARGET();
  fclose(out);

  std::ifstream ifs(FILE_NAME);
  std::stringstream ss;

  ss << ifs.rdbuf();
  ifs.close();

  ::remove(FILE_NAME);                  // Delete temp file.

  std::string s = ss.str();
  CHECK(s.npos != s.find("TRACE1 messAge test 123"));
  CHECK(s.npos != s.find("TRACE1 wArning test 456"));
  CHECK(s.npos != s.find("TRACE1 errOr test 789"));
  CHECK(s.npos == s.find("TRACE2 messAge test 123"));
  CHECK(s.npos == s.find("TRACE2 wArning test 456"));
  CHECK(s.npos == s.find("TRACE2 errOr test 789"));
}

TEST(Util, crc32)
{
  std::stringstream ss(sSampleText);
  uint crc32 = 0;
  Util::crc32(crc32, ss);
  CHECK(0x6b8edcbf == crc32);
}

TEST(Util, utf8ToUnicode)
{
  Archive* ar = Archive::alloc();
  CHECK(ar->addFileSystem("./data/utf8.zip"));

  std::stringstream ss1;
  CHECK(ar->loadFile("utf8.txt", ss1));

  std::stringstream ss2;
  CHECK(ar->loadFile("dec.txt", ss2));

  Archive::free(ar);

  std::vector<int> u;
  Util::utf8ToU32(ss1.str().c_str(), u);

  std::vector<int> u32;
  u32.assign(std::istream_iterator<int>(ss2), std::istream_iterator<int>());
  CHECK(u == u32);
}

TEST(Util, toLowerStr)
{
  std::string a("Util::toLowerString");
  Util::toLowerString(a);
  CHECK("util::tolowerstring" == a);

  std::string b("util::tolowerstring");
  Util::toLowerString(b);
  CHECK("util::tolowerstring" == b);

  std::string c("UTIL::TOLOWERSTRING");
  Util::toLowerString(c);
  CHECK("util::tolowerstring" == c);
}

// end of TestUtil.cpp
