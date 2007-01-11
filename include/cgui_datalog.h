/*
 * Copyright (c) 2007 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: $
 * $Log: $
 *
 */

#ifndef _CGUI_DATALOG_INCLUDE
#define _CGUI_DATALOG_INCLUDE

#include "cgui_graphics.h"
#include "datalog.h"

// Extension to the Data Log streams for CGUIRegion's
inline DataLog_Stream & operator << (DataLog_Stream &os, const CGUIRegion &region)
{
   os << "{x=" << region.x << ";";
   os << "y=" << region.y << ";";
   os << "width=" << region.width << ";";
   os << "height=" << region.height << "}";
   return os;
};

#endif
