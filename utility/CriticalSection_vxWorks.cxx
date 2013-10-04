/*
* Copyright (C) 2009 Gambro BCT, Inc.  All rights reserved.
*
*/

/*
  This file defines the functionallity for a vxWorks implementatoin of a Critical Section.
  NOTE: This file should not be built, as it is included from the file CriticalSection.cpp
  when VXWORKS is defined.
*/

#include <vxWorks.h>
#include <taskLib.h>

using namespace Bct;

CriticalSection::CriticalSection()
{
}

CriticalSection::~CriticalSection()
{
}

void CriticalSection::lock(void)
{
  taskLock();
}

void CriticalSection::unlock(void)
{
  taskUnlock();
}
