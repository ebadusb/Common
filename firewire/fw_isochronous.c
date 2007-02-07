/*
 *  Copyright(c) 2007 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains the firewire isochronous communications
 * routines.
 *
 * $Log$
 *
 */

#include <vxWorks.h>
#include <stdio.h>
#include <sysLib.h>
#include <taskLib.h>
#include <string.h>
#include "fw_utility.h"
#include "fw_ohci_hw.h"
#include "fw_isochronous.h"

enum FWContextControlFlags
{
	FWContextCtrlActive = 0x00000400,
	FWContextCtrlDead = 0x00000800,
	FWContextCtrlWake = 0x00001000,
	FWContextCtrlRun = 0x00008000
};

/* Local function prototypes */
static FWIsoChannel *fwCreateIsoChannel(int channel);
static FWStatus fwDeleteIsoChannel(FWIsoChannel *isoChannel);
static FWStatus fwIsoEnableRecvChannel(FWDriverData *pDriver, FWIsoChannel *isoChannel);
static FWStatus fwIsoDisableRecvChannel(FWDriverData *pDriver, FWIsoChannel *isoChannel);
static FWStatus fwIsoCreateProgram(FWDriverData *pDriver, FWIsoChannel *isoChannel);
static FWStatus fwIsoEnableRecvChannel(FWDriverData *pDriver, FWIsoChannel *isoChannel);
static FWStatus fwIsoEnableSendChannel(FWDriverData *pDriver, FWIsoChannel *isoChannel);
static FWStatus fwIsoDisableRecvChannel(FWDriverData *pDriver, FWIsoChannel *isoChannel);
static FWStatus fwIsoDisableSendChannel(FWDriverData *pDriver, FWIsoChannel *isoChannel);
static FWStatus fwDestroyIsoContextProgram(FWIsoContextProgram *contextProgram);
static FWStatus fwCreateIsoContextProgram(FWIsoChannel *isoChannel, FWIsoContextProgram *contextProgram);

static FWIsoContextList *fwIsoContextListCreate(void);
static void fwIsoContextListDestroy(FWIsoContextList * list);
static FWStatus fwIsoContextListPushBack(FWIsoContextList *list, FWIsoContextProgram *isoContextProgram);
static FWStatus fwIsoContextListPushFront(FWIsoContextList *list, FWIsoContextProgram *isoContextProgram);
static FWIsoContextProgram *fwIsoContextListPopFront(FWIsoContextList *list);
static FWIsoContextProgram *fwIsoContextListPopBack(FWIsoContextList *list);
static FWStatus fwIsoContextListUnlink(FWIsoContextList *list, FWIsoContextProgram *isoContextProgram);

