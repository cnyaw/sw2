
//
//  Unhandled exception filter.
//
//  Copyright (c) 2016 Waync Cheng.
//  All Rights Reserved.
//
//  2016/5/6 Waync created.
//

#include <time.h>

#include <string>

#ifdef WIN32
  #include <windows.h>
  #include <dbghelp.h>
  #include "Shlwapi.h"
  #ifdef _MSC_VER
    #pragma comment(lib, "dbghelp.lib")
    #pragma comment(lib,"Shlwapi.lib")
  #endif
#endif

#include "swUtil.h"

namespace sw2 {

namespace impl {

#ifdef WIN32
class implUnhandledException
{
public:

  BOOL PreventSetUnhandledExceptionFilter()
  {
    HMODULE hKernel32 = LoadLibrary("kernel32.dll");
    if (0 == hKernel32) {
      return FALSE;
    }

    void *pOrgEntry = (void*)GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
    if (0 == pOrgEntry) {
      return FALSE;
    }

#ifdef _M_IX86
    // Code for x86:
    // 33 C0                xor         eax,eax
    // C2 04 00             ret         4
    unsigned char szExecute[] = { 0x33, 0xC0, 0xC2, 0x04, 0x00 };
#elif _M_X64
    // 33 C0                xor         eax,eax
    // C3                   ret
    unsigned char szExecute[] = { 0x33, 0xC0, 0xC3 };
#else
  #error "The following code only works for x86 and x64!"
#endif

    SIZE_T bytesWritten = 0;
    return WriteProcessMemory(GetCurrentProcess(), pOrgEntry, szExecute, sizeof(szExecute), &bytesWritten);
  }

  implUnhandledException()
  {
    SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
    PreventSetUnhandledExceptionFilter();
  }

  static LONG WINAPI MyUnhandledExceptionFilter(EXCEPTION_POINTERS* pExceptionPointers)
  {
    OutputCrashLog(pExceptionPointers);
    return EXCEPTION_EXECUTE_HANDLER;
  }

  static void OutputCrashLog(EXCEPTION_POINTERS* pExceptionPointers)
  {
    std::string strCallStack = GetStackDetails(pExceptionPointers->ContextRecord);
    if (strCallStack.empty()) {
      return;
    }

    TCHAR szFileName[MAX_PATH];
    GetModuleFileName(NULL, szFileName, MAX_PATH);
    PathRemoveFileSpec(szFileName);

    TCHAR szLogName[MAX_PATH];
    PathCombine(szLogName, szFileName, TEXT("Crash.log"));

    FILE* pFile = fopen(szLogName, "at+");
    if (0 == pFile) {
      return;
    }

    time_t CurrTime = time(0);
    struct tm* pTimeInfo = localtime(&CurrTime);

    char buf[128];
    strftime(buf, sizeof(buf), "Exception Time: %Y-%m-%d %H:%M:%S\n", pTimeInfo);

    std::string strException = std::string(buf) + strCallStack;

    fwrite(strException.c_str(), strException.length(), 1, pFile);

    fclose(pFile);
  }

  static std::string GetStackDetails(CONTEXT* pContext)
  {
    if (0 == pContext) {
      return "";
    }

    std::string strDetails;

    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();

    SymSetOptions(SYMOPT_DEFERRED_LOADS);
    if (!SymInitialize(hProcess, NULL, TRUE)) {
      return "";
    }

    strDetails += "Call Stack:\n";
    strDetails += "Address\t\tFrame\t\tSource#Line\n";

    CONTEXT TempContext = *pContext;

    STACKFRAME StackFrame;
    memset(&StackFrame, 0, sizeof(StackFrame));
#if defined(_AMD64_)
    StackFrame.AddrPC.Offset = TempContext.Rip;
    StackFrame.AddrPC.Mode = AddrModeFlat;
    StackFrame.AddrStack.Offset = TempContext.Rsp;
    StackFrame.AddrStack.Mode = AddrModeFlat;
    StackFrame.AddrFrame.Offset = TempContext.Rbp;
    StackFrame.AddrFrame.Mode = AddrModeFlat;
    DWORD dwMachineType = IMAGE_FILE_MACHINE_AMD64;
#elif defined(_X86_)
    StackFrame.AddrPC.Offset = TempContext.Eip;
    StackFrame.AddrPC.Mode = AddrModeFlat;
    StackFrame.AddrStack.Offset = TempContext.Esp;
    StackFrame.AddrStack.Mode = AddrModeFlat;
    StackFrame.AddrFrame.Offset = TempContext.Ebp;
    StackFrame.AddrFrame.Mode = AddrModeFlat;
    DWORD dwMachineType = IMAGE_FILE_MACHINE_I386;
#endif

    while (true)
    {
      //
      // Get the next stack frame.
      //

      if (!StackWalk(dwMachineType, hProcess, hThread, &StackFrame, &TempContext, 0, SymFunctionTableAccess, SymGetModuleBase, 0)) {
        break;
      }

      if (0 == StackFrame.AddrFrame.Offset) {
        break;
      }

      char szBuffer[512];
      sprintf(szBuffer, "%08X\t%08X\t", (unsigned int)StackFrame.AddrPC.Offset, (unsigned int)StackFrame.AddrFrame.Offset);
      strDetails += szBuffer;

      //
      // Get the source line for this stack frame entry.
      //

      IMAGEHLP_LINE ImageHlpLine = { sizeof(IMAGEHLP_LINE) };
      DWORD dwLineDisplacement;
      if (SymGetLineFromAddr(hProcess, StackFrame.AddrPC.Offset, &dwLineDisplacement, &ImageHlpLine)) {
        strDetails += ImageHlpLine.FileName;
        sprintf(szBuffer, " line %u", (unsigned int)ImageHlpLine.LineNumber);
        strDetails += szBuffer;
      }

      strDetails += "\n";
    }

    strDetails += "\n";

    SymCleanup(hProcess);

    return strDetails;
  }
};
#endif

} // namespace impl

void Util::setUnhandledExceptionFilter()
{
#ifdef WIN32
  impl::implUnhandledException();
#endif
}

} // namespace sw2

// end of swException.cpp
