/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains the data structures used in the isochronous
 * communications protocol.
 *
 * $Log$
 *
 */

#ifndef _FW_ISOCHRONOUS_DATA_INCLUDE_
#define _FW_ISOCHRONOUS_DATA_INCLUDE_

#include <semLib.h>
#include <firewire/fw_types.h>
#include <firewire/fw_dma_memory.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FWIsoDescriptorBlockStruct
{
	FWDmaMemoryEntry 	*commandList;			/* DMA memory being used to hold the descriptor command blocks */
	FWDmaMemoryEntry	*data;					/* DMA memory used to hold the send or receive data */
	boolean				inuse;					/* True if the descriptor is in use */
	unsigned long		length;					/* Amount of valid data in bytes in the data area */
	unsigned long		dataSize;				/* Size in bytes of the allocated data buffer (read only) */
	UINT32				*program;				/* Isochronous context program template */
	unsigned long		programSize;			/* Program size in bytes */
	struct FWIsoDescriptorBlockStruct *next;	/* Pointer to next descriptor block or NULL if tail */
	struct FWIsoDescriptorBlockStruct *prev;	/* Pointer to previous descriptor block or NULL if head */

} FWIsoDescriptorBlock;

typedef struct FWIsoContextProgramStruct
{
	FWIsoDescriptorBlock	*descriptorBlock;	/* Descriptor block used by this context program */
	UINT32					*lastBlock;			/* Virtual address of the last input more block */
	UINT32 					xferStatus;			/* Transfer status of isochronous transfer. */
	FWDmaMemoryPool		*dmaMemoryPool;	/* Points to the head of the DMA memory pool. Can be NULL. */
	struct FWIsoContextProgramStruct *next; /* Next context program in the list or NULL if tail */
	struct FWIsoContextProgramStruct *prev; /* Previous context program in the list or NULL if head */

} FWIsoContextProgram;

typedef struct FWIsoContextListStruct
{
	FWIsoContextProgram		*head;	/* Context program list head. NULL if list is empty */
	FWIsoContextProgram		*tail;	/* Context program list tail. NULL if list is empty */
	unsigned long				count;	/* Number of items in the list */

} FWIsoContextList;

typedef struct FWIsoChannelStruct
{
	int						channel;				/* Isochronous channel */
	boolean					direction;			/* TRUE=receive data FALSE=send data */
	unsigned long			totalSize;			/* Required total buffer size in bytes for the isochronous data */
	unsigned short			firstPacketSize;	/* Size in bytes of the first packet */
	unsigned short			sync;					/* Sync code used by the first packet */
	unsigned long			numBuffers;			/* Number of buffers to allocate */
	SEM_ID					clientSem;			/* Semaphore used to notify the isochronous client. */
	SEM_ID					channelSem;			/* Mutual exclusion semaphore so only one task manipulates the channel at a time. */
	FWIsoContextProgram	*current;			/* Current context program. May be NULL if not running. */
	FWIsoContextList		*contextList;		/* Context list for this channel */
	struct FWIsoChannelStruct *next;			/* Next channel in the list or NULL if tail */
	struct FWIsoChannelStruct *prev;			/* Previous channel in the list or NULL if head */

} FWIsoChannel;

typedef struct FWIsoChannelListStruct
{
	FWIsoChannel		*head;				/* Context program list head. NULL if list is empty */
	FWIsoChannel		*tail;				/* Context program list tail. NULL if list is empty */
	unsigned long		count;				/* Number of items in the list */
	UINT32				recvChannels;		/* Bit mask of implemented isochronous receive channels */
	UINT32				xmitChannels;		/* Bit mask of implemented isochronous transmit channels */
	SEM_ID				listSem;				/* Semaphore used to syncronize list access. */

} FWIsoChannelList;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_ISOCHRONOUS_DATA_INCLUDE_ */

