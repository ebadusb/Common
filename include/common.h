/*
 * Copyright (c) 1995, 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: Q:/home1/COMMON_PROJECT/Source/INCLUDE/rcs/COMMON.H 1.2 1999/07/09 15:24:25 BS04481 Exp BS04481 $
 * $Log: COMMON.H $
 * Revision 1.7  1999/03/24 21:50:38  BS04481
 * Change hardware version information to provide base version of
 * hardware plus computer type, board rev and FPGA revision byte.
 * Add message support in both drivers to respond to queries for 
 * hardware type.
 * Revision 1.6  1998/05/12 19:42:25  bs04481
 * Change all fatal error calls from the low-level drivers to the 
 * fatal_error_drv function which does not exit.  The router  shutdown
 * sequence will cause the driver to exit in its shutdown sequence.
 * Revision 1.5  1997/12/02 15:43:58  bs04481
 * Removed delay in fatal error functions which will cause the 
 * hardware watchdog to go off before the logs are flushed
 * Revision 1.4  1997/05/30 20:24:16  bs04481
 * Separate expected data for control and safety driver's sensing
 * of hardware type.  Caused by an address conflict between 2.2 and 
 * 2.6 but something that should be done anyway.
 * Revision 1.3  1997/05/21 17:57:30  bs04481
 * Changed fatal error message to match what I told the tech writer.
 * Revision 1.2  1997/05/09 21:56:54  bs04481
 * Change from Fatal Error call to local fatal operation.
 * Revision 1.1  1997/04/21 22:35:53  bs04481
 * Initial revision
 *
 * TITLE:      comon.h., Focussed System common defines which
 *             apply to both the control and the safety systems
 *
 * ABSTRACT:   Universally useful stuff
 *
 * DOCUMENTS
 * Requirements:     I:\ieee1498\SSS8.DOC
 * Test:             I:\ieee1498\STD8.DOC
 */

#ifndef COMMON_H
#define COMMON_H

#include <i86.h>
#include <stdlib.h>
#include "error.h"
#include "hardware.h"

enum BOOL
{
   FALSE=0,
   TRUE=1,
};


// SPECIFICATION:    Outputs 8 bits to IO space for hardware control.  If the "read"
//                   variable is TRUE, also reads back the just written value
//                   and compares to the expected.
//
// ERROR HANDLING:   Terminates program is mismatch between written value and
//                   expected value.

inline void Outp( long address, unsigned char data, BOOL read)
{
   unsigned char read_back;

   outp(address,data);

   if (read == TRUE)
   {
      read_back = (unsigned char)inp(address);
      if(data != read_back)
      {
         _FATAL_ERROR_DRV( __FILE__, __LINE__, TRACE_CTL_DRV, address, "Read back Mismatch?!?");
      }
   }
}

// SPECIFICATION:    Outputs 16 bits to IO space for hardware control.  If the "read"
//                   variable is TRUE, also reads back the just written value
//                   and compares to the expected.
//
// ERROR HANDLING:   Terminates program is mismatch between written value and
//                   expected value.

inline void Outpw( long address, long data, BOOL read)
{
   long read_back;

   outpw(address,data);

   if (read == TRUE)
   {
      read_back = inpw(address);
      if(data != read_back)
      {
         _FATAL_ERROR_DRV( __FILE__, __LINE__, TRACE_CTL_DRV, address, "Read back Mismatch?!?");
      }
   }
}


#endif
      

