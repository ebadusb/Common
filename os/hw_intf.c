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

static unsigned long theCcaPciVerno = 0; /* for development/testing purposes */

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

      /* For development/testing purposes */
      if (ccaPciResourcesAvailable())
      {
         ccaPciResources ccaInfo = {0};
         ccaPciGetResource(0, &ccaInfo);
         theCcaPciVerno = (ccaInfo.subsystemId << 8) | (ccaInfo.revisionId);
      }

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

static BOOL diffExceeded(UINT v1, UINT v2, UINT negDiff, UINT posDiff, UINT maxVal)
{
   BOOL result = FALSE;
   UINT diff;

   if ( v2 > v1 )
   {
      // Check for underflow on a decrementing counter
      if ( v1 < negDiff && v2 >= maxVal-negDiff )
      {
         diff = ( v1+maxVal ) - v2;
         result = ( diff > negDiff );
      }
      else
      {
         diff = v2 - v1;
         result = ( diff > posDiff );
      }
   }
   else if ( v2 < v1 )
   {
      // Check for overflow on an incrementing counter
      if ( v2 < posDiff && v1 >= maxVal-posDiff )
      {
         diff = ( v2+maxVal ) - v1;
         result = ( diff > posDiff );
      }
      else
      {
         diff = v1 - v2;
         result = ( diff > negDiff );
      }
   }
   return result;
}

UCHAR hwReadAndCheckByte(HwPortId portId, UINT negDiff, UINT posDiff, UINT maxVal,
                         HwLogDiscrepancyFunc* logFunc, const char* file, int line)
{
   HwPortReg port = hwGetPortRegister(portId);
   UCHAR result = theImpl.InByte(port);

   UCHAR v1 = result;
   UCHAR v2 = theImpl.InByte(port);

   if ( (v1 != v2) && diffExceeded(v1, v2, negDiff, posDiff, maxVal) )
   {
      /* Use the 3rd reading as the final result */
      result = theImpl.InByte(port);

      /* And optionally log the discrepancy */
      if ( logFunc != NULL ) logFunc(file, line, portId, v1, v2, result);
   }
   return result;
}

USHORT hwReadAndCheckWord(HwPortId portId, UINT negDiff, UINT posDiff, UINT maxVal,
                          HwLogDiscrepancyFunc* logFunc, const char* file, int line)
{
   HwPortReg port = hwGetPortRegister(portId);
   USHORT result = theImpl.InWord(port);

   USHORT v1 = result;
   USHORT v2 = theImpl.InWord(port);

   BOOL   isOutOfRange = FALSE;

   if (v1 == v2) return result;

   if (maxVal <= 256)
   {
      UCHAR v1Lo = (v1 & 0x00FF);
      UCHAR v2Lo = (v2 & 0x00FF);
      UCHAR v1Hi = (v1 & 0xFF00) >> 8;
      UCHAR v2Hi = (v2 & 0xFF00) >> 8;

      isOutOfRange = ( diffExceeded(v1Lo, v2Lo, negDiff, posDiff, maxVal) ||
                       diffExceeded(v1Hi, v2Hi, negDiff, posDiff, maxVal) );
   }
   else
   {
      isOutOfRange = diffExceeded(v1, v2, negDiff, posDiff, maxVal);
   }

   if (isOutOfRange)
   {
      /* Use the 3rd reading as the final result */
      result = theImpl.InWord(port);

      /* And optionally log the discrepancy */
      if (logFunc != NULL) logFunc(file, line, portId, v1, v2, result);
   }
   return result;
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
