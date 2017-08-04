
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

TEST(Ini, loadutf8)
{
  Ini ini;
  CHECK(0 == ini.size());

  CHECK(ini.load("./data/testw.ini"));
  CHECK(1 == ini.size());

  const Ini &sec = ini.items[0];
  CHECK(5 == sec.size());

  static const int SEC_NAME[] = {36889, 26159, 20013, 25991};
  std::vector<int> secName;
  Util::utf8ToU16(sec.key.c_str(), secName);
  CHECK(secName == std::vector<int>(SEC_NAME, SEC_NAME + sizeof(SEC_NAME)/sizeof(SEC_NAME[0])));

  static const int ITEM_1[] = {36825, 26159, 39033, 30446, 19968};
  std::vector<int> item1;
  Util::utf8ToU16(sec.items[0].value.c_str(), item1);
  CHECK(item1 == std::vector<int>(ITEM_1, ITEM_1 + sizeof(ITEM_1)/sizeof(ITEM_1[0])));

  static const int ITEM_2[] = {12371, 12428, 12399, 12289, 12503, 12525, 12472, 12455, 12463, 12488, 73, 73, 12391, 12377};
  std::vector<int> item2;
  Util::utf8ToU16(sec.items[1].value.c_str(), item2);
  CHECK(item2 == std::vector<int>(ITEM_2, ITEM_2 + sizeof(ITEM_2)/sizeof(ITEM_2[0])));

  static const int ITEM_3[] = {51060, 32, 54532, 47196, 51229, 53944, 32, 73, 73, 73, 51077, 45768, 45796};
  std::vector<int> item3;
  Util::utf8ToU16(sec.items[2].value.c_str(), item3);
  CHECK(item3 == std::vector<int>(ITEM_3, ITEM_3 + sizeof(ITEM_3)/sizeof(ITEM_3[0])));

  static const int ITEM_4[] = {1069, 1090, 1086, 32, 1095, 1077, 1090, 1099, 1088, 1077, 32, 1090, 1086, 1074, 1072, 1088, 1072};
  std::vector<int> item4;
  Util::utf8ToU16(sec.items[3].value.c_str(), item4);
  CHECK(item4 == std::vector<int>(ITEM_4, ITEM_4 + sizeof(ITEM_4)/sizeof(ITEM_4[0])));

  static const int ITEM_5[] = {3609, 3637, 3657, 3648, 3611, 3655, 3609, 3650, 3588, 3619, 3591, 3585, 3634, 3619, 3607, 3637, 3656, 3627, 3657, 3634};
  std::vector<int> item5;
  Util::utf8ToU16(sec.items[4].value.c_str(), item5);
  CHECK(item5 == std::vector<int>(ITEM_5, ITEM_5 + sizeof(ITEM_5)/sizeof(ITEM_5[0])));
}

TEST(Ini, loadstore)
{
  Ini ini;
  CHECK(ini.load("./data/test.ini"));

  std::stringstream ss;
  CHECK(ini.store(ss));

  Ini ini2;
  CHECK(ini2.load(ss));

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

  CHECK(0 != sec5.find("i4"));
  sec5.remove("i4");
  CHECK(0 == sec5.find("i4"));

  CHECK(0 != sec5.find("i3"));
  sec5.remove("i3");
  CHECK(0 == sec5.find("i3"));

  CHECK(0 != sec5.find("i2"));
  sec5.remove("i2");
  CHECK(0 == sec5.find("i2"));

  CHECK(0 != sec5.find("i1"));
  sec5.remove("i1");
  CHECK(0 == sec5.find("i1"));

  CHECK(0 != sec5.find("i0"));
  sec5.remove("i0");
  CHECK(0 == sec5.find("i0"));

  ini.remove("sec5");
  CHECK(0 == ini.find("sec5"));
}

// end of TestIni.cpp
