
//
//  Virtual key code.
//
//  Copyright (c) 2006 waync cheng.
//  All Rights Reserved.
//
//  2006/04/28 waync created.
//

///
/// \file
/// \brief Virtual key code.
/// \author Waync Cheng
/// \date 2006/04/28
///

#pragma once

namespace sw2 {

///
/// virtual key code.
///

enum VIRTUAL_KEY_CODE
{
  SWK_BACK    =   0x08,                 ///< Backspace.
  SWK_TAB     =   0x09,                 ///< Tab.

  SWK_CLEAR   =   0x0C,                 ///< Clear.
  SWK_RETURN  =   0x0D,                 ///< Return.

  SWK_SHIFT   =   0x10,                 ///< Shift.
  SWK_CONTROL =   0x11,                 ///< Ctrl.
  SWK_MENU    =   0x12,                 ///< Alt.
  SWK_PAUSE   =   0x13,                 ///< Pause.
  SWK_CAPITAL =   0x14,                 ///< Caps lock.

  SWK_ESCAPE  =   0x1B,                 ///< ESC.

  SWK_SPACE   =   0x20,                 ///< Space bar.
  SWK_PRIOR   =   0x21,                 ///< Page up.
  SWK_NEXT    =   0x22,                 ///< Page down.
  SWK_END     =   0x23,                 ///< End.
  SWK_HOME    =   0x24,                 ///< Home.
  SWK_LEFT    =   0x25,                 ///< Left arrow.
  SWK_UP      =   0x26,                 ///< Up arrow.
  SWK_RIGHT   =   0x27,                 ///< Right arrow.
  SWK_DOWN    =   0x28,                 ///< Down arrow.

  SWK_SNAPSHOT=   0x2C,                 ///< Print screen.
  SWK_INSERT  =   0x2D,                 ///< Ins.
  SWK_DELETE  =   0x2E,                 ///< Del.

/// ASCII '0' thru '9' (0x30 - 0x39).
/// ASCII 'A' thru 'Z' (0x41 - 0x5A).

  SWK_NUMPAD0 =   0x60,                 ///< Numeric keypad 0.
  SWK_NUMPAD1 =   0x61,                 ///< Numeric keypad 1.
  SWK_NUMPAD2 =   0x62,                 ///< Numeric keypad 2.
  SWK_NUMPAD3 =   0x63,                 ///< Numeric keypad 3.
  SWK_NUMPAD4 =   0x64,                 ///< Numeric keypad 4.
  SWK_NUMPAD5 =   0x65,                 ///< Numeric keypad 5.
  SWK_NUMPAD6 =   0x66,                 ///< Numeric keypad 6.
  SWK_NUMPAD7 =   0x67,                 ///< Numeric keypad 7.
  SWK_NUMPAD8 =   0x68,                 ///< Numeric keypad 8.
  SWK_NUMPAD9 =   0x69,                 ///< Numeric keypad 9.
  SWK_MULTIPLY=   0x6A,                 ///< Mul .
  SWK_ADD     =   0x6B,                 ///< Add.
  SWK_SUBTRACT=   0x6D,                 ///< Sub.
  SWK_DECIMAL =   0x6E,                 ///< Decimal.
  SWK_DIVIDE  =   0x6F,                 ///< Div.

  SWK_F1      =   0x70,                 ///< F1.
  SWK_F2      =   0x71,                 ///< F2.
  SWK_F3      =   0x72,                 ///< F3.
  SWK_F4      =   0x73,                 ///< F4.
  SWK_F5      =   0x74,                 ///< F5.
  SWK_F6      =   0x75,                 ///< F6.
  SWK_F7      =   0x76,                 ///< F7.
  SWK_F8      =   0x77,                 ///< F8.
  SWK_F9      =   0x78,                 ///< F9.
  SWK_F10     =   0x79,                 ///< F10.
  SWK_F11     =   0x7A,                 ///< F11 .
  SWK_F12     =   0x7B,                 ///< F12.
  SWK_F13     =   0x7C,                 ///< F13.
  SWK_F14     =   0x7D,                 ///< F14.
  SWK_F15     =   0x7E,                 ///< F15.
  SWK_F16     =   0x7F,                 ///< F16.
  SWK_F17     =   0x80,                 ///< F17.
  SWK_F18     =   0x81,                 ///< F18.
  SWK_F19     =   0x82,                 ///< F19.
  SWK_F20     =   0x83,                 ///< F20.
  SWK_F21     =   0x84,                 ///< F21.
  SWK_F22     =   0x85,                 ///< F22.
  SWK_F23     =   0x86,                 ///< F23.
  SWK_F24     =   0x87,                 ///< F24.

  SWK_NUMLOCK =   0x90,                 ///< Num lock/
  SWK_SCROLL  =   0x91                  ///< Scroll lock.
};

///
/// Key state.
//

enum KEY_STATE
{
  SWKS_LBUTTON    = 1,                  ///< L-Button.
  SWKS_RBUTTON    = 1 << 1,             ///< R-Button.
  SWKS_MBUTTON    = 1 << 2,             ///< M-Button.
  SWKS_DBLCLK     = 1 << 3,             ///< Double click.
  SWKS_ALT        = 1 << 4,             ///< Alt.
  SWKS_CTRL       = 1 << 5,             ///< Ctrl.
  SWKS_SHIFT      = 1 << 6              ///< Shift.
};

} // namespace sw2

// end of file swKeyDef.h
