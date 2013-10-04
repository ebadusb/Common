/*
 * Copyright (c) 2009 by Gambro BCT, Inc.  All rights reserved.
 *
 */

/*! \file CriticalSection.cpp
*/

#include "CriticalSection.h"
#include "stdlib.h"

#ifdef VXWORKS
#include "CriticalSection_vxWorks.cxx"
#elif defined WIN32

#include "CriticalSection_win32.cxx"
#endif
