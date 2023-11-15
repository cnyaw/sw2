
//
//  INI unit test.
//
//  Copyright (c) 2008 Waync Cheng.
//  All Rights Reserved.
//
//  2008/05/26 Waync created.
//

#include "CppUnitLite/TestHarness.h"

#include "swIni.h"
#include "swUtil.h"
using namespace sw2;

TEST(Ini, init)
{
  Ini ini;
  CHECK(0 == ini.size());

  Ini sec;
  CHECK(0 == sec.size());
  CHECK(sec.key.empty());

  Ini item;
  CHECK(item.key.empty());
  CHECK(item.value.empty());
}

TEST(Ini, load)
{
  Ini ini;
  CHECK(0 == ini.size());

  CHECK(ini.load("./data/test.ini"));
  CHECK(4 == ini.size());

  CHECK(0 != ini.find("sec1"));
  CHECK(4 == ini["sec1"].size());
  CHECK(0 != ini["sec1"].find("item0"));
  CHECK(0 != ini["sec1"].find("item1"));
  CHECK(0 != ini["sec1"].find("item2"));
  CHECK(0 != ini["sec1"].find("item3"));
  CHECK(0 == (int)ini["sec1"]["item0"]);
  CHECK(1 == (int)ini["sec1"]["item1"]);
  CHECK(2 == (int)ini["sec1"]["item2"]);
  CHECK(0 == (int)ini["sec1"]["item3"]);

  CHECK(0 != ini.find("sec5"));
  CHECK(5 == ini["sec5"].size());
  CHECK(0 != ini["sec5"].find("i0"));
  CHECK(0 != ini["sec5"].find("i1"));
  CHECK(0 != ini["sec5"].find("i2"));
  CHECK(0 != ini["sec5"].find("i3"));
  CHECK(0 != ini["sec5"].find("i4"));
  CHECK(1 == (int)ini["sec5"]["i0"]);
  CHECK("test" == ini["sec5"]["i1"].value);
  CHECK(1.4f == (float)ini["sec5"]["i2"]);
  CHECK(1.8 == (double)ini["sec5"]["i3"]);
  CHECK(12 == (int)ini["sec5"]["i4"]);

  CHECK(0 != ini.find("sec7"));
  CHECK(1 == ini["sec7"].size());
  CHECK(0 != ini["sec7"].find("item0"));
  CHECK(0 == (int)ini["sec7"]["item0"]);

  CHECK(" this is string1" == ini["sec8"]["s1"].value);
  CHECK("this is string2 " == ini["sec8"]["s2"].value);
  CHECK("'string3'" == ini["sec8"]["s3"].value);
  CHECK("\"string4\"" == ini["sec8"]["s4"].value);
}

static const wchar_t UNICODE_SEC_NAME[] = L"這是中文";
static const wchar_t UNICODE_ITEM_1[] = L"这是项目一";
static const wchar_t UNICODE_ITEM_2[] = L"これは、プロジェクトIIです";
static const wchar_t UNICODE_ITEM_3[] = L"이 프로젝트 III입니다";
static const wchar_t UNICODE_ITEM_4[] = L"Это четыре товара";
static const wchar_t UNICODE_ITEM_5[] = L"นี้เป็นโครงการที่ห้า";

#define GET_UNICODE_ARR(w) std::vector<int>(w, w + sizeof(w)/sizeof(w[0]) - 1)

TEST(Ini, loadutf8)
{
  Ini ini;
  CHECK(0 == ini.size());

  CHECK(ini.load("./data/testw.ini"));
  CHECK(1 == ini.size());

  const Ini &sec = ini.items[0];
  CHECK(5 == sec.size());

  std::vector<int> secName;
  Util::utf8ToU32(sec.key.c_str(), secName);
  CHECK(secName == GET_UNICODE_ARR(UNICODE_SEC_NAME));

  std::vector<int> item1;
  Util::utf8ToU32(sec["1"].value.c_str(), item1);
  CHECK(item1 == GET_UNICODE_ARR(UNICODE_ITEM_1));

  std::vector<int> item2;
  Util::utf8ToU32(sec["2"].value.c_str(), item2);
  CHECK(item2 == GET_UNICODE_ARR(UNICODE_ITEM_2));

  std::vector<int> item3;
  Util::utf8ToU32(sec["3"].value.c_str(), item3);
  CHECK(item3 == GET_UNICODE_ARR(UNICODE_ITEM_3));

  std::vector<int> item4;
  Util::utf8ToU32(sec["4"].value.c_str(), item4);
  CHECK(item4 == GET_UNICODE_ARR(UNICODE_ITEM_4));

  std::vector<int> item5;
  Util::utf8ToU32(sec["5"].value.c_str(), item5);
  CHECK(item5 == GET_UNICODE_ARR(UNICODE_ITEM_5));
}

