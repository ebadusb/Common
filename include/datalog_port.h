/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/include/rcs/datalog_port.h 1.16 2003/10/03 12:32:57Z jl11312 Exp rm70006 $
 * $Log: datalog_port.h $
 *
 */

#ifndef _DATALOG_PORT_INCLUDE
#define _DATALOG_PORT_INCLUDE

/*
 * Platform specific configuration
 */
#ifdef TRIMA_VXWORKS

/*
 * Trima on vxWorks platform
 */
 
typedef int DataLog_TaskID;
#define DATALOG_CURRENT_TASK 0

#else
#error "Unknown platform - no definition for DataLog_TaskID type"
#endif

#endif /* ifndef _DATALOG_PORT_INCLUDE */
