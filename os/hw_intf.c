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
#include <stdio.h>

#include "hw_intf.h"
#include "cca_pci_intf.h"
#include "datalog.h"

/* ----------------------------- MACROS ---------------------------- */
#undef HW_DEBUG
#ifdef HW_DEBUG

#include <assert.h>
int xxBadPort = 0;

#define BAD_PORT_ID() \
 printf("%s(): invalid portId=%d (GetPortRegFn=%#x)", __FUNCTION__, portId, (int)theImpl.GetPortRegister); \
 assert(++xxBadPort < 10);

#define ASSERT_OPORT(port) if (port == HwPortReg_NA) { BAD_PORT_ID(); return;   }
#define ASSERT_IPORT(port) if (port == HwPortReg_NA) { BAD_PORT_ID(); return 0; }

#else

#define ASSERT_OPORT(port)
#define ASSERT_IPORT(port)

#endif /* HW_DEBUG */

/* ----------------------------- CONSTANTS ------------------------- */
/* ----------------------------- PROTOTYPES------------------------- */


static HwInterfaceImpl theImpl = {0};

BOOL hwInitInterface(const HwInterfaceImpl* pImpl)
{
   BOOL isValid = FALSE;

   /* Once initialized, don't re-initialize */
   if (theImpl.GetPortRegister != NULL)
   {
      printf("HwInterfaceImpl already initialized to %s\n", theImpl.name);
      isValid = TRUE;
   }
   else if (pImpl && pImpl->GetPortRegister &&
            pImpl->InByte  && pImpl->InWord  && pImpl->InLong &&
            pImpl->OutByte && pImpl->OutWord && pImpl->OutLong &&
            pImpl->name)
   {
      theImpl = *pImpl;
      isValid = TRUE;
   #if 0
      hwShowPortMap();
   #endif
   }
   else
   {
      printf("Invalid HwInterfaceImpl\n");
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
   ASSERT_IPORT(port);
   return theImpl.InByte(port);
}

USHORT hwInWord(HwPortId portId)
{
   HwPortReg port = hwGetPortRegister(portId);
   ASSERT_IPORT(port);
   return theImpl.InWord(port);
}

ULONG hwInLong(HwPortId portId)
{
   HwPortReg port = hwGetPortRegister(portId);
   ASSERT_IPORT(port);
   return theImpl.InLong(port);
}

void hwOutByte(HwPortId portId, HwByte data)
{
   HwPortReg port = hwGetPortRegister(portId);
   ASSERT_OPORT(port);
   theImpl.OutByte(port, data);
}

void hwOutWord(HwPortId portId, HwWord data)
{
   HwPortReg port = hwGetPortRegister(portId);
   ASSERT_OPORT(port);
   theImpl.OutWord(port, data);
}

void hwOutLong(HwPortId portId, HwLong data)
{
   HwPortReg port = hwGetPortRegister(portId);
   ASSERT_OPORT(port);
   theImpl.OutLong(port, data);
}

/**
 * Command-line utility to print the current HwPortId-to-HwPortReg mapping.
 */
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

      printf("HwInterfaceImpl:\n Name=%s\n GetPortRegister=%p\n Port mapping:\n",
             (theImpl.name ? theImpl.name : "<NoName>"), theImpl.GetPortRegister );

      mapSize = theImpl.numPorts;

      for (portId = 0; portId < mapSize; portId++)
      {
         printf("portId=%02d | ", portId);
         hwShowPortId(portId);
      }
   }

   return mapSize;
}

/**
 * Command-line utility to print CCA register info for a HwPortId
 */
void hwShowPortId(HwPortId portId)
{
   HwPortReg portReg = hwGetPortRegister(portId);
   ccaIoPortShow(portReg);
}
