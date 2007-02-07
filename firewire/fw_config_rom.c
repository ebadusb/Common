/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header: H:/BCT_Development/vxWorks/Common/firewire/rcs/fw_config_rom.c 1.1 2007/02/07 15:22:29Z wms10235 Exp wms10235 $
 *
 * This file contains the firewire configuration ROM routines.
 *
 * $Log: fw_config_rom.c $
 *
 */

#include <vxWorks.h>
#include "fw_utility.h"
#include "fw_dma_memory.h"
#include "fw_config_rom.h"

/* Local prototypes */
static FWStatus fwBuildDefaultConfigROM(FWDriverData *pDriver, UINT32 *configROM);

FWStatus fwCreateConfigROM(FWDriverData *pDriver)
{
	FWStatus retVal = FWNotInitialized;
	FWDmaMemoryEntry *pDmaMem;
	unsigned char* pAlign;
	unsigned int	offset;

	do
	{
		if( pDriver == NULL ) break;

		if( pDriver->configROM != NULL )
		{
			retVal = FWSuccess;
			break;
		}

		pDriver->configROM = fwMalloc( sizeof(FWConfigROM) );

		if( pDriver->configROM == NULL )
		{
			retVal = FWMemAllocateError;
			break;
		}

		pDriver->configROM->memPtr = NULL;
		pDriver->configROM->config1PhysicalAddr = NULL;
		pDriver->configROM->config1VirtualAddr = NULL;
		pDriver->configROM->config2PhysicalAddr = NULL;
		pDriver->configROM->config2VirtualAddr = NULL;
		pDriver->configROM->selectedConfig = -1;

		retVal = fwAllocateDMAMemPool( &pDriver->dmaMemoryPool, 3072, &pDmaMem );
		if( retVal != FWSuccess )
		{
			break;
		}

		pDriver->configROM->memPtr = pDmaMem;

		/* Align the physical memory address on a 1K boundry */
		pAlign = pDmaMem->physicalAddr + 0x000003FF;
		offset = (unsigned int)pAlign & 0x000003FF;

		pDriver->configROM->config1PhysicalAddr = (UINT32*)(pAlign - offset);
		pDriver->configROM->config1VirtualAddr = fwPhysicalToVirtualAddress( pDriver->configROM->config1PhysicalAddr );

		/* The next block is + 1024 bytes away */
		pDriver->configROM->config2PhysicalAddr = (UINT32*)((char*)pDriver->configROM->config1PhysicalAddr + 1024);
		pDriver->configROM->config2VirtualAddr = (UINT32*)((char*)pDriver->configROM->config1VirtualAddr + 1024);

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

FWStatus fwDestroyConfigROM(FWDriverData *pDriver)
{
	FWStatus retVal = FWNotInitialized;

	if( pDriver )
	{
		if( pDriver->configROM )
		{
			if( pDriver->configROM->memPtr )
			{
				fwFreeDMAMemPool( &pDriver->dmaMemoryPool, pDriver->configROM->memPtr );
			}

			fwFree( pDriver->configROM );
			pDriver->configROM = NULL;

			retVal = FWSuccess;
		}
	}

	return retVal;
}

FWStatus fwInitializeConfigROM(FWDriverData *pDriver)
{
	FWStatus retVal = FWNotInitialized;
	UINT32 *pConfigROM = NULL;
	UINT32 *pPhysicalAddr = NULL;
	UINT32 linkStatus;
	int newSelected = -1;

	do
	{
		if( pDriver == NULL ) break;
		if( pDriver->configROM == NULL ) break;
		if( pDriver->configROM->memPtr == NULL ) break;

		/* Create a config ROM image using available space */
		if( pDriver->configROM->selectedConfig == 1 )
		{
			/* If 1 is currently active, use 2 for the new config ROM */
			pConfigROM = pDriver->configROM->config2VirtualAddr;
			pPhysicalAddr = pDriver->configROM->config2PhysicalAddr;
			newSelected = 2;
		}
		else if( pDriver->configROM->selectedConfig == 2 )
		{
			/* If 2 is currently active, use 1 for the new config ROM */
			pConfigROM = pDriver->configROM->config1VirtualAddr;
			pPhysicalAddr = pDriver->configROM->config1PhysicalAddr;
			newSelected = 1;
		}
		else if( pDriver->configROM->selectedConfig == -1 )
		{
			/* Nothing is active, use 1 for the new config ROM */
			pConfigROM = pDriver->configROM->config1VirtualAddr;
			pPhysicalAddr = pDriver->configROM->config1PhysicalAddr;
			newSelected = 1;
		}
		else
		{
			break;
		}

		/* Build the default config ROM structure */
		retVal = fwBuildDefaultConfigROM( pDriver, pConfigROM );

		/* Flush the memory so the hardware can see the updated config ROM */
		fwFlushDMAMemPool( pDriver->configROM->memPtr );

		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Error building default config ROM. Error:%d\n", retVal );
			break;
		}

		linkStatus = pDriver->ohci->hcControlClr;

		/* If the link enable bit is 0 and nothing is selected, update the config ROM */
		if( pDriver->configROM->selectedConfig == -1 && ( linkStatus & 0x00020000 ) == 0 )
		{
			pDriver->configROM->selectedConfig = newSelected;
			pDriver->ohci->configRomHeader = pConfigROM[0];

			/* Let the hardware know about the new config ROM */
			pDriver->ohci->configROMAddr = (UINT32)pPhysicalAddr;

			/* Enable the config rom */
			pDriver->ohci->hcControlSet = 0x80000000;
		}

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

FWStatus fwUpdateConfigROM(FWDriverData *pDriver)
{
	FWStatus retVal = FWNotInitialized;

	if( pDriver )
	{
	}

	return retVal;
}

static FWStatus fwBuildDefaultConfigROM(FWDriverData *pDriver, UINT32 *pConfigROM)
{
	FWStatus retVal = FWInternalError;
	UINT16 crc = 0;
	int romLen = 0;
	int dirIndex;
	int index;

	if( pDriver && pConfigROM )
	{
		/* Populate the bus info block */
		dirIndex = romLen++;
		pConfigROM[romLen++] = pDriver->ohci->busId;
		pConfigROM[romLen++] = 0xF000A222;
		pConfigROM[romLen++] = pDriver->ohci->guiIdHi;
		pConfigROM[romLen++] = pDriver->ohci->guiIdLo;

		pDriver->ohci->busOptions = 0xF000A222;

		/* Populate the root directory */
		dirIndex = romLen++;
		pConfigROM[romLen++] = 0x0C0083C0;	/* Node capabilitities */
		pConfigROM[romLen++] = (pConfigROM[3] >> 8) | 0x03000000;
		pConfigROM[romLen++] = 0x81000007;	/* Text leaf for vendor name */
		pConfigROM[romLen++] = 0xD1000001;	/* Unit directory offset */

		/* compute the CRC */
		crc = 0;
		for( index=dirIndex+1; index<romLen; index++ )
		{
			crc = fwComputeCRC16( crc, pConfigROM[index] );
		}

		pConfigROM[dirIndex] = 0x00040000 | crc; /* Root directory has 4 entries */

		/* Unit directory */
		dirIndex = romLen++;
		pConfigROM[romLen++] = 0x12000000;	/* Vendor spec */
		pConfigROM[romLen++] = 0x13001000;	/* Software version */
		pConfigROM[romLen++] = 0x17000007;	/* Model ID */
		pConfigROM[romLen++] = 0x81000005;	/* Text leaf for unit directory */

		/* compute the CRC */
		crc = 0;
		for( index=dirIndex+1; index<romLen; index++ )
		{
			crc = fwComputeCRC16( crc, pConfigROM[index] );
		}

		pConfigROM[dirIndex] = 0x00040000 | crc; /* Unit directory has 4 entries */

		/* Vendor text leaf */
		dirIndex = romLen++;
		pConfigROM[romLen++] = 0x00000000;	/* Language spec */
		pConfigROM[romLen++] = 0x00000000;	/* Minimal ASCII character set */
		pConfigROM[romLen++] = 0x42435400;	/* Company name "BCT" */

		/* compute the CRC */
		crc = 0;
		for( index=dirIndex+1; index<romLen; index++ )
		{
			crc = fwComputeCRC16( crc, pConfigROM[index] );
		}

		pConfigROM[dirIndex] = 0x00030000 | crc; /* Vendor name has 3 entries */

		/* Model text leaf */
		dirIndex = romLen++;
		pConfigROM[romLen++] = 0x00000000;	/* Language spec */
		pConfigROM[romLen++] = 0x00000000;	/* Minimal ASCII character set */
		pConfigROM[romLen++] = 0x49454545;	/* Model name "IEEE1394" */
		pConfigROM[romLen++] = 0x31333934;

		/* compute the CRC */
		crc = 0;
		for( index=dirIndex+1; index<romLen; index++ )
		{
			crc = fwComputeCRC16( crc, pConfigROM[index] );
		}

		pConfigROM[dirIndex] = 0x00040000 | crc; /* Model text leaf has 4 entries */

		/* compute the overall CRC for the config ROM */
		crc = 0;
		for( index=1; index<romLen; index++ )
		{
			crc = fwComputeCRC16( crc, pConfigROM[index] );
		}

		pConfigROM[0] = 0x04000000 | crc; /* Bus info block has romLen entries with the CRC covering all entries */
		pConfigROM[0] |= ((UINT32)romLen & 0x000000FF) << 16;

		retVal = FWSuccess;
	}

	return retVal;
}