FWStatus fwCreateIsochronousLayer(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	do
	{
		if( pDriver == NULL ) break;

		pDriver->isoChannelList = fwIsoChannelListCreate();

		if( pDriver->isoChannelList == NULL )
		{
			retVal = FWMemAllocateError;
			break;
		}

		/* To determine the number of channels supported by
		 * the hardware, set all the bit in the interrupt mask
		 * and read back how many are actually set. Bits for
		 * unimplemented channels will always read back as 0.
		 */
		pDriver->ohci->isoRecvIntMaskSet = 0xFFFFFFFF;
		pDriver->ohci->isoXmitIntMaskSet = 0xFFFFFFFF;

		pDriver->isoChannelList->recvChannels = pDriver->ohci->isoRecvIntMaskSet;
		pDriver->isoChannelList->xmitChannels = pDriver->ohci->isoXmitIntMaskSet;

		pDriver->ohci->isoRecvIntMaskClr = 0xFFFFFFFF;
		pDriver->ohci->isoXmitIntMaskClr = 0xFFFFFFFF;

		pDriver->ohci->irChannelMaskHiClr = 0xFFFFFFFF;
		pDriver->ohci->irChannelMaskLoClr = 0xFFFFFFFF;

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

FWStatus fwDestroyIsochronousLayer(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	do
	{
		if( pDriver == NULL ) break;

		pDriver->ohci->isoRecvIntMaskClr = 0xFFFFFFFF;
		pDriver->ohci->isoXmitIntMaskClr = 0xFFFFFFFF;

		fwIsoChannelListDestroy( pDriver->isoChannelList );

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

FWStatus fwIsoAllocateChannel(FWDriverData *pDriver, int channel, int bandwidth)
{
	FWStatus retVal = FWInternalError;
	FWIsoChannel *isoChannel;

	do
	{
		if( pDriver == NULL ) break;

		/* Determine if the channel is valid */
		if( channel < 0 )
		{
			retVal = FWInvalidArg;
			break;
		}

		if( ((1 << channel) & pDriver->isoChannelList->recvChannels) == 0 )
		{
			retVal = FWInvalidArg;
			break;
		}

		/* Scan the current list of channels to see if it exists */
		isoChannel = fwFindIsoChannelListItem( pDriver->isoChannelList, channel );

		if( isoChannel == NULL )
		{
			isoChannel = fwCreateIsoChannel( channel );

			if( isoChannel == NULL )
			{
				retVal = FWMemAllocateError;
				break;
			}
			else
			{
				retVal = fwIsoChannelListPushBack( pDriver->isoChannelList, isoChannel );
			}
		}
		else
		{
			retVal = FWSuccess;
		}

	} while(0);

	return retVal;
}

FWStatus fwIsoReleaseChannel(FWDriverData *pDriver, int channel)
{
	FWStatus retVal = FWInternalError;
	FWIsoChannel *isoChannel;

	if( pDriver )
	{
		retVal = fwIsoDisableChannel( pDriver, channel );

		isoChannel = fwRemoveIsoChannelListItem( pDriver->isoChannelList, channel );

		if( isoChannel )
		{
			retVal = fwDeleteIsoChannel( isoChannel );
		}
		else
		{
			retVal = FWNotFound;
		}
	}

	return retVal;
}

FWStatus fwIsoInitRecvChannel(FWDriverData *pDriver, FWIsochronousRecvCmd *command)
{
	FWStatus retVal = FWInternalError;
	FWIsoChannel *isoChannel = NULL;
	boolean semLocked = FALSE;

	do
	{
		if( pDriver == NULL ) break;

		/* Find the channel */
		isoChannel = fwFindIsoChannelListItem( pDriver->isoChannelList, command->channel );

		if( isoChannel == NULL )
		{
			retVal = FWNotFound;
			break;
		}

		if( isoChannel->channelSem == NULL )
		{
			isoChannel->channelSem = semMCreate( SEM_Q_PRIORITY | SEM_INVERSION_SAFE );
			if( isoChannel->channelSem == NULL )
			{
				retVal = FWSemAllocateError;
				break;
			}
		}

		if( semTake( isoChannel->channelSem, WAIT_FOREVER ) != OK )
		{
			retVal = FWInternalError;
			break;
		}

		semLocked = TRUE;

		if( isoChannel->current )
		{
			retVal = FWIsoChannelBusy;
			break;
		}

		if( command->numBuffers < 2 || command->firstPacketSize < 1 || command->totalSize < command->firstPacketSize )
		{
			retVal = FWInvalidArg;
			break;
		}

		isoChannel->direction = TRUE;
		isoChannel->clientSem = command->clientSem;
		isoChannel->firstPacketSize = command->firstPacketSize;
		isoChannel->sync = command->sync;
		isoChannel->totalSize = command->totalSize;
		isoChannel->numBuffers = command->numBuffers;

		retVal = fwIsoCreateProgram( pDriver, isoChannel );

	} while(0);

	if( semLocked )
		semGive( isoChannel->channelSem );

	return retVal;
}

FWStatus fwIsoEnableChannel(FWDriverData *pDriver, int channel)
{
	FWStatus retVal = FWInternalError;
	FWIsoChannel *isoChannel = NULL;
	boolean semLocked = FALSE;

	do
	{
		if( pDriver == NULL ) break;

		/* Find the channel */
		isoChannel = fwFindIsoChannelListItem( pDriver->isoChannelList, channel );

		if( isoChannel == NULL )
		{
			retVal = FWNotFound;
			break;
		}

		if( semTake( isoChannel->channelSem, WAIT_FOREVER ) != OK )
		{
			break;
		}

		semLocked = TRUE;

		if( isoChannel->clientSem == NULL )
		{
			retVal = FWNotInitialized;
			break;
		}

		/* Check if already enabled and running */
		if( isoChannel->current )
		{
			retVal = FWSuccess;
			break;
		}

		/* Call the appropriate enable */
		if( isoChannel->direction )
		{
			retVal = fwIsoEnableRecvChannel( pDriver, isoChannel );
		}
		else
		{
			retVal = fwIsoEnableSendChannel( pDriver, isoChannel );
		}

	} while(0);

	if( semLocked )
		semGive( isoChannel->channelSem );

	return retVal;
}

FWStatus fwIsoDisableChannel(FWDriverData *pDriver, int channel)
{
	FWStatus retVal = FWInternalError;
	FWIsoChannel *isoChannel = NULL;
	boolean semLocked = FALSE;

	do
	{
		if( pDriver == NULL ) break;

		/* Find the channel */
		isoChannel = fwFindIsoChannelListItem( pDriver->isoChannelList, channel );

		if( isoChannel == NULL )
		{
			retVal = FWNotFound;
			break;
		}

		if( semTake( isoChannel->channelSem, WAIT_FOREVER ) != OK )
		{
			break;
		}

		semLocked = TRUE;

		/* Call the appropriate disable */
		if( isoChannel->direction )
		{
			retVal = fwIsoDisableRecvChannel( pDriver, isoChannel );
		}
		else
		{
			retVal = fwIsoDisableSendChannel( pDriver, isoChannel );
		}

	} while(0);

	if( semLocked )
		semGive( isoChannel->channelSem );

	return retVal;
}

FWStatus fwIsoAdvanceRecvBuffer(FWDriverData *pDriver, FWIsoChannel *isoChannel)
{
	FWStatus retVal = FWInternalError;
	boolean semLocked = FALSE;
	FWIsoContextProgram *contextProgram = NULL;

	do
	{
		if( pDriver == NULL ) break;
		if( isoChannel == NULL ) break;

		if( semTake( isoChannel->channelSem, WAIT_FOREVER ) != OK )
		{
			break;
		}

		semLocked = TRUE;

		/* Advance the current context program to the next context program */
		if( isoChannel->current == NULL )
		{
			retVal = FWIsoChannelStopped;
			break;
		}

		/* Determine if the buffer is full by checking resCount in the last block. */
		fwInvalidateDMAMemPool( contextProgram->descriptorBlock->commandList );

		if( isoChannel->current->lastBlock[3] & 0x0000FFFF )
		{
			retVal = FWIsoBufferSyncError;
			break;
		}

		/* Find the next buffer */
		contextProgram = isoChannel->current->next;

		if( contextProgram == NULL )
		{
			contextProgram = isoChannel->contextList->head;
		}

		if( contextProgram == NULL )
		{
			retVal = FWIsoBufferNotFound;
			break;
		}

		/* initialize the context program */
		contextProgram->xferStatus = 0;

		memcpy( contextProgram->descriptorBlock->commandList->virtualAddr,
				  contextProgram->descriptorBlock->program,
				  contextProgram->descriptorBlock->programSize );

		/* Link the current program to this program and wake the context */
		isoChannel->current->lastBlock[2] = (UINT32)contextProgram->descriptorBlock->commandList->physicalAddr | 1;
		isoChannel->current->xferStatus = isoChannel->current->lastBlock[3] >> 16;

		fwFlushDMAMemPool( isoChannel->current->descriptorBlock->commandList );
		fwFlushDMAMemPool( contextProgram->descriptorBlock->commandList );

		pDriver->ohci->isoRecv[isoChannel->channel].contextControlSet = FWContextCtrlWake;

		isoChannel->current = contextProgram;

		retVal = FWSuccess;

	} while(0);

	if( semLocked )
		semGive( isoChannel->channelSem );

	return retVal;
}

FWStatus fwIsoReadRecvBuffer(FWDriverData *pDriver, int channel, unsigned char *pBuffer, unsigned long bufferSize, UINT32 *xferStatus)
{
	FWStatus retVal = FWInternalError;
	FWIsoChannel *isoChannel = NULL;
	boolean semLocked = FALSE;
	FWIsoContextProgram *contextProgram;
	unsigned int size;

	do
	{
		if( pDriver == NULL ) break;

		/* Find the channel */
		isoChannel = fwFindIsoChannelListItem( pDriver->isoChannelList, channel );

		if( isoChannel == NULL )
		{
			retVal = FWNotFound;
			break;
		}

		if( semTake( isoChannel->channelSem, WAIT_FOREVER ) != OK )
		{
			break;
		}

		semLocked = TRUE;

		/* Get the previous buffer */
		if( isoChannel->current == NULL )
		{
			retVal = FWIsoChannelStopped;
			break;
		}

		contextProgram = isoChannel->current->prev;

		if( contextProgram == NULL )
		{
			contextProgram = isoChannel->contextList->tail;
		}

		if( contextProgram == NULL )
		{
			retVal = FWIsoBufferNotFound;
			break;
		}

		if( contextProgram->descriptorBlock->dataSize > bufferSize )
		{
			size = bufferSize;
			retVal = FWDataTruncated;
		}
		else
		{
			size = contextProgram->descriptorBlock->dataSize;
			retVal = FWSuccess;
		}

		fwInvalidateDMAMemPool( contextProgram->descriptorBlock->data );

		memcpy( pBuffer, contextProgram->descriptorBlock->data->virtualAddr, size );
		*xferStatus = contextProgram->xferStatus;

	} while(0);

	if( semLocked )
		semGive( isoChannel->channelSem );

	return retVal;
}

FWIsoChannelList * fwIsoChannelListCreate(void)
{
	FWIsoChannelList * newList = NULL;

	newList = (FWIsoChannelList*)fwMalloc(sizeof(FWIsoChannelList));

	if( newList )
	{
		newList->head = newList->tail = NULL;
		newList->count = 0;
		newList->listSem = semMCreate( SEM_Q_PRIORITY | SEM_INVERSION_SAFE );
		newList->recvChannels = 0;
		newList->xmitChannels = 0;

		if( newList->listSem == NULL )
		{
			fwFree( newList );
			newList = NULL;
		}
	}

	return newList;
}

void fwIsoChannelListDestroy(FWIsoChannelList *list)
{
	FWIsoChannel *isoChannel;

	if( list )
	{
  		isoChannel = fwIsoChannelListPopFront( list );

		while( isoChannel )
		{
			fwDeleteIsoChannel( isoChannel );
			isoChannel = fwIsoChannelListPopFront( list );
		}

		if( list->listSem )
			semDelete( list->listSem );

		fwFree( list );
	}
}

FWStatus fwIsoChannelListPushBack(FWIsoChannelList *list, FWIsoChannel *isoChannel)
{
	FWStatus retVal = FWInternalError;

	do
	{
		if( list == NULL ) break;
		if( isoChannel == NULL ) break;
		if( list->listSem == NULL ) break;

		if( semTake( list->listSem, WAIT_FOREVER ) != OK )
		{
			break;
		}

		if( list->tail )
		{
			isoChannel->next = NULL;
			isoChannel->prev = list->tail;
			list->tail->next = isoChannel;
			list->tail = isoChannel;
		}
		else
		{
			isoChannel->next = NULL;
			isoChannel->prev = NULL;
			list->tail = isoChannel;
			list->head = isoChannel;
		}

		list->count++;

		semGive( list->listSem );

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

FWIsoChannel *fwIsoChannelListPopFront(FWIsoChannelList *list)
{
	FWIsoChannel *isoChannel = NULL;

	do
	{
		if( list == NULL ) break;
		if( list->listSem == NULL ) break;

		if( semTake( list->listSem, WAIT_FOREVER ) != OK )
		{
			break;
		}

		if( list->head )
		{
			isoChannel = list->head;
			list->head = list->head->next;

			if( list->head == NULL )
			{
				list->tail = NULL;
			}

			isoChannel->prev = isoChannel->next = NULL;

			list->count--;
		}

		semGive( list->listSem );

	} while(0);

	return isoChannel;
}

FWIsoChannel *fwFindIsoChannelListItem(FWIsoChannelList *list, int channel)
{
	FWIsoChannel *isoChannel = NULL;

	do
	{
		if( list == NULL ) break;
		if( list->listSem == NULL ) break;

		if( semTake( list->listSem, WAIT_FOREVER ) != OK )
		{
			break;
		}

		isoChannel = list->head;

		while( isoChannel )
		{
			if( channel == isoChannel->channel )
			{
				break;
			}

			isoChannel = isoChannel->next;
		}

		semGive( list->listSem );

	} while(0);

	return isoChannel;
}

FWIsoChannel *fwRemoveIsoChannelListItem(FWIsoChannelList *list, int channel)
{
	FWIsoChannel *isoChannel = NULL;
	FWIsoChannel *pNext;
	FWIsoChannel *pPrev;

	do
	{
		if( list == NULL ) break;
		if( list->listSem == NULL ) break;

		if( semTake( list->listSem, WAIT_FOREVER ) != OK )
		{
			break;
		}

		isoChannel = list->head;

		while( isoChannel )
		{
			if( channel == isoChannel->channel )
			{
				pNext = isoChannel->next;
				pPrev = isoChannel->prev;

				/* unlink the isoChannel from the list */
				if( pNext )
				{
					pNext->prev = pPrev;
				}
				else
				{
					list->tail = pPrev;
				}

				if( pPrev )
				{
					pPrev->next = pNext;
				}
				else
				{
					list->head = pNext;
				}

				isoChannel->next = isoChannel->prev = NULL;

				list->count--;
				break;
			}

			isoChannel = isoChannel->next;
		}

		semGive( list->listSem );

	} while(0);

	return isoChannel;
}

/*
 *  Local functions
 */

static FWIsoChannel *fwCreateIsoChannel(int channel)
{
	FWIsoChannel *isoChannel = NULL;

	do
	{
		isoChannel = (FWIsoChannel*)fwMalloc( sizeof(FWIsoChannel) );

		if( isoChannel == NULL )
		{
			break;
		}

		memset( isoChannel, 0, sizeof(FWIsoChannel) );

		isoChannel->contextList = fwIsoContextListCreate();
		if( isoChannel->contextList == NULL )
		{
			fwFree( isoChannel );
			isoChannel = NULL;
			break;
		}

		isoChannel->channel = channel;

	} while(0);

	return isoChannel;
}

static FWStatus fwDeleteIsoChannel(FWIsoChannel *isoChannel)
{
	FWStatus retVal = FWInternalError;

	if( isoChannel )
	{
		fwIsoContextListDestroy( isoChannel->contextList );

		semDelete( isoChannel->channelSem );

		fwFree( isoChannel );

		retVal = FWSuccess;
	}

	return retVal;
}

static FWStatus fwIsoCreateProgram(FWDriverData *pDriver, FWIsoChannel *isoChannel)
{
	FWStatus retVal = FWInternalError;
	volatile OhciContextRecv *recvContext;
	UINT32 contextCtrl;
	FWIsoContextProgram *contextProgram;
	int i;

	do
	{
		/* Make sure the isochronous channel is idle! */
		recvContext = &pDriver->ohci->isoRecv[isoChannel->channel];

		contextCtrl = recvContext->contextControlClr;

		if( contextCtrl & FWContextCtrlRun )
		{
			retVal = FWIsoChannelBusy;
			break;
		}

		/* Delete the current context program (if any) */
		contextProgram = fwIsoContextListPopFront( isoChannel->contextList );

		while( contextProgram )
		{
			fwDestroyIsoContextProgram( contextProgram );
			contextProgram = fwIsoContextListPopFront( isoChannel->contextList );
		}

		/* Create a new context program */
		for(i=0; i<isoChannel->numBuffers; i++)
		{
			contextProgram = (FWIsoContextProgram*)fwMalloc( sizeof(FWIsoContextProgram) );

			if( contextProgram == NULL )
			{
				retVal = FWMemAllocateError;
				break;
			}

			memset( contextProgram, 0, sizeof(FWIsoContextProgram) );

			retVal = fwCreateIsoContextProgram( isoChannel, contextProgram );

			if( retVal != FWSuccess )
			{
				fwDestroyIsoContextProgram( contextProgram );
				break;
			}

			retVal = fwIsoContextListPushBack( isoChannel->contextList, contextProgram );
			if( retVal != FWSuccess )
			{
				break;
			}
		}

	} while(0);

	return retVal;
}

static FWStatus fwCreateIsoContextProgram(FWIsoChannel *isoChannel, FWIsoContextProgram *contextProgram)
{
	FWStatus retVal = FWInternalError;
	FWDmaMemoryEntry *dmaMem;
	unsigned long numInputMoreBlocks;
	unsigned long blockDataSize;
	unsigned long numFullDataBlocks;
	unsigned long lastBlockSize;
	unsigned long index;
	unsigned char *dataAddress;
	unsigned char *branchAddress;
	UINT32 maxPacketSize = 65532;
	UINT32 *program;

	do
	{
		/* compute the memory requirements */
		blockDataSize = isoChannel->totalSize - isoChannel->firstPacketSize;

		numFullDataBlocks = blockDataSize / maxPacketSize;

		numInputMoreBlocks = numFullDataBlocks + 2; /* Full blocks + first block + last block */

		lastBlockSize = blockDataSize - numFullDataBlocks * maxPacketSize;

		/* Create the descriptor block */
		contextProgram->descriptorBlock = (FWIsoDescriptorBlock*)fwMalloc( sizeof(FWIsoDescriptorBlock) );
		if( contextProgram->descriptorBlock == NULL )
		{
			retVal = FWMemAllocateError;
			break;
		}

		contextProgram->descriptorBlock->inuse = FALSE;
		contextProgram->descriptorBlock->length = 0;
		contextProgram->descriptorBlock->dataSize = 0;
		contextProgram->descriptorBlock->commandList = NULL;
		contextProgram->descriptorBlock->data = NULL;
		contextProgram->descriptorBlock->next = NULL;
		contextProgram->descriptorBlock->prev = NULL;
		contextProgram->descriptorBlock->programSize = 0;

		/* allocate DMA memory for the command descriptors */
		retVal = fwAllocateDMAMemPool( &contextProgram->dmaMemoryPool, numInputMoreBlocks * 16, &dmaMem );
		if( retVal != FWSuccess )
		{
			break;
		}

		contextProgram->descriptorBlock->commandList = dmaMem;
		contextProgram->descriptorBlock->programSize = numInputMoreBlocks * 16;

		/* Allocate space for the context program image */
		contextProgram->descriptorBlock->program = (UINT32*)fwMalloc( numInputMoreBlocks * 16 );
		if( contextProgram->descriptorBlock->program == NULL )
		{
			retVal = FWMemAllocateError;
			break;
		}

		/* allocate DMA memory for the data area */
		retVal = fwAllocateDMAMemPool( &contextProgram->dmaMemoryPool, isoChannel->totalSize, &dmaMem );
		if( retVal != FWSuccess )
		{
			fwFreeDMAMemPool( &contextProgram->dmaMemoryPool, contextProgram->descriptorBlock->commandList );
			contextProgram->descriptorBlock->commandList = NULL;
			break;
		}

		contextProgram->descriptorBlock->dataSize = isoChannel->totalSize;
		contextProgram->descriptorBlock->data = dmaMem;

		memset( contextProgram->descriptorBlock->data->virtualAddr, 0, contextProgram->descriptorBlock->dataSize );

		program = contextProgram->descriptorBlock->program;
		dataAddress = contextProgram->descriptorBlock->data->physicalAddr;
		branchAddress = contextProgram->descriptorBlock->commandList->physicalAddr + 16;

		for(index=0; index<numInputMoreBlocks; index++)
		{
			if( index == 0 )
			{
				/* Configure the first block */
				*program = 0x280F0000 | isoChannel->firstPacketSize;
				program++;
				*program = (UINT32)dataAddress;
				program++;
				*program = (UINT32)branchAddress | 1;
				program++;
				*program = isoChannel->firstPacketSize;
			}
			else if( index == numInputMoreBlocks - 1 )
			{
				*program = 0x283C0000 | lastBlockSize;
				program++;
				*program = (UINT32)dataAddress;
				program++;
				*program = 0;
				program++;
				*program = lastBlockSize;
			}
			else
			{
				*program = 0x280C0000 | maxPacketSize;
				program++;
				*program = (UINT32)dataAddress;
				program++;
				*program = (UINT32)branchAddress | 1;
				program++;
				*program = maxPacketSize;
			}

			branchAddress += 16;
			dataAddress += *program;
			program++;
		}

		/* Get the virtual address of the last input more descriptor block */
		contextProgram->lastBlock = (UINT32*)(contextProgram->descriptorBlock->commandList->virtualAddr + ((numInputMoreBlocks - 1) * 16));

		memcpy( contextProgram->descriptorBlock->commandList->virtualAddr,
				  contextProgram->descriptorBlock->program,
				  contextProgram->descriptorBlock->programSize );

		fwFlushDMAMemPool( contextProgram->descriptorBlock->commandList );

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

static FWStatus fwDestroyIsoContextProgram(FWIsoContextProgram *contextProgram)
{
	FWStatus retVal = FWInternalError;
	FWDmaMemoryPool *pDmaPool;
	FWDmaMemoryPool *pTempPool;
	FWDmaMemoryEntry *dmaMemEntry;

	if( contextProgram )
	{
		if( contextProgram->descriptorBlock )
		{
			if( contextProgram->descriptorBlock->program )
			{
				fwFree( contextProgram->descriptorBlock->program );
				contextProgram->descriptorBlock->program = NULL;
			}

			if( contextProgram->descriptorBlock->commandList )
			{
				fwFreeDMAMemPool( &contextProgram->dmaMemoryPool, contextProgram->descriptorBlock->commandList );
				contextProgram->descriptorBlock->commandList = NULL;
			}

			if( contextProgram->descriptorBlock->data )
			{
				fwFreeDMAMemPool( &contextProgram->dmaMemoryPool, contextProgram->descriptorBlock->data );
				contextProgram->descriptorBlock->data = NULL;
			}

			fwFree( contextProgram->descriptorBlock );
			contextProgram->descriptorBlock = NULL;
		}

		pDmaPool = contextProgram->dmaMemoryPool;
		while( pDmaPool )
		{
			pTempPool = pDmaPool->next;
			if( pDmaPool->startPtr )
				fwFreeDMA( pDmaPool->startPtr );
			fwFree( pDmaPool );
			pDmaPool = pTempPool;
		}

		contextProgram->dmaMemoryPool = NULL;

		fwFree( contextProgram );

		retVal = FWSuccess;
	}

	return retVal;
}

static FWStatus fwIsoEnableRecvChannel(FWDriverData *pDriver, FWIsoChannel *isoChannel)
{
	FWStatus retVal = FWInternalError;
	volatile OhciContextRecv *recvContext;
	UINT32 contextCtrl;
	UINT32 contextMatch;
	FWIsoContextProgram *contextProgram;

	do
	{
		/* Make sure the context is not running */
		recvContext = &pDriver->ohci->isoRecv[isoChannel->channel];

		contextCtrl = recvContext->contextControlClr;

		if( contextCtrl & FWContextCtrlRun )
		{
			retVal = FWIsoChannelBusy;
			break;
		}

		/* Reset the context control */
		recvContext->contextControlClr = 0xF8009000;

		/* Setup the match register */
		contextMatch = 0x10000000 | isoChannel->channel;
		contextMatch |= (isoChannel->sync & 0x000F) << 8;

		recvContext->contextMatch = contextMatch;

		/* Setup the context using the first context program. */
		contextProgram = isoChannel->contextList->head;

		if( contextProgram == NULL )
		{
			retVal = FWNotInitialized;
			break;
		}

		memcpy( contextProgram->descriptorBlock->commandList->virtualAddr,
				  contextProgram->descriptorBlock->program,
				  contextProgram->descriptorBlock->programSize );

		fwFlushDMAMemPool( contextProgram->descriptorBlock->commandList );

		recvContext->commandPtr = (UINT32)contextProgram->descriptorBlock->commandList->physicalAddr | 1;

		isoChannel->current = contextProgram;

		/* Enable the interrupt */
		pDriver->ohci->isoRecvIntMaskSet = 1 << isoChannel->channel;;

		/* Start the context running */
		recvContext->contextControlSet = 0x80008000;

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

static FWStatus fwIsoEnableSendChannel(FWDriverData *pDriver, FWIsoChannel *isoChannel)
{
	FWStatus retVal = FWInternalError;

	return retVal;
}

static FWStatus fwIsoDisableRecvChannel(FWDriverData *pDriver, FWIsoChannel *isoChannel)
{
	FWStatus retVal = FWInternalError;
	volatile OhciContextRecv *recvContext;
	UINT32 contextCtrl;
	UINT32 contextMatch;
	FWIsoContextProgram *contextProgram;
	int count = 0;
	UINT32 interrupt;

	/* Disable the interrupt */
	interrupt = 1 << isoChannel->channel;
	pDriver->ohci->isoRecvIntMaskClr = interrupt;

	/* Make sure the context is not running */
	recvContext = &pDriver->ohci->isoRecv[isoChannel->channel];

	recvContext->contextControlClr = FWContextCtrlRun;
	retVal = FWSuccess;

	do
	{
		taskDelay(1);
		contextCtrl = recvContext->contextControlClr;
		count++;

		if( count > 20 )
		{
			retVal = FWTimeout;
			break;
		}

	} while( contextCtrl & FWContextCtrlRun );

	if( retVal == FWSuccess )
	{
		isoChannel->current = NULL;
	}

	return retVal;
}

static FWStatus fwIsoDisableSendChannel(FWDriverData *pDriver, FWIsoChannel *isoChannel)
{
	FWStatus retVal = FWInternalError;

	return retVal;
}

static FWIsoContextList *fwIsoContextListCreate(void)
{
	FWIsoContextList * newList = NULL;

	newList = (FWIsoContextList*)fwMalloc(sizeof(FWIsoContextList));

	if( newList )
	{
		newList->head = newList->tail = NULL;
		newList->count = 0;
	}

	return newList;
}

static void fwIsoContextListDestroy(FWIsoContextList * list)
{
	FWIsoContextProgram *isoContextProgram;
	FWIsoContextProgram *temp;

	if( list )
	{
		isoContextProgram = fwIsoContextListPopFront( list );

		while( isoContextProgram )
		{
			fwDestroyIsoContextProgram( isoContextProgram );
			isoContextProgram = fwIsoContextListPopFront( list );
		}

		fwFree( list );
	}
}

static FWStatus fwIsoContextListPushBack(FWIsoContextList *list, FWIsoContextProgram *isoContextProgram)
{
	FWStatus retVal = FWInternalError;

	do
	{
		if( list == NULL ) break;
		if( isoContextProgram == NULL ) break;

		if( list->tail )
		{
			isoContextProgram->next = NULL;
			isoContextProgram->prev = list->tail;
			list->tail->next = isoContextProgram;
			list->tail = isoContextProgram;
		}
		else
		{
			isoContextProgram->next = NULL;
			isoContextProgram->prev = NULL;
			list->tail = isoContextProgram;
			list->head = isoContextProgram;
		}

		list->count++;

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

static FWStatus fwIsoContextListPushFront(FWIsoContextList *list, FWIsoContextProgram *isoContextProgram)
{
	FWStatus retVal = FWInternalError;

	do
	{
		if( list == NULL ) break;
		if( isoContextProgram == NULL ) break;

		if( list->head )
		{
			isoContextProgram->prev = NULL;
			isoContextProgram->next = list->head;
			list->head->prev = isoContextProgram;
			list->head = isoContextProgram;
		}
		else
		{
			isoContextProgram->next = NULL;
			isoContextProgram->prev = NULL;
			list->tail = isoContextProgram;
			list->head = isoContextProgram;
		}

		list->count++;

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

static FWIsoContextProgram *fwIsoContextListPopFront(FWIsoContextList *list)
{
	FWIsoContextProgram *isoContextProgram = NULL;

	do
	{
		if( list == NULL ) break;

		if( list->head )
		{
			isoContextProgram = list->head;
			list->head = list->head->next;

			if( list->head == NULL )
			{
				list->tail = NULL;
			}

			isoContextProgram->prev = isoContextProgram->next = NULL;

			list->count--;
		}

	} while(0);

	return isoContextProgram;
}

static FWIsoContextProgram *fwIsoContextListPopBack(FWIsoContextList *list)
{
	FWIsoContextProgram *isoContextProgram = NULL;

	do
	{
		if( list == NULL ) break;

		if( list->tail )
		{
			isoContextProgram = list->tail;
			list->tail = list->tail->prev;

			if( list->tail == NULL )
			{
				list->head = NULL;
			}

			isoContextProgram->prev = isoContextProgram->next = NULL;

			list->count--;
		}

	} while(0);

	return isoContextProgram;
}

static FWStatus fwIsoContextListUnlink(FWIsoContextList *list, FWIsoContextProgram *isoContextProgram)
{
	FWStatus retVal = FWInternalError;
	FWIsoContextProgram *pNext;
	FWIsoContextProgram *pPrev;

	pNext = isoContextProgram->next;
	pPrev = isoContextProgram->prev;

	if( pNext )
	{
		pNext->prev = pPrev;
	}
	else
	{
		list->tail = pPrev;
	}

	if( pPrev )
	{
		pPrev->next = pNext;
	}
	else
	{
		list->head = pNext;
	}

	list->count--;

	isoContextProgram->next = isoContextProgram->prev = NULL;

	retVal = FWSuccess;

	return retVal;
}

