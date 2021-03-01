
//
//  GUI module.
//
//  Copyright (c) 2006 Waync Cheng.
//  All Rights Reserved.
//
//  2006/04/06 Waync created.
//

#if defined(_linux_)
# include <limits.h>
#endif

#include "swObjectPool.h"
#include "swWidget.h"

#include "swWidgetImpl.h"
using namespace sw2::impl;

#define thePool impl::implWindow::pool()

namespace sw2 {

bool InitializeWidget()
{
  thePool.clear();
  return true;
}

void UninitializeWidget()
{
  thePool.clear();
}

namespace ui {

//
// Window.
//

Window::Window() : handle(-1)
{
}

Window::Window(int hWindow) : handle(hWindow)
{
}

int Window::create(int hParent, IntRect const& dim, std::string const& text, std::string const& tip, std::string const& id)
{
  return handle = implWindow::create(SWWT_WINDOW, hParent, dim, text, tip, id);
}

void Window::destroy()
{
  if (!isWindow()) {
    return;
  }

  if (SWWT_DESKTOP != getType() && (isFocused() || isHot() || isSelected())) {
    implWindow& desktop = thePool[getDesktop()];
    if (isFocused()) {
      desktop.m_focus = -1;
    }
    if (isHot()) {
      desktop.m_hot = -1;
    }
    if (isSelected()) {
      desktop.m_selected = -1;
    }
  }

  if (getChild()) {
    Window child(getChild());
    while (-1 != child.handle && -1 != child.getSibling()) {
      Window(child.getSibling()).destroy();
    }
    child.destroy();
  }

  implWindow& iw = thePool[handle];
  iw.free();
  iw.remove();

  handle = -1;
}

int Window::getChild() const
{
  return !isWindow() ? -1 : thePool[handle].m_child;
}

int Window::findChild(std::string const& id, bool bRecursive) const
{
  if (!isWindow()) {
    return -1;
  }

  Window child(getChild());
  while (-1 != child.handle) {
    if (child.getId() == id) {
      return child.handle;
    }
    if (bRecursive) {
      int h = child.findChild(id, true);
      if (-1 != h) {
        return h;
      }
    }
    child = child.getSibling();
  }

  return -1;
}

int Window::getDesktop() const
{
  if (!isWindow()) {
    return -1;
  }

  if (SWWT_DESKTOP == getType()) {
    return handle;
  }

  Window desktop(getParent());
  while (-1 != desktop.handle && SWWT_DESKTOP != desktop.getType()) {
    desktop.handle = desktop.getParent();
  }

  return desktop.handle;
}

IntRect Window::getDim() const
{
  if (!isWindow()) {
    return IntRect();
  } else {
    return thePool[handle].m_dim;
  }
}

std::string Window::getId() const
{
  return !isWindow() ? "" : thePool[handle].m_id;
}

int Window::getParent() const
{
  return !isWindow() ? -1 : thePool[handle].m_parent;
}

IntRect Window::getRect() const
{
  if (!isWindow()) {
    return IntRect();
  } else {
    return thePool[handle].getRect();
  }
}

int Window::getSibling() const
{
  return !isWindow() ? -1 : thePool[handle].m_sibling;
}

std::string Window::getText() const
{
  return !isWindow() ? "" : thePool[handle].m_text;
}

std::string Window::getTip() const
{
  return !isWindow() ? "" : thePool[handle].m_tip;
}

int Window::getType() const
{
  return !isWindow() ? SWWT_END_TAG : thePool[handle].m_type;
}

uint_ptr Window::getUserData() const
{
  return !isWindow() ? 0 : thePool[handle].m_user;
}

bool Window::isEnable() const
{
  return !isWindow() ? false : thePool[handle].isEnable();
}

bool Window::isEnableFocus() const
{
  return !isWindow() ? false : thePool[handle].isEnableFocus();
}

bool Window::isFocused() const
{
  return !isWindow() ? false : thePool[handle].isFocused();
}

bool Window::isHot() const
{
  return !isWindow() ? false : thePool[handle].isHot();
}

bool Window::isSelected() const
{
  return !isWindow() ? false : thePool[handle].isSelected();
}

bool Window::isVisible() const
{
  return !isWindow() ? false : thePool[handle].isVisible();
}

bool Window::isWindow() const
{
  return thePool.isUsed(handle);
}

void Window::setDim(IntRect const& dim)
{
  if (isWindow()) {
    thePool[handle].m_dim = dim;
  }
}

void Window::setEnable(bool bEnable)
{
  if (isWindow()) {
    thePool[handle].setEnable(bEnable);
  }
}

void Window::setEnableFocus(bool bEnable)
{
  if (isWindow()) {
    thePool[handle].setEnableFocus(bEnable);
  }
}

void Window::setFocus(bool bFocus)
{
  if (!isWindow()) {
    return;
  }

  implWindow& iw = thePool[handle];
  implWindow& desktop = thePool[getDesktop()];

  //
  // Kill focus.
  //

  if (!bFocus) {
    if (iw.isFocused()) {
      iw.setFocused(false);
      desktop.m_focus = -1;
    }
    return;
  }

  //
  // Ensure pWnd is visible, then set focus.
  //

  if (!iw.isVisible() || !iw.isEnableFocus()) { // Cannot gain focus.
    return;
  }

  int dlg = desktop.m_dlgStack.back();

  int p = iw.m_parent;
  while (-1 != p) {
    if (p == dlg) {
      if (thePool[dlg].isVisible()) {
        break;
      }
      return;                         // Invisible.
    }
    if (!thePool[p].isVisible()) {
      return;
    }
    p = thePool[p].m_parent;
  }

  if (-1 == p) {                      // Invisible(not valid).
    return;
  }

  iw.setFocused(true);

  //
  // Kill focus of prev focus.
  //

  if (-1 != desktop.m_focus) {
    thePool[desktop.m_focus].setFocused(false);
  }

  desktop.m_focus = handle;           // Set new focus window.
}

void Window::setId(std::string const& id)
{
  if (isWindow()) {
    thePool[handle].m_id = id;
  }
}

void Window::setText(std::string const& newText)
{
  if (isWindow()) {
    thePool[handle].setText(newText);
  }
}

void Window::setTip(std::string const& newTip)
{
  if (isWindow()) {
    thePool[handle].m_tip = newTip;
  }
}

void Window::setUserData(uint_ptr userData)
{
  if (isWindow()) {
    thePool[handle].m_user = userData;
  }
}

void Window::setVisible(bool bVisible)
{
  if (isWindow()) {
    thePool[handle].setVisible(bVisible);
  }
}

//
// Dialog.
//

int Dialog::create(int hParent, IntRect const& dim, std::string const& text, std::string const& tip, std::string const& id)
{
  if (-1 == Window::create(hParent, dim, text, tip, id)) {
    return -1;
  }

  setVisible(false);

  return handle;
}

bool Dialog::hideDialog()
{
  if (!isWindow()) {
    return false;
  }

  //
  // Get desktop and validate some stuff.
  //

  implWindow& desktop = thePool[getDesktop()];

  if (desktop.m_dlgStack.empty()) {
    return false;
  }

  if (handle != desktop.m_dlgStack.back()) {
    return false;
  }

  //
  // Always kill current focus, because it will invisible when dialog hide.
  //

  if (-1 != desktop.m_focus) {
    thePool[desktop.m_focus].setFocused(false);
    desktop.m_focus = -1;
  }

  //
  // Hide it.
  //

  desktop.m_dlgStack.pop_back();
  thePool[handle].setVisible(false);

  //
  // Fire a mouse move event to update internal state.
  //

  int x = desktop.m_lastX, y = desktop.m_lastY;
  desktop.m_lastX = 0, desktop.m_lastY = 0;
  Desktop(desktop.m_handle).inputMouseMove(x, y, 0);

  return true;
}

bool Dialog::showDialog()
{
  if (!isWindow()) {
    return false;
  }

  //
  // Get desktop and validate some stuff.
  //

  implWindow& dlg = thePool[handle];
  if (SWWT_WINDOW != dlg.m_type) {
    return false;
  }

  //
  // Always kill current focus, because it will lose focus when new dialog is shown.
  //

  implWindow& desktop = thePool[getDesktop()];

  if (-1 != desktop.m_focus) {
    thePool[desktop.m_focus].setFocused(false);
    desktop.m_focus = -1;
  }

  //
  // Show it.
  //

  desktop.m_dlgStack.push_back(handle);
  dlg.setParent(desktop.m_handle);      // Adjust z-order, bring to top-most.
  dlg.setVisible(true);

  //
  // Fire a mouse move event to update internal state.
  //

  int x = desktop.m_lastX, y = desktop.m_lastY;
  desktop.m_lastX = 0, desktop.m_lastY = 0;
  Desktop(desktop.m_handle).inputMouseMove(x, y, 0);

  return true;
}

//
// Desktop.
//

int Desktop::create(DesktopCallback* pCallback, IntRect const& dim, std::string const& text, std::string const& tip, std::string const& id)
{
  if (0 == pCallback) {
    return -1;
  }

  if (-1 == (handle = implWindow::create(SWWT_DESKTOP, -1, dim, text, tip, id))) {
    return -1;
  }

  implWindow& iw = thePool[handle];
  iw.m_lastX = iw.m_lastY = - 999999;   // Hack, to trigger first mouse move event.
  iw.m_hot = iw.m_selected = iw.m_focus = iw.m_tipHandle = -1;
  iw.m_pCb = pCallback;
  iw.m_dlgStack.clear();
  iw.m_dlgStack.push_back(handle);

  return handle;
}

void Desktop::inputChar(char ch, uint keyStat)
{
  if (!isWindow()) {
    return;
  }

  //
  // No focus wnd or not enable.
  //

  implWindow& iw = thePool[handle];

  if (-1 == iw.m_focus || !thePool[iw.m_focus].isEnable()) {

    //
    // Dispatch to current top level window.
    //

    thePool[iw.m_dlgStack.back()].OnChar(ch, keyStat);
    return;
  }

  //
  // Dispatch.
  //

  thePool[iw.m_focus].OnChar(ch, keyStat);

  //
  // Force show caret.
  //

  thePool[handle].m_caretFly = false;
  thePool[handle].m_caretTimer.setTimeout(0);
}

void Desktop::inputKeyDown(uint key, uint keyStat)
{
  if (!isWindow()) {
    return;
  }

  //
  // No focus wnd or not enable.
  //

  implWindow& iw = thePool[handle];

  if (-1 == iw.m_focus || !thePool[iw.m_focus].isEnable()) {

    //
    // Dispatch to current top level window.
    //

    thePool[iw.m_dlgStack.back()].OnKeyDown(key, keyStat);
    return;
  }

  //
  // Dispatch.
  //

  thePool[iw.m_focus].OnKeyDown(key, keyStat);

  //
  // Force show caret.
  //

  thePool[handle].m_caretFly = false;
  thePool[handle].m_caretTimer.setTimeout(0);
}

void Desktop::inputKeyUp(uint key, uint keyStat)
{
  if (!isWindow()) {
    return;
  }

  //
  // No focus wnd or not enable.
  //

  implWindow& iw = thePool[handle];

  if (-1 == iw.m_focus || !thePool[iw.m_focus].isEnable()) {

    //
    // Dispatch to current top level window.
    //

    thePool[iw.m_dlgStack.back()].OnKeyUp(key, keyStat);
    return;
  }

  //
  // Dispatch.
  //

  thePool[iw.m_focus].OnKeyUp(key, keyStat);

  //
  // Force show caret.
  //

  thePool[handle].m_caretFly = false;
  thePool[handle].m_caretTimer.setTimeout(0);
}

void Desktop::inputMouseDown(int x, int y, uint keyStat)
{
  if (!isWindow()) {
    return;
  }

  //
  // Always hide tip when mouse down.
  //

  implWindow& iw = thePool[handle];

  if (-1 != iw.m_tipHandle) {
    Window(iw.m_tipHandle).destroy();
    iw.m_tipHandle = -1;
  }

  //
  // Handle mouse down only when not click down.
  //

  if (-1 != iw.m_selected) {
    return;
  }

  //
  // Update focus.
  //

  if (iw.m_focus != iw.m_hot && -1 != iw.m_hot && thePool[iw.m_hot].isEnableFocus()) {

    //
    // Kill old focus.
    //

    if (-1 != iw.m_focus) {
      thePool[iw.m_focus].setFocused(false);
    }

    //
    // Set new focus.
    //

    iw.m_focus = iw.m_hot;
    thePool[iw.m_focus].setFocused(true);

    iw.m_caretFly = false;
    iw.m_caretTimer.setTimeout(0);
  }

  //
  // Do nothing when no hot window found, dispatch to top level wnd.
  //

  if (-1 == iw.m_hot) {
    thePool[iw.m_dlgStack.back()].OnMouseDown(x, y, keyStat);
    return;
  }

  //
  // Do nothing when the window is disabled.
  //

  if (!thePool[iw.m_hot].isVisible()) {
    iw.m_hot = -1;
    return;
  }

  if (!thePool[iw.m_hot].isEnable()) {
    return;
  }

  //
  // Set hot windows as the selected window and trigger a mouse down event.
  //

  iw.m_selected = iw.m_hot;
  thePool[iw.m_selected].setSelected(true);
  thePool[iw.m_selected].OnMouseDown(x, y, keyStat);
}

void Desktop::inputMouseMove(int x, int y, uint keyStat)
{
  if (!isWindow()) {
    return;
  }

  //
  // Avoid reentry.
  //

  implWindow& iw = thePool[handle];

  if (x == iw.m_lastX && y == iw.m_lastY) {
    return;
  }

  //
  // Save last cursor position, also prepare for tooltip.
  //

  iw.m_lastX = x, iw.m_lastY = y;

  //
  // Always hide tip when mouse move and reset tip timer.
  //

  if (-1 != iw.m_tipHandle) {
    Window(iw.m_tipHandle).destroy();
    iw.m_tipHandle = -1;
  }

  iw.m_tipTimer.setTimeout(TIMER_TIP_PREPARE_SHOW);

  //
  // No mouse capture, find who is under cursor now.
  //

  if (-1 == iw.m_selected) {

    //
    // Trigger current hot wnd.
    //

    if (-1 != iw.m_hot && thePool[iw.m_hot].isEnable()) {
      thePool[iw.m_hot].OnMouseMove(x, y, keyStat);
    }

    implWindow& iw2 = thePool[handle];    // It is possible iw becomes invalid becaused thePool growed in OnMouseMove above.

    //
    // Try to get new hot wnd.
    //

    int hot = thePool[iw2.m_dlgStack.back()].findMouseOver(x, y);
    if (hot == iw2.m_hot) {
      return;
    }

    //
    // Bingo, new hot wnd found.
    //

    if (-1 != iw2.m_hot) {
      implWindow& wndHot = thePool[iw2.m_hot];
      wndHot.setHot(false);             // Update hot state.
      if (-1 != hot && wndHot.isEnable()) {
        wndHot.OnMouseMove(x, y, keyStat);
      }
    }

    if (-1 != (iw2.m_hot = hot)) {      // Update hot state.
      thePool[iw2.m_hot].setHot(true);
    } else {

      //
      // No hot wnd found, dispatch to top level wnd.
      //

      thePool[iw2.m_dlgStack.back()].OnMouseMove(x, y, keyStat);
    }
  } else {

    //
    // Dispatch all mess to mouse capture.
    //

    IntRect rc = thePool[iw.m_selected].getRect();
    thePool[iw.m_selected].setHot(rc.ptInRect(IntPoint(x,y))); // Update every time is necessary.
    thePool[iw.m_selected].OnMouseMove(x, y, keyStat);
  }
}

void Desktop::inputMouseUp(int x, int y, uint keyStat)
{
  if (!isWindow()) {
    return;
  }

  //
  // Handle mouse up only when mouse has clicked down else dispatch to top level wnd.
  //

  implWindow& iw = thePool[handle];

  if (-1 == iw.m_selected) {
    thePool[iw.m_dlgStack.back()].OnMouseUp(x, y, keyStat);
    return;
  }

  //
  // Trigger a mouse up event and release the selected window.
  //

  thePool[iw.m_selected].OnMouseUp(x, y, keyStat);

  implWindow& iw2 = thePool[handle];    // It is possible iw becomes invalid becaused thePool growed in OnMouseUp above.

  if (-1 != iw2.m_selected) {
    thePool[iw2.m_selected].setSelected(false);
    iw2.m_selected = -1;
  }

  //
  // Try to find a new hot window.
  //

  iw2.m_lastX = iw2.m_lastY = -999999;  // Ensure mouse move not return immediately by of same event positon.
  inputMouseMove(x, y, keyStat);
}

void Desktop::inputMouseWheel(int x, int y, uint keyStat, int delta)
{
  if (!isWindow()) {
    return;
  }

  implWindow& iw = thePool[handle];

  if (-1 != iw.m_hot) {
    thePool[iw.m_hot].OnMouseWheel(x, y, keyStat, delta);
  }
}

void Desktop::render()
{
  if (!isWindow()) {
    return;
  }

  implWindow& iw = thePool[handle];
  if (!iw.isVisible()) {
    return;
  }

  iw.renderWidget();                    // Render self.

  if (-1 != iw.m_child) {
    thePool[iw.m_child].renderAll();    // Recursively render child.
  }
}

void Desktop::trigger()
{
  if (!isWindow()) {
    return;
  }

  implWindow& desktop = thePool[handle];

  //
  // Update focused editbox caret.
  //

  desktop.checkCaretFly();

  //
  // Update tooltip.
  //

  desktop.checkTipFly();

  //
  // Update scroll auto scroll.
  //

  desktop.checkAutoScroll();
}

//
// Button.
//

int Button::create(int hParent, IntRect const& dim, std::string const& text, std::string const& tip, std::string const& id)
{
  if (-1 == Window::create(hParent, dim, text, tip, id)) {
    return -1;
  }

  thePool[handle].m_type = SWWT_BUTTON;

  return handle;
}

//
// Checkbox.
//

int Checkbox::create(int hParent, IntRect const& dim, std::string const& text, std::string const& tip, std::string const& id)
{
  if (-1 == Window::create(hParent, dim, text, tip, id)) {
    return -1;
  }

  thePool[handle].m_type = SWWT_CHECKBOX;

  return handle;
}

bool Checkbox::isChecked()
{
  return !isWindow() ? false : thePool[handle].isChecked();
}

void Checkbox::setChecked(bool bChecked)
{
  if (isWindow()) {
    thePool[handle].setChecked(bChecked);
  }
}

//
// Radiobox.
//

int Radiobox::create(int hParent, IntRect const& dim, std::string const& text, std::string const& tip, std::string const& id)
{
  if (-1 == Window::create(hParent, dim, text, tip, id)) {
    return -1;
  }

  thePool[handle].m_type = SWWT_RADIOBOX;

  return handle;
}

//
// Editbox.
//

int Editbox::create(int hParent, IntRect const& dim, std::string const& text, std::string const& tip, std::string const& id)
{
  if (-1 == Window::create(hParent, dim, "", tip, id)) {
    return -1;
  }

  implWindow& iw = thePool[handle];
  iw.m_type = SWWT_EDITBOX;
  iw.m_chCaret = iw.m_chFirst = iw.m_chLast = iw.m_posCaret = iw.m_nchQueued = 0;
  iw.m_limit = INT_MAX;
  iw.m_chQueued = (uchar)-1;
  iw.m_state |= SWWS_FOCUS_ENABLE;      // Enable focus, and cannot be disabled.

  setText(text);

  return handle;
}

int Editbox::getDispTextPos() const
{
  return !isWindow() ? 0 : thePool[handle].m_chFirst;
}

int Editbox::getDispTextLen() const
{
  if (!isWindow()) {
    return 0;
  }

  implWindow& iw = thePool[handle];

  return iw.m_chLast - iw.m_chFirst;
}

int Editbox::getLimit() const
{
  return !isWindow() ? 0 : thePool[handle].m_limit;
}

bool Editbox::isNumber() const
{
  return !isWindow() ? false : thePool[handle].isNumber();
}

bool Editbox::isPassword() const
{
  return !isWindow() ? false : thePool[handle].isPassword();
}

void Editbox::setLimit(int cchMax)
{
  if (!isWindow()) {
    return;
  }

  cchMax = std::max(0, std::min(INT_MAX, cchMax)); // Adjust.
  if (0 >= cchMax) {
    cchMax = INT_MAX;
  }

  implWindow& iw = thePool[handle];

  iw.m_limit = cchMax;

  if ((int)iw.m_text.length() > cchMax) {
    iw.m_text.resize(cchMax);
  }
}

void Editbox::setNumberMode(bool bNumber)
{
  if (!isWindow()) {
    return;
  }

  implWindow& iw = thePool[handle];
  if (bNumber) {
    iw.m_state |= SWWS_NUMBER;
  } else {
    iw.m_state &= ~SWWS_NUMBER;
  }
}

void Editbox::setPasswordMode(bool bPassword)
{
  if (!isWindow()) {
    return;
  }

  implWindow& iw = thePool[handle];

  if (iw.isPassword() != bPassword) {
    iw.updateBoundary();
  }

  if (bPassword) {
    iw.m_state |= SWWS_PASSWORD;
  } else {
    iw.m_state &= ~SWWS_PASSWORD;
  }
}

//
// Scrollbar.
//

int Scrollbar::create(int hParent, IntRect const& dim, std::string const& text, std::string const& tip, std::string const& id)
{
  if (-1 == Window::create(hParent, dim, text, tip, id)) {
    return -1;
  }

  implWindow& iw = thePool[handle];
  iw.m_type = SWWT_SCROLLBAR;
  iw.m_min = iw.m_pos = 0;
  iw.m_max = 100;
  iw.m_page = 10;
  iw.m_caretFly = false;

  return handle;
}

int Scrollbar::getPageSize() const
{
  return !isWindow() ? 0 : thePool[handle].m_page;
}

int Scrollbar::getPos() const
{
  return !isWindow() ? 0 : thePool[handle].m_pos;
}

void Scrollbar::getRange(int& min, int& max) const
{
  if (!isWindow()) {
    return;
  }

  implWindow& iw = thePool[handle];
  min = iw.m_min;
  max = iw.m_max;
}

bool Scrollbar::isDecHot() const
{
  return !isWindow() ? false : thePool[handle].isDecHot();
}

bool Scrollbar::isDecSelected() const
{
  return !isWindow() ? false : thePool[handle].isDecSelected();
}

bool Scrollbar::isHorz() const
{
  return !isWindow() ? 0 : thePool[handle].isHorz();
}

bool Scrollbar::isIncHot() const
{
  return !isWindow() ? false : thePool[handle].isIncHot();
}

bool Scrollbar::isIncSelected() const
{
  return !isWindow() ? false : thePool[handle].isIncSelected();
}

bool Scrollbar::isNoBtn() const
{
  return !isWindow() ? false : thePool[handle].isNoBtn();
}

bool Scrollbar::isShowNoThumb() const
{
  return !isWindow() ? false : thePool[handle].isShowNoThumb();
}

bool Scrollbar::isThumbHot() const
{
  return !isWindow() ? false : thePool[handle].isThumbHot();
}

bool Scrollbar::isThumbSelected() const
{
  return !isWindow() ? false : thePool[handle].isThumbSelected();
}

void Scrollbar::setHorz(bool bHorz)
{
  if (!isWindow()) {
    return;
  }

  implWindow& iw = thePool[handle];

  if (bHorz) {
    iw.m_state |= SWWS_HORZ;
  } else {
    iw.m_state &= ~SWWS_HORZ;
  }
}

void Scrollbar::setNoBtn(bool bNoBtn)
{
  if (!isWindow()) {
    return;
  }

  implWindow& iw = thePool[handle];

  if (bNoBtn) {
    iw.m_state |= SWWS_NOBTN;
  } else {
    iw.m_state &= ~SWWS_NOBTN;
  }
}

void Scrollbar::setPageSize(int page)
{
  if (!isWindow()) {
    return;
  }

  if (0 > page) {
    page = 0;
  }

  implWindow& iw = thePool[handle];

  if (iw.m_max - iw.m_min < page) {
    page = iw.m_max - iw.m_min;
  }

  iw.m_page = page;
}

void Scrollbar::setPos(int pos)
{
  if (!isWindow()) {
    return;
  }

  implWindow& iw = thePool[handle];

  if (iw.m_min > pos) {
    pos = iw.m_min;
  }

  if (iw.m_max - iw.m_page < pos) {
    pos = iw.m_max - iw.m_page;
  }

  iw.m_pos = pos;
}

void Scrollbar::setRange(int min, int max)
{
  if (!isWindow() || min >= max) {
    return;
  }

  implWindow& iw = thePool[handle];
  iw.m_min = min, iw.m_max = max;
  setPageSize(iw.m_page), setPos(iw.m_pos); // Adjust.
}

void Scrollbar::setShowNoThumb(bool bShowNoThumb)
{
  if (isWindow()) {
    thePool[handle].setShowNoThumb(bShowNoThumb);
  }
}

//
// Listbox.
//

int Listbox::addString(std::string const& str)
{
  if (!isWindow()) {
    return -1;
  }

  implWindow& iw = thePool[handle];
  iw.m_lst.push_back(implListbox<implWindow>::ListItem(str));

  implWindow& scrollbar = thePool[getScrollbar()];

  if ((int)iw.m_lst.size() > scrollbar.m_page) { // Adjust range.
    scrollbar.m_max += 1;
  }

  if ((int)iw.m_lst.size() > iw.m_limit) { // Remvoe top if exceed limit.
    delString(0);
  }

  scrollbar.setVisible(scrollbar.m_max - scrollbar.m_min > scrollbar.m_page);

  return (int)iw.m_lst.size() - 1;
}

void Listbox::clear()
{
  if (!isWindow()) {
    return;
  }

  implWindow& iw = thePool[handle];
  iw.m_lst.clear();
  iw.m_pos = -1;                        // Current hot.
  iw.m_lastPt = -1;                     // Current selection.

  implWindow& scrollbar = thePool[getScrollbar()];
  scrollbar.m_pos = 0, scrollbar.m_max = scrollbar.m_page;
  scrollbar.setVisible(false);
}

int Listbox::create(int hParent, IntRect const& dim, std::string const& text, std::string const& tip, std::string const& id)
{
  if (-1 == Window::create(hParent, dim, text, tip, id)) {
    return -1;
  }

  implWindow& iw = thePool[handle];
  iw.m_type = SWWT_LISTBOX;

  //
  // Create and setup scrollbar.
  //

  IntRect dimSB(dim.right - SB_EMBEDED_CX, 0, SB_EMBEDED_CX, dim.bottom);
  Scrollbar sb;

  if (-1 == sb.create(handle, dimSB)) {
    destroy();
    return -1;
  }

  //
  // Query item dim.
  //

  implWindow& iw2 = thePool[handle];    // &iw may change because sb.create cause pool grow.
  IntPoint pt(0, 16);
  iw2.m_pCb->onWidgetQueryItemMetrics(handle, -1, pt);
  iw2.m_cyItem = pt.y;

  iw2.m_pos = iw2.m_lastPt = -1;        // Current hot && current selection.
  iw2.m_limit = INT_MAX;
  iw2.m_lst.clear();

  //
  // Initialize scrollbar.
  //

  implWindow& scrollbar = thePool[sb.handle];
  scrollbar.m_min = scrollbar.m_pos = 0;
  scrollbar.m_page = scrollbar.m_max = (int)(dim.bottom / (float)iw2.m_cyItem);

  return handle;
}

void Listbox::delString(int index)
{
  if (!isWindow()) {
    return;
  }

  implWindow& iw = thePool[handle];
  if (0 > index || (int)iw.m_lst.size() <= index) {
    return;
  }

  iw.m_lst.erase(iw.m_lst.begin() + index); // Delete item.

  if (iw.m_lastPt == index) {           // Adjust current selected item.
    iw.m_lastPt = -1;
  } else if (index < iw.m_lastPt) {
    iw.m_lastPt -= 1;
  }

  implWindow& scrollbar = thePool[getScrollbar()]; // Adjust scrollbar pos and range.

  if ((int)iw.m_lst.size() >= scrollbar.m_page) {
    scrollbar.m_max -= 1;
    if (scrollbar.m_max - scrollbar.m_page < scrollbar.m_pos) {
      scrollbar.m_pos = scrollbar.m_max - scrollbar.m_page;
    }
  }

  scrollbar.setVisible(scrollbar.m_max - scrollbar.m_min > scrollbar.m_page);
}

int Listbox::getCount() const
{
  return !isWindow() ? 0 : (int)thePool[handle].m_lst.size();
}

int Listbox::getCurHot() const
{
  return !isWindow() ? -1 : thePool[handle].m_pos;
}

int Listbox::getCurSel() const
{
  return !isWindow() ? -1 : thePool[handle].m_lastPt;
}

uint_ptr Listbox::getData(int index) const
{
  if (!isWindow()) {
    return 0;
  }

  implWindow& iw = thePool[handle];
  if (0 > index || (int)iw.m_lst.size() <= index) {
    return 0;
  }

  return iw.m_lst[index].user;
}

int Listbox::getScrollbar() const
{
  return getChild();                    // First child.
}

int Listbox::getFirstItem() const
{
  return !isWindow() ? -1 : thePool[getScrollbar()].m_pos;
}

int Listbox::getLimit() const
{
  return !isWindow() ? 0 : thePool[handle].m_limit;
}

std::string Listbox::getString(int index) const
{
  if (!isWindow()) {
    return "";
  }

  implWindow& iw = thePool[handle];
  if (0 > index || (int)iw.m_lst.size() <= index) {
    return "";
  }

  return iw.m_lst[index].str;
}

void Listbox::setCurSel(int index)
{
  if (!isWindow()) {
    return;
  }

  implWindow& iw = thePool[handle];

  if (-1 != index) {
    if (0 > index || (int)iw.m_lst.size() <= index) {
      return;
    }
    iw.m_lastPt = index;
  } else {
    iw.m_lastPt = index;
  }
}

void Listbox::setData(int index, uint_ptr user)
{
  if (!isWindow()) {
    return;
  }

  implWindow& iw = thePool[handle];
  if (0 > index || (int)iw.m_lst.size() <= index) {
    return;
  }

  iw.m_lst[index].user = user;
}

void Listbox::setFirstItem(int index)
{
  if (!isWindow()) {
    return;
  }

  implWindow& iw = thePool[handle];
  if (0 > index || (int)iw.m_lst.size() <= index) {
    return;
  }

  implWindow& scrollbar = thePool[getScrollbar()];
  scrollbar.m_pos = index;

  if (scrollbar.m_min > scrollbar.m_pos) { // Adjust pos in range.
    scrollbar.m_pos = scrollbar.m_min;
  }

  if (scrollbar.m_max - scrollbar.m_page < scrollbar.m_pos) {
    scrollbar.m_pos = scrollbar.m_max - scrollbar.m_page;
  }
}

void Listbox::setLimit(int maxItem)
{
  if (!isWindow()) {
    return;
  }

  maxItem = std::max(0, std::min(INT_MAX, maxItem)); // Adjust.
  if (0 >= maxItem) {
    maxItem = INT_MAX;
  }

  implWindow &iw = thePool[handle];
  iw.m_limit = maxItem;

  while ((int)iw.m_lst.size() > maxItem) {

    //
    // Remove exceed items from top.
    //

    delString(0);
  }
}

void Listbox::setString(int index, std::string const& str)
{
  if (!isWindow()) {
    return;
  }

  implWindow& iw = thePool[handle];
  if (0 > index || (int)iw.m_lst.size() <= index) {
    return;
  }

  iw.m_lst[index].str = str;
}

//
// Menu.
//

int Menu::create(int hParent, std::string const& id)
{
  if (-1 == Listbox::create(hParent, IntRect(), "", "", id)) {
    return -1;
  }

  implWindow& iw = thePool[handle];
  iw.m_type = SWWT_MENU;

  Scrollbar sb(getScrollbar());

  implWindow& scrollbar = thePool[sb.handle];
  scrollbar.m_page = scrollbar.m_max = MENU_MAX_ITEM;

  sb.setNoBtn(true);

  setVisible(false);

  return handle;
}

bool Menu::showMenu(IntPoint const& pt)
{
  if (!isWindow()) {
    return false;
  }

  implWindow& menu= thePool[handle];
  if (SWWT_MENU != menu.m_type) {
    return false;
  }

  if (menu.m_lst.empty()) {
    return false;
  }

  //
  // Get desktop and validate some stuff.
  //

  implWindow& desktop = thePool[getDesktop()];

  if (-1 != desktop.m_focus) {

    //
    // Always kill current focus, because it will lose focus when menu is shown.
    //

    thePool[desktop.m_focus].setFocused(false);
    desktop.m_focus = -1;
  }

  implWindow& scrollbar = thePool[getScrollbar()];

  //
  // Initialize menu.
  //

  menu.m_pos = menu.m_lastPt = -1;

  int nItem = std::min((int)menu.m_lst.size(), (int)MENU_MAX_ITEM);
  bool bShowSb = MENU_MAX_ITEM < menu.m_lst.size();

  menu.m_dim.left = pt.x, menu.m_dim.top = pt.y;
  menu.m_dim.bottom = menu.m_cyItem * nItem;
  menu.m_dim.right = 2 * MENU_MIN_WIDTH + SB_EMBEDED_CX * bShowSb;

  for (int i = 0; i < (int)menu.m_lst.size(); i++) {
    IntPoint pt(MENU_MIN_WIDTH, menu.m_cyItem);
    desktop.m_pCb->onWidgetQueryTextMetrics(handle, menu.m_lst[i].str, 0, pt);
    pt.x += 2 * MENU_MIN_WIDTH + SB_EMBEDED_CX * bShowSb;
    if (pt.x > menu.m_dim.right) {
      menu.m_dim.right = pt.x;
    }
  }

  //
  // Adjust.
  //

  IntRect rcMenu = menu.getRect(), rcDesktop = desktop.getRect();
  if (rcMenu.right > rcDesktop.right) {
    menu.m_dim.left -= rcMenu.width();
  }

  if (rcMenu.bottom > rcDesktop.bottom) {
    menu.m_dim.top -= rcMenu.height();
  }

  scrollbar.m_dim = IntRect(
                      menu.m_dim.right - SB_EMBEDED_CX,
                      0,
                      SB_EMBEDED_CX,
                      menu.m_dim.bottom);

  Scrollbar sb(getScrollbar());
  sb.setPos(0);
  sb.setVisible(bShowSb);

  //
  // Update window stack and show menu.
  //

  desktop.m_dlgStack.push_back(handle);
  menu.setParent(desktop.m_handle);     // Adjust Z-order.
  menu.setVisible(true);

  return true;
}

//
// Textbox.
//

int Textbox::create(int hParent, IntRect const& dim, std::string const& text, std::string const& tip, std::string const& id)
{
  if (-1 == Window::create(hParent, dim, text, tip, id)) {
    return -1;
  }

  implWindow& iw = thePool[handle];
  iw.m_type = SWWT_TEXTBOX;

  //
  // Create and setup scrollbar.
  //

  IntRect dimSB(dim.right - SB_EMBEDED_CX, 0, SB_EMBEDED_CX, dim.bottom);
  Scrollbar sb;

  if (-1 == sb.create(handle, dimSB)) {
    destroy();
    return -1;
  }

  implWindow& scrollbar = thePool[sb.handle];
  scrollbar.m_min = scrollbar.m_pos = 0;
  scrollbar.m_page = scrollbar.m_max = (int)(dim.bottom / (float)iw.m_cyItem);

  return handle;
}

std::string Textbox::getLine(int line) const
{
  if (!isWindow()) {
    return "";
  }

  implWindow& iw = thePool[handle];
  if (0 > line || (int)iw.m_lst.size() <= line) {
    return "";
  }

  return iw.m_text.substr((int)iw.m_lst[line].str[0], (int)iw.m_lst[line].str[1]);
}

int Textbox::getLineCount() const
{
  return !isWindow() ? 0 : (int)thePool[handle].m_lst.size();
}

int Textbox::getScrollbar() const
{
  return getChild();                    // First child.
}

} // ui

} // sw2

// end of swWidget.cpp
