
//
//  Thread pool.
//
//  Copyright (c) 2005 Waync Cheng.
//  All Rights Reserved.
//
//  2005/02/24 Waync created.
//

#include <queue>
#include <map>

#if defined(WIN32)
# define NOMINMAX
# include <windows.h>
#elif defined(_linux_)
# include "pthread.h"
#endif

#include "swObjectPool.h"
#include "swThreadPool.h"
#include "swUtil.h"

namespace sw2 {

namespace impl {

#define MIN_THREAD_POOL_SIZE 4          // Min worker thread.
#define MAX_THREAD_POOL_SIZE 256        // Max worker thread.
#define MAX_THREAD_TASK 256             // Max task.

class implLock : public ThreadLock
{
public:
#if defined(WIN32)
  CRITICAL_SECTION m_cs;

  implLock()
  {
    ::InitializeCriticalSection(&m_cs);
  }

  virtual ~implLock()
  {
    ::DeleteCriticalSection(&m_cs);
  }

  virtual void lock()
  {
    ::EnterCriticalSection(&m_cs);
  }

  virtual void unlock()
  {
    ::LeaveCriticalSection(&m_cs);
  }

#elif defined(_linux_)
  pthread_mutex_t m_mutex;

  implLock()
  {
    pthread_mutex_init(&m_mutex, NULL);
  }

  virtual ~implLock()
  {
    pthread_mutex_destroy(&m_mutex);
  }

  virtual void lock()
  {
    pthread_mutex_lock(&m_mutex);
  }

  virtual void unlock()
  {
    pthread_mutex_unlock(&m_mutex);
  }
#endif
};

class implSignal
{
public:
#if defined(WIN32)
  HANDLE m_event;

  explicit implSignal(BOOL bManualReset)
  {
    m_event = ::CreateEvent(NULL, bManualReset, FALSE, NULL);
  }

  ~implSignal()
  {
    ::CloseHandle(m_event);
  }

  void reset()
  {
    ::ResetEvent(m_event);
  }

  void fire()
  {
    ::SetEvent(m_event);
  }

  void wait()
  {
    ::WaitForSingleObject(m_event, INFINITE);
  }

#elif defined(_linux_)
  pthread_cond_t m_signal;

  implSignal()
  {
    pthread_cond_init(&m_signal, NULL);
  }

  ~implSignal()
  {
    pthread_cond_destroy(&m_signal);
  }

  void broadcast()
  {
    pthread_cond_broadcast(&m_signal);
  }

  void fire()
  {
    pthread_cond_signal(&m_signal);
  }

  void wait(pthread_mutex_t* pMutex)
  {
    pthread_cond_wait(&m_signal, pMutex);
  }
#endif
};

class implThreadTask
{
public:

  bool m_bRunning, m_bQueued;
  ThreadTask* m_pTask;                  // A real task to run.
  implLock m_lock;

  implThreadTask() : m_bRunning(false), m_bQueued(false), m_pTask(0)
  {
  }

  //
  // Public interface.
  //

  bool isRunning()
  {
    m_lock.lock();
    bool bRunning = m_bRunning || m_bQueued;
    m_lock.unlock();
    return bRunning;
  }

  void reset(ThreadTask* pTask)
  {
    m_bQueued = m_bRunning = false;
    m_pTask = pTask;
  }

  void runTask()
  {
    m_lock.lock();
    m_bQueued = false;
    m_bRunning = true;
    m_lock.unlock();

    m_pTask->threadTask();              // Run task actually.

    m_lock.lock();
    m_bRunning = false;
    m_lock.unlock();
  }

  void setQueued()
  {
    m_lock.lock();
    m_bQueued = true;
    m_lock.unlock();
  }
};

class implThreadPool
{
#if defined(WIN32)
  implThreadPool() : m_singalWakeup(FALSE), m_signalQuit(TRUE), m_nThread(0)
  {
  }
#elif defined(_linux_)
  implThreadPool() : m_singalWakeup(), m_nThread(0)
  {
  }
#endif
public:

  static implThreadPool& inst()
  {
    static implThreadPool i;
    return i;
  }

  ObjectPool<implThreadTask, MAX_THREAD_TASK> m_poolTask; // Task pool.
  implLock m_poolLock;

  std::queue<int> m_queue;              // Task queue, wait for available worker thread to run task.
  implLock m_queueLock;

  implSignal m_singalWakeup;            // Signal to awake a sleep thread to perform task.
#if defined(WIN32)
  implSignal m_signalQuit;              // Signal to tell all thread end.
#endif

#if defined(WIN32)
  HANDLE m_hThread[MAX_THREAD_POOL_SIZE]; // Worker thread handle, saved for uninit.
#elif defined(_linux_)
  bool m_quitEvent;
  pthread_t m_thread[MAX_THREAD_POOL_SIZE];
#endif

  std::map<void*, int> m_taskMap;       // Task instace, idTask

  int m_nThread;                        // Thread pool size.

  ~implThreadPool()
  {
    uninit();                           // Close all worker threads.
  }

