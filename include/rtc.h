/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 *	vxWorks utilities for accessing the real-time clock.  This file should be
 * included as part of the build for an vxWorks OS image and is not intended
 * to be compiled separately.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/include/rcs/rtc.h 1.4 2002/10/08 15:35:17Z jl11312 Exp ms10234 $
 * $Log: rtc.h $
 *
 */

STATUS setCurrentTimeFromRTC(void);
STATUS setRTCFromCurrentTime(void);

