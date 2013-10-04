/*
* Copyright (C) 2008 Gambro BCT, Inc.  All rights reserved.
*
*/

/*
  This file defines the functionallity for a win32 implementatoin of Mutex.
  NOTE: This file should not be built, as it is included from the file Mutex.cpp
  when WIN32 is defined.
*/

#include "windows.h"

using namespace Bct;
Mutex::Mutex(int options)
{
  _handle = NULL;
  _handle = CreateMutex(0, FALSE, 0);
}

Mutex::~Mutex()
{
  unlock();
  if(_handle != NULL)
  {
    ReleaseMutex(_handle);
    CloseHandle(_handle);
  }
}

bool Mutex::lock(int dMilliSecondTimeout)
{
  bool bRet = false;
  if(_handle != NULL)
  {
    bRet = (WaitForSingleObject(_handle, dMilliSecondTimeout) == WAIT_OBJECT_0) ? true : false;
  }
  return bRet;
}

void Mutex::unlock()
{
  if(_handle != NULL)
    ReleaseMutex(_handle);
}
