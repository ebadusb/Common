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
#include <stdio.h>

#include "hw_intf.h"
#include "cca_pci_intf.h"

/* ----------------------------- MACROS ---------------------------- */
#define ASSERT(cond)    assert(cond)

/* ----------------------------- CONSTANTS ------------------------- */
/* ----------------------------- PROTOTYPES------------------------- */

static HwInterfaceImpl theImpl = {0};

BOOL hwInitInterface(const HwInterfaceImpl* pImpl)
{
   BOOL isValid = FALSE;

   /* Once initialized, don't re-initialize */
   if (theImpl.GetPortRegister != NULL)
   {
      printf("%s(): HwInterfaceImpl already initialized to %s\n", __FUNCTION__, theImpl.name);
      isValid = TRUE;
   }
   else if (pImpl && pImpl->GetPortRegister &&
            pImpl->InByte  && pImpl->InWord  && pImpl->InLong &&
            pImpl->OutByte && pImpl->OutWord && pImpl->OutLong &&
            pImpl->name)
   {
      theImpl = *pImpl;
      isValid = TRUE;
      /*
       * hwShowPortMap();
       */
   }
   else
   {
      printf("%s(): invalid HwInterfaceImpl\n", __FUNCTION__);
   }

   return isValid;
}

HwPortReg hwGetPortRegister(HwPortId portId)
{
   return (theImpl.GetPortRegister ? theImpl.GetPortRegister(portId) : HwPortReg_NA);
}

UCHAR hwInByte(HwPortId portId)
{
   HwPortReg port = hwGetPortRegister(portId);
   ASSERT(port != HwPortReg_NA);
   return theImpl.InByte(port);
}

USHORT hwInWord(HwPortId portId)
{
   HwPortReg port = hwGetPortRegister(portId);
   ASSERT(port != HwPortReg_NA);
   return theImpl.InWord(port);
}

ULONG hwInLong(HwPortId portId)
{
   HwPortReg port = hwGetPortRegister(portId);
   ASSERT(port != HwPortReg_NA);
   return theImpl.InLong(port);
}

void hwOutByte(HwPortId portId, HwByte data)
{
   HwPortReg port = hwGetPortRegister(portId);
   ASSERT(port != HwPortReg_NA);
   theImpl.OutByte(port, data);
}

void hwOutWord(HwPortId portId, HwWord data)
{
   HwPortReg port = hwGetPortRegister(portId);
   ASSERT(port != HwPortReg_NA);
   theImpl.OutWord(port, data);
}

void hwOutLong(HwPortId portId, HwLong data)
{
   HwPortReg port = hwGetPortRegister(portId);
   ASSERT(port != HwPortReg_NA);
   theImpl.OutLong(port, data);
}

int hwShowPortMap(void)
{
   int mapSize = 0;

   if (theImpl.GetPortRegister == NULL)
   {
      printf("HwInterfaceImpl not defined!\n");
   }
   else
   {
      HwPortId portId;

      printf("HwInterfaceImpl port mapping for: %s\n", theImpl.name);

      mapSize = theImpl.numPorts;

      for (portId = 0; portId < mapSize; portId++)
      {
         HwPortReg portReg = theImpl.GetPortRegister(portId);
         printf("portId=%#x : portReg=%#x\n", portId, portReg);
      }
   }

   return mapSize;
}
