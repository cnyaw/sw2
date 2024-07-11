
//
//  GUI module.
//
//  Copyright (c) 2006 Waync Cheng.
//  All Rights Reserved.
//
//  2006/04/06 Waync created.
//

///
/// \file
/// \brief GUI module.
///
/// Widget module provides simple and general widgets for GUI applications.
///
/// Example:
///
/// It is easy to use widget module with several simple steps:
///  - Create widget.
///  - Implement DesktopCallback interface.
///    - Handle widget command notify.
///    - Handle render widget notify.
///  - Trigger widget module(for tooltip requirement).
///
/// \code
/// #include "swWidget.h"
///
/// //
/// // Create widget.
/// //
///
/// Desktop desktop;                    // At least a desktop is required.
/// desktop.create(pCallback, IntRect(0,0,640,480)); // See below for more info about pCallback.
///
/// Button button;                      // All widgets are children of desktop.
/// button.create(desktop, IntRect(405, 160, 80, 22), "button", "tooltip");
///
/// //
/// // Implement DesktopCallback.
/// //
///
/// class MyWidgetHandler : public DesktopCallback
/// {
/// public:
///
///   //
///   // Handle command notify. Some widgets will produce this notify after user
///   // input. ex: click on a button or scroll scrollbar, etc.
///   //
///
///   virtual void onWidgetCommand(int sender)
///   {
///     if (button1 == sender)
///     { // button1 is clicked.
///     }
///   }
///
///   //
///   // Handle render widget notify. When wDeskt::render() is called, all widget
///   // will generate render notify to be drawn.
///   //
///
///   virtual void onWidgetRenderWidget(int sender, int action, int index, IntRect const& rc)
///   {
///     if (button1 == sender)
///     { // Render button1.
///     }
///   }
/// };
///
/// //
/// // To trigger tooltip works, this function needs to be called periodically.
/// //
///
/// desktop.trigger();
///
/// \endcode
///
/// \author Waync Cheng
/// \date 2006/04/06
///

#pragma once

#include "swGeometry.h"
#include "swKeyDef.h"
#include "swUtil.h"

namespace sw2 {

///
/// \brief Initialize widget module.
/// \return Return true if success else return false.
///

bool InitializeWidget();

///
/// \brief Uninitialize widget module.
///

void UninitializeWidget();

///
/// Widget type/class.
///

enum WIDGET_TYPE
{
  SWWT_ROOT = 0,                        ///< The root, internal use.
  SWWT_DESKTOP,                         ///< A desktop.
  SWWT_WINDOW,                          ///< A window, dialog.
  SWWT_BUTTON,                          ///< A button.
  SWWT_CHECKBOX,                        ///< A checkbox style button.
  SWWT_RADIOBOX,                        ///< A radio style button.
  SWWT_EDITBOX,                         ///< A single line text edit.
  SWWT_SCROLLBAR,                       ///< A scroll bar.
  SWWT_LISTBOX,                         ///< A listbox.
  SWWT_MENU,                            ///< A popup menu.
  SWWT_TEXTBOX,                         ///< A text box, multiline edit box.
  SWWT_TOOLTIP,                         ///< A tooltip, internal class.
  SWWT_END_TAG
};

///
/// Render state.
///

enum WIDGET_RENDER_STATE
{
  SWRS_BACKGROUND = 0,                  ///< Render background area.
  SWRS_ITEM,                            ///< Render item.
  SWRS_ED_TEXT,                         ///< Render text(edit text).
  SWRS_ED_CARET,                        ///< Render edit control caret.
  SWRS_SB_DEC,                          ///< Render scroll bar up button.
  SWRS_SB_INC,                          ///< Render scroll bar down button.
  SWRS_SB_THUMB,                        ///< Render scroll bar thumb button.
  SWRS_END_TAG
};

namespace ui {

///
/// \brief Widget event notify interface.
///

struct DesktopCallback
{
  ///
  /// \brief Notify when a widget is need to draw.
  /// \param [in] hSender The event sender.
  /// \param [in] action A widget can consist with several parts, this parameter
  ///             indicates which part needs to draw.(see WIDGET_RENDER_ACTION)
  /// \param [in] index When a widget has sub items, there will have N # RA_ITEM(action)
  ///             notifications to render each item(ex: menu), this parameter is
  ///             the index of the item.
  /// \param [in] rc The render area of the widget or item.
  ///

