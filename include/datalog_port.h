/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/include/rcs/datalog_port.h 1.16 2003/10/03 12:32:57Z jl11312 Exp rm70006 $
 * $Log: datalog_port.h $
 * Revision 1.3  2002/04/29 21:48:42  jl11312
 * - updated to reflect API changes
 * Revision 1.2  2002/03/27 16:43:25  jl11312
 * - updated to allow multiple data sets for periodic logging
 * - updated error handling interface
 * Revision 1.1  2002/03/19 16:11:35  jl11312
 * Initial revision
 *
 */

#ifndef _DATALOG_PORT_INCLUDE
#define _DATALOG_PORT_INCLUDE

/*
 * Error type is defined here to allow for platform specific errors.  Most
 * errors however are common to all platforms.
 */
typedef enum
{
	DataLog_NoError
} DataLog_ErrorType;

/*
 * Platform specific configuration
 */
#ifdef VXWORKS

/*
 * Trima on vxWorks platform
 */
typedef int DataLog_TaskID;
#define DATALOG_CURRENT_TASK 0

#define DataLog_SharedPtr(type) type *

typedef unsigned char DataLog_BufferData;

#else
#error "Unknown platform - no definition for DataLog_TaskID type"
#endif

#endif /* ifndef _DATALOG_PORT_INCLUDE */
