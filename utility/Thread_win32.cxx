/*
* Copyright (C) 2008 Gambro BCT, Inc.  All rights reserved.
*
*/

/*
  This file defines the functionallity for a Windows implementatoin of Thread.
  NOTE: This file should not be built, as it is included from the file Thread.cpp
  when WIN32 is defined.  ONLY Include functionality in this file that cannot be 
  defined cross platform.
*/

#include <windows.h>

using namespace Bct;

int Thread::start()
{
  MutexLocker locker(_exitLock);
  if(!isRunning() && !_exitReady)
  {
    ThreadContainer* pContainer = new ThreadContainer(); /// This will be deleted in entryPoint
    pContainer->pThread = this;
    _canExit = false;
    setRunning(true);
    _handle = ::CreateThread(NULL, 0, (unsigned long (__stdcall *)(void *))this->entryPoint, 
			(void *)pContainer, 0, NULL);

    int priority;

    if( (_priority.isLowerPriority()) || (_priority.isLowPriority()) ) 
      priority = THREAD_PRIORITY_BELOW_NORMAL;
    else if ( (_priority.isHighPriority()) || (_priority.isMediumHighPriority()) )
      priority = THREAD_PRIORITY_ABOVE_NORMAL; 
    else 
      priority = THREAD_PRIORITY_NORMAL;

    SetThreadPriority(_handle, priority) ;

    return 0;
  }
  else
    return 0;
}

void Thread::threadWait(int nMilliSec)
{
  Sleep(nMilliSec);
}

void Thread::setPriority(const Priority& priority)
{
  _priority = priority;
  if(_handle && isRunning())
  {
    int priority;

    if( (_priority.isLowerPriority()) || (_priority.isLowPriority()) ) 
      priority = THREAD_PRIORITY_BELOW_NORMAL;
    else if ( (_priority.isHighPriority()) || (_priority.isMediumHighPriority()) )
      priority = THREAD_PRIORITY_ABOVE_NORMAL; 
    else 
      priority = THREAD_PRIORITY_NORMAL;

    SetThreadPriority(_handle, priority);
  }
}