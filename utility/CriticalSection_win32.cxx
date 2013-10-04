/*
* Copyright (C) 2009 Gambro BCT, Inc.  All rights reserved.
*
*/

/*
  This file defines the functionallity for a win32 implementatoin of a Critical Section.
  NOTE: This file should not be built, as it is included from the file CriticalSection.cpp
  when WIN32 is defined.
*/

#include <windows.h>

#include "CriticalSection.h"

using namespace Bct;

CriticalSection::CriticalSection() : _handle(NULL)
{
  ::InitializeCriticalSection(static_cast<CRITICAL_SECTION *>(_handle));
}

CriticalSection::~CriticalSection()
{
  DeleteCriticalSection(static_cast<CRITICAL_SECTION *>(_handle));
}

void CriticalSection::lock(void)
{
  EnterCriticalSection(static_cast<CRITICAL_SECTION *>(_handle));
}

void CriticalSection::unlock(void)
{
  LeaveCriticalSection(static_cast<CRITICAL_SECTION *>(_handle));
}