
//
//  Widget unit test.
//
//  Copyright (c) 2008 Waync Cheng.
//  All Rights Reserved.
//
//  2008/11/10 Waync created.
//

#include "CppUnitLite/TestHarness.h"

#include "swIni.h"
#include "swWidget.h"
using namespace sw2;
using namespace sw2::ui;

class TestWidget : public DesktopCallback
{
public:
  int LastCommandSender;

  TestWidget() : LastCommandSender(-1)
  {
  }

  virtual void onWidgetCommand(int hSender)
  {
    LastCommandSender = hSender;
  }

  void click(int x, int y, Desktop& d) const
  {
    d.inputMouseMove(x, y, 0);
    d.inputMouseDown(x, y, 0);
    d.inputMouseUp(x, y, 0);
  }
};

//
// Initialization.
//

TEST(Widget, init)
{
  CHECK(InitializeWidget());
  UninitializeWidget();
}

//
// get/set.
//

TEST(Widget, getset)
{
  CHECK(InitializeWidget());

  {
    TestWidget host;

    Desktop d;
    CHECK(-1 != d.create(&host, IntRect(0,0,800,600)));

    Window w;
    CHECK(-1 != w.create(d, IntRect(10,10,200,200)));
    CHECK(d == w.getParent());

    w.setVisible(false);
    CHECK(!w.isVisible());
    w.setVisible(true);
    CHECK(w.isVisible());

    w.setEnable(false);
    CHECK(!w.isEnable());
    w.setEnable(true);
    CHECK(w.isEnable());

    w.setEnableFocus(false);
    CHECK(!w.isEnableFocus());
    w.setEnableFocus(true);
    CHECK(w.isEnableFocus());

    w.setFocus(false);
    CHECK(!w.isFocused());
    w.setFocus(true);
    CHECK(w.isFocused());

    w.setId("test.1");
    CHECK("test.1" == w.getId());
    w.setId("test.2");
    CHECK("test.2" == w.getId());

    w.setText("window.1");
    CHECK("window.1" == w.getText());
    w.setText("window.2");
    CHECK("window.2" == w.getText());

    w.setTip("tip.1");
    CHECK("tip.1" == w.getTip());
    w.setTip("tip.2");
    CHECK("tip.2" == w.getTip());

    w.setUserData(100);
    CHECK(100 == w.getUserData());
    w.setUserData(123456789);
    CHECK(123456789 == w.getUserData());

    w.setDim(IntRect(20,20,50,50));
    CHECK(IntRect(20,20,50,50) == w.getDim());
    w.setDim(IntRect(100,120,150,150));
    CHECK(IntRect(100,120,150,150) == w.getDim());
  }

  UninitializeWidget();
}

//
// Test create widget.
//

