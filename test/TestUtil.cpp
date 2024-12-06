
//
//  Util unit test.
//
//  Copyright (c) 2008 Waync Cheng.
//  All Rights Reserved.
//
//  2008/05/26 Waync created.
//

#include <iterator>
#include <sstream>

#include "CppUnitLite/TestHarness.h"

#include "swArchive.h"
#include "swGeometry.h"
#include "swIni.h"
#include "swUtil.h"
using namespace sw2;

static const std::string sSampleText("Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.");
static const std::string sTestPassword("smallworld2");

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
  Util::loadFileContent("./data/widget.txt", str);

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
  Util::loadFileContent("./data/ThePoolOfTears.txt", str1);\
  std::string str2;\
  Util::loadFileContent("./data/test.txt", str2);\
  std::string out1, out2;\
  CHECK(ini1.storeToStream(out1));\
  CHECK(ini2.storeToStream(out2));

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
    CHECK(ar->addPathFileSystem(TEST_ADD_ZIP_FILE_NAME));

    std::stringstream ss1;
    CHECK(ar->loadFile("test.ini", ss1));

    Ini ini1_;
    CHECK(ini1_.loadFromStream(ss1.str()));

    std::string out1_;
    CHECK(ini1_.storeToStream(out1_));
    CHECK(out1_ == out1);

    Archive::free(ar);
  }

  //
  // Test 2.
  //

  v.clear();
  v.push_back("ThePoolOfTears.txt");

  CHECK(Util::zipArchive(false, TEST_ADD_ZIP_FILE_NAME, v, sTestPassword)); // Append with password.

  {
    Archive* ar = Archive::alloc();
    CHECK(ar->addPathFileSystem(TEST_ADD_ZIP_FILE_NAME));

    std::stringstream ss1;
    CHECK(ar->loadFile("test.ini", ss1)); // Existing item.

    std::stringstream ss2;
    CHECK(ar->loadFile("ThePoolOfTears.txt", ss2, sTestPassword));

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
    CHECK(ar->addPathFileSystem(TEST_ADD_ZIP_FILE_NAME));

    std::stringstream ss2;
    CHECK(ar->loadFile("widget.txt", ss2));

    Ini ini2_;
    CHECK(ini2_.loadFromStream(ss2.str()));

    std::string out2_;
    CHECK(ini2_.storeToStream(out2_));
    CHECK(out2_ == out2);

    Archive::free(ar);
  }

  //
  // Test 4.
  //

  v.clear();
  v.push_back("test.txt");

  CHECK(Util::zipArchive(false, TEST_ADD_ZIP_FILE_NAME, v, sTestPassword)); // Append with password.

  {
    Archive* ar = Archive::alloc();
    CHECK(ar->addPathFileSystem(TEST_ADD_ZIP_FILE_NAME));

    std::stringstream ss2;
    CHECK(ar->loadFile("test.txt", ss2, sTestPassword));

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
    CHECK(ar->addStreamFileSystem(stream.str()));

    std::stringstream ss1;
    CHECK(ar->loadFile("test.ini", ss1));

    Ini ini1_;
    CHECK(ini1_.loadFromStream(ss1.str()));

    std::string out1_;
    CHECK(ini1_.storeToStream(out1_));
    CHECK(out1_ == out1);

    Archive::free(ar);
  }

  //
  // Test 2.
  //

  v.clear();
  v.push_back("ThePoolOfTears.txt");

  std::stringstream stream2;
  CHECK(Util::zipStream("./data/", stream, stream2, v, sTestPassword)); // Append with password.

  {
    Archive* ar = Archive::alloc();
    CHECK(ar->addStreamFileSystem(stream2.str()));

    std::stringstream ss2;
    CHECK(ar->loadFile("ThePoolOfTears.txt", ss2, sTestPassword));

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
    CHECK(ar->addStreamFileSystem(stream3.str()));

    std::stringstream ss2;
    CHECK(ar->loadFile("widget.txt", ss2));

    Ini ini2_;
    CHECK(ini2_.loadFromStream(ss2.str()));

    std::string out2_;
    CHECK(ini2_.storeToStream(out2_));
    CHECK(out2_ == out2);

    Archive::free(ar);
  }

  //
  // Test 4.
  //

  v.clear();
  v.push_back("test.txt");

  std::stringstream stream4;
  CHECK(Util::zipStream("./data/", stream3, stream4, v, sTestPassword)); // Append with password.

  {
    Archive* ar = Archive::alloc();
    CHECK(ar->addStreamFileSystem(stream4.str()));

    std::stringstream ss2;
    CHECK(ar->loadFile("test.txt", ss2, sTestPassword));

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

  const char *s0 = "Test trace string variable";
  SW2_TRACE(s0);

  SW2_TRACE_RESET_TARGET();
  fclose(out);

  std::string s;
  Util::loadFileContent(FILE_NAME, s);
  ::remove(FILE_NAME);                  // Delete temp file.

  CHECK(s.npos != s.find(s0));
  CHECK(s.npos != s.find("[MESSAGE] TRACE1 messAge test 123"));
  CHECK(s.npos != s.find("[WARNING] TRACE2 wArning test 456"));
  CHECK(s.npos != s.find("[ERROR] TRACE3 errOr test 789"));
}

