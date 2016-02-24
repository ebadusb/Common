/*
 *
 * This file contains the TerumoBCT CCA hardware initialization routines.
 *
 * These routines detect any TerumoBCT CCA FPGA cards with:
 *    type     : BEFORE_STD
 *    vendor-id: 0x11AA
 * And then maps them into the kernel's system memory.
 *
 * Note: this mapping must occur prior to the system calling usrMmuInit()
 *
 * Note: TerumoBCT CCAs are not interrupt-driven
 *
 */

#include <vxWorks.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <vmLib.h>
#include <drv/pci/pciConfigLib.h>
#include <drv/pci/pciIntLib.h>
#include "config.h"


#include "cca_pci_support.h"

/* Local prototypes */
LOCAL void   ccaResourceArrayInit (ccaPciResources data[CCA_MAX_PCI_RESOURCES]);
LOCAL void   ccaResourceArraySave (ccaPciResources data[CCA_MAX_PCI_RESOURCES]);
LOCAL STATUS ccaPciDetect (int index, ccaPciResources data[CCA_MAX_PCI_RESOURCES]);
LOCAL STATUS ccaPciInstall (ccaPciResources data[CCA_MAX_PCI_RESOURCES]);
LOCAL STATUS installOneBarDevice (UINT index, ccaPciResources ccaData[CCA_MAX_PCI_RESOURCES], UINT32 mapSize);
LOCAL STATUS installTwoBarDevice (UINT index, ccaPciResources ccaData[CCA_MAX_PCI_RESOURCES], UINT32 mapSize1, UINT32 mapSize2);

/* Prototypes (vxWorks prototypes included here because they are not in the vxWorks headers!) */
IMPORT STATUS sysMmuMapAdd (void* address,
                            UINT length,
                            UINT initialStateMask,
                            UINT initialState);

/* holds the PCI information CCA resources for ccaPciShow */
ccaPciResources ccaPciData[CCA_MAX_PCI_RESOURCES];

/*
 * ccaPciShow
 *
 * This function displays a list of detected CCA resources.
 * Uses stored data (ccaPciData[]) from CCA install to show what happened at point of call
 *
 * Returns:
 *    void
 */
void ccaPciShow (void)
{
   int index = 0;

   printf("CCA FPGA Cards\n");
   for (index = 0; index < CCA_MAX_PCI_RESOURCES; index++)
   {
      if (ccaPciData[index].statusCode != CCA_DEVICE_FOUND &&
          ccaPciData[index].statusCode != CCA_DEVICE_INIT_STATE)
      {
         printf("  Failure Code(index# %d):  0x%02X   \n", index,
                ccaPciData[index].statusCode);
      }
   }

   printf("Indx VendID DeviceID SubSysID RevID Bus Device Func Pin  BAR0 Addr   BAR1 Addr\n");
   for (index = 0; index < CCA_MAX_PCI_RESOURCES; index++)
   {
      printf("  %d: 0x%04X 0x%04X   0x%04X   0x%02d  %3d %6d %4d %3d  0x%08X  0x%08X\n",
             index,
             ccaPciData[index].vendorId,
             ccaPciData[index].deviceId,
             ccaPciData[index].subsystemId,
             ccaPciData[index].revisionId,
             ccaPciData[index].busNo,
             ccaPciData[index].deviceNo,
             ccaPciData[index].funcNo,
             ccaPciData[index].ipin,
             ccaPciData[index].pBAR0,
             ccaPciData[index].pBAR1);
   }
}

STATUS sysCCAHwInit (void)
{
   STATUS          retVal = ERROR;
   int             index  = 0;
   int             count  = 0;
   ccaPciResources ccaData[CCA_MAX_PCI_RESOURCES];

   /* Initialize local resource array */
   ccaResourceArrayInit(ccaData);

   /* detect what cards are available */
   for (index = 0; index < CCA_MAX_PCI_RESOURCES; index++)
   {
      retVal = ccaPciDetect(index, ccaData);
      if (retVal == OK)
      {
         ++count;
      }
   }

   /* if CCA cards found then install them into system memory map */
   if (count != 0)
   {
      ccaPciInstall(ccaData);
   }

   /* copy results to global ccaPciData array */
   ccaResourceArraySave(ccaData);

   if ( count > 0 ) retVal = OK;
   return retVal;
}