  virtual void onWidgetRenderWidget(int hSender, int action, int index, IntRect const& rc)
  {
  }

  ///
  /// \brief Notify when press a button or select an item of menu, etc.
  /// \param [in] hSender The event sender.
  ///

  virtual void onWidgetCommand(int hSender)
  {
  }

  ///
  /// \brief Notify when receive a char input.
  /// \param [in] hSender The event sender.
  /// \param [in] ch The char.
  /// \param [in] keyStat The key state.(see KEY_STATE)
  /// \return Return true if user handled this event manually and stop dispatch to widget.
  ///

  virtual bool onWidgetChar(int hSender, uchar ch, uint keyStat)
  {
    return false;
  }

  ///
  /// \brief Notify when press a key down.
  /// \param [in] hSender The event sender.
  /// \param [in] key Key code.(see swKeyDef.h)
  /// \param [in] keyStat The key state.(see KEY_STATE)
  /// \return Return true if user handled this event manually and stop dispatch to widget.
  ///

  virtual bool onWidgetKeyDown(int hSender, uint key, uint keyStat)
  {
    return false;
  }

  ///
  /// \brief Notify when release the key.
  /// \param [in] hSender The event sender.
  /// \param [in] key Key code.(see swKeyDef.h)
  /// \param [in] keyStat The key state.(see KEY_STATE)
  /// \return Return true if user handled this event manually and stop dispatch to widget.
  ///

  virtual bool onWidgetKeyUp(int hSender, uint key, uint keyStat)
  {
    return false;
  }

  ///
  /// \brief Notify when mouse button is pressed down.
  /// \param [in] hSender The event sender.
  /// \param [in] x X coordinate of mouse cursor.
  /// \param [in] y Y coordinate of mouse cursor.
  /// \param [in] keyStat The key state.(see KEY_STATE)
  /// \return Return true if user handled this event manually and stop dispatch to widget.
  ///

  virtual bool onWidgetMouseDown(int hSender, int x, int y, uint keyStat)
  {
    return false;
  }

  ///
  /// \brief Notify when mouse is moved.
  /// \param [in] hSender The event sender.
  /// \param [in] x X coordinate of mouse cursor.
  /// \param [in] y Y coordinate of mouse cursor.
  /// \param [in] keyStat The key state.(see KEY_STATE)
  /// \return Return true if user handled this event manually and stop dispatch to widget.
  ///

  virtual bool onWidgetMouseMove(int hSender, int x, int y, uint keyStat)
  {
    return false;
  }

  ///
  /// \brief Notify when release mouse button.
  /// \param [in] hSender The event sender.
  /// \param [in] x X coordinate of mouse cursor.
  /// \param [in] y Y coordinate of mouse cursor.
  /// \param [in] keyStat The key state.(see KEY_STATE)
  /// \return Return true if user handled this event manually and stop dispatch to widget.
  ///

  virtual bool onWidgetMouseUp(int hSender, int x, int y, uint keyStat)
  {
    return false;
  }

  ///
  /// \brief Notify when mouse wheel is rolling.
  /// \param [in] hSender The event sender.
  /// \param [in] x X coordinate of mouse cursor.
  /// \param [in] y Y coordinate of mouse cursor.
  /// \param [in] keyStat The key state.(see KEY_STATE)
  /// \param [in] delta The rolling distance, >0 means roll forward.
  /// \return Return true if user handled this event manually and stop dispatch to widget.
  ///

  virtual bool onWidgetMouseWheel(int hSender, int x, int y, uint keyStat, int delta)
  {
    return false;
  }

  ///
  /// \brief Notify when need to know or adjust mouse cursor size and position,
  ///        ex: before a tooltip is visible.
  /// \param [in] hSender The event sender.(desktop)
  /// \param [out] rc Return the metrics of cursor position and size(x,y,w,h).
  /// \note The rc parameter is filled with default value, and the application
  ///       can modify it to proper values.
  ///

  virtual void onWidgetQueryCursorMetrics(int hSender, IntRect& rc)
  {
  }

  ///
  /// \brief Notify when need to calculate an item dimension, ex: a listbox item.
  /// \param [in] hSender The event sender.
  /// \param [in] index Item index.
  /// \param [out] sz Dimension of the item(width,height)
  ///