TEST(Util, TraceTool_level)
{
  const char *FILE_NAME = "tmltrace.txt";

  FILE* out = fopen(FILE_NAME, "wt");
  CHECK(out);

  SW2_TRACE_ADD_TARGET(out, 1);

  SW2_TRACE_MESSAGE_LEVEL(1, "[MESSAGE] TRACE1 messAge test %d", 123);
  SW2_TRACE_WARNING_LEVEL(1, "[WARNING] TRACE1 wArning test %d", 456);
  SW2_TRACE_ERROR_LEVEL(1, "[ERROR] TRACE1 errOr test %d", 789);
  SW2_TRACE_MESSAGE_LEVEL(2, "[MESSAGE] TRACE2 messAge test %d", 123);
  SW2_TRACE_WARNING_LEVEL(2, "[WARNING] TRACE2 wArning test %d", 456);
  SW2_TRACE_ERROR_LEVEL(2, "[ERROR] TRACE2 errOr test %d", 789);

  SW2_TRACE_RESET_TARGET();
  fclose(out);

  std::string s;
  Util::loadFileContent(FILE_NAME, s);
  ::remove(FILE_NAME);                  // Delete temp file.

  CHECK(s.npos != s.find("TRACE1 messAge test 123"));
  CHECK(s.npos != s.find("TRACE1 wArning test 456"));
  CHECK(s.npos != s.find("TRACE1 errOr test 789"));
  CHECK(s.npos == s.find("TRACE2 messAge test 123"));
  CHECK(s.npos == s.find("TRACE2 wArning test 456"));
  CHECK(s.npos == s.find("TRACE2 errOr test 789"));
}

TEST(Util, crc32)
{
  uint crc32 = 0;
  Util::crc32(crc32, sSampleText);
  CHECK(0x6b8edcbf == crc32);
}

