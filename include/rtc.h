/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 *	vxWorks utilities for accessing the real-time clock.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/include/rcs/rtc.h 1.4 2002/10/08 15:35:17Z jl11312 Exp ms10234 $
 * $Log: rtc.h $
 * Revision 1.2  2002/06/04 20:24:26Z  jl11312
 * - added protection for multiple includes
 * Revision 1.1  2002/05/30 13:00:30  jl11312
 * Initial revision
 *
 */

#ifndef _RTC_INCLUDE
#define _RTC_INCLUDE

#include "vxWorks.h"    // Definition of STATUS

STATUS setCurrentTimeFromRTC(void);
STATUS setRTCFromCurrentTime(void);

#endif /* ifndef _RTC_INCLUDE */

