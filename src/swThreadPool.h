
//
//  Thread pool.
//
//  Copyright (c) 2005 Waync Cheng.
//  All Rights Reserved.
//
//  2005/02/24 Waync created.
//

///
/// \file
/// \brief Thread pool.
///
/// Thread pool module provides a very simple interface to use thread. Your class
/// simplely inherit ThreadTask class and implement threadTask, then it has threading
/// ability right away.
///
/// Example:
///
/// \code
/// #include "swThreadPool.h"
///
/// //
/// // Inherit TreadTask to enable threading ability.
/// //
///
/// class TestThreadTask : public ThreadTask
/// {
/// public:
///
///   //
///   // Implement threadTask, so this function will execute in a worker thread.
///   //
///
///   virtual void threadTask()
///   {
///      // Do something. This will execute in a worker thread.
///   }
/// };
///
/// //
/// // Schedule the task.
/// //
///
/// TestThreadTask task;                // Declare a ThreadTask object.
/// if (!task.runTask())                // Schedule it.
/// { // Fail, retry later.
/// }
///
/// //
/// // Wait task done.
/// //
///
/// while (task.isRunning()) // Is task still running?
/// { // Wait it done.
/// }
///
/// \endcode
///
/// \author Waync Cheng
/// \date 2005/02/24
///

#pragma once

#include <vector>

#include "swinc.h"

namespace sw2 {

///
/// \brief Initialize thread pool module.
/// \param [in] nPoolSize Thread pool size, number of worker threads.
/// \return Return true if success else return false.
/// \note Actual created worker thread number is limited to the system.
///

bool InitializeThreadPool(uint nPoolSize);

///
/// \brief Unitialize thread pool module.
///

void UninitializeThreadPool();

///
/// \brief Thread task.
///

class ThreadTask
{
public:

  virtual ~ThreadTask();

  ///
  /// \brief Schedule this task and execute in a worker thread.
  /// \return Return true if success else return false.
  /// \note Return success means the task is added to the task queue, doesn't mean
  ///       the task is executing. Need to wait a worker thread to get the task
  ///       from queue and execute it.
  /// \note Return false means can't add the task to the queue, please try again
  ///       later.
  ///

  bool runTask();

  ///
  /// \brief Check is the task still running?
  /// \return Return true if the task is still running or waiting in the queue.
  ///         Return false if the task is done.
  /// \note The effect of this function is like a lock. Application should not
  ///        access the object before isRunning returns true otherwise may cause
  ///        synchronization problem.
  ///

  bool isRunning();

  ///
  /// \brief Thread task procedure.
  /// \note Application implements this function to perform user task, thread pool
  ///       will execute this task by a worker thread. Then application checks
  ///       isRunning to see is the task done.
  ///

  virtual void threadTask()=0;
};

///
/// \brief Thread lock.
///

class ThreadLock
{
public:

  ///
  /// \brief Allocate a thread lock.
  /// \return If success return an interface pointer else return 0.
  ///

  static ThreadLock* alloc();

  ///
  /// \brief Release a unused thread lock instance.
  /// \param [in] pLock Instance to free.
  ///

  static void free(ThreadLock* pLock);

  ///
  /// \brief Enter critical section.
  ///

  virtual void lock()=0;

  ///
  /// \brief Leave critical section.
  ///

  virtual void unlock()=0;
};

///
/// \brief Helper to run and wait multiple thread tasks.
///

class ThreadTaskPipe
{
public:

  ///
  /// \brief Run and wait a thread task.
  /// \param [in] pTask A thread task.
  /// \return Return this thread task pipe.
  ///

  ThreadTaskPipe& run(ThreadTask *pTask);

  ///
  /// \brief Run and wait two thread tasks.
  /// \param [in] pTask1 Thread task1.
  /// \param [in] pTask2 Thread task2.
  /// \return Return this thread task pipe.
  ///

  ThreadTaskPipe& run(ThreadTask *pTask1, ThreadTask *pTask2);

  ///
  /// \brief Run and wait three thread tasks.
  /// \param [in] pTask1 Thread task1.
  /// \param [in] pTask2 Thread task2.
  /// \param [in] pTask3 Thread task3.
  /// \return Return this thread task pipe.
  ///

  ThreadTaskPipe& run(ThreadTask *pTask1, ThreadTask *pTask2, ThreadTask *pTask3);

  ///
  /// \brief Run and wait multiple thread tasks.
  /// \param [in] tasks Thread tasks.
  /// \return Return this thread task pipe.
  ///

  ThreadTaskPipe& run(const std::vector<ThreadTask*> &tasks);
};

} // end of namespace sw2

// end of swThreadPool.h
