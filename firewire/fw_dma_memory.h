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

#ifndef _FW_DMA_MEMORY_INCLUDE_
#define _FW_DMA_MEMORY_INCLUDE_

#include <firewire/fw_types.h>
#include <firewire/fw_error_codes.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Structure definitions */
typedef struct FWDmaMemoryEntryStruct
{
	struct FWDmaMemoryPoolStruct	*poolPtr;	/* Pointer to the pool from which this buffer is allocated. */
	unsigned char	*virtualAddr;					/* Pointer to start of data area within this block (virtual address) */
	unsigned char	*physicalAddr;					/* Physical address (16 bit aligned) of the data area used by the hardware */
	unsigned long	length;							/* Length of block within the pool (data area size + descriptor size + 4 byte align) */
	boolean			inUse;							/* True if the block is inuse (not deallocated) */
	struct FWDmaMemoryEntryStruct *next;		/* Pointer to next entry or NULL if tail */
	struct FWDmaMemoryEntryStruct *prev;		/* Pointer to previous entry or NULL if head */
	unsigned char	pad[16];							/* Pad for 16 bit alignment */

} FWDmaMemoryEntry;

typedef struct FWDmaMemoryPoolStruct
{
	FWDmaMemoryEntry					*startPtr;	/* Pointer to beginning of allocated DMA memory block and the first DMA entry */
	FWDmaMemoryEntry					*endPtr;		/* Pointer to the end or last entry or NULL if no memory is allocated */
	unsigned long						length;		/* Total length in bytes of the allocated memory block */
	struct FWDmaMemoryPoolStruct	*next;		/* Pointer to the next pool or NULL if tail */
	struct FWDmaMemoryPoolStruct	*prev;		/* Pointer to previous pool or NULL if head */

} FWDmaMemoryPool;

/* Allocate a block of memory from the
 * DMA memory pool.
 */
FWStatus fwAllocateDMAMemPool(FWDmaMemoryPool **dmaMemoryPoolHead, unsigned long numBytes, FWDmaMemoryEntry **dmaMemory);

/* Free a block of memory allocated with
 * the fwAllocateDMAMemPool() function.
 */
FWStatus fwFreeDMAMemPool(FWDmaMemoryPool **dmaMemoryPoolHead, FWDmaMemoryEntry *dmaMemory);

/* Function for allocating DMA memory.
 */
void *fwMallocDMA(size_t numBytes);

/* Function to free DMA memory allocated
 * with the fwMallocDMA() function.
 */
void fwFreeDMA(void *ptr);

/* Convert a virtual DMA address to a physical address.
 */
void *fwVirtualToPhysicalAddress(void *virtualAddr);

/* Convert a physical DMA address to a virtual address.
 */
void *fwPhysicalToVirtualAddress(void *physicalAddr);

/* Flush the DMA memory after writting buffer (included for portability)
 */
void fwFlushDMAMemPool(FWDmaMemoryEntry *dmaEntry);

/* Invalidate the DMA memory before reading buffer (included for portability)
 */
void fwInvalidateDMAMemPool(FWDmaMemoryEntry *dmaEntry);

/* Reports current memory allocations statistics for
 * DMA memory.
 */
void fwDMAMemShow(void);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_DMA_MEMORY_INCLUDE_ */