TEST(Util, utf8ToUnicode)
{
  Archive* ar = Archive::alloc();
  CHECK(ar->addPathFileSystem("./data/utf8.zip"));

  std::stringstream ss1;
  CHECK(ar->loadFile("utf8.txt", ss1));

  std::stringstream ss2;
  CHECK(ar->loadFile("dec.txt", ss2));

  Archive::free(ar);

  std::vector<int> u;
  Util::utf8ToU32(ss1.str().c_str(), u);

  std::vector<int> u32;
  Util::split(ss2.str(), u32);
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

TEST(Util, fmtUpTime)
{
  char buff[64];
  time_t t0 = 60;
  CHECK(std::string("00:01:00") == Util::fmtUpTime(buff, sizeof(buff), &t0));
  time_t t1 = 60 * 60 + 60 + 5;
  CHECK(std::string("01:01:05") == Util::fmtUpTime(buff, sizeof(buff), &t1));
  time_t t2 = 60 * 60 * 24 + 1;
  CHECK(std::string("001d00:00:01") == Util::fmtUpTime(buff, sizeof(buff), &t2));
  time_t t3 = 60 * 60 * 24 * 365 + 60;
  CHECK(std::string("001y00:01:00") == Util::fmtUpTime(buff, sizeof(buff), &t3));
  time_t t4 = 60 * 60 * 24 * 500 + 60 * 60;
  CHECK(std::string("001y135d01:00:00") == Util::fmtUpTime(buff, sizeof(buff), &t4));
}

TEST(Util, fmtSizeByte)
{
  char buff[64];
  unsigned long long s0 = 1000;
  CHECK(std::string("1000") == Util::fmtSizeByte(buff, sizeof(buff), &s0));
  unsigned long long s1 = 4096;
  CHECK(std::string("4k") == Util::fmtSizeByte(buff, sizeof(buff), &s1));
  s1 = 5000;
  CHECK(std::string("4.88k") == Util::fmtSizeByte(buff, sizeof(buff), &s1));
  unsigned long long s2 = 4194304;
  CHECK(std::string("4m") == Util::fmtSizeByte(buff, sizeof(buff), &s2));
  s2 = 5000000;
  CHECK(std::string("4.77m") == Util::fmtSizeByte(buff, sizeof(buff), &s2));
  unsigned long long s3 = 4294967296;
  CHECK(std::string("4g") == Util::fmtSizeByte(buff, sizeof(buff), &s3));
  s3 = 5005001000;
  CHECK(std::string("4.66g") == Util::fmtSizeByte(buff, sizeof(buff), &s3));
}

TEST(Util, keystate)
{
  const uint UP = 1;
  const uint DOWN = 2;
  const uint LEFT = 4;
  const uint RIGHT = 8;

  KeyStates ks;

  CHECK(0 == ks.keys());
  CHECK(0 == ks.prevKeys());
  CHECK(!ks.isKeyDown(UP));
  CHECK(!ks.isKeyDown(DOWN));
  CHECK(!ks.isKeyDown(LEFT));
  CHECK(!ks.isKeyDown(RIGHT));
  CHECK(!ks.isKeyPressed(UP));
  CHECK(!ks.isKeyPressed(DOWN));
  CHECK(!ks.isKeyPressed(LEFT));
  CHECK(!ks.isKeyPressed(RIGHT));
  CHECK(!ks.isKeyPushed(UP));
  CHECK(!ks.isKeyPushed(DOWN));
  CHECK(!ks.isKeyPushed(LEFT));
  CHECK(!ks.isKeyPushed(RIGHT));

  ks.update(UP|LEFT);
  CHECK((UP|LEFT) == ks.keys());
  CHECK(0 == ks.prevKeys());
  CHECK(ks.isKeyDown(UP));
  CHECK(!ks.isKeyDown(DOWN));
  CHECK(ks.isKeyDown(LEFT));
  CHECK(!ks.isKeyDown(RIGHT));
  CHECK(ks.isKeyDown(UP|LEFT));
  CHECK(!ks.isKeyPressed(UP));
  CHECK(!ks.isKeyPressed(DOWN));
  CHECK(!ks.isKeyPressed(LEFT));
  CHECK(!ks.isKeyPressed(RIGHT));
  CHECK(ks.isKeyPushed(UP));
  CHECK(!ks.isKeyPushed(DOWN));
  CHECK(ks.isKeyPushed(LEFT));
  CHECK(!ks.isKeyPushed(RIGHT));

  ks.update(DOWN|RIGHT);
  CHECK((DOWN|RIGHT) == ks.keys());
  CHECK((UP|LEFT) == ks.prevKeys());
  CHECK(!ks.isKeyDown(UP));
  CHECK(ks.isKeyDown(DOWN));
  CHECK(!ks.isKeyDown(LEFT));
  CHECK(ks.isKeyDown(RIGHT));
  CHECK(ks.isKeyDown(DOWN|RIGHT));
  CHECK(ks.isKeyPressed(UP));
  CHECK(!ks.isKeyPressed(DOWN));
  CHECK(ks.isKeyPressed(LEFT));
  CHECK(!ks.isKeyPressed(RIGHT));
  CHECK(!ks.isKeyPushed(UP));
  CHECK(ks.isKeyPushed(DOWN));
  CHECK(!ks.isKeyPushed(LEFT));
  CHECK(ks.isKeyPushed(RIGHT));

  ks.update(0);
  CHECK(0 == ks.keys());
  CHECK((DOWN|RIGHT) == ks.prevKeys());
  CHECK(!ks.isKeyDown(UP));
  CHECK(!ks.isKeyDown(DOWN));
  CHECK(!ks.isKeyDown(LEFT));
  CHECK(!ks.isKeyDown(RIGHT));
  CHECK(!ks.isKeyPressed(UP));
  CHECK(ks.isKeyPressed(DOWN));
  CHECK(!ks.isKeyPressed(LEFT));
  CHECK(ks.isKeyPressed(RIGHT));
  CHECK(!ks.isKeyPushed(UP));
  CHECK(!ks.isKeyPushed(DOWN));
  CHECK(!ks.isKeyPushed(LEFT));
  CHECK(!ks.isKeyPushed(RIGHT));
}

// end of TestUtil.cpp