  virtual void onWidgetQueryItemMetrics(int hSender, int index, IntPoint& sz)
  {
  }

  ///
  /// \brief Notify when need to calculate a string dimension.
  /// \param [in] hSender The event sender.
  /// \param [in] str A string.
  /// \param [out] pChW Array of width of each char(from the string start to char),
  ///              size of the array is equal to string length. Skip if pChW = 0.
  /// \param [out] sz Dimension of string(width,height)
  ///

  virtual void onWidgetQueryTextMetrics(int hSender, std::string const& str, int* pChW, IntPoint& sz)
  {
  }
};

///
/// \brief Window widget.
///
/// - Render notify: Send one time for background render.(RA_BACKGROUND)
/// - Command notify: None.
/// - Other notify: Mouse and keyboard.
///

class Window
{
public:

  Window();
  Window(int hWindow);

  operator int() const
  {
    return handle;
  }

  ///
  /// \brief Check is this a valid window.
  /// \return Return true if this is a valid window else return false.
  ///

  bool isWindow() const;

  ///
  /// \brief Create a window widget.
  /// \param [in] hParent Parent widget.
  /// \param [in] dim Dimension of widget, left-top corner relative to
  ///             parent, (left/top) is x/y, (right/bottom) is width/height.
  /// \param [in] text Name or display text.
  /// \param [in] tip Tooltip text.
  /// \param [in] id User defined widget ID, can be duplicate.
  /// \return Return widget handle if success else return -1.
  ///

  int create(int hParent, IntRect const& dim, std::string const& text = "", std::string const& tip = "", std::string const& id = "");

  ///
  /// \brief Destroy a widget, include all it's child widget.
  ///

  void destroy();

  ///
  /// \brief Get topmost parent widget(desktop).
  /// \return Return topmost parent widget handle(desktop).
  ///

  int getDesktop() const;

  ///
  /// \brief Get first child widget.
  /// \return Return first child widget handle else return -1 if no child.
  ///

  int getChild() const;

  ///
  /// \brief Find child widget with specified ID.
  /// \param [in] id Specified ID.
  /// \param [in] bRecursive Find in recursive?
  /// \return Return child widget handle with specified ID else return -1.
  /// \note There may not only one widget has specified ID, but the implementation
  ///       returns the first match.
  ///

  int findChild(std::string const& id, bool bRecursive = true) const;

  ///
  /// \brief Get parent widget.
  /// \return Return parent widget handle.
  ///

  int getParent() const;

  ///
  /// \brief Get first sibling widget.
  /// \return Return first sibling widget handle else return -1 if no sibling.
  ///

  int getSibling() const;

  ///
  /// \brief Get widget type.
  /// \return See WIDGET_TYPE.
  ///

  int getType() const;

  ///
  /// \brief Get widget name or display text.
  /// \return Return widget name or display text.
  ///

  std::string getText() const;

  ///
  /// \brief Get widget tooltip text.
  /// \return Return widget tooltip text.
  ///

  std::string getTip() const;

  ///
  /// \brief Set widget name or display text.
  /// \param [in] newText New name or display text.
  ///

  void setText(std::string const& newText);

  ///
  /// \brief Set widget tooltip text.
  /// \param [in] newTip New tooltip text.
  ///

  void setTip(std::string const& newTip);

  ///
  /// \brief Check is widget enable?
  /// \return Return true if widget is enable else return false.
  ///

  bool isEnable() const;

  ///
  /// \brief Check is widget enable to get keyboard focus?
  /// \return Return true if widget is enable to get focus else return false.
  ///

  bool isEnableFocus() const;

  ///
  /// \brief Check is widget focused?
  /// \return Return true if widget is focused else return false.
  ///

  bool isFocused() const;

  ///
  /// \brief Check is mouse cursor over the widget?
  /// \return Return true if mouse cursor is over the widget else return false.
  ///

  bool isHot() const;

  ///
  /// \brief Check is widget selected by mouse(before release button)?
  /// \return Return true if widget is selected by mouse else return false.
  ///

  bool isSelected() const;

  ///
  /// \brief Check is widget visible?
  /// \return Return true if widget is visible else return false.
  ///

  bool isVisible() const;

  ///
  /// \brief Set enable state.
  /// \param [in] bEnable true to set widget enable; false to set disable.
  ///

  void setEnable(bool bEnable);