STATUS ccaPciGetResource (UINT rsrcIndx, ccaPciResources* pResource)
{
   if (rsrcIndx < CCA_MAX_PCI_RESOURCES && pResource != NULL)
   {
      *pResource = ccaPciData[rsrcIndx];
      return OK;
   }
   return ERROR;
}

UINT8 ccaInByte (UINT8 offset, UINT rsrcIndx, BOOL useBar1)
{
   UINT8* pBar = (UINT8*)(useBar1 ? ccaPciData[rsrcIndx].pBAR1 : ccaPciData[rsrcIndx].pBAR0 );
   if (pBar && rsrcIndx < CCA_MAX_PCI_RESOURCES)
      return pBar[offset];
   return 0;
}

UINT16 ccaInWord (UINT8 offset, UINT rsrcIndx, BOOL useBar1)
{
   UINT8* pBar = (UINT8*)(useBar1 ? ccaPciData[rsrcIndx].pBAR1 : ccaPciData[rsrcIndx].pBAR0 );
   if (pBar && rsrcIndx < CCA_MAX_PCI_RESOURCES)
      return *(UINT16*)(pBar+offset);
   return 0;
}

void ccaOutByte (UINT8 offset, UINT8 value, UINT rsrcIndx, BOOL useBar1)
{
   UINT8* pBar = (UINT8*)(useBar1 ? ccaPciData[rsrcIndx].pBAR1 : ccaPciData[rsrcIndx].pBAR0 );
   if (pBar && rsrcIndx < CCA_MAX_PCI_RESOURCES)
      pBar[offset] = value;
}

void ccaOutWord (UINT8 offset, UINT16 value, UINT rsrcIndx, BOOL useBar1)
{
   UINT16* pBar = (UINT16*)(useBar1 ? ccaPciData[rsrcIndx].pBAR1 : ccaPciData[rsrcIndx].pBAR0 );
   if (pBar && rsrcIndx < CCA_MAX_PCI_RESOURCES)
      *(UINT16*)(pBar+offset) = value;
}

void ccaOut32Word(UINT8 offset, UINT32 value, UINT rsrcIndx, BOOL useBar1)
{
   UINT32 * pBar = (UINT32*)(useBar1 ? ccaPciData[rsrcIndx].pBAR1 : ccaPciData[rsrcIndx].pBAR0 );
   if (pBar && rsrcIndx < CCA_MAX_PCI_RESOURCES)
      *(UINT32*)(pBar+offset) = value;
}

LOCAL void ccaResourceArrayInit (ccaPciResources data[CCA_MAX_PCI_RESOURCES])
{
   int index = 0;

   /* Initialize the resource array */
   for (index = 0; index < CCA_MAX_PCI_RESOURCES; index++)
   {
      memset(&data[index], 0, sizeof(ccaPciResources));
      data[index].statusCode = CCA_DEVICE_INIT_STATE;
   }
}

LOCAL void ccaResourceArraySave (ccaPciResources data[CCA_MAX_PCI_RESOURCES])
{
   int index = 0;

   for (index = 0; index < CCA_MAX_PCI_RESOURCES; index++)
   {
      ccaPciData[index] = data[index];
   }
}

