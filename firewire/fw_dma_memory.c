/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains the routines to manage DMA memory pool.
 *
 * $Log$
 *
 */

#include <vxWorks.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cacheLib.h>
#include "fw_utility.h"
#include "fw_dma_memory.h"

/* Constants and enumerated types */
enum FWDMAInternalConstants
{
	FWDmaMinAllocationSize = 8192		/* Minimum block size to allocate for DMA memory pool */
};

/* Local function prototypes */
static FWStatus fwAddDmaMemPool(FWDmaMemoryPool **dmaMemoryPoolHead, unsigned long numBytes, FWDmaMemoryPool **newDmaPool);

static long fwDMAMemAllocationCount = 0;
static long fwDMAMemAllocationFailed = 0;

FWStatus fwAllocateDMAMemPool(FWDmaMemoryPool **dmaMemoryPoolHead, unsigned long numBytes, FWDmaMemoryEntry **dmaMemory)
{
	FWStatus retVal = FWInternalError;
	FWDmaMemoryPool *dmaPool = NULL;
	FWDmaMemoryEntry *dmaEntry = NULL;
	unsigned long allocSize = numBytes + sizeof(FWDmaMemoryEntry);
	unsigned long freeMemSize = 0;
	unsigned char *pAlign;

	do
	{
		if( dmaMemory == NULL || dmaMemoryPoolHead == NULL ) break;
		*dmaMemory = NULL;
		dmaPool = *dmaMemoryPoolHead;

		/* Make the allocation an even multiple of 4 bytes. */
		allocSize = (allocSize + 3) & ~0x03;

		/* Find a block of memory in the DMA memory pool
		 * that will satisfy this allocation request.
		 */
		while( dmaPool )
		{
			dmaEntry = dmaPool->startPtr;

			/* First look for a free entry that is big enough for this allocation */
			while( dmaEntry )
			{
				/* Find an entry that is big enough but not wasteful of memory either */
				if( dmaEntry->inUse == FALSE && dmaEntry->length >= allocSize && dmaEntry->length < allocSize * 2 )
				{
					/* Found an approriate block to allocate */
					dmaEntry->inUse = TRUE;
					break;
				}

				dmaEntry = dmaEntry->next;
			}

			/* Found a DMA entry so exit the loop */
			if( dmaEntry ) break;

			if( dmaPool->endPtr )
				freeMemSize = dmaPool->length - (unsigned long)( ((UINT8*)dmaPool->endPtr) + dmaPool->endPtr->length - (UINT8*)dmaPool->startPtr );
			else
				freeMemSize = dmaPool->length;

			if( freeMemSize >= allocSize )
			{
				break;
			}

			dmaPool = dmaPool->next;
		}

		if( dmaEntry == NULL )
		{
			/* If a block cannot be located with enough space,
			 * add a new memory block to the pool.
			 */
			if( dmaPool == NULL )
			{
				/* No DMA memory is allocated in the pool list. Need to add some. */
				retVal = fwAddDmaMemPool( dmaMemoryPoolHead, allocSize, &dmaPool );
				if( retVal != FWSuccess ) break;
			}

			/* The endPtr is NULL when no memory has been allocated from this pool */
			if( dmaPool->endPtr == NULL )
			{
				dmaEntry = dmaPool->startPtr;
			}
			else
			{
				/* If there is enough free space within this pool, create a new
				 * dma entry with the allocation size at the end of the pool */
				dmaEntry = (FWDmaMemoryEntry*)((UINT8*)dmaPool->endPtr + dmaPool->endPtr->length);

				assert( (UINT8*)dmaEntry < ((UINT8*)dmaPool->startPtr) + dmaPool->length );

				dmaPool->endPtr->next = dmaEntry;
			}

			dmaEntry->inUse = TRUE;
			dmaEntry->length = allocSize;
			dmaEntry->next = NULL;
			dmaEntry->prev = dmaPool->endPtr;
			/* align the physical address on a 16-bit boundry */
			pAlign = CACHE_DMA_VIRT_TO_PHYS( dmaEntry->pad );
			dmaEntry->physicalAddr = pAlign + 16 - ((unsigned int)pAlign & 0x0F);
			dmaEntry->virtualAddr = CACHE_DMA_PHYS_TO_VIRT( dmaEntry->physicalAddr );
			dmaEntry->poolPtr = dmaPool;
			dmaPool->endPtr = dmaEntry;
		}

		*dmaMemory = dmaEntry;

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

FWStatus fwFreeDMAMemPool(FWDmaMemoryPool **dmaMemoryPoolHead, FWDmaMemoryEntry *dmaMemory)
{
	FWStatus retVal = FWInternalError;
	FWDmaMemoryPool *dmaPool = NULL;
	FWDmaMemoryEntry *dmaEntry = NULL;
	boolean found = FALSE;

	do
	{
		if( dmaMemory == NULL || dmaMemoryPoolHead == NULL ) break;

		/* search for the DMA entry */
		dmaPool = *dmaMemoryPoolHead;

		retVal = FWNotFound;

		while( dmaPool )
		{
			dmaEntry = dmaPool->startPtr;

			while( dmaEntry )
			{
				if( dmaEntry == dmaMemory )
				{
					/* Found the block to deallocate */
					dmaEntry->inUse = FALSE;
					found = TRUE;
					retVal = FWSuccess;
					break;
				}

				dmaEntry = dmaEntry->next;
			}

			if( found ) break;

			dmaPool = dmaPool->next;
		}

	} while(0);

	return retVal;
}

static FWStatus fwAddDmaMemPool(FWDmaMemoryPool **dmaMemoryPoolHead, unsigned long numBytes, FWDmaMemoryPool **newDmaPool)
{
	FWStatus retVal = FWInternalError;
	unsigned long allocSize = (numBytes / FWDmaMinAllocationSize + 1) * FWDmaMinAllocationSize;
	void *dmaMemPtr = NULL;
	FWDmaMemoryPool *dmaPool = *dmaMemoryPoolHead;
	FWDmaMemoryPool *newPool;

	if( newDmaPool && allocSize < 10000000 )
	{
		dmaMemPtr = fwMallocDMA( allocSize );
		newPool = fwMalloc( sizeof(FWDmaMemoryPool) );

		if( dmaMemPtr && newPool )
		{
			newPool->startPtr = (FWDmaMemoryEntry*)dmaMemPtr;
			newPool->endPtr = NULL;
			newPool->length = allocSize;
			newPool->next = NULL;
			newPool->prev = NULL;

			newPool->startPtr->inUse = FALSE;
			newPool->startPtr->length = 0;
			newPool->startPtr->next = NULL;
			newPool->startPtr->prev = NULL;
			newPool->startPtr->virtualAddr = NULL;
			newPool->startPtr->physicalAddr = NULL;
			newPool->startPtr->poolPtr = newPool;

			*newDmaPool = newPool;

			/* Either start a new list or add to existing list */
			if( dmaPool )
			{
				/* Move to the end of the pool list */
				while( dmaPool->next )
				{
					dmaPool = dmaPool->next;
				}

				newPool->prev = dmaPool;
				dmaPool->next = newPool;
			}
			else
			{
				*dmaMemoryPoolHead = newPool;
			}

			retVal = FWSuccess;
		}
		else
		{
			if( dmaMemPtr ) fwFreeDMA( dmaMemPtr );
			if( newPool ) fwFree( newPool );
			retVal = FWMemAllocateError;
		}
	}

	return retVal;
}

void *fwMallocDMA(size_t numBytes)
{
	void *memPtr = cacheDmaMalloc( numBytes );

	if( memPtr )
	{
		fwDMAMemAllocationCount++;
	}
	else
	{
		fwDMAMemAllocationFailed++;
	}

	return memPtr;
}

void fwFreeDMA(void *ptr)
{
	if( ptr )
	{
		cacheDmaFree( ptr );
		fwDMAMemAllocationCount--;
	}
}

void *fwVirtualToPhysicalAddress(void *virtualAddr)
{
	return CACHE_DMA_VIRT_TO_PHYS( virtualAddr );
}

void *fwPhysicalToVirtualAddress(void *physicalAddr)
{
	return CACHE_DMA_PHYS_TO_VIRT( physicalAddr );
}

void fwFlushDMAMemPool(FWDmaMemoryEntry *dmaEntry)
{
	CACHE_DMA_FLUSH( dmaEntry->poolPtr->startPtr, dmaEntry->poolPtr->length );
}

void fwInvalidateDMAMemPool(FWDmaMemoryEntry *dmaEntry)
{
	CACHE_DMA_INVALIDATE( dmaEntry->poolPtr->startPtr, dmaEntry->poolPtr->length );
}

void fwDMAMemShow(void)
{
	printf("DMA memory. Allocations:%d Failures:%d\n", fwDMAMemAllocationCount, fwDMAMemAllocationFailed);
}

