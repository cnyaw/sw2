
//
//  GUI module.
//
//  Copyright (c) 2007 Waync Cheng.
//  All Rights Reserved.
//
//  2007/08/29 Waync created.
//

#pragma once

namespace sw2 {

namespace impl {

//
// Inner constants.
//

enum CONST_WIDGET
{
  SB_EMBEDED_CX           = 16,         // Inner scrollbar width.
  SB_MIN_THUMB            = 8,          // Min scroll bar thumb size.
  SB_THUMB_DRAG_RANGE     = 60,         // Extra thumb range when dragging.
  MENU_MAX_ITEM           = 6,          // Max menu item.
  MENU_MIN_WIDTH          = 16,         // Min menu width.
  DEF_CX_CURSOR           = 32,         // Default mouse cursor width.
  DEF_CY_CURSOR           = 32,         // Default mouse cursor height.
  DEF_CX_TIP              = 60,         // Default tip window width.
  DEF_CY_TIP              = 18,         // Default tip window height.
  TIMER_TIP_PREPARE_SHOW  = 650,        // Timer prepare to set tip visible when mouse over change.
  TIMER_CARET_SHOW        = 600,        // Caret display time.
  TIMER_CARET_HIDE        = 400,        // Caret hide time.
  TIMER_PREPARE_AUTOSCROLL= 500,        // Time interval for prepare autoscroll scrollbar.
  TIMER_AUTOSCROLL        = 5,          // Time interval for autoscroll scrollbar.

  //
  // Window state/substate/style.
  //

  SWWS_VISIBLE            = 1,          // Window is visible.
  SWWS_DISABLE            = 1 << 1,     // Window is disable.
  SWWS_HOTLIGHT           = 1 << 2,     // Mouse is over the window.
  SWWS_SELECTED           = 1 << 3,     // Left mouse button is click on the window, before release.
  SWWS_CHECKED            = 1 << 4,     // Button checked state.
  SWWS_FOCUSED            = 1 << 5,     // Has key board focus.
  SWWS_NUMBER             = 1 << 6,     // Edit number mode.
  SWWS_PASSWORD           = 1 << 7,     // Edit password mode.
  SWWS_HORZ               = 1 << 8,     // Scroll horizontal style.
  SWWS_NOBTN              = 1 << 9,     // Scroll no btn style.
  SWWS_DEC_HOT            = 1 << 10,    // Scroll dec btn is hot.
  SWWS_INC_HOT            = 1 << 11,    // Scroll inc btn is hot.
  SWWS_THUMB_HOT          = 1 << 12,    // Scroll thumb is hot.
  SWWS_DEC_SELECTED       = 1 << 13,    // Scroll dec btn is selected.
  SWWS_INC_SELECTED       = 1 << 14,    // Scroll inc btn is selected.
  SWWS_THUMB_SELECTED     = 1 << 15,    // Scroll thumb is selected.
  SWWS_SHOW_NO_THUMB      = 1 << 16,    // Scroll display thumb even can not scroll by thumb.
  SWWS_FOCUS_ENABLE       = 1 << 17,    // Window is enable keyboard focus.
  SWWS_END_TAG
};

class implWindowState
{
public:

  //
  // General.
  //

  int m_parent, m_sibling, m_prevSibling, m_child; // Relationship.

  int m_handle;                         // Handle of this window. idx to widget pool.
  int m_type;                           // Widget type, see CR_WIDGET_TYPE_.

  uint m_state;                         // Window state, see CR_WINDOW_STATE_.
  IntRect m_dim;                        // Position and size of the widget, relative to parent's.

  std::string m_text;                   // Widget display text or name.
  std::string m_tip;                    // Tooltip text, not show tip if tip is empty(0 length).

  std::string m_id;                     // User define ID.
  uint_ptr m_user;                      // User define data.

  ui::DesktopCallback* m_pCb;           // Desktop event callback.

  //
  // Get/set.
  //

  bool isChecked() const
  {
    return 0 != (m_state & SWWS_CHECKED);
  }

  bool isDecHot() const
  {
    return isHot() && (0 != (m_state & SWWS_DEC_HOT));
  }

  bool isDecSelected() const
  {
    return isSelected() && (0 != (m_state & SWWS_DEC_SELECTED));
  }

  bool isEnable() const
  {
    return 0 == (m_state & SWWS_DISABLE);
  }

  bool isEnableFocus() const
  {
    return 0 != (m_state & SWWS_FOCUS_ENABLE) && isEnable();
  }

  bool isFocused() const
  {
    return 0 != (m_state & SWWS_FOCUSED);
  }

  bool isHorz() const
  {
    return 0 != (m_state & SWWS_HORZ);
  }

  bool isHot() const
  {
    return 0 != (m_state & SWWS_HOTLIGHT);
  }

  bool isIncHot() const
  {
    return isHot() && (0 != (m_state & SWWS_INC_HOT));
  }

  bool isIncSelected() const
  {
    return isSelected() && (0 != (m_state & SWWS_INC_SELECTED));
  }

  bool isNoBtn() const
  {
    return 0 != (m_state & SWWS_NOBTN);
  }