  ///
  /// \brief Set enable focus state.
  /// \param [in] bEnable true to set widget enable focus; false to disable it.
  /// \note Edit widget default is enable focus.
  ///

  void setEnableFocus(bool bEnable);

  ///
  /// \brief Set keyboard focus state.
  /// \param [in] bFocus true to set current focus to this widget; false to
  ///             release focus.
  ///

  void setFocus(bool bFocus);

  ///
  /// \brief Set visible state.
  /// \param [in] bVisible true to set widget visible; false to set invisible.
  ///

  void setVisible(bool bVisible);

  ///
  /// \brief Get widget dimension(left,top,width,height)
  /// \return Return widget dimension.
  ///

  IntRect getDim() const;

  ///
  /// \brief Get widget rect area relative to desktop(left,top,right,bottom)
  /// \return Return widget rect area.
  ///

  IntRect getRect() const;

  ///
  /// \brief Set widget dimension.
  /// \param [in] dim New widget dimension, left-top corner is relative to
  ///             parent(left/top), (right/bottom) are width/height.
  ///

  void setDim(IntRect const& dim);

  ///
  /// \brief Get user define widget ID.
  /// \return Return widget ID.
  ///

  std::string getId() const;

  ///
  /// \brief Set widget ID.
  /// \param [in] id New widget ID.
  ///

  void setId(std::string const& id);

  ///
  /// \brief Get user define data.
  /// \return Return user define data.
  ///

  uint_ptr getUserData() const;

  ///
  /// \brief Set user define data.
  /// \param [in] userData User define data.
  ///

  void setUserData(uint_ptr userData);

  ///
  /// \brief The handle of widget, -1 is invalid handle.
  ///

  int handle;
};

///
/// \brief Dialog widget.
///
/// - Render notify: Send one time for background render.(RA_BACKGROUND)
/// - Command notify: None.
/// - Other notify: Mouse and keyboard.
///

class Dialog : public Window
{
public:

  Dialog() : Window()
  {
  }

  Dialog(int hWindow) : Window(hWindow)
  {
  }

  ///
  /// \brief Create a dialog widget.
  /// \param [in] hParent Parent widget.
  /// \param [in] dim Dimension of widget, left-top corner relative to
  ///             parent, (left/top) is x/y, (right/bottom) is width/height.
  /// \param [in] text Name or display text.
  /// \param [in] tip Tooltip text.
  /// \param [in] id Widget ID.
  /// \return Return widget handle if success else return -1.
  ///

  int create(int hParent, IntRect const& dim, std::string const& text = "", std::string const& tip = "", std::string const& id = "");

  ///
  /// \brief Show dialog in modal mode.
  /// \return Return true if success else return false.
  ///

  bool showDialog();

  ///
  /// \brief Close and hide dialog.
  /// \return Return true if success else return false.
  ///

  bool hideDialog();
};

///
/// \brief Desktop widget.
///
/// - Render notify: Send one time for background render.(RA_BACKGROUND)
/// - Command notify: None.
/// - Other notify: Mouse and keyboard.
///

class Desktop : public Window
{
public:

  Desktop() : Window()
  {
  }

  Desktop(int hWindow) : Window(hWindow)
  {
  }

  ///
  /// \brief Create a desktop widget; it is the super parent of all widgets; an
  ///        application can have different desktop widgets at the same time.
  /// \param [in] pCallback Event callback must set, else can't get any notify.
  /// \param [in] dim Dimension of widget, left-top corner relative to
  ///             screen, (left/top) is x/y, (right/bottom) is width/height.
  /// \param [in] text Name or display text.
  /// \param [in] tip Tooltip text.
  /// \param [in] id Widget ID.
  /// \return Return widget handle if success else return -1.
  ///

  int create(DesktopCallback* pCallback, IntRect const& dim, std::string const& text = "", std::string const& tip = "", std::string const& id = "");

  ///
  /// \brief Trigger this desktop and all it's child widget.
  /// \note If this function is not called periodically, the widgets can still
  ///       work properly except tooltip feature.
  ///

  void trigger();

  ///
  /// \brief Trigger mouse button down event.
  /// \param [in] x X coordinate of mouse cursor.
  /// \param [in] y Y coordinate of mouse cursor.
  /// \param [in] keyStat Key state, see KEY_STATE.
  ///

