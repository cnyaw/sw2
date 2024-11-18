//
// Log file.
//
// Copyright (c) 2023 Waync Cheng.
// All Rights Reserved.
//
// 2023/5/3 Waync Created.
//

#include <time.h>

#include <vector>

#ifdef WIN32
#include <Shlwapi.h>
#endif

#include "swThreadPool.h"
#include "swUtil.h"

namespace sw2 {

namespace impl {

#define MAX_SWAP_LOG_BUFF 2

class implLogThreadTask : public ThreadTask
{
public:
  std::string m_dir, m_name;
  std::string m_logs;

  virtual void threadTask()
  {
#ifdef WIN32
    if (!PathIsDirectory(m_dir.c_str())) {
      CreateDirectory(m_dir.c_str(), NULL);
    }
#endif

    char sdate[32];
    time_t tNow = ::time(0);
    struct tm tmNow = *::localtime(&tNow);
    strftime(sdate, sizeof(sdate), "%Y-%m-%d", &tmNow);

    char buf[256];
    sprintf(buf, "%s/%s%s", m_dir.c_str(), sdate, m_name.c_str());

    FILE *pFile = fopen(buf, "a+t");
    if (pFile) {
      fprintf(pFile, "%s", m_logs.c_str());
      fclose(pFile);
    }
  }
};

class implLogFile : public LogFile
{
public:
  std::string m_dir, m_name;

  int m_swapIndex;
  std::string m_swapBuff[MAX_SWAP_LOG_BUFF];

  ThreadLock *m_lock;
  implLogThreadTask m_task;

  implLogFile() : m_swapIndex(0), m_lock(0)
  {
  }

  virtual ~implLogFile()
  {
    while (m_task.isRunning()) {
      // Wait unitl current task done.
    }

    saveLogs();

    while (m_task.isRunning()) {
      // Wait unitl new task done.
    }

    ThreadLock::free(m_lock);
    m_lock = 0;
  }

  virtual void setDir(const std::string &dir)
  {
    m_dir = dir;
  }

  virtual void setFileName(const std::string &name)
  {
    m_name = name;
  }

  virtual void addLog(const std::string &log)
  {
    lock();
    m_swapBuff[m_swapIndex] += log + "\n";
    unlock();
  }

  void lock()
  {
    if (0 == m_lock) {
      m_lock = ThreadLock::alloc();
    }
    if (m_lock) {
      m_lock->lock();
    }
  }

  virtual void saveLogs()
  {
    if (m_task.isRunning()) {
      return;
    }

    if (m_swapBuff[m_swapIndex].empty()) {
      return;
    }

    m_task.m_dir = m_dir;
    m_task.m_name = m_name;

    lock();

    m_task.m_logs = m_swapBuff[m_swapIndex];
    m_swapBuff[m_swapIndex].clear();

    unlock();

    m_task.runTask();

    m_swapIndex = (m_swapIndex + 1) % MAX_SWAP_LOG_BUFF;
  }

  void unlock()
  {
    if (m_lock) {
      m_lock->unlock();
    }
  }
};

} // namespace impl

LogFile* LogFile::alloc()
{
  return new impl::implLogFile;
}

void LogFile::free(LogFile* pi)
{
  delete (impl::implLogFile*)pi;
}

} // namespace sw2