  bool isNumber() const
  {
    return 0 != (m_state & SWWS_NUMBER);
  }

  bool isPassword() const
  {
    return 0 != (m_state & SWWS_PASSWORD);
  }

  bool isSelected() const
  {
    return 0 != (m_state & SWWS_SELECTED);
  }

  bool isShowNoThumb() const
  {
    return 0 != (m_state & SWWS_SHOW_NO_THUMB);
  }

  bool isThumbHot() const
  {
    return isHot() && (0 != (m_state & SWWS_THUMB_HOT));
  }

  bool isThumbSelected() const
  {
    return isSelected() && (0 != (m_state & SWWS_THUMB_SELECTED));
  }

  bool isVisible() const
  {
    return 0 != (m_state & SWWS_VISIBLE);
  }

  void setEnable(bool bEnable)
  {
    if (bEnable) {
      m_state &= ~SWWS_DISABLE;
    } else {
      m_state |= SWWS_DISABLE;
    }
  }

  void setEnableFocus(bool bEnable)
  {
    if (bEnable) {
      m_state |= SWWS_FOCUS_ENABLE;
    } else {
      m_state &= ~SWWS_FOCUS_ENABLE;
    }
  }

  void setFocused(bool bFocus)
  {
    if (bFocus) {
      m_state |= SWWS_FOCUSED;
    } else {
      m_state &= ~SWWS_FOCUSED;
    }
  }

  void setHot(bool bHotlight)
  {
    if (bHotlight) {
      m_state |= SWWS_HOTLIGHT;
    } else {
      m_state &= ~SWWS_HOTLIGHT;
    }
  }

  void setSelected(bool bSelected)
  {
    if (bSelected) {
      m_state |= SWWS_SELECTED;
    } else {
      m_state &= ~SWWS_SELECTED;
    }
  }

  void setShowNoThumb(bool bShowNoThumb)
  {
    if (bShowNoThumb) {
      m_state |= SWWS_SHOW_NO_THUMB;
    } else {
      m_state &= ~SWWS_SHOW_NO_THUMB;
    }
  }

