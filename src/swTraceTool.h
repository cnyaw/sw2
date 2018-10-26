
//
//  Trace utility.
//
//  Copyright (c) 2006 Waync Cheng.
//  All Rights Reserved.
//
//  2006/03/15 Waync created.
//

///
/// \file
/// \brief Trace utility.
///
/// TraceTool provides several simple MACROs help to trace debug messages.
///
/// Example:
///
/// \code
/// #include "swTraceTool.h"
///
/// //
/// // Output messages.
/// //
///
/// SW2_TRACE_MESSAGE("This is a message"); // Aug-27-07 21:02:19 [MESSAGE] This is a message
/// SW2_TRACE_WARNING("This is a warning"); // Aug-27-07 21:02:19 [WARNING] This is a warning
/// SW2_TRACE_ERROR("This is an error");    // Aug-27-07 21:02:19 [ERROR] This is an error
///
/// //
/// // Output formated messages.
/// //
///
/// SW2_TRACE_MESSAGE("This is an int %d", aInt);
/// SW2_TRACE_WARNING("This is a flost %.2f", aFloat);
/// SW2_TRACE_ERROR("This is a string %s", aStr);
///
/// //
/// // Enable/disable trace message.
/// //
///
/// SW2_TRACE_ENABLE_TARGET(false); // Disable all output message.
/// SW2_TRACE_MESSAGE("trace nothing");
///
/// SW2_TRACE_ENABLE_TARGET(true); // Enable all output message.
/// SW2_TRACE_MESSAGE("trace again");
///
/// SW2_TRACE_ENABLE_TARGET(false, stdout); // Disable stdout output message.
///
/// SW2_TRACE_ENABLE_TARGET(true, stdout); // Enable stdout output message.
///
/// //
/// // Manage trace target.
/// //
///
/// SW2_TRACE_RESET_TARGET(); // Clear all output targets.
/// SW2_TRACE_ADD_TARGET(stdout); // Add stdout as output target.
/// SW2_TRACE_ADD_TARGET(fHandle); // Added file fHandle as output target.
///
/// \endcode
///
/// \author Waync Cheng
/// \date 2006/03/15
///

#pragma once

#include <stdarg.h>
#include <stdio.h>

namespace sw2 {

class TraceTool
{
public:

  static void message(int level, const char* format, ...);
  static void warning(int level, const char* format, ...);
  static void error(int level, const char* format, ...);

  static void enableTarget(bool bEnable, FILE* out = 0);
  static void resetTarget();
  static void addOutputTarget(FILE* out, int level = 0);

  static void setTimeStampFormat(const char* format);
  static void setTraceFunc(void (*pfnTrace)(int level, const char* format, va_list args));
};

#define SW2_TRACE_MESSAGE(fmt,...) sw2::TraceTool::message(0, fmt, ##__VA_ARGS__)
#define SW2_TRACE_WARNING(fmt,...) sw2::TraceTool::warning(0, fmt, ##__VA_ARGS__)
#define SW2_TRACE_ERROR(fmt,...) sw2::TraceTool::error(0, fmt, ##__VA_ARGS__)
#define SW2_TRACE_MESSAGE_LEVEL(lvl,fmt,...) sw2::TraceTool::message(lvl, fmt, ##__VA_ARGS__)
#define SW2_TRACE_WARNING_LEVEL(lvl,fmt,...) sw2::TraceTool::warning(lvl, fmt, ##__VA_ARGS__)
#define SW2_TRACE_ERROR_LEVEL(lvl,fmt,...) sw2::TraceTool::error(lvl, fmt, ##__VA_ARGS__)

#define SW2_TRACE_ENABLE_TARGET sw2::TraceTool::enableTarget
#define SW2_TRACE_RESET_TARGET() sw2::TraceTool::resetTarget();
#define SW2_TRACE_ADD_TARGET sw2::TraceTool::addOutputTarget

#define SW2_TRACE_TIMESTAMP_FORMAT(Fmt) sw2::TraceTool::setTimeStampFormat(Fmt);
#define SW2_TRACE_FUNC(Func) sw2::TraceTool::setTraceFunc(Func);

} // namespace sw2

// end of swTraceTool.h
