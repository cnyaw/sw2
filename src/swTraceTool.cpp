
//
//  Trace utility.
//
//  Copyright (c) 2006 Waync Cheng.
//  All Rights Reserved.
//
//  2006/03/15 Waync created.
//

#include <assert.h>
#include <stdio.h>

#include <cstdarg>
#include <ctime>

#include <sstream>

#include "swTraceTool.h"

namespace sw2 {

namespace impl {

#define MAX_OUTPUT 32

class implTraceTool
{
  implTraceTool() : nOut(0), nEnable(0), fmt("%Y-%m-%d %H:%M:%S "), pfnTrace(0)
  {
    addOutputTarget(stdout);
  }

public:

  int nOut, nEnable;
  std::string fmt;
  FILE* fOut[MAX_OUTPUT];
  bool isEnable[MAX_OUTPUT];
  void (*pfnTrace)(const char* format, va_list args);

  enum CONST_TRACETOOL
  {
    MAX_STR_LEN = 1024,
    TRACECAT_MESSAGE = 0,
    TRACECAT_WARNING,
    TRACECAT_ERROR
  };

  static implTraceTool& inst()
  {
    static implTraceTool i;
    return i;
  }

  void addOutputTarget(FILE* out)
  {
    if (MAX_OUTPUT > nOut) {
      isEnable[nOut] = true;
      fOut[nOut++] = out;
      nEnable += 1;
    }
  }

  void enableTarget(bool bEnable, FILE* out)
  {
    if (0 == out) {
      nEnable = bEnable ? 0 : nOut;
      for (int i = 0; i < nOut; i++) {
        isEnable[i] = bEnable;
      }
    } else {
      for (int i = 0; i < nOut; i++) {
        if (fOut[i] == out) {
          if (bEnable) {
            if (isEnable[i] != bEnable) {
              nEnable += 1;
            }
          } else {
            if (isEnable[i] != bEnable) {
              nEnable -= 1;
            }
          }
          isEnable[i] = bEnable;
          break;
        }
      }
    }
  }

  void doTrace(int cat, char const* str) const
  {
    char buf[MAX_STR_LEN];
    buf[0] = '\0';

    time_t now = ::time(0);
    struct tm tmNow = *::localtime(&now);
    ::strftime(buf, sizeof(buf), fmt.c_str(), &tmNow);

    char const* scat[] = {"[MESSAGE] ", "[WARNING] ", "[ERROR] "};

    std::stringstream ss;
    ss << buf << scat[cat] << str << "\n";

    std::string s = ss.str();
    for (int i = 0; i < nOut; i++) {
      if (isEnable[i]) {
        fprintf(fOut[i], "%s", s.c_str());
        fflush(fOut[i]);
      }
    }
  }
};

} // namespace impl

#define DO_TRACE(type) \
  assert(format); \
  impl::implTraceTool& inst = impl::implTraceTool::inst(); \
  if (0 >= inst.nEnable || 0 == inst.nOut) { \
    return; \
  } \
  va_list va; \
  va_start(va, format); \
  if (inst.pfnTrace) { \
    inst.pfnTrace(format, va); \
  } else { \
    char buf[impl::implTraceTool::MAX_STR_LEN]; \
    vsnprintf(buf, impl::implTraceTool::MAX_STR_LEN, format, va); \
    inst.doTrace((type), buf); \
  } \
  va_end(va);

void TraceTool::error(const char* format, ...)
{
  DO_TRACE(impl::implTraceTool::TRACECAT_ERROR)
}

void TraceTool::message(const char* format, ...)
{
  DO_TRACE(impl::implTraceTool::TRACECAT_MESSAGE)
}

void TraceTool::warning(const char* format, ...)
{
  DO_TRACE(impl::implTraceTool::TRACECAT_WARNING)
}

void TraceTool::enableTarget(bool bEnable, FILE* out)
{
  impl::implTraceTool::inst().enableTarget(bEnable, out);
}

void TraceTool::resetTarget()
{
  impl::implTraceTool::inst().nOut = 0;
  impl::implTraceTool::inst().pfnTrace = 0;
}

void TraceTool::addOutputTarget(FILE* out)
{
  assert(out);
  if (out) {
    impl::implTraceTool::inst().addOutputTarget(out);
  }
}

void TraceTool::setTimeStampFormat(const char* format)
{
  assert(format);
  if (format) {
    impl::implTraceTool::inst().fmt = format;
  }
}

void TraceTool::setTraceFunc(void (*pfnTrace)(const char* format, va_list args))
{
  impl::implTraceTool::inst().pfnTrace = pfnTrace;
}

} // namespace sw2

// end of swTraceTool.cpp