  void setVisible(bool bVisible)
  {
    if (bVisible) {
      m_state |= SWWS_VISIBLE;
    } else {
      m_state &= ~SWWS_VISIBLE;
    }
  }
};

template<class TBase> class implDesktop
{
public:

  int m_lastX, m_lastY;                 // Last mouse position.

  int m_selected;                       // The window that has been click down
                                        // before release the mouse button, also
                                        // use to check is mouse down.
  int m_hot;                            // The window that mouse over it.
  int m_focus;                          // The window that has key board focus.

  int m_tipHandle;                      // The tip window handle.
  TimeoutTimer m_tipTimer;              // Timer for control tooltip visibility.

  bool m_caretFly;                      // Is caret visible?
  TimeoutTimer m_caretTimer;            // Timer to control caret visibility.

  std::vector<int> m_dlgStack;          // Dialog stack.

  //
  // Helper.
  //

  void checkAutoScroll()
  {
    TBase& base = (TBase&)*this;

    if (-1 == m_selected || SWWT_SCROLLBAR != base.pool()[m_selected].m_type) {
      return;
    }

    TBase& sb = base.pool()[m_selected];

    if (sb.m_caretTimer.isExpired() &&
        ((sb.isIncSelected() &&
         sb.isIncHot()) ||
         (sb.isDecSelected() &&
         sb.isDecHot()))) {
      sb.OnMouseDown(m_lastX, m_lastY, 0);
      sb.m_caretFly = true;
    }
  }

  void checkCaretFly()
  {
    TBase& base = (TBase&)*this;

    //
    // No focus wnd or not enable.
    //

    if (-1 == m_focus || !base.pool()[m_focus].isEnable()) {
      return;
    }

    //
    // Toggle show/hide.
    //

    if (m_caretTimer.isExpired()) {
      if (m_caretFly) {
        m_caretTimer.setTimeout(TIMER_CARET_HIDE);
      } else {
        m_caretTimer.setTimeout(TIMER_CARET_SHOW);
      }
      m_caretFly = !m_caretFly;
    }

    base.pool()[m_focus].m_caretFly = m_caretFly;
  }

  void checkTipFly()
  {
    TBase& base = (TBase&)*this;

    //
    // Ignore if in mouse down process.
    //

    if (-1 != m_selected) {
      return;
    }

    //
    // Ignore if no hot window.
    //

    if (-1 == m_hot) {
      return;
    }

    //
    // Time to show tip, and no tip visible and hot window found?
    //

    if (m_tipTimer.isExpired() && -1 == m_tipHandle) {

      //
      // Is a tip text assigned?
      //

      if (base.pool()[m_hot].m_tip.empty()) {
        return;
      }

      //
      // Setup tip window.
      //

      m_tipHandle = base.create(SWWT_TOOLTIP, base.m_handle, IntRect(), base.pool()[m_hot].m_tip, "", "");
      if (-1 == m_tipHandle) {
        return;
      }

      TBase& tip = base.pool()[m_tipHandle];

      //
      // Query tip text size.
      //

      ui::Window tipWnd(m_tipHandle);   // Dummy.

      IntPoint szTip(DEF_CX_TIP, DEF_CY_TIP);
      base.m_pCb->onWidgetQueryTextMetrics(m_tipHandle, tip.m_text, 0, szTip);
      szTip.x += 8, szTip.y += 4;       // Adjust.

      //
      // Query cursor metrics.
      //

      IntRect rcCursor(m_lastX, m_lastY, DEF_CX_CURSOR, DEF_CY_CURSOR);
      base.m_pCb->onWidgetQueryCursorMetrics(base.m_handle, rcCursor);

      //
      // Adjust tip window position.
      //

      tip.m_dim = IntRect(rcCursor.left, rcCursor.top + rcCursor.bottom, szTip.x, szTip.y);

      IntRect rcTip = tip.getRect(), rcDesktop = base.getRect();

      if (rcTip.right > rcDesktop.right) {
        tip.m_dim.left = rcCursor.left - szTip.x;
        tip.m_dim.top = rcCursor.top - szTip.y;
      }

      if (rcTip.bottom > rcDesktop.bottom) {
        tip.m_dim.top = rcCursor.top - szTip.y;
      }

      //
      // Show it.
      //

      tip.setVisible(true);
    }
  }
};

template<class TBase> class implEditbox
{
public:

  int m_posCaret;                       // Caret position.

  int m_limit;                          // Max text length.
  int m_chCaret;                        // Caret position.
  int m_chFirst;                        // First visible char position.
  int m_chLast;                         // Last visible char position.

  int m_nchQueued;                      // Number char queued.
  uchar m_chQueued;                     // Queued char.

  void setText(std::string const& newText)
  {
    TBase& base = (TBase&)*this;

    if ((int)base.m_text.length() > base.m_limit) {
      base.m_text.resize(base.m_limit);
    }

    m_chCaret = (int)base.m_text.length();
    implEditbox<TBase>::updateBoundary();
  }

  bool OnChar(uchar ch, uint keyStat)
  {
    TBase& base = (TBase&)*this;

    //
    // Check for ASCII & BIG5 code.
    //

    int n;
    if ((uchar)-1 != m_chQueued) {      // 2nd byte arrived.
      if (!Util::isBIG5((m_chQueued << CHAR_BIT) | ch)) { // Is this a BIG5 code?
        m_chQueued = (uchar)-1;
        return false;
      }
      n = 2;
    } else { // 1st byte arrived

      //
      // Will handled by keydown handler?
      //

      if (char('\r') == ch || char('\t') == ch || char('\b') == ch || char(23) == ch) {
        return false;
      }

      if (char(127) == ch) {            // Unused.
        return false;
      }

      if (!isACSII(ch)) {               // Maybe a BIG5 code, wait for 2nd byte.
        m_chQueued = ch;
        return true;
      }

      n = 1;
    }

    //
    // Number mode, accept?
    //

    if (base.isNumber()) {
      if (2 == n) {                     // Not allowed.
        return false;
      }
      if (char('0') > ch || char('9') < ch) { // Not a number.
        return false;
      }
    }

    //
    // Check limit.
    //

    if ((int)base.m_text.length() + n > base.m_limit) {
      return false;
    }

    //
    // Insert char.
    //

    if (2 == n) {
      base.m_text.insert(m_chCaret++, 1, m_chQueued);
      m_chQueued = (uchar)-1;
    }

    base.m_text.insert(m_chCaret++, 1, ch);
    updateBoundary();

    return true;
  }

  bool OnKeyDown(uint key, uint keyStat)
  {
    TBase& base = (TBase&)*this;

    switch (key)
    {
    case SWK_BACK:
      if (0 < m_chCaret) {
        int pos = movePrev(m_chCaret, 0 != (keyStat & SWKS_CTRL));
        base.m_text.erase(pos, m_chCaret - pos);
        m_chCaret = pos;
        updateBoundary();
      }
      return true;

    case SWK_RETURN:
      base.m_pCb->onWidgetCommand(base.m_handle);
      return true;

    case SWK_END:
      if ((int)base.m_text.length() > m_chCaret) {
        m_chCaret = (int)base.m_text.length();
        updateBoundary();
      }
      return true;

    case SWK_HOME:
      if (0 < m_chCaret) {
        m_chCaret = 0;
        updateBoundary();
      }
      return true;

    case SWK_LEFT:
      if (0 < m_chCaret) {
        m_chCaret = movePrev(m_chCaret, 0 != (keyStat & SWKS_CTRL));
        updateBoundary();
      }
      return true;

    case SWK_RIGHT:
      if ((int)base.m_text.length() > m_chCaret) {
        m_chCaret = moveNext(m_chCaret, 0 != (keyStat & SWKS_CTRL));
        updateBoundary();
      }
      return true;

    case SWK_DELETE:
      if ((int)base.m_text.length() > m_chCaret) {
        int pos = moveNext(m_chCaret, 0 != (keyStat & SWKS_CTRL));
        base.m_text.erase(m_chCaret, pos - m_chCaret);
        updateBoundary();
      }
      return true;

    }

    return false;                       // Not handled.
  }

  void render()
  {
    TBase& base = (TBase&)*this;

    IntRect rc = base.getRect();
    rc.inflate(-4, -4);

    //
    // Render text.
    //

    if (!base.m_text.empty()) {
      base.m_pCb->onWidgetRenderWidget(base.m_handle, SWRS_ED_TEXT, -1, rc);
    }

    //
    // Render caret.
    //

    rc.left += m_posCaret;
    rc.right = rc.left + 1;

    if (base.isFocused() && base.m_caretFly) {
      base.m_pCb->onWidgetRenderWidget(base.m_handle, SWRS_ED_CARET, -1, rc);
    }
  }

  //
  // Helper.
  //

  bool isACSII(int ch) const            // ACSII and printable.
  {
    return 0x20 <= ch && 0x7f > ch;
  }

  int moveNext(int anchor, bool bJump) const
  {
    TBase& base = (TBase&)*this;

    if ((int)base.m_text.length() == anchor) {
      return anchor;
    }

    int pos = anchor;
    if (bJump) {
      while (pos != (int)base.m_text.length()) {
        if (char(' ') == base.m_text[pos++]) {
          while (pos != (int)base.m_text.length() &&
                 char(' ') == base.m_text[pos]) {
            pos += 1;
          }
          return pos;
        }
      }
      return pos;
    }

    return anchor + (isACSII((uchar)base.m_text[pos]) ? 1 : 2);
  }

  int movePrev(int anchor, bool bJump) const
  {
    if (0 == anchor) {
      return 0;
    }

    TBase& base = (TBase&)*this;
    int pos = anchor;

    while (pos && char(' ') == base.m_text[--pos]) {
    }

    while (pos) {
      if (char(' ') == base.m_text[--pos]) {
        pos += 1;
        if (bJump) {
          return pos;
        } else {
          break;
        }
      }
    }

    if (bJump) {
      return pos;
    }

    int pos2 = pos;
    while (pos2 != anchor) {
      pos = pos2;
      pos2 += isACSII((uchar)base.m_text[pos]) ? 1 : 2;
    }

    return pos;
  }

  void updateBoundary()
  {
    TBase& base = (TBase&)*this;

    //
    // Query text size.
    //

    IntPoint sz(0,0);
    std::vector<int> w;
    w.resize(std::max(8, (int)base.m_text.length()));
    if (base.isPassword()) {
      std::string s;
      s.reserve(base.m_text.length());
      s.insert(0, std::string(base.m_text.length(), '*'));
      base.m_pCb->onWidgetQueryTextMetrics(base.m_handle, s, &w[0], sz);
    } else {
      base.m_pCb->onWidgetQueryTextMetrics(base.m_handle, base.m_text, &w[0], sz);
    }

    //
    // Adjust left boundary.
    //

    if (m_chCaret < m_chFirst) {
      m_chFirst = m_chCaret;
    }

    //
    // Get right bound from left to right.
    //

    m_chLast = m_chFirst;

    int c = 0 == m_chFirst ? 0 : w[movePrev(m_chFirst, false)];
    for (; (int)base.m_text.length() > m_chLast;) {
      if (w[m_chLast] - c > base.m_dim.right - 8) {
        break;
      } else {
        m_chLast = moveNext(m_chLast, false);
      }
    }

    //
    // Right bound exceeds boundary, get left bound from right to left.
    //

    if (m_chCaret > m_chLast) {

      m_chFirst = m_chLast = m_chCaret;

      int c = (int)base.m_text.length() == m_chLast ? w[movePrev(m_chLast, false)] : w[m_chLast];
      for (; 0 < m_chFirst;) {
        int p = movePrev(m_chFirst, false);
        if (c - w[p]< base.m_dim.right - 8) {
          m_chFirst = p;
        } else {
          break;
        }
      }

      if ((int)base.m_text.length() == m_chCaret) {
        m_chFirst = moveNext(m_chFirst, false);
      }
    }

    //
    // Calculate caret position.
    //

    int l = 0 == m_chFirst ? 0 : w[m_chFirst - 1];
    int r = 0 == m_chCaret ? 0 : w[m_chCaret - 1];
    m_posCaret = r - l;
  }
};

template<class TBase> class implListbox
{
public:

  struct ListItem
  {
    std::string str;
    uint_ptr user;

    ListItem() : str(), user((uint_ptr)0)
    {
    }

    explicit ListItem(std::string const& s) : str(s), user((uint_ptr)0)
    {
    }
  };

  int m_cyItem;                         // Item height, query when listbox create.
  std::vector<ListItem> m_lst;          // List item.

  void OnMouseDown(int x, int y, uint keyStat)
  {
    TBase& base = (TBase&)*this;

    int last = base.m_lastPt;           // Current selection.
    base.m_lastPt = base.pool()[base.m_child].m_pos + (y - base.getRect().top) / m_cyItem;

    if (base.m_lastPt >= (int)base.m_lst.size()) {
      base.m_lastPt = -1;
    }

    if (last != base.m_lastPt) {
      base.m_pCb->onWidgetCommand(base.m_handle); // Selection change.
    }
  }

  void OnMouseMove(int x, int y, uint keyStat)
  {
    TBase& base = (TBase&)*this;
    if (base.isHot()) {
      base.m_pos = base.pool()[base.m_child].m_pos + // Current hot.
                   (y - base.getRect().top) / m_cyItem;
      if (base.m_lastPt >= (int)base.m_lst.size()) {
        base.m_pos = -1;
      }
    } else {
      base.m_pos = -1;
    }
  }

  void OnMouseWheel(int x, int y, uint keyStat, int delta)
  {
    TBase& base = (TBase&)*this;

    TBase& sb = TBase::pool()[base.m_child];
    sb.OnMouseWheel(x, y, keyStat, delta);
  }

  void render()
  {
    TBase& base = (TBase&)*this;
    TBase& sb = base.pool()[base.m_child];

    IntRect rcItem = base.getRect();

    if (sb.isVisible()) {
      rcItem.right -= SB_EMBEDED_CX;
    }

    rcItem.bottom = rcItem.top + m_cyItem;

    //
    // Render items.
    //

    for (int i = sb.m_pos; i < sb.m_pos + sb.m_page && i < (int)m_lst.size(); i++) {
      base.m_pCb->onWidgetRenderWidget(base.m_handle, SWRS_ITEM, i, rcItem);
      rcItem.offset(0, m_cyItem);
    }
  }
};

template<class TBase> class implMenu
{
public:

  void hideMenu()
  {
    ui::Dialog(((TBase*)this)->m_handle).hideDialog();
  }

  bool OnKeyDown(uint key, uint keyStat)
  {
    if (SWK_ESCAPE == key) {
      hideMenu();
      return true;
    }

    return false;
  }

  void OnMouseDown(int x, int y, uint keyStat)
  {
    TBase& base = (TBase&)*this;
    base.m_lastPt = base.m_pos;
    if (-1 == base.m_lastPt) {

      //
      // Hide menu.
      //

      hideMenu();

      //
      // Pass mouse down event to desk.
      //

      int hParent = base.m_parent;
      while (-1 != hParent && SWWT_DESKTOP != base.pool()[hParent].m_type) {
        hParent = base.pool()[hParent].m_parent;
      }

      if (-1 != hParent) {
        ui::Desktop(hParent).inputMouseDown(x, y, keyStat);
      }
    }
  }

  void OnMouseUp(int x, int y, uint keyStat)
  {
    TBase& base = (TBase&)*this;
    base.m_lastPt = base.m_pos;

    if (-1 != base.m_lastPt) {
      hideMenu();
      base.m_pCb->onWidgetCommand(base.m_handle);
    }
  }
};

template<class TBase> class implRadiobox
{
public:

  void OnMouseUp(int x, int y, uint keyStat)
  {
    TBase& base = (TBase&)*this;

    if (!base.isChecked()) {            // Uncheck grouped siblings.
      base.setChecked(true);
      base.m_pCb->onWidgetCommand(base.m_handle);
    }
  }

  //
  // Helper.
  //

  void rangeUncheck()                   // Uncheck siblings.
  {
    TBase& base = (TBase&)*this;
    int ps = base.m_prevSibling;

    while (-1 != ps) {
      if (SWWT_RADIOBOX != base.pool()[ps].m_type) {
        break;
      }
      base.pool()[ps].setChecked(false);
      ps = base.pool()[ps].m_prevSibling;
    }

    int s = base.m_sibling;
    while (-1 != s) {
      if (SWWT_RADIOBOX != base.pool()[s].m_type) {
        break;
      }
      base.pool()[s].setChecked(false);
      s = base.pool()[s].m_sibling;
    }
  }
};

template<class TBase> class implScrollbar
{
public:

  int m_pos, m_lastPos;                 // Thumb pos, pos also used by listbox/menu as iCurHot.
  int m_min, m_max;                     // Range.
  int m_page;                           // Page size.
  int m_lastPt;                         // Last point when start dragging thumb,
                                        // also used by listbox/menu as iCurSel.

  void OnMouseDown(int x, int y, uint keyStat)
  {
    TBase& base = (TBase&)*this;

    if (0 != (base.m_state & SWWS_DEC_HOT)) {
      dec();
    } else if (0 != (base.m_state & SWWS_INC_HOT)) {
      inc();
    } else if (0 != (base.m_state & SWWS_THUMB_HOT)) {
      base.m_state |= SWWS_THUMB_SELECTED;
      m_lastPos = m_pos;
      m_lastPt = base.isHorz() ? x : y;
    } else {                            // Click on jump area.
      if (base.isHorz()) {
        if (getThumbRect().left > x) {
          m_pos -= m_page;
        } else {
          m_pos += m_page;
        }
      } else {
        if (getThumbRect().top > y) {
          m_pos -= m_page;
        } else {
          m_pos += m_page;
        }
      }
      if (m_min > m_pos) {
        m_pos = m_min;
      } else if (m_max - m_page < m_pos) {
        m_pos = m_max - m_page;
      }

      base.m_pCb->onWidgetCommand(base.m_handle); // Position change.
    }
  }

  void OnMouseMove(int x, int y, uint keyStat)
  {
    TBase& base = (TBase&)*this;
    IntPoint pt(x, y);

    base.m_state &= ~(SWWS_DEC_HOT|SWWS_INC_HOT|SWWS_THUMB_HOT);
    if (base.isHot()) {
      if (!base.isNoBtn() && getDecRect().ptInRect(pt)) {
        base.m_state |= SWWS_DEC_HOT;
      } else if (!base.isNoBtn() && getIncRect().ptInRect(pt)) {
        base.m_state |= SWWS_INC_HOT;
      } else if (getThumbRect().ptInRect(pt)) {
        base.m_state |= SWWS_THUMB_HOT;
      }
    }

    if (base.isThumbSelected()) {

      IntRect rc = base.getRect();
      float u = (base.isHorz() ?
                rc.width() - (base.isNoBtn() ? 0 : 2 * rc.height()) :
                rc.height() - (base.isNoBtn() ? 0 : 2 * rc.width())) / (float)(m_max - m_min);

      if (base.isHorz()) {
        rc.inflate(0, SB_THUMB_DRAG_RANGE);
      } else {
        rc.inflate(SB_THUMB_DRAG_RANGE, 0);
      }

      int pos = m_pos;
      if (rc.ptInRect(pt)) {
        m_pos = m_lastPos + (base.isHorz() ? int((pt.x - m_lastPt) / u) : int((pt.y - m_lastPt) / u));
        if (m_min > m_pos) {
          m_pos = m_min;
        } else if (m_max - m_page < m_pos) {
          m_pos = m_max - m_page;
        }
      } else {
        m_pos = m_lastPos;
      }

      if (pos != m_pos) {
        base.m_pCb->onWidgetCommand(base.m_handle);
      }
    }
  }

  void OnMouseUp(int x, int y, uint keyStat)
  {
    TBase& base = (TBase&)*this;

    if (0 != (base.m_state & SWWS_THUMB_SELECTED) && m_pos != m_lastPos) {
      base.m_pCb->onWidgetCommand(base.m_handle);
    }

    base.m_state &= ~(SWWS_DEC_SELECTED | SWWS_INC_SELECTED | SWWS_THUMB_SELECTED);
    base.m_caretFly = false;
  }

  void OnMouseWheel(int x, int y, uint keyStat, int delta)
  {
    if (!((TBase&)*this).isEnable()) {
      return;
    }
    if (0 < delta) {                    // Rotated forward, away from the user.
      dec();
    } else {                            // Rotated backward, toward the user.
      inc();
    }
  }

  void render()
  {
    TBase& base = (TBase&)*this;

    //
    // Render scroll buttons.
    //

    if (!base.isNoBtn()) {
      base.m_pCb->onWidgetRenderWidget( // Down button.
                    base.m_handle,
                    SWRS_SB_DEC,
                    -1,
                    getDecRect());
      base.m_pCb->onWidgetRenderWidget( // Up button.
                    base.m_handle,
                    SWRS_SB_INC,
                    -1,
                    getIncRect());
    }

    //
    // Render thumb button.
    //

    if (base.isShowNoThumb() || m_max - m_min > m_page) {
      base.m_pCb->onWidgetRenderWidget(base.m_handle, SWRS_SB_THUMB, -1, getThumbRect());
    }
  }

  //
  // Helper.
  //

  void dec()
  {
    TBase& base = (TBase&)*this;
    base.m_state |= SWWS_DEC_SELECTED;

    if (base.m_caretFly) {
      base.m_caretTimer.setTimeout(TIMER_AUTOSCROLL);
    } else {
      base.m_caretTimer.setTimeout(TIMER_PREPARE_AUTOSCROLL);
    }

    if (m_min < m_pos) {
      m_pos -= 1;
      base.m_pCb->onWidgetCommand(base.m_handle); // Position change.
    }
  }

  IntRect getDecRect() const
  {
    TBase& base = (TBase&)*this;

    IntRect rc = base.getRect();
    if (base.isHorz()) {
      rc.right = rc.left + rc.height();
    } else {
      rc.bottom = rc.top + rc.width();
    }

    return rc;
  }

  IntRect getIncRect() const
  {
    TBase& base = (TBase&)*this;

    IntRect rc = base.getRect();
    if (base.isHorz()) {
      rc.left = rc.right - rc.height();
    } else {
      rc.top = rc.bottom - rc.width();
    }

    return rc;
  }

  IntRect getThumbRect() const
  {
    TBase& base = (TBase&)*this;

    IntRect rc = base.getRect();
    if (base.isHorz()) {
      float u = (rc.width() - (base.isNoBtn() ? 0 : 2 * rc.height())) / (float)(m_max - m_min);
      int sz = SB_MIN_THUMB >= int(m_page * u) ? SB_MIN_THUMB : int(m_page * u);
      rc.right = rc.left + sz;
      rc.offset((base.isNoBtn() ? 0 : rc.height()) + int((m_pos - m_min) * u), 0);
    } else {
      float u = (rc.height() - (base.isNoBtn() ? 0 : 2 * rc.width())) / (float)(m_max - m_min);
      int sz = SB_MIN_THUMB >= int(m_page * u) ? SB_MIN_THUMB : int(m_page * u);
      rc.bottom = rc.top + sz;
      rc.offset(0, (base.isNoBtn() ? 0 : rc.width()) + int((m_pos - m_min) * u));
    }

    return rc;
  }

  void inc()
  {
    TBase& base = (TBase&)*this;
    base.m_state |= SWWS_INC_SELECTED;

    if (base.m_caretFly) {
      base.m_caretTimer.setTimeout(TIMER_AUTOSCROLL);
    } else {
      base.m_caretTimer.setTimeout(TIMER_PREPARE_AUTOSCROLL);
    }

    if (m_max - m_page > m_pos) {
      m_pos += 1;
      base.m_pCb->onWidgetCommand(base.m_handle); // Position change.
    }
  }
};

template<class TBase> class implTextbox
{
};

class implWindow :
        public implWindowState,
        public implDesktop<implWindow>,
        public implEditbox<implWindow>,
        public implListbox<implWindow>,
        public implMenu<implWindow>,
        public implRadiobox<implWindow>,
        public implScrollbar<implWindow>,
        public implTextbox<implWindow>
{
public:

  //
  // Instance pool.
  //

  static ObjectPool<implWindow, 1, true>& pool()
  {
    static ObjectPool<implWindow, 1, true> i;
    return i;
  }

  //
  // Creation.
  //

  static int create(int type, int hParent, IntRect const& dim, std::string const& text, std::string const& tip, std::string const& id)
  {
    if (SWWT_DESKTOP != type) {
      if (!pool().isUsed(hParent)) {
        return -1;
      }
      switch (pool()[hParent].m_type)
      {
      case SWWT_WINDOW:                 // Valid type.
      case SWWT_DESKTOP:
      case SWWT_LISTBOX:
        break;
      default:
        return -1;                      // Invalid type.
      }
    }

    int handle = pool().alloc();
    if (-1 == handle) {
      return -1;
    }

    implWindow& iw = pool()[handle];
    iw.m_parent = iw.m_child = iw.m_sibling = iw.m_prevSibling = -1;
    iw.m_type = type;
    iw.m_state = SWWS_VISIBLE;
    iw.m_handle = handle;
    iw.m_dim = dim;
    iw.m_text = text;
    iw.m_tip = tip;
    iw.m_id = id;
    iw.m_user = 0;

    if (SWWT_DESKTOP != type) {
      pool()[hParent].addChild(handle); // Initialize Relationship.
      iw.m_pCb = pool()[hParent].m_pCb;
    }

    return handle;
  }

  //
  // Relationship.
  //

  int findMouseOver(int x, int y)
  {
    if (isVisible()) {

      //
      // Find child first, from last child to first child.
      //

      int c1 = m_child, cN = getLastChild();
      while (-1 != cN) {
        int hot = -1;
        if (pool()[cN].isVisible()) {
          hot = pool()[cN].findMouseOver(x, y);
        }
        if (-1 != hot) {
          return hot;
        }
        if (cN == c1) {
          break;
        }
        cN = pool()[cN].m_prevSibling;
      }

      //
      // Check self.
      //

      if (getRect().ptInRect(IntPoint(x,y))) {
        return m_handle;
      }
    }

    return -1;
  }

  void free()                           // Free this node.
  {
    pool().free(m_handle);
    m_type = SWWT_END_TAG;
  }

  int getLastChild()
  {
    int c = m_child;
    while (-1 != c) {
      if (-1 == pool()[c].m_sibling) {
        break;
      }
      c = pool()[c].m_sibling;
    }

    return c;
  }

  void addChild(int handle)             // Insert new child node.
  {
    assert(-1 != handle);
    if (-1 != m_child) {
      int h = m_child;
      while (-1 != pool()[h].m_sibling) {
        h = pool()[h].m_sibling;
      }
      pool()[h].m_sibling = handle;
      pool()[handle].m_prevSibling = h;
    } else {
      m_child = handle;
    }

    pool()[handle].m_parent = m_handle;
  }

  void remove()                         // Disconnect relationship.
  {
    if (-1 != m_prevSibling) {
      pool()[m_prevSibling].m_sibling = m_sibling;
      if (-1 != m_sibling) {
        pool()[m_sibling].m_prevSibling = m_prevSibling;
      }
    } else {
      if (-1 != m_sibling) {
        pool()[m_sibling].m_prevSibling = -1;
      }
      if (-1 != m_parent) {
        pool()[m_parent].m_child = m_sibling;
      }
    }

    m_parent = m_sibling = m_prevSibling = -1;
  }

  void setParent(int hNewParent)        // Change relations.
  {
    if (SWWT_DESKTOP == m_type) {       // Desktop's parent is not allowed to change.
      return;
    }

    remove();                           // Disconnect.
    pool()[hNewParent].addChild(m_handle); // Re-connect.
  }

  //
  // Advanced state.
  //

  void setChecked(bool bChecked)
  {
    if (bChecked) {
      m_state |= SWWS_CHECKED;
      if (SWWT_RADIOBOX == m_type) {
        implRadiobox<implWindow>::rangeUncheck();
      }
    } else {
      m_state &= ~SWWS_CHECKED;
    }
  }

  void setText(std::string const& newText)
  {
    m_text = newText;

    if (SWWT_EDITBOX == m_type) {
      implEditbox<implWindow>::setText(newText);
    }
  }

  //
  // Key event handler.
  //

  void OnChar(uchar ch, uint keyStat)
  {
    if (m_pCb->onWidgetChar(m_handle, ch, keyStat)) {
      return;                           // Handled.
    }
    if (SWWT_EDITBOX == m_type) {
      implEditbox<implWindow>::OnChar(ch, keyStat);
    }
  }

  void OnKeyDown(uint key, uint keyStat)
  {
    if (m_pCb->onWidgetKeyDown(m_handle, key, keyStat)) {
      return;                           // Handled.
    }
    if (SWWT_EDITBOX == m_type) {
      implEditbox<implWindow>::OnKeyDown(key, keyStat);
    } else if (SWWT_MENU == m_type) {
      implMenu<implWindow>::OnKeyDown(key, keyStat);
    }
  }

  void OnKeyUp(uint key, uint keyStat)
  {
    m_pCb->onWidgetKeyUp(m_handle, key, keyStat);
  }

  //
  // Mouse event handler.
  //

  void OnMouseDown(int x, int y, uint keyStat)
  {
    if (m_pCb->onWidgetMouseDown(m_handle, x, y, keyStat)) {
      return;                           // Handled.
    }
    if (SWWT_SCROLLBAR == m_type) {
      implScrollbar<implWindow>::OnMouseDown(x, y, keyStat);
    } else if (SWWT_LISTBOX == m_type) {
      implListbox<implWindow>::OnMouseDown(x, y, keyStat);
    } else if (SWWT_MENU == m_type) {
      implMenu<implWindow>::OnMouseDown(x, y, keyStat);
    }
  }

  void OnMouseMove(int x, int y, uint keyStat)
  {
    if (m_pCb->onWidgetMouseMove(m_handle, x, y, keyStat)) {
      return;                           // Handled.
    }
    if (SWWT_SCROLLBAR == m_type) {
      implScrollbar<implWindow>::OnMouseMove(x, y, keyStat);
    } else if (SWWT_LISTBOX == m_type || SWWT_MENU == m_type) {
      implListbox<implWindow>::OnMouseMove(x, y, keyStat);
    }
  }

  void OnMouseUp(int x, int y, uint keyStat)
  {
    if (m_pCb->onWidgetMouseUp(m_handle, x, y, keyStat)) {
      return;                           // Handled.
    }
    if (SWWT_BUTTON == m_type && isHot()) {
      m_pCb->onWidgetCommand(m_handle);
    } else if (SWWT_CHECKBOX == m_type && isHot()) {
      setChecked(!isChecked());
      m_pCb->onWidgetCommand(m_handle);
    } else if (SWWT_RADIOBOX == m_type && isHot()) {
      implRadiobox<implWindow>::OnMouseUp(x, y, keyStat);
    } else if (SWWT_SCROLLBAR == m_type) {
      implScrollbar<implWindow>::OnMouseUp(x, y, keyStat);
    } else if (SWWT_MENU == m_type) {
      implMenu<implWindow>::OnMouseUp(x, y, keyStat);
    }
  }

  void OnMouseWheel(int x, int y, uint keyStat, int delta)
  {
    if (m_pCb->onWidgetMouseWheel(m_handle, x, y, keyStat, delta)) {
      return;                           // Handled.
    }
    if (SWWT_SCROLLBAR == m_type) {
      implScrollbar<implWindow>::OnMouseWheel(x, y, keyStat, delta);
    } else if (SWWT_LISTBOX == m_type) {
      implListbox<implWindow>::OnMouseWheel(x, y, keyStat, delta);
    }
  }

  //
  // Windows.
  //

  void renderAll()                      // Recursive render, self 1st then child and sibling.
  {
    if (isVisible()) {
      renderWidget();                   // Render self.
      if (-1 != m_child) {              // Recursive render child.
        pool()[m_child].renderAll();
      }
    }

    if (-1 != m_sibling) {              // Recursive render sibling.
      pool()[m_sibling].renderAll();
    }
  }

  void renderWidget()
  {
    //
    // Render background.
    //

    m_pCb->onWidgetRenderWidget(m_handle, SWRS_BACKGROUND, -1, getRect());

    if (SWWT_EDITBOX == m_type) {
      implEditbox<implWindow>::render();
    } else if (SWWT_SCROLLBAR == m_type) {
      implScrollbar<implWindow>::render();
    } else if (SWWT_LISTBOX == m_type || SWWT_MENU == m_type || SWWT_TEXTBOX == m_type) {
      implListbox<implWindow>::render();
    }
  }

  //
  // Window rect.
  //

  IntRect getRect() const
  {
    IntRect rc(m_dim.left, m_dim.top, m_dim.left + m_dim.right, m_dim.top + m_dim.bottom);

    if (-1 == m_parent || SWWT_TOOLTIP == m_type) {
      return rc;
    }

    IntRect rcParent = pool()[m_parent].getRect();
    rc.offset(rcParent.left, rcParent.top);

    return rc;
  }
};

} // namespace impl

} // namespace sw2

// end of swWidgetImpl.h
