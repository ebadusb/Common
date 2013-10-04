/*
 * Copyright (c) 2008 by Gambro BCT, Inc.  All rights reserved.
 *
 */

/*! \file Thread.cpp
*/

#include "Thread.h"
#include <iostream>

#include "MutexLocker.h"
#include "CriticalSection.h"

#ifdef VXWORKS
#include <vxWorks.h>
#include "Thread_vxWorks.cxx"
#elif defined WIN32
#include "Thread_win32.cxx"
#endif

using namespace Bct;

Thread::Thread(std::string sName, unsigned int waitMilliseconds, Priority threadPriority) : 
  _sName(sName),
  _waitMilliSeconds(waitMilliseconds),
  _priority(threadPriority),
  _exitReady(false) ,
  _bRunning(false) , 
  _canExit (true) ,
  _handle(0)
                                                                  
{
}

Thread::~Thread()
{
  _bRunning = false;
  _exitLock.lock();
  _exitReady = true;
  _exitLock.unlock();
  
  while(!_canExit)
    threadWait(10);

#ifdef VXWORKS
  if(_handle != 0)
    taskDelete(_handle);
#endif
}

void Thread::setExiting()
{
  _exitLock.lock();
  _exitReady = true;
  _exitLock.unlock();
}

void Thread::stop()
{
  MutexLocker lock(_stopMux);
  _bRunning = false;
}

int Thread::entryPoint(void* val)
{
  int nRet = 0;
  if(val != NULL)
  {
    ThreadContainer* pContainer = static_cast<ThreadContainer*>(val);
    Thread* obj = pContainer->pThread;
    delete pContainer; // Remember this was allocated in the call to start
    if(obj)
    {
      // Don't call any of our virtual methods if someone is trying to 
      // delete us.
      obj->_exitLock.lock();
      if(!obj->_exitReady)
        obj->preRun();

      obj->_exitLock.unlock();

      nRet = obj->run();

      obj->_exitLock.lock();
      if(!obj->_exitReady)
        obj->postRun();
      obj->_exitLock.unlock();
      obj->_canExit = true;

    }
  }
  return nRet;
}

void Thread::setPriorityHigh()
{
  setPriority(Priority::high());
}
void Thread::setPriorityMedium()
{
  setPriority(Priority::medium());
}
void Thread::setPriorityLow()
{
  setPriority(Priority::low());
}

int Thread::run()
{
  while(isRunning())
  {
    _exitLock.lock();
    if(!_exitReady)
      onePass();
    _exitLock.unlock();
    threadWait(_waitMilliSeconds);
  }
  return 0;
}

bool Thread::isRunning()  const
{
  MutexLocker lock(_stopMux);
  return _bRunning;
}

void Thread::setRunning(bool bRunning)
{
  MutexLocker lock(_stopMux);
  _bRunning = bRunning;
}

void Thread::setIntervalMilliSec(unsigned int milliSec)
{
  _waitMilliSeconds = milliSec;
}