  //
  // Alloc/free thread task instance(ThreadTask).
  //

  int alloc(ThreadTask* pTask)
  {
    if (0 == m_nThread) {               // Not initialized.
      return -1;
    }

    lockPool();

    int idTask = m_poolTask.alloc();
    if (-1 != idTask) {
      implThreadTask& task = m_poolTask[idTask];
      task.reset(pTask);
    }

    unlockPool();

    if (-1 != idTask) {
      m_taskMap[pTask] = idTask;
    }

    return idTask;
  }

  //
  // Remove from task pool.
  //

  void free(ThreadTask* pTask)
  {
    if (0 == m_nThread) {               // Not initialized.
      return;
    }

    std::map<void*, int>::const_iterator it = m_taskMap.find(pTask);
    if (m_taskMap.end() == it) {
      return;
    }

    lockPool();
    m_poolTask.free(it->second);
    unlockPool();

    m_taskMap.erase(pTask);
  }

  //
  // Init/uninit.
  //

  bool init(uint nPoolSize)
  {
    if (0 != m_nThread) {               // Already initialized.
      return false;
    }

    nPoolSize = std::max((uint)MIN_THREAD_POOL_SIZE, std::min((uint)MAX_THREAD_POOL_SIZE, nPoolSize));

#if defined(WIN32)
    m_singalWakeup.reset();
    m_signalQuit.reset();
#elif defined(_linux_)
    m_quitEvent = false;
#endif

    //
    // Create worker threads.
    //

    for (uint i = 0; i < nPoolSize; i++) {

#if defined(WIN32)
      DWORD idThread;
      HANDLE hThread = ::CreateThread(
                           NULL,
                           0,
                           (LPTHREAD_START_ROUTINE)WorkerThread,
                           (void*)this,
                           0,
                           &idThread);
      if (NULL == hThread) {
        break;
      }
      m_hThread[m_nThread++] = hThread;
#elif defined(_linux_)
      pthread_t t;
      int res = pthread_create(&t, NULL, WorkerThread, (void*)this);
      if (res) {
        break;
      }
      m_thread[m_nThread++] = t;
#endif
    }

    return 0 != m_nThread;
  }

  void uninit()
  {
    if (0 == m_nThread) {               // Not initialized.
      return;
    }

    //
    // Wait all worker threads end.
    //

#if defined(WIN32)
    m_signalQuit.fire();                // Fire quit signal.

    int idx = 0;
    while (idx < m_nThread) {
      int nThread = std::min(MAXIMUM_WAIT_OBJECTS, m_nThread - idx);
      ::WaitForMultipleObjects(nThread, m_hThread + idx, TRUE, INFINITE);
      idx += nThread;
    }
#endif

    //
    // Close worker threads.
    //

#if defined(WIN32)
    for (int i = 0; i < m_nThread; i++) {
      ::CloseHandle(m_hThread[i]);
      m_hThread[i] = NULL;
    }
#elif defined(_linux_)
    m_queueLock.lock();
    m_quitEvent = true;
    m_singalWakeup.broadcast();
    m_queueLock.unlock();

    for (int i = 0; i < m_nThread; i++) {
      pthread_join(m_thread[i], NULL);
    }

    m_quitEvent = false;
#endif
    m_nThread = 0;

    //
    // Release queued tasks, unnecessary to lock because all threads already down.
    //

    while (!m_queue.empty()) {
      m_queue.pop();
    }
  }

  //
  // Lock/unlock task queue/pool.
  //

  void lockPool()
  {
    m_poolLock.lock();
  }

  void lockQueue()
  {
    m_queueLock.lock();
  }

  void unlockPool()
  {
    m_poolLock.unlock();
  }

  void unlockQueue()
  {
    m_queueLock.unlock();
  }

  //
  // Signals.
  //

  void fireWakeupSignal()
  {
    m_singalWakeup.fire();
  }

  //
  // Interface for ThreadTask.
  //

  bool isRunning(ThreadTask* pTask)
  {
    if (0 == m_nThread) {               // Not initialized.
      return false;
    }

    std::map<void*, int>::const_iterator it = m_taskMap.find(pTask);
    if (m_taskMap.end() == it) {
      return false;
    }

    lockPool();
    bool bRunning = m_poolTask[it->second].isRunning();
    unlockPool();

    return bRunning;
  }

  bool runTask(int idTask)
  {
    if (0 == m_nThread) {               // Not initialized.
      return false;
    }

    //
    // ThreadTask::runTask already check is it running. So schedule it now.
    //

    lockPool();
    m_poolTask[idTask].setQueued();     // Schedule it.

    lockQueue();
    m_queue.push(idTask);               // Queue it.
    unlockQueue();

    unlockPool();

    //
    // Fire a signal to run it.
    //

    fireWakeupSignal();

    return true;
  }

