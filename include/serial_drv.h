/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * vxWorks serial port driver with support for Ampro LB-486
 *
 * $Header: K:/BCT_Development/vxWorks/Common/include/rcs/serial_drv.h 1.1 2002/06/07 21:42:31 jl11312 Exp jl11312 $
 * $Log: serial_drv.h $
 * Revision 1.1  2002/06/07 21:42:31  jl11312
 * Initial revision
 *
 */

#ifndef _SERIALDRV_INCLUDE
#define _SERIALDRV_INCLUDE

#include "vxWorks.h"

STATUS setupSerialPort(unsigned int baseAddress, unsigned int intLevel);

#endif /* ifndef _SERIALDRV_INCLUDE */