  void inputMouseDown(int x, int y, uint keyStat);

  ///
  /// \brief Trigger mouse move event.
  /// \param [in] x X coordinate of mouse cursor.
  /// \param [in] y Y coordinate of mouse cursor.
  /// \param [in] keyStat Key state, see KEY_STATE.
  ///

  void inputMouseMove(int x, int y, uint keyStat);

  ///
  /// \brief Trigger mouse button up event.
  /// \param [in] x X coordinate of mouse cursor.
  /// \param [in] y Y coordinate of mouse cursor.
  /// \param [in] keyStat Key state, see KEY_STATE.
  ///

  void inputMouseUp(int x, int y, uint keyStat);

  ///
  /// \brief Trigger mouse wheel event.
  /// \param [in] x X coordinate of mouse cursor.
  /// \param [in] y Y coordinate of mouse cursor.
  /// \param [in] keyStat Key state, see KEY_STATE.
  /// \param [in] delta Mouse wheel distance, >0 means roll forward.
  ///

  void inputMouseWheel(int x, int y, uint keyStat, int delta);

  ///
  /// \brief Trigger char input event.
  /// \param [in] ch Input char; if input a unicode char should call twice.
  /// \param [in] keyStat Key state, see KEY_STATE.
  ///

  void inputChar(char ch, uint keyStat);

  ///
  /// \brief Trigger key down event.
  /// \param [in] key Input key code, see swKeyDef.h.
  /// \param [in] keyStat Key state, see KEY_STATE.
  ///

  void inputKeyDown(uint key, uint keyStat);

  ///
  /// \brief Trigger key up event.
  /// \param [in] key Input key code, see swKeyDef.h.
  /// \param [in] keyStat Key state, see KEY_STATE.
  ///

  void inputKeyUp(uint key, uint keyStat);

  ///
  /// \brief Trigger render event.
  /// \note According to current state of any visible widget, application renders
  ///       the widget in DesktopCallback::onWidgetRenderWidget.
  ///

  void render();
};

///
/// \brief Button widget.
///
/// - Render notify: Send once for background render(RA_BACKGROUND). Use select/hot
///                  state to render the button.
/// - Command notify: Notify on mouse button release, the release action is valid
///                   when mouse cursor is over the button.
/// - Other notify: None.
///

class Button : public Window
{
public:

  Button() : Window()
  {
  }

  Button(int hWindow) : Window(hWindow)
  {
  }

  ///
  /// \brief Create a button widget.
  /// \param [in] hParent Parent widget.
  /// \param [in] dim Dimension of widget, left-top corner relative to
  ///             parent, (left/top) is x/y, (right/bottom) is width/height.
  /// \param [in] text Name or display text.
  /// \param [in] tip Tooltip text.
  /// \param [in] id Widget ID.
  /// \return Return widget handle if success else return -1.
  ///

  int create(int hParent, IntRect const& dim, std::string const& text = "", std::string const& tip = "", std::string const& id = "");
};

///
/// \brief Checkbox widget.
///
/// - Render notify: Send once for background render(RA_BACKGROUND). Use select/hot
///                  state to render the checkbox.
/// - Command notify: Notify on mouse button release, the release action is valid
///                   when mouse cursor is over the button.
/// - Other notify: None.
///

class Checkbox : public Button
{
public:

  Checkbox() : Button()
  {
  }

  Checkbox(int hWindow) : Button(hWindow)
  {
  }

  ///
  /// \brief Create a checkbox widget.
  /// \param [in] hParent Parent widget.
  /// \param [in] dim Dimension of widget, left-top corner relative to
  ///             parent, (left/top) is x/y, (right/bottom) is width/height.
  /// \param [in] text Name or display text.
  /// \param [in] tip Tooltip text.
  /// \param [in] id Widget ID.
  /// \return Return widget handle if success else return -1.
  ///

  int create(int hParent, IntRect const& dim, std::string const& text = "", std::string const& tip = "", std::string const& id = "");

  ///
  /// \brief Get checkbox check state?
  /// \return Return true if checkbox is checked else return false.
  ///

  bool isChecked();

  ///
  /// \brief Set check state.
  /// \param [in] bChecked true to set checked; false to set unchecked.
  ///