TEST(Widget, create1)
{
  Ini ini;
  CHECK(ini.load("./data/widget.txt"));

  CHECK(InitializeWidget());

  TestWidget host;

  Desktop d;
  CHECK(-1 != d.create(&host, IntRect(0,0,800,600)));

  //
  // window.1
  //

  Window w1 = Util::createWidget(d, ini, "window.1");
  CHECK(-1 != w1);
  CHECK(d == w1.getParent());
  CHECK(SWWT_WINDOW == w1.getType());
  CHECK("this is window.1" == w1.getText());
  CHECK("window.1" == w1.getTip());
  CHECK("window.1" == w1.getId());
  CHECK(!w1.isEnable());
  CHECK(w1.getDim() == IntRect(10,10,400,400));

  Checkbox cw1 = w1.findChild("checkbox.w1");
  CHECK(-1 != cw1);
  CHECK(w1 == cw1.getParent());
  CHECK(SWWT_CHECKBOX == cw1.getType());
  CHECK(cw1.isChecked());
  CHECK(cw1.getDim() == IntRect(10,10,60,30));

  Editbox ew1 = w1.findChild("editbox.w1");
  CHECK(-1 != ew1);
  CHECK(w1 == ew1.getParent());
  CHECK(SWWT_EDITBOX == ew1.getType());
  CHECK(ew1.isNumber());
  CHECK(ew1.isPassword());
  CHECK(20 == ew1.getLimit());
  CHECK(ew1.getDim() == IntRect(10,50,60,30));

  Window w1w1 = w1.findChild("window.w1.w1", true);
  CHECK(-1 != w1w1);
  CHECK(w1 == Window(w1w1.getParent()).getParent());
  CHECK(w1w1.getDim() == IntRect(0,0,10,10));

  Scrollbar sbw1 = w1.findChild("scrollbar.w1");
  CHECK(-1 != sbw1);
  CHECK(!sbw1.isEnable());
  CHECK(sbw1.isHorz());
  int min1, max1;
  sbw1.getRange(min1, max1);
  CHECK(0 == min1 && 100 == max1);
  CHECK(10 == sbw1.getPageSize());
  CHECK(38 == sbw1.getPos());

  //
  // button.1
  //

  Window b1 = Util::createWidget(d, ini, "button.1");
  CHECK(-1 != b1);
  CHECK(d == b1.getParent());
  CHECK(SWWT_BUTTON == b1.getType());
  CHECK("button.1" == b1.getText());
  CHECK(b1.getDim() == IntRect(20,20,60,25));

  //
  // dialog.1
  //

  Window d1 = Util::createWidget(d, ini, "dialog.1");
  CHECK(-1 != d1);
  CHECK(d == d1.getParent());
  CHECK(SWWT_WINDOW == d1.getType());
  CHECK(d1.getDim() == IntRect(200,100,400,400));

  Window rd1 = d1.findChild("radiobox.d1");
  CHECK(-1 != rd1);
  CHECK(d1 == rd1.getParent());
  CHECK(SWWT_RADIOBOX == rd1.getType());
  CHECK(rd1.getDim() == IntRect(10,10,80,26));

  Window rd2 = d1.findChild("radiobox.d2");
  CHECK(-1 != rd2);
  CHECK(d1 == rd2.getParent());
  CHECK(SWWT_RADIOBOX == rd2.getType());
  CHECK(!rd2.isVisible());
  CHECK(rd2.getDim() == IntRect(10,40,80,26));

  Radiobox rd3 = d1.findChild("radiobox.d3");
  CHECK(-1 != rd2);
  CHECK(d1 == rd3.getParent());
  CHECK(SWWT_RADIOBOX == rd3.getType());
  CHECK(rd3.isChecked());
  CHECK(rd3.getDim() == IntRect(10,70,80,26));

  Listbox lbd1 = d1.findChild("listbox.d1");
  CHECK(-1 != lbd1);
  CHECK(d1 == lbd1.getParent());
  CHECK(SWWT_LISTBOX == lbd1.getType());
  CHECK(3 == lbd1.getCount());
  CHECK("hello" == lbd1.getString(0));
  CHECK("world" == lbd1.getString(1));
  CHECK("smallworld" == lbd1.getString(2));
  CHECK(lbd1.getDim() == IntRect(10,100,100,100));

  //
  // menu.1
  //

  Menu m1 = Util::createWidget(d, ini, "menu.1");
  CHECK(-1 != m1);
  CHECK(d == m1.getParent());
  CHECK(SWWT_MENU == m1.getType());
  CHECK(4 == m1.getCount());
  const char *s[] = {"open", "close", "save", "exit"};
  for (int i = 0; i < (int)(sizeof(s)/sizeof(s[0])); i++) {
    CHECK(s[i] == m1.getString(i));
  }

  //
  // Create another scrollbar.
  //

  Scrollbar sb1 = Util::createWidget(d, ini, "scrollbar.w1");
  CHECK(-1 != sb1);
  CHECK(!sb1.isEnable());
  CHECK(sb1.isHorz());
  sb1.getRange(min1, max1);
  CHECK(0 == min1 && 100 == max1);
  CHECK(10 == sb1.getPageSize());
  CHECK(38 == sb1.getPos());

  CHECK(-1 != d.findChild("scrollbar.w1", false));
  sb1.destroy();
  CHECK(-1 == d.findChild("scrollbar.w1", false));

  d.destroy();

  UninitializeWidget();
}

//
// Limitation test.
//

TEST(Widget, editbox_limit)
{
  CHECK(InitializeWidget());

  TestWidget host;

  Desktop d;
  CHECK(-1 != d.create(&host, IntRect(0,0,800,600)));

  Editbox e;
  e.create(d, IntRect(10, 10, 100, 30), "1234567890");
  e.setFocus(true);

  e.setLimit(8);
  CHECK(8 == e.getLimit());
  CHECK("12345678" == e.getText());

  e.setLimit(5);
  CHECK(5 == e.getLimit());
  e.setText("abcdefghijklmn");
  CHECK("abcde" == e.getText());

  d.inputChar('x', 0);
  CHECK("abcde" == e.getText());

  e.setLimit(6);
  CHECK(6 == e.getLimit());
  d.inputChar('x', 0);
  CHECK("abcdex" == e.getText());

  d.destroy();

  UninitializeWidget();
}

TEST(Widget, listbox_limit)
{
  CHECK(InitializeWidget());

  TestWidget host;

  Desktop d;
  CHECK(-1 != d.create(&host, IntRect(0,0,800,600)));

  Listbox lb;
  lb.create(d, IntRect(10, 10, 100, 30));

  lb.addString("1");
  lb.addString("2");
  lb.addString("3");
  lb.addString("4");
  lb.addString("5");
  CHECK(5 == lb.getCount());

  lb.setLimit(3);
  CHECK(3 == lb.getCount());
  CHECK(3 == lb.getLimit());

  lb.setLimit(4);
  CHECK(3 == lb.getCount());
  CHECK(4 == lb.getLimit());

  lb.addString("6");
  CHECK(4 == lb.getCount());

  CHECK("3" == lb.getString(0));
  CHECK("4" == lb.getString(1));
  CHECK("5" == lb.getString(2));
  CHECK("6" == lb.getString(3));

  d.destroy();

  UninitializeWidget();
}

