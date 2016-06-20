/**
 * Copyright (C) 2016 Terumo BCT, Inc.  All rights reserved.
 *
 * @file   hw_intf.c
 *
 * @author mra1usb
 * @date   Apr 26, 2016
 *
 * @brief  Common interface between hardware drivers and hardware FPGA/CCA boards.
 *
 * Implements the hardware I/O routines for either a legacy ISA-based FPGA or the
 * PCI-based CCA boards introduced with E-Box 2016.
 *
 */
#include <vxWorks.h>
#include <assert.h>

#include "hw_intf.h"

/* ----------------------------- MACROS ---------------------------- */
#define ASSERT(cond)    assert(cond)

/* ----------------------------- CONSTANTS ------------------------- */
/* ----------------------------- PROTOTYPES------------------------- */

static HwInterfaceImpl theImpl = {};

BOOL hwInitInterface(HwInterfaceImpl *pImpl)
{
   if (pImpl)
   {
      theImpl = *pImpl;
   }

   theImpl.isValid = (theImpl.GetPortRegister &&
                      theImpl.InByte  && theImpl.InWord  && theImpl.InLong &&
                      theImpl.OutByte && theImpl.OutWord && theImpl.OutLong);

   return theImpl.isValid;
}


UCHAR hwInByte(HwPortId portId)
{
   HwPortReg port = theImpl.GetPortRegister(portId);
   ASSERT(port != HwPortReg_INVALID);
   return theImpl.InByte(port);
}

USHORT hwInWord(HwPortId portId)
{
   HwPortReg port = theImpl.GetPortRegister(portId);
   ASSERT(port != HwPortReg_INVALID);
   return theImpl.InWord(port);
}

ULONG hwInLong(HwPortId portId)
{
   HwPortReg port = theImpl.GetPortRegister(portId);
   ASSERT(port != HwPortReg_INVALID);
   return theImpl.InLong(port);
}

void hwOutByte(HwPortId portId, HwByte data)
{
   HwPortReg port = theImpl.GetPortRegister(portId);
   ASSERT(port != HwPortReg_INVALID);
   theImpl.OutByte(port, data);
}

void hwOutWord(HwPortId portId, HwWord data)
{
   HwPortReg port = theImpl.GetPortRegister(portId);
   ASSERT(port != HwPortReg_INVALID);
   theImpl.OutWord(port, data);
}

void hwOutLong(HwPortId portId, HwLong data)
{
   HwPortReg port = theImpl.GetPortRegister(portId);
   ASSERT(port != HwPortReg_INVALID);
   theImpl.OutLong(port, data);
}
