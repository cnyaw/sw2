
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

#include <string>

#include "swTraceTool.h"

namespace sw2 {

namespace impl {

#define MAX_OUTPUT 32
#define MAX_STR_LEN 1024

struct implTraceToolTarget
{
  bool isEnable;
  FILE* fOut;
  int level;
};

class implTraceTool
{
  implTraceTool() : nOut(0), nEnable(0), fmt("%Y-%m-%d %H:%M:%S "), pfnTrace(0)
  {
    addOutputTarget(stdout, 0);
  }

public:

  int nOut, nEnable;
  std::string fmt;
  implTraceToolTarget target[MAX_OUTPUT];
  void (*pfnTrace)(int level, const char* format, va_list args);

  static implTraceTool& inst()
  {
    static implTraceTool i;
    return i;
  }

  void addOutputTarget(FILE* out, int level)
  {
    if (MAX_OUTPUT > nOut) {
      implTraceToolTarget &t = target[nOut];
      t.isEnable = true;
      t.fOut = out;
      t.level = level;
      nOut += 1;
      nEnable += 1;
    }
  }

  void enableTarget(bool bEnable, FILE* out)
  {
    if (0 == out) {
      nEnable = bEnable ? nOut : 0;
      for (int i = 0; i < nOut; i++) {
        target[i].isEnable = bEnable;
      }
    } else {
      for (int i = 0; i < nOut; i++) {
        implTraceToolTarget &t = target[i];
        if (t.fOut == out) {
          if (bEnable) {
            if (t.isEnable != bEnable) {
              nEnable += 1;
            }
          } else {
            if (t.isEnable != bEnable) {
              nEnable -= 1;
            }
          }
          t.isEnable = bEnable;
          break;
        }
      }
    }
  }

  void doTrace(int level, char const* str) const
  {
    char buf[MAX_STR_LEN];
    buf[0] = '\0';

    time_t now = ::time(0);
    struct tm tmNow = *::localtime(&now);
    ::strftime(buf, sizeof(buf), fmt.c_str(), &tmNow);

    std::string s(buf);
    s.append(str);
    s.push_back('\n');

    for (int i = 0; i < nOut; i++) {
      const implTraceToolTarget &t = target[i];
      if (t.isEnable && (0 == level || t.level == level)) {
        fprintf(t.fOut, "%s", s.c_str());
        fflush(t.fOut);
      }
    }
  }
};

} // namespace impl

void TraceTool::trace(int level, const char* format, ...)
{
  assert(format);
  impl::implTraceTool& inst = impl::implTraceTool::inst();
  if (0 >= inst.nEnable || 0 == inst.nOut) {
    return;
  }
  va_list va;
  va_start(va, format);
  if (inst.pfnTrace) {
    inst.pfnTrace(level, format, va);
  } else {
    char buf[MAX_STR_LEN];
    vsnprintf(buf, MAX_STR_LEN, format, va);
    inst.doTrace(level, buf);
  }
  va_end(va);
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

void TraceTool::addOutputTarget(FILE* out, int level)
{
  assert(out);
  if (out) {
    impl::implTraceTool::inst().addOutputTarget(out, level);
  }
}

void TraceTool::setTimeStampFormat(const char* format)
{
  assert(format);
  if (format) {
    impl::implTraceTool::inst().fmt = format;
  }
}

void TraceTool::setTraceFunc(void (*pfnTrace)(int level, const char* format, va_list args))
{
  impl::implTraceTool::inst().pfnTrace = pfnTrace;
}

} // namespace sw2

// end of swTraceTool.cpp