  void setChecked(bool bChecked);
};

///
/// \brief Radio button widget.
///
/// - Render notify: Send once for background render(RA_BACKGROUND). Use select/hot
///                  state to render the button.
/// - Command notify: Notify on mouse button release, the release action is valid
///                   when mouse cursor is over the button.
/// - Other notify: None.
///

class Radiobox : public Checkbox
{
public:

  Radiobox() : Checkbox()
  {
  }

  Radiobox(int hWindow) : Checkbox(hWindow)
  {
  }

  ///
  /// \brief Create a radio button widget.
  /// \param [in] hParent Parent widget.
  /// \param [in] dim Dimension of widget, left-top corner relative to
  ///             parent, (left/top) is x/y, (right/bottom) is width/height.
  /// \param [in] text Name or display text.
  /// \param [in] tip Tooltip text.
  /// \param [in] id Widget ID.
  /// \return Return widget handle if success else return -1.
  ///

  int create(int hParent, IntRect const& dim, std::string const& text = "", std::string const& tip = "", std::string const& id = "");
};

///
/// \brief Editbox widget.
///
/// - Render notify: At most 3 notify, 1: erase background(RA_BACKGROUND), 2: render
///                  text(RA_ED_TEXT), use getDispTextPos and getDispTextLen for
///                  current state, 3: render cursor(RA_ED_CARET).
/// - Command notify: Notify when press Enter key and has keyboard focus.
/// - Other notify: None.
///

class Editbox : public Window
{
public:

  Editbox() : Window()
  {
  }

  Editbox(int hWindow) : Window(hWindow)
  {
  }

  ///
  /// \brief Create a editbox widget.
  /// \param [in] hParent Parent widget.
  /// \param [in] dim Dimension of widget, left-top corner relative to
  ///             parent, (left/top) is x/y, (right/bottom) is width/height.
  /// \param [in] text Name or display text.
  /// \param [in] tip Tooltip text.
  /// \param [in] id Widget ID.
  /// \return Return widget handle if success else return -1.
  ///

  int create(int hParent, IntRect const& dim, std::string const& text = "", std::string const& tip = "", std::string const& id = "");

  ///
  /// \brief Check is in number mode?
  /// \return Return true if in number mode else return false.
  ///

  bool isNumber() const;

  ///
  /// \brief Check is in password mode?
  /// \return Return if in password mode else return false.
  ///

  bool isPassword() const;

  ///
  /// \brief Set number mode.
  /// \param [in] bNumber true to set number mode; false to disable number mode.
  ///

  void setNumberMode(bool bNumber);

  ///
  /// \brief Set password mode.
  /// \param [in] bPassword true to set password mode; false to disable password mode.
  ///

  void setPasswordMode(bool bPassword);

  ///
  /// \brief Get max text length.
  /// \return Return max text length.
  ///

  int getLimit() const;

  ///
  /// \brief Set max text length.
  /// \param [in] cchMax Max text length.
  ///

  void setLimit(int cchMax);

  ///
  /// \brief Get first visible char index.
  /// \return Return first visible char index.
  ///

  int getDispTextPos() const;

  ///
  /// \brief Get visible text length.
  /// \return Return visible text length.
  ///

  int getDispTextLen() const;
};

///
/// \brief Scrollbar widget.
///
/// - Render notify: At most 4 notify: 1, erase background(RA_BACKGROUND), 2, draw
///                  scrollbar thumb(RA_SB_THUMB), 3, draw decrease button(RA_SB_DEC),
///                  4, draw increase button(RA_SB_INC).
/// - Command notify: Notify when scroll position change.
/// - Other notify: None.
///

class Scrollbar : public Window
{
public:

  Scrollbar() : Window()
  {
  }

  Scrollbar(int hWindow) : Window(hWindow)
  {
  }

  ///
  /// \brief Create a scrollbar widget.
  /// \param [in] hParent Parent widget.
  /// \param [in] dim Dimension of widget, left-top corner relative to
  ///             parent, (left/top) is x/y, (right/bottom) is width/height.
  /// \param [in] text Name or display text.
  /// \param [in] tip Tooltip text.
  /// \param [in] id Widget ID.
  /// \return Return widget handle if success else return -1.
  ///

  int create(int hParent, IntRect const& dim, std::string const& text = "", std::string const& tip = "", std::string const& id = "");

  ///
  /// \brief Get scroll position.
  /// \return Return scroll position.
  ///

  int getPos() const;

