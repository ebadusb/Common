/*
* Copyright (C) 2008 Gambro BCT, Inc.  All rights reserved.
*
*/

/*
  This file defines the functionallity for a vxWorks implementatoin of Mutex.
  NOTE: This file should not be built, as it is included from the file Mutex.cpp
  when VXWORKS is defined.
*/


#include <semLib.h>
#include <iostream>
using namespace Bct;
Mutex::Mutex(int options)
{
  _handle = semMCreate(options);
}

Mutex::~Mutex()
{
  unlock();
  if(_handle != NULL)
  {
    semTake(_handle, WAIT_FOREVER);
    semDelete(_handle);
  }
}

bool Mutex::lock(int dMilliSecondTimeout)
{
  bool bRet = false;
  if(_handle != NULL)
  {
    
    bRet = (semTake(_handle, dMilliSecondTimeout) == OK) ? true : false;
  }
  return bRet;
}

void Mutex::unlock()
{
  if(_handle != NULL)
  {
    semGive(_handle);
  }
}