LOCAL STATUS ccaPciDetect (int index, ccaPciResources data[CCA_MAX_PCI_RESOURCES])
{
   STATUS        retVal      = ERROR;
   int           busNo       = 0;
   int           deviceNo    = 0;
   int           funcNo      = 0;
   unsigned char ipin        = 0;
   UINT16        vendorId    = 0;
   UINT16        deviceId    = 0;
   UINT16        subsystemId = 0;
   UINT8         revisionId  = 0;

   do
   {
      /* check for CCA device class */
      retVal = pciFindClass(CCA_PCI_DEVICE_CLASS,
                            index,
                            &busNo,
                            &deviceNo,
                            &funcNo);
      if (retVal != OK)
      {
         data[index].statusCode = CCA_DEVICE_CLASS_NOT_FOUND;
         break;
      }

      /* get the vendor ID */
      retVal = pciConfigInWord(busNo,
                               deviceNo,
                               funcNo,
                               PCI_CFG_VENDOR_ID,
                               &vendorId);
      if (retVal != OK)
      {
         data[index].statusCode = CCA_DEVICE_VENDID_FAIL;
         break;
      }

      /* check for the correct vendor id */
      if (vendorId != CCA_PCI_VID)
      {
         data[index].statusCode = CCA_DEVICE_NOT_FOUND;
         retVal                 = ERROR;
         break;
      }

      /* get the device ID */
      retVal = pciConfigInWord(busNo,
                               deviceNo,
                               funcNo,
                               PCI_CFG_DEVICE_ID,
                               &deviceId);
      if (retVal != OK)
      {
         data[index].statusCode = CCA_DEVICE_DEVID_FAIL;
         break;
      }

      /* get the CCA board revision number, stored in PCI SubSytem ID field */
      retVal = pciConfigInWord(busNo,
                               deviceNo,
                               funcNo,
                               PCI_CFG_SUB_SYSTEM_ID,
                               &subsystemId);

      /* get the CCA interface revision number, stored in PCI Revision field */
      retVal = pciConfigInByte(busNo,
                               deviceNo,
                               funcNo,
                               PCI_CFG_REVISION,
                               &revisionId);

      /* get ipin -- value should always be zero */
      retVal = pciConfigInByte(busNo,
                               deviceNo,
                               funcNo,
                               PCI_CFG_DEV_INT_PIN,
                               &ipin);

      /* save off information for installation */
      data[index].busNo       = busNo;
      data[index].deviceNo    = deviceNo;
      data[index].funcNo      = funcNo;
      data[index].ipin        = ipin;
      data[index].vendorId    = vendorId;
      data[index].deviceId    = deviceId;
      data[index].subsystemId = subsystemId;
      data[index].revisionId  = revisionId;
      data[index].statusCode  = CCA_DEVICE_FOUND;

      retVal = OK;

   } while (0);

   return retVal;
}

LOCAL STATUS ccaPciInstall (ccaPciResources data[CCA_MAX_PCI_RESOURCES])
{
   STATUS stat     = ERROR;
   int    ii       = 0;
   UINT32 mapsize1 = 0;
   UINT32 mapsize2 = 0;

   for (ii = 0; ii < CCA_MAX_PCI_RESOURCES; ii++)
   {
      if (data[ii].statusCode == CCA_DEVICE_FOUND)
      {
         switch (data[ii].deviceId)
         {
            case CCA_PCI_CTRL3_DEV_ID :
               mapsize1 = CCA_PCI_CTRL3_MMAP_SIZE;
               stat     = installOneBarDevice(ii, data, mapsize1);
               break;

            case CCA_PCI_SAFETYP_DEV_ID :
               mapsize1 = CCA_PCI_SAFETYP_MMAP_SIZE;
               mapsize2 = CCA_PCI_SAFETYP_PFR_MMAP_SIZE;
               stat     = installTwoBarDevice(ii, data, mapsize1, mapsize2);
               break;

            case CCA_PCI_STCPCI_DEV_ID :
               mapsize1 = CCA_PCI_STCPCI_MMAP_SIZE;
               stat     = installOneBarDevice(ii, data, mapsize1);
               break;

            case CCA_PCI_USTCP_DEV_ID :
               mapsize1 = CCA_PCI_USTCP_MMAP_SIZE;
               mapsize2 = CCA_PCI_USTCP_PFR_MMAP_SIZE;
               stat     = installTwoBarDevice(ii, data, mapsize1, mapsize2);
               break;

            default :
               break;

         }
      }
   }
   return stat;
}

