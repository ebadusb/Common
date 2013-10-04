/*
 * Copyright (c) 2008 by Gambro BCT, Inc.  All rights reserved.
 *
 */

/*! \file Mutex.cpp
*/

#include "Mutex.h"
#include "stdlib.h"

#ifdef VXWORKS
#include "Mutex_vxWorks.cxx"
#elif defined WIN32

#include "Mutex_win32.cxx"
#endif