std::string getUtf8Str(const std::vector<int> &v)
{
  std::string s;
  Util::u32ToUtf8(v, s);
  return s;
}

#define GET_UTF8_STR(w) getUtf8Str(std::vector<int>(w, w + sizeof(w)/sizeof(w[0]) - 1))

TEST(Ini, loadutf8_2)
{
  Ini ini;
  CHECK(0 == ini.size());

  CHECK(ini.load("./data/testw.ini"));
  CHECK(1 == ini.size());

  const Ini &sec = ini.items[0];
  CHECK(5 == sec.size());

  std::string secName = GET_UTF8_STR(UNICODE_SEC_NAME);
  CHECK(secName == sec.key);

  std::string item1 = GET_UTF8_STR(UNICODE_ITEM_1);
  CHECK(item1 == sec["1"].value);

  std::string item2 = GET_UTF8_STR(UNICODE_ITEM_2);
  CHECK(item2 == sec["2"].value);

  std::string item3 = GET_UTF8_STR(UNICODE_ITEM_3);
  CHECK(item3 == sec["3"].value);

  std::string item4 = GET_UTF8_STR(UNICODE_ITEM_4);
  CHECK(item4 == sec["4"].value);

  std::string item5 = GET_UTF8_STR(UNICODE_ITEM_5);
  CHECK(item5 == sec["5"].value);
}

TEST(Ini, loadstore)
{
  Ini ini;
  CHECK(ini.load("./data/test.ini"));

  std::string ss;
  CHECK(ini.storeToStream(ss));

  Ini ini2;
  CHECK(ini2.loadFromStream(ss));

  CHECK(ini.size() == ini2.size());

  for (int i = 0; i < ini.size(); ++i) {
    Ini const& sec1 = ini.items[i];
    Ini const& sec2 = ini2.items[i];
    CHECK(sec1.key == sec2.key);
    CHECK(sec1.size() == sec2.size());
    for (int j = 0; j < sec1.size(); ++j) {
      Ini const& item1 = sec1.items[j];
      Ini const& item2 = sec2.items[j];
      CHECK(item1.key == item2.key);
      CHECK(item1.value == item2.value);
    }
  }
}

TEST(Ini, insert)
{
  Ini ini;

  CHECK(0 == ini.find("sec_1"));
  ini["sec_1"];
  CHECK(0 != ini.find("sec_1"));

  Ini sec1 = ini["sec_1"];

  CHECK(0 == sec1.find("item_1"));
  sec1["item_1"] = 123;
  CHECK(0 != sec1.find("item_1"));
  CHECK(123 == (int)sec1["item_1"]);

  CHECK(0 == sec1.find("item_2"));
  sec1["item_2"] = 3.1415f;
  CHECK(0 != sec1.find("item_2"));
  CHECK(3.1415f == (float)sec1["item_2"]);

  CHECK(0 == sec1.find("item_3"));
  sec1["item_3"] = "str";
  CHECK(0 != sec1.find("item_3"));
  CHECK("str" == sec1["item_3"].value);

  CHECK(0 == ini.find("sec_2"));
  Ini sec2 = ini["sec_2"];

  CHECK(0 == sec2.find("item_4"));
  ini["sec_2"]["item_4"] = true;
  CHECK(0 == sec2.find("item_4"));
  CHECK(0 != ini["sec_2"].find("item_4"));
  CHECK((bool)ini["sec_2"]["item_4"]);

  CHECK(0 == ini.find("sec_3"));
  Ini& sec3 = ini["sec_3"];

  CHECK(0 == sec3.find("item_5"));
  ini["sec_3"]["item_5"] = 12198013;
  CHECK(0 != sec3.find("item_5"));
  CHECK(12198013 == (int)ini["sec_3"]["item_5"]);
  CHECK(12198013 == (int)sec3["item_5"]);
}

TEST(Ini, del)
{
  Ini ini;
  CHECK(ini.load("./data/test.ini"));

  CHECK(0 != ini.find("sec5"));
  Ini& sec5 = ini["sec5"];

  CHECK(0 == sec5.find("i5"));

  CHECK(0 != sec5.find("i0"));
  sec5.remove("i0");
  CHECK(0 == sec5.find("i0"));

  CHECK(0 != sec5.find("i2"));
  sec5.remove("i2");
  CHECK(0 == sec5.find("i2"));

  CHECK(0 != sec5.find("i1"));
  sec5.remove("i1");
  CHECK(0 == sec5.find("i1"));

  CHECK(0 != sec5.find("i4"));
  sec5.remove("i4");
  CHECK(0 == sec5.find("i4"));

  CHECK(0 != sec5.find("i3"));
  sec5.remove("i3");
  CHECK(0 == sec5.find("i3"));

  ini.remove("sec5");
  CHECK(0 == ini.find("sec5"));
}

// end of TestIni.cpp
