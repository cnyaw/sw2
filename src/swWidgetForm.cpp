
//
//  Create form widget utility.
//
//  Copyright (c) 2008 Waync Cheng.
//  All Rights Reserved.
//
//  2008/11/10 Waync created.
//

#include "swIni.h"
#include "swWidget.h"

#include <iterator>

namespace sw2 {

int Util::createWidget(int hParent, sw2::Ini const& res, std::string const& name)
{
  if (0 == res.find(name)) {
    SW2_TRACE_ERROR("RES name [%s] not found.", name.c_str());
    return -1;
  }

  Ini sec = res[name];

  IntRect dim;
  dim.left = dim.top = 0;
  dim.right = dim.bottom = 32;

  if (sec.find("dim")) {
    std::vector<int> vdim;
    split(sec["dim"].value, vdim);
    if (0 < vdim.size()) {
      dim.left = vdim[0];
    }
    if (1 < vdim.size()) {
      dim.top = vdim[1];
    }
    if (2 < vdim.size()) {
      dim.right = vdim[2];
    }
    if (3 < vdim.size()) {
      dim.bottom = vdim[3];
    }
  }

  std::string text = sec["text"].value;
  std::string tip = sec["tip"].value;
  std::string id = sec["id"].value;

  if (id.empty()) {
    id = name;
  }

  int handle = -1;

  if (0 == name.find("window.") || 0 == name.find("dialog."))  {

    //
    // Create window.
    //

    ui::Window w;
    if (-1 == (handle = w.create(hParent, dim, text, tip, id))) {
      return -1;
    }

    if (sec.find("child")) {
      std::vector<std::string> vchild;
      split(sec["child"].value, vchild);
      for (size_t i = 0; i < vchild.size(); i++) {
        (void)createWidget(w.handle, res, vchild[i]);
      }
    }
  } else if (0 == name.find("button.")) {

    //
    // Create button.
    //

    ui::Button w;
    if (-1 == (handle = w.create(hParent, dim, text, tip, id))) {
      return -1;
    }

  } else if (0 == name.find("checkbox.")) {

    //
    // Create checkbox.
    //

    ui::Checkbox w;
    if (-1 == (handle = w.create(hParent, dim, text, tip, id))) {
      return -1;
    }

    if (sec.find("isChecked")) {
      w.setChecked(sec["isChecked"]);
    }

  } else if (0 == name.find("radiobox.")) {

    //
    // Create radiobox.
    //

    ui::Radiobox w;
    if (-1 == (handle = w.create(hParent, dim, text, tip, id))) {
      return -1;
    }

    if (sec.find("isChecked")) {
      w.setChecked(sec["isChecked"]);
    }

  } else if (0 == name.find("editbox.")) {

    //
    // Create editbox.
    //

    ui::Editbox w;
    if (-1 == (handle = w.create(hParent, dim, text, tip, id))) {
      return -1;
    }

    if (sec.find("isNumber")) {
      w.setNumberMode(sec["isNumber"]);
    }

    if (sec.find("isPassword")) {
      w.setPasswordMode(sec["isPassword"]);
    }

    if (sec.find("maxLength")) {
      w.setLimit(sec["maxLength"]);
    }

  } else if (0 == name.find("textbox.")) {

    //
    // Create textbox.
    //

    ui::Textbox w;
    if (-1 == (handle = w.create(hParent, dim, text, tip, id))) {
      return -1;
    }

  } else if (0 == name.find("listbox.")) {

    //
    // Create listbox.
    //

    ui::Listbox w;
    if (-1 == (handle = w.create(hParent, dim, text, tip, id))) {
      return -1;
    }

    if (res.find(sec["strings"])) {
      Ini const& strs = res[sec["strings"].value];
      for (size_t i = 0; i < strs.items.size(); i++) {
        w.addString(strs.items[i]);
      }
    }

    ui::Scrollbar sb(w.getScrollbar());

    if (sec.find("noBtn")) {
      sb.setNoBtn(sec["noBtn"]);
    }

    if (sec.find("noThumb")) {
      sb.setShowNoThumb(sec["noThumb"]);
    }

  } else if (0 == name.find("menu.")) {

    //
    // Create menu.
    //

    ui::Menu w;
    if (-1 == (handle = w.create(hParent, id))) {
      return -1;
    }

    if (res.find(sec["strings"])) {
      Ini const& strs = res[sec["strings"].value];
      for (size_t i = 0; i < strs.items.size(); i++) {
        w.addString(strs.items[i]);
      }
    }

  } else if (0 == name.find("scrollbar.")) {

    //
    // Create scrollbar.
    //

    ui::Scrollbar w;
    if (-1 == (handle = w.create(hParent, dim, text, tip, id))) {
      return -1;
    }

    if (sec.find("range")) {
      std::vector<int> vrange;
      split(sec["range"].value, vrange);
      if (2 <= vrange.size()) {
        w.setRange(vrange[0], vrange[1]);
      } else {
        SW2_TRACE_WARNING("'range' of res name [%s] does not have enough param (min max).", name.c_str());
      }
    }

    if (sec.find("pageSize")) {
      w.setPageSize(sec["pageSize"]);
    }

    if (sec.find("noBtn")) {
      w.setNoBtn(sec["noBtn"]);
    }

    if (sec.find("noThumb")) {
      w.setShowNoThumb(sec["noThumb"]);
    }

    if (sec.find("isHorz")) {
      w.setHorz(sec["isHorz"]);
    }

    if (sec.find("pos")) {
      w.setPos(sec["pos"]);
    }
  }

  //
  // General property.
  //

  ui::Window w(handle);

  if (0 != name.find("dialog.") && 0 != name.find("menu.")) { // Discard set visible to dialog and menu.
    if (sec.find("isVisible")) {
      w.setVisible(sec["isVisible"]);
    }
  } else {
    w.setVisible(false);
  }

  if (sec.find("isEnable")) {
    w.setEnable(sec["isEnable"]);
  }

  if (sec.find("isEnableFocus")) {
    w.setEnableFocus(sec["isEnableFocus"]);
  }

  return handle;
}

} // sw2

// end of swWidgetForm.cpp