  ///
  /// \brief Set scroll position.
  /// \param [in] pos New position.
  ///

  void setPos(int pos);

  ///
  /// \brief Get scroll range.
  /// \param [out] min Minimum position.
  /// \param [out] max Maximum position.
  ///

  void getRange(int& min, int& max) const;

  ///
  /// \brief Set scroll range.
  /// \param [in] min Minimum position.
  /// \param [in] max Maximum position.
  ///

  void setRange(int min, int max);

  ///
  /// \brief Get page size.
  /// \return Return page size.
  ///

  int getPageSize() const;

  ///
  /// \brief Set page size.
  /// \param [in] szPage New page size(in pixel).
  ///

  void setPageSize(int szPage);

  ///
  /// \brief Get is horizontal scrollbar?
  /// \return Return true if this is a horizontal scrollbar else return false.
  ///

  bool isHorz() const;

  ///
  /// \brief Set scrollbar style.
  /// \param [in] bHorz true to set horizontal scrollbar; false to set vertical
  ///             scrollbar.
  ///

  void setHorz(bool bHorz);

  ///
  /// \brief Get is no button style.
  /// \return Return true if is no button style else return false.
  ///

  bool isNoBtn() const;

  ///
  /// \brief Set no button style.
  /// \param [in] bNoBtn true to set no button style; false to set has button style.
  ///

  void setNoBtn(bool bNoBtn);

  ///
  /// \brief Get is no thumb button style.
  /// \return Return true if is no thumb button style else return false.
  ///

  bool isShowNoThumb() const;

  ///
  /// \brief Set no thumb button style.
  /// \param[in] bShowNoThumb true to set no thumb button style; false to set has
  ///            thumb button style.
  ///

  void setShowNoThumb(bool bShowNoThumb);

  ///
  /// \brief Get is dec button selected?
  /// \return Return true if dec button is selected else return false.
  ///

  bool isDecSelected() const;

  ///
  /// \brief Get is inc button selected?
  /// \return Return true if inc button is selected else return false.
  ///

  bool isIncSelected() const;

  ///
  /// \brief Get is thumb button selected?
  /// \return Return true if thumb button is selected else return false.
  ///

  bool isThumbSelected() const;

  ///
  /// \brief Get is dec button hot?
  /// \return Return true if dec button is hot.
  ///

  bool isDecHot() const;

  ///
  /// \brief Get is inc button hot?
  /// \return Return true if inc button is hot else return false.
  ///

  bool isIncHot() const;

  ///
  /// \brief Get is thumb button hot?
  /// \return Return true if thumb button is hot else return false.
  ///

  bool isThumbHot() const;
};

///
/// \brief Listbox widget.
///
/// - Query notify: Notify query metrics on creation for item's height(fix size).
/// - Render notify: At least produce one render background notify(RA_BACKGROUND),
///                  and produce several times render item notify(RA_ITEM).
/// - Command notify: Notify when selected item change.
/// - Other notify: None.
///

class Listbox : public Window
{
public:

  Listbox() : Window()
  {
  }

  Listbox(int hWindow) : Window(hWindow)
  {
  }

  ///
  /// \brief Create a listbox widget.
  /// \param [in] hParent Parent widget.
  /// \param [in] dim Dimension of widget, left-top corner relative to
  ///             parent, (left/top) is x/y, (right/bottom) is width/height.
  /// \param [in] text Name or display text.
  /// \param [in] tip Tooltip text.
  /// \param [in] id Widget ID.
  /// \return Return widget handle if success else return -1.
  ///

  int create(int hParent, IntRect const& dim, std::string const& text = "", std::string const& tip = "", std::string const& id = "");

  ///
  /// \brief Get item count.
  /// \return Return item count.
  ///

  int getCount() const;

  ///
  /// \brief Add an item to the end of list.
  /// \param [in] str A text.
  /// \return Return index of new item else return -1 if fail.
  ///

  int addString(std::string const& str);

  ///
  /// \brief Remove specified item.
  /// \param [in] index Index of the item to remove.
  ///

  void delString(int index);

  ///
  /// \brief Clear all items.
  ///

  void clear();

  ///
  /// \brief Get text of specified item.
  /// \param [in] index Specified item index.
  /// \return Return text of specified item.
  ///

  std::string getString(int index) const;

  ///
  /// \brief Set text of specified item.
  /// \param [in] index Specified item index.
  /// \param [in] str New item text.
  ///

  void setString(int index, std::string const& str);

  ///
  /// \brief Get user define data of specified item.
  /// \param [in] index Specified item index.
  /// \return Return user define data of specified item.
  ///

  uint_ptr getData(int index) const;

  ///
  /// \brief Set user define data of specified item.
  /// \param [in] index Specified item index.
  /// \param [in] user New user define data.
  ///

  void setData(int index, uint_ptr user);

  ///
  /// \brief Get first visible item index.
  /// \return Return first visible item index return -1 if list is empty.
  ///

  int getFirstItem() const;

  ///
  /// \brief Set first visible item index.
  /// \param [in] index Specified item index.
  ///

  void setFirstItem(int index);

  ///
  /// \brief Get selected item index.
  /// \return Return selected item index else return -1 if no selected item.
  ///

  int getCurSel() const;

  ///
  /// \brief Set selected item index.
  /// \param [in] index Specified item index; -1 for deselect.
  /// \note If the item index is invalid then the selection state is no change.
  ///

  void setCurSel(int index);

  ///
  /// \brief Get hot item index.(item under mouse cursor)
  /// \return Return hot item index else return -1 if no hot item.
  ///

  int getCurHot() const;

  ///
  /// \brief Get max item count.
  /// \return Return max item count.
  ///

  int getLimit() const;

  ///
  /// \brief Set max item count.
  /// \param[in] maxItem Max item count.
  ///

  void setLimit(int maxItem);

  ///
  /// \brief Get scrollbar widget.
  /// \return Return scrollbar widget handle.
  /// \note This srcollbar is maintained internally, do not destroy it.
  ///

  int getScrollbar() const;
};

///
/// \brief Menu widget.
///
/// - Query notify: Notify on creation to query menu item height(fix size).
/// - Render notify: At least send a render notify to render background(RA_BACKGROUND),
///                  and may produce several render menu item notify(RA_ITEM).
/// - Command notify: Notify when click on a menu item, and close the menu.
/// - Other notify: None.
///

class Menu : public Listbox
{
public:

  Menu() : Listbox()
  {
  }

  Menu(int hWindow) : Listbox(hWindow)
  {
  }

  ///
  /// \brief Create a menu widget.
  /// \param [in] hParent Parent widget.
  /// \param [in] id Widget ID.
  /// \return Return widget handle if success else return -1.
  ///

  int create(int hParent, std::string const& id = "");

  ///
  /// \brief Show popup menu.
  /// \param [in] pt Left-top position of menu to display, relative to desktop.
  /// \return Return true if success else return false.
  /// \note When menu is displayed, click any position on the screen or press ESC
  ///       key, the menu is closed automatically. Therefore there is no CloseMenu
  ///       function.
  ///

  bool showMenu(IntPoint const& pt);
};

///
/// \brief Textbox widget.
///
/// - Render notify: At least send a render background notify(RA_BACKGROUND), and
///                  may send several times of render item(RA_ITEM).
/// - Command notify: None.
/// - Other notify: None.
///

class Textbox : public Editbox
{
public:

  Textbox() : Editbox()
  {
  }

  Textbox(int hWindow) : Editbox(hWindow)
  {
  }

  ///
  /// \brief Create a textbox widget.
  /// \param [in] hParent Parent widget.
  /// \param [in] dim Dimension of widget, left-top corner relative to
  ///             parent, (left/top) is x/y, (right/bottom) is width/height.
  /// \param [in] text Name or display text.
  /// \param [in] tip Tooltip text.
  /// \param [in] id Widget ID.
  /// \return Return widget handle if success else return -1.
  ///

  int create(int hParent, IntRect const& dim, std::string const& text = "", std::string const& tip = "", std::string const& id = "");

  ///
  /// \brief Get line count.
  /// \return Return line count.
  //

  int getLineCount() const;

  ///
  /// \brief Get text of specified line.
  /// \param [in] line Line number.
  /// \return Return text of specified line number.
  ///

  std::string getLine(int line) const;

  ///
  /// \brief Get scrollbar widget.
  /// \return Return scrollbar widget handle.
  /// \note This srcollbar is maintained internally, do not destroy it.
  ///

  int getScrollbar() const;
};

} // namespace ui

} // namespace sw2

// end of swWidget.h
