/*
 * Copyright (c) 2001 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:     hal_notify.cpp
 *
 * Handle notification of HAL (hardware anomaly locator) of a pending reboot.
 * This is called during fatal error handling, or during a normal system
 * shutdown.
 *
 * $Header: K:/BCT_Development/Common/router/rcs/hal_notify.c 1.1 2001/09/28 14:36:12 jl11312 Exp jl11312 $
 * $Log: hal_notify.c $
 *
 */

#include "hal_notify.h"
#include <conio.h>

#define  NORMAL_NOTIFY_CODE   0xe7a3   // completely arbitrary codes to let HAL know that
#define  FATAL_NOTIFY_CODE    0x6b5c   //  shutdown data is coming

#define  HAL_IO_PORT          0x3ff    // scratchpad register for COM1
#define  MAX_DATA_BYTES       16       // maximum storage in HAL for fatal error information
                                       //  must be at least 4

void hal_notify_shutdown(void)
{
   outp(HAL_IO_PORT, NORMAL_NOTIFY_CODE & 0xff);
   outp(HAL_IO_PORT, (NORMAL_NOTIFY_CODE >> 8) & 0xff);
}

void hal_notify_fatal(int line, int code, const char * file)
{
   const char * ptr = file;
   int   numBytes = 4;

   outp(HAL_IO_PORT, FATAL_NOTIFY_CODE & 0xff);
   outp(HAL_IO_PORT, (FATAL_NOTIFY_CODE >> 8) & 0xff);

   outp(HAL_IO_PORT, line & 0xff);
   outp(HAL_IO_PORT, (line >> 8) & 0xff);

   outp(HAL_IO_PORT, code & 0xff);
   outp(HAL_IO_PORT, (code >> 8) & 0xff);

   while ( numBytes < MAX_DATA_BYTES )
   {
      outp(HAL_IO_PORT, *ptr);
      numBytes += 1;
      if ( *ptr )
      {
         ptr += 1;
      }
   }
}
