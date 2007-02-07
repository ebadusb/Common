/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains the firewire link layer data structures.
 *
 * $Log$
 *
 */

#ifndef _FW_LINK_DATA_INCLUDE_
#define _FW_LINK_DATA_INCLUDE_

#include <vxWorks.h>
#include <firewire/fw_dma_memory.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FWDescriptorBlockStruct
{
	FWDmaMemoryEntry 	*commandList;			/* DMA memory being used to hold the descriptor command blocks */
	FWDmaMemoryEntry	*data;					/* DMA memory used to hold the send or receive data */
	boolean				inuse;					/* True if the descriptor is in use */
	unsigned int		numCommands;			/* Number of command descriptors in the block (Z value) */
	unsigned long		length;					/* Amount of valid data in bytes in the data area */
	unsigned long		dataSize;				/* Size in bytes of the allocated data buffer (read only) */
	unsigned long		transactionID;			/* Transaction ID or serial number that uniquely identifies the transaction */
	struct FWDescriptorBlockStruct *next;	/* Pointer to next descriptor block or NULL if tail */
	struct FWDescriptorBlockStruct *prev;	/* Pointer to previous descriptor block or NULL if head */

} FWDescriptorBlock;

typedef struct FWContextProgramStruct
{
	FWDescriptorBlock	*descriptorBlock;	/* Link list of available descriptor blocks used by this context program */
	FWDescriptorBlock	*firstBlock;		/* Address of the first block in a multi block context chain. NULL if single block */
	boolean				active;				/* True if the context program is active (running) */
	SEM_ID				contextSem;			/* Mutual exclusion semaphore so only one task manipulates the DMA at a time. */

} FWContextProgram;

typedef struct FWLinkDataStruct
{
	boolean				initialized;					/* Non-zero when initialized */
	FWContextProgram	*asyncTxRequestContext;		/* DMA context for transmit requests */
	FWContextProgram	*asyncTxResponseContext;	/* DMA context for transmit responses */
	FWContextProgram	*asyncRxRequestContext;		/* DMA context for receive requests */
	FWContextProgram	*asyncRxResponseContext;	/* DMA context for receive responses */
	FWDmaMemoryEntry	*selfIdRecvMemory;			/* DMA receive memory for self ID packets */
	UINT32				*selfIdBufferVirtualAddr;	/* Virtual address for 2K of DMA memory for selfID buffer */
	UINT32				*selfIdBufferPhysicalAddr;	/* Physical address for 2K of DMA memory aligned on a 2K boundry for selfID buffer */
	boolean				stalled;							/* Non-zero when transmit requests are stalled waiting for response receive buffers */
	unsigned long		cycleCount;						/* Incremented each 128th bus cycle */
	unsigned int		splitTransTimeoutSec;		/* Current split subaction seconds timeout */
	unsigned int		splitTransTimeoutFrac;		/* Current split subaction fraction of a second timeout */

} FWLinkData;

typedef struct FWOutputMoreDescriptorStruct
{
	UINT32	reqCount;
	UINT32	dataAddress;
	UINT32	undefined1;
	UINT32	undefined2;

} FWOutputMoreDescriptor;

typedef struct FWOutputMoreImmediateDescriptorStruct
{
	UINT32	reqCount;
	UINT32	undefined1;
	UINT32	undefined2;
	UINT32	timeStamp;

	UINT32	data1;
	UINT32	data2;
	UINT32	data3;
	UINT32	data4;

} FWOutputMoreImmediateDescriptor;

typedef struct FWOutputLastDescriptorStruct
{
	UINT32	reqCount;
	UINT32	dataAddress;
	UINT32	branchAddress;
	UINT32	statusAndCount;

} FWOutputLastDescriptor;

typedef struct FWOutputLastImmediateDescriptorStruct
{
	UINT32	reqCount;
	UINT32	undefined;
	UINT32	branchAddress;
	UINT32	statusAndCount;

	UINT32	data1;
	UINT32	data2;
	UINT32	data3;
	UINT32	data4;

} FWOutputLastImmediateDescriptor;

typedef struct FWInputMoreDescriptorStruct
{
	UINT32	reqCount;
	UINT32	dataAddress;
	UINT32	branchAddress;
	UINT32	statusAndCount;

} FWInputMoreDescriptor;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_LINK_DATA_INCLUDE_ */