  //
  // Worker thread.
  //

#if defined(WIN32)
  static unsigned __stdcall WorkerThread(void* pMgr)
  {
    implThreadPool* pThreadPool = (implThreadPool*)pMgr;
    HANDLE h[2] = {
      pThreadPool->m_signalQuit.m_event,
      pThreadPool->m_singalWakeup.m_event
    };

    while (true) {

      //
      // Wait quit or new task signal fired.
      //

      DWORD ret = ::WaitForMultipleObjects(2, h, FALSE, INFINITE);
      switch (ret)
      {
      case WAIT_OBJECT_0 + 0:           // Notify end.
        return 0;

      case WAIT_OBJECT_0 + 1:           // New task.
        {
          //
          // Get queued task id.
          //

          int idTask = -1;

          pThreadPool->lockQueue();

          if (!pThreadPool->m_queue.empty()) {

            idTask = pThreadPool->m_queue.front();
            pThreadPool->m_queue.pop();

            //
            // Queued task remains, fire an awake signal to handle it.
            //

            if (!pThreadPool->m_queue.empty()) {
              pThreadPool->fireWakeupSignal();
            }
          }

          pThreadPool->unlockQueue();

          pThreadPool->execTask_i(idTask);
        }
        break;

      default:
        assert(0);                      // Should not happen.
        break;
      }
    }

    return 0;
  }

#elif defined(_linux_)
  static void* WorkerThread(void* pMgr)
  {
    implThreadPool* pThreadPool = (implThreadPool*)pMgr;

    while (true) {

      pThreadPool->lockQueue();

      //
      // Wait until wakeup.
      //

      while (!pThreadPool->m_quitEvent && pThreadPool->m_queue.empty()) {
        pThreadPool->m_singalWakeup.wait(&pThreadPool->m_queueLock.m_mutex);
      }

      //
      // Quit thread?
      //

      if (pThreadPool->m_quitEvent) {
        pThreadPool->unlockQueue();
        break;
      }

      //
      // Get queued task ID.
      //

      int idTask = -1;

      if (!pThreadPool->m_queue.empty()) {

        idTask = pThreadPool->m_queue.front();
        pThreadPool->m_queue.pop();

        //
        // Queued task remains, fire an awake signal to handle it.
        //

        if (!pThreadPool->m_queue.empty()) {
          pThreadPool->fireWakeupSignal();
        }
      }

      pThreadPool->unlockQueue();

      pThreadPool->execTask_i(idTask);
    }

    return 0;
  }
#endif

  void execTask_i(int idTask)
  {
    if (-1 != idTask) {
      lockPool();
      implThreadTask& task = m_poolTask[idTask];
      unlockPool();
      task.runTask();
    }
  }
};

} // namespace impl

//
// Implement ThreadTask.
//

ThreadTask::~ThreadTask()
{
  //
  // Wait thread task ends.
  //

  while (isRunning()) {
  }

  impl::implThreadPool::inst().free(this);
}

bool ThreadTask::isRunning()
{
  bool bRunning = impl::implThreadPool::inst().isRunning(this);
  if (!bRunning) {                      // Task done? then release the task handle.
    impl::implThreadPool::inst().free(this);
  }

  return bRunning;
}

bool ThreadTask::runTask()
{
  if (isRunning()) {
    return false;
  }

  int id = impl::implThreadPool::inst().alloc(this);
  if (-1 == id) {
    return false;
  }

  return impl::implThreadPool::inst().runTask(id);
}

bool InitializeThreadPool(uint nPoolSize)
{
  return impl::implThreadPool::inst().init(nPoolSize);
}

void UninitializeThreadPool()
{
  impl::implThreadPool::inst().uninit();
}

ThreadLock* ThreadLock::alloc()
{
  return new impl::implLock();
}

void ThreadLock::free(ThreadLock* pLock)
{
  delete (impl::implLock*)pLock;
}

//
// ThreadTaskPipe.
//

ThreadTaskPipe& ThreadTaskPipe::run(ThreadTask *pTask)
{
  std::vector<ThreadTask*> v;
  v.push_back(pTask);
  return run(v);
}

ThreadTaskPipe& ThreadTaskPipe::run(ThreadTask *pTask1, ThreadTask *pTask2)
{
  std::vector<ThreadTask*> v;
  v.push_back(pTask1);
  v.push_back(pTask2);
  return run(v);
}

ThreadTaskPipe& ThreadTaskPipe::run(ThreadTask *pTask1, ThreadTask *pTask2, ThreadTask *pTask3)
{
  std::vector<ThreadTask*> v;
  v.push_back(pTask1);
  v.push_back(pTask2);
  v.push_back(pTask3);
  return run(v);
}

ThreadTaskPipe& ThreadTaskPipe::run(const std::vector<ThreadTask*> &tasks)
{
  std::vector<ThreadTask*> v;
  for (size_t i = 0; i < tasks.size(); i++) {
    ThreadTask *p = tasks[i];
    assert(p);
    while (p->isRunning()) {
      Util::sleep(1);
    }
    v.push_back(p);
    p->runTask();
  }
wait_1:
  for (size_t i = 0; i < v.size(); i++) {
    if (v[i]->isRunning()) {
      Util::sleep(1);
      goto wait_1;
    }
  }
  return *this;
}

} // namespace sw2

// end of swThreadPool.cpp
