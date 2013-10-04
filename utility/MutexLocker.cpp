/*
 * Copyright (c) 2008 by Gambro BCT, Inc.  All rights reserved.
 *
 */

/*! \file MutexLocker.cpp
*/

#include "MutexLocker.h"
#include "Mutex.h"

using namespace Bct;

MutexLocker::MutexLocker(Mutex& pMutex) : _pMutex(pMutex)
{
  _pMutex.lock();
}

MutexLocker::~MutexLocker()
{
  _pMutex.unlock();
}

