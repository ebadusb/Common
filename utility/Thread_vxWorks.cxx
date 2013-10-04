/*
  This file defines the functionallity for a vxWorks implementatoin of Thread.
  NOTE: This file should not be built, as it is included from the file Thread.cpp
  when VXWORKS is defined.  ONLY Include functionality in this file that cannot be 
  defined cross platform.
*/

#include "task_start.h"
#include "tasklib.h"
#include "sysLib.h"
#include <iostream>
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
    _handle = taskSpawn((char*)_sName.c_str(), _priority.getPriority(), VX_FP_TASK, 40000, (FUNCPTR)entryPoint, (int)pContainer,  0, 0, 0, 0, 0, 0, 0, 0, 0);// *static_cast<int*>(_handle);
    return _handle;
  }
  else
    return 0;
}

void Thread::threadWait(int nMilliSec)
{
  int nTics =  static_cast<int>(static_cast<double>(sysClkRateGet()) * static_cast<double>(nMilliSec)/1000.0 + 0.5);
  taskDelay(nTics);
}

void Thread::setPriority(const Priority& priority)
{
  _priority = priority;
  if(isRunning() && _handle != 0)
  {
    taskPrioritySet(_handle, priority.getPriority());
  }
}