TEST(Widget, listbox_item)
{
  CHECK(InitializeWidget());

  TestWidget host;

  Desktop d;
  CHECK(-1 != d.create(&host, IntRect(0,0,800,600)));

  Listbox lb;
  lb.create(d, IntRect(10, 10, 100, 30));

  //
  // Add string test.
  //

  lb.addString("1");
  lb.addString("2");
  lb.addString("3");
  CHECK(3 == lb.getCount());

  //
  // Set string test.
  //

  lb.setString(0, "a");
  lb.setString(1, "b");
  lb.setString(2, "C");
  CHECK("a" == lb.getString(0));
  CHECK("b" == lb.getString(1));
  CHECK("C" == lb.getString(2));

  //
  // Get/set cur sel test.
  //

  lb.setCurSel(1);
  CHECK(1 == lb.getCurSel());
  lb.setCurSel(5);
  CHECK(1 == lb.getCurSel());
  lb.setCurSel(-2);
  CHECK(1 == lb.getCurSel());
  lb.setCurSel(-1);
  CHECK(-1 == lb.getCurSel());

  //
  // Get/set first item test.
  //

  lb.setFirstItem(1);
  CHECK(1 == lb.getFirstItem());
  lb.setFirstItem(-1);
  CHECK(1 == lb.getFirstItem());
  lb.setFirstItem(5);
  CHECK(1 == lb.getFirstItem());

  //
  // Get/set item data test.
  //

  lb.setData(0, 10);
  lb.setData(1, 20);
  lb.setData(2, 30);
  CHECK(10 == lb.getData(0));
  CHECK(20 == lb.getData(1));
  CHECK(30 == lb.getData(2));

  d.destroy();

  UninitializeWidget();
}

TEST(Widget, hierarchy1)
{
  CHECK(InitializeWidget());

  TestWidget host;

  //
  // d -> w1 -> b1
  //         -> w2
  //         -> lb1
  //   -> w3
  //

  Desktop d;
  CHECK(-1 != d.create(&host, IntRect(0,0,800,600)));

  Window w1;
  w1.create(d, IntRect(10, 10, 500, 500), "", "", "w1");

  Button b1;
  b1.create(w1, IntRect(20, 20, 100, 40), "", "", "w1b1");

  Window w2;
  w2.create(w1, IntRect(20, 40, 100, 100), "", "", "w1w2");

  Listbox lb1;
  lb1.create(w2, IntRect(0, 0, 10, 10), "", "", "w2lb1");

  Window w3;
  w3.create(d, IntRect(100, 100, 100, 100), "", "", "w3");

  CHECK(w1 == d.findChild("w1"));
  CHECK(b1 == d.findChild("w1b1"));
  CHECK(w2 == d.findChild("w1w2"));
  CHECK(lb1 == d.findChild("w2lb1"));
  CHECK(w3 == d.findChild("w3"));

  CHECK(-1 == d.findChild("w1b1", false));
  CHECK(-1 == d.findChild("w1w2", false));
  CHECK(-1 == d.findChild("w2lb1", false));

  CHECK(b1 == w1.findChild("w1b1"));
  CHECK(w2 == w1.findChild("w1w2"));
  CHECK(lb1 == w1.findChild("w2lb1"));

  w1.destroy();

  CHECK(-1 == d.findChild("w1"));
  CHECK(-1 == d.findChild("w1b1"));
  CHECK(-1 == d.findChild("w1w2"));
  CHECK(-1 == d.findChild("w2lb1"));
  CHECK(w3 == d.findChild("w3"));

  d.destroy();

  UninitializeWidget();
}

TEST(Widget, interact1)
{
  Ini ini;
  CHECK(ini.load("./data/widget.txt"));

  CHECK(InitializeWidget());

  TestWidget host;

  Desktop d;
  CHECK(-1 != d.create(&host, IntRect(0,0,800,600)));

  Window w1 = Util::createWidget(d, ini, "window.1");
  CHECK(-1 != w1);

  Checkbox chk1 = w1.findChild("checkbox.w1");
  CHECK(-1 != chk1);

  host.click(22, 22, d);
  CHECK(chk1 == host.LastCommandSender);
  CHECK(!chk1.isChecked());
  host.click(22, 22, d);
  CHECK(chk1.isChecked());

  Editbox ed1 = w1.findChild("editbox.w1");
  CHECK(-1 != ed1);
  ed1.setNumberMode(false);
  CHECK(!ed1.isNumber());

  host.click(22, 62, d);
  CHECK(ed1.isFocused());
  d.inputChar('b', 0);
  d.inputChar('x', 0);
  d.inputChar('D', 0);
  d.inputChar('5', 0);
  d.inputChar('_', 0);
  CHECK("bxD5_" == ed1.getText());

  d.inputKeyDown(SWK_RETURN, 0);
  d.inputKeyUp(SWK_RETURN, 0);
  CHECK(ed1 == host.LastCommandSender);

  UninitializeWidget();
}

// end of TestWidget.cpp