LOCAL STATUS installOneBarDevice (UINT index,
                                  ccaPciResources ccaData[CCA_MAX_PCI_RESOURCES],
                                  UINT32 mapSize)
{
   int    busNo    = ccaData[index].busNo;
   int    deviceNo = ccaData[index].deviceNo;
   int    funcNo   = ccaData[index].funcNo;
   UINT32 bar0;
   STATUS retVal = ERROR;

   do
   {
      /* get BAR 0 */
      retVal = pciConfigInLong(busNo,
                               deviceNo,
                               funcNo,
                               PCI_CFG_BASE_ADDRESS_0,
                               &bar0);
      if (retVal != OK)
      {
         ccaData[index].statusCode = CCA_DEVICE_BAR0_FAIL;
         break;
      }

      bar0 &= PCI_MEMBASE_MASK;

      /* map memory area */
      retVal = sysMmuMapAdd((void*)bar0,
                            mapSize,
                            VM_STATE_MASK_FOR_ALL,
                            VM_STATE_FOR_PCI);
      if (retVal != OK)
      {
         ccaData[index].statusCode = CCA_DEVICE_MEM_MAP_FAIL;
         break;
      }

      /* enable mapped memory decoder */
      retVal = pciConfigOutWord(busNo,
                                deviceNo,
                                funcNo,
                                PCI_CFG_COMMAND,
                                PCI_CMD_MEM_ENABLE | PCI_CMD_MASTER_ENABLE);
      if (retVal != OK)
      {
         ccaData[index].statusCode = CCA_DEVICE_ENABLE_DECODER_FAIL;
         break;
      }

      ccaData[index].pBAR0 = (void*)bar0;

   } while (0);

   return retVal;
}

LOCAL STATUS installTwoBarDevice (UINT index,
                                  ccaPciResources ccaData[CCA_MAX_PCI_RESOURCES],
                                  UINT32 mapSize1,
                                  UINT32 mapSize2)
{
   int    busNo    = ccaData[index].busNo;
   int    deviceNo = ccaData[index].deviceNo;
   int    funcNo   = ccaData[index].funcNo;
   STATUS retVal   = ERROR;
   UINT32 bar0;
   UINT32 bar1;

   do
   {
      /* get BAR 0 */
      retVal = pciConfigInLong(busNo,
                               deviceNo,
                               funcNo,
                               PCI_CFG_BASE_ADDRESS_0,
                               &bar0);
      if (retVal != OK)
      {
         ccaData[index].statusCode = CCA_DEVICE_BAR0_FAIL;
         break;
      }

      bar0 &= PCI_MEMBASE_MASK;

      /* map memory area */
      retVal = sysMmuMapAdd((void*)bar0,
                            mapSize1,
                            VM_STATE_MASK_FOR_ALL,
                            VM_STATE_FOR_PCI);
      if (retVal != OK)
      {
         ccaData[index].statusCode = CCA_DEVICE_MEM_MAP_FAIL;
         break;
      }

      /* get BAR 1 */
      retVal = pciConfigInLong(busNo,
                               deviceNo,
                               funcNo,
                               PCI_CFG_BASE_ADDRESS_1,
                               &bar1);
      if (retVal != OK)
      {
         ccaData[index].statusCode = CCA_DEVICE_BAR1_FAIL;
         break;
      }

      bar1 &= PCI_MEMBASE_MASK;

      /* map memory area */
      retVal = sysMmuMapAdd((void*)bar1,
                            mapSize2,
                            VM_STATE_MASK_FOR_ALL,
                            VM_STATE_FOR_PCI);
      if (retVal != OK)
      {
         ccaData[index].statusCode = CCA_DEVICE_MEM_MAP_FAIL;
         break;
      }

      /* enable mapped memory decoder */
      retVal = pciConfigOutWord(busNo,
                                deviceNo,
                                funcNo,
                                PCI_CFG_COMMAND,
                                PCI_CMD_MEM_ENABLE | PCI_CMD_MASTER_ENABLE);
      if (retVal != OK)
      {
         ccaData[index].statusCode = CCA_DEVICE_ENABLE_DECODER_FAIL;
         break;
      }

      ccaData[index].pBAR0 = (void*)bar0;
      ccaData[index].pBAR1 = (void*)bar1;

   } while (0);

   return retVal;
}

