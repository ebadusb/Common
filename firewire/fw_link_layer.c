/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header: H:/BCT_Development/vxWorks/Common/firewire/rcs/fw_link_layer.c 1.2 2007/02/12 16:07:00Z wms10235 Exp wms10235 $
 *
 * This file contains the firewire level routines to access
 * and manipulate the link layer.
 *
 * $Log: fw_link_layer.c $
 *
 */

#include <vxWorks.h>
#include <string.h>
#include <taskLib.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fw_utility.h"
#include "fw_dma_memory.h"
#include "fw_link_layer.h"
#include "fw_physical_layer.h"
#include "fw_driver.h"
#include "fw_ohci_hw.h"

/* Constants and enumerated types */
enum FWLinkLayerInternalConstants
{
	FWDescriptorCmdSize = 256,
	FWMaxDescriptorBlocks = 10,
	FWDescriptorDataSize = 2048
};

enum FWContextControlFlags
{
	FWContextCtrlActive = 0x00000400,
	FWContextCtrlDead = 0x00000800,
	FWContextCtrlWake = 0x00001000,
	FWContextCtrlRun = 0x00008000
};

typedef struct FWReadContextSupportStruct
{
	FWContextProgram	*pContext;
	volatile OhciContext *pContextCtrl;
	UINT32				*cmdDesc;
	UINT32				*dataPtr;
	FWDescriptorBlock	*pBlock;
	unsigned long		reqCount;
	unsigned long		resCount;

} FWReadContextSupport;

/* Local module prototypes. */
static FWStatus fwCreateLinkContext(FWDriverData *pDriver, FWContextProgram **context, int numDescriptorBlocks, unsigned long blockSize);
static FWStatus fwInitializeLinkContext(FWDriverData *pDriver, FWContextProgram *context);
static FWStatus fwSendAsyncRequest(FWDriverData *pDriver, FWTransaction *transaction);
static FWStatus fwSendAsyncResponse(FWDriverData *pDriver, FWTransaction *transaction);
static FWStatus fwInitializeAsyncRxRequestContext(FWDriverData *pDriver);
static FWStatus fwStartAsyncRxRequest(FWDriverData *pDriver);
static FWStatus fwStopAsyncRxRequest(FWDriverData *pDriver);
static FWStatus fwInitializeAsyncRxResponseContext(FWDriverData *pDriver);
static FWStatus fwStartAsyncRxResponse(FWDriverData *pDriver);
static FWStatus fwStopAsyncRxResponse(FWDriverData *pDriver);
static boolean  fwIsAsyncResponseBufferAvailable(FWDriverData *pDriver, unsigned long neededSize);
static FWStatus fwIncrementContextReadBuffer(FWReadContextSupport *pReadContext, UINT32 *dataValue);

/* API Implementation */
FWStatus fwCreateLinkLayer(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	unsigned long blockSize;

	if( pDriver )
	{
		if( pDriver->linkLayerData == NULL )
		{
			do
			{
				pDriver->linkLayerData = (FWLinkData*)fwMalloc( sizeof(FWLinkData) );

				if( pDriver->linkLayerData == NULL )
				{
					retVal = FWMemAllocateError;
					break;
				}

				/* initialize the link layer data */
				pDriver->linkLayerData->cycleCount = 0;
				pDriver->linkLayerData->initialized = FALSE;
				pDriver->linkLayerData->asyncRxRequestContext = NULL;
				pDriver->linkLayerData->asyncRxResponseContext = NULL;
				pDriver->linkLayerData->asyncTxRequestContext = NULL;
				pDriver->linkLayerData->asyncTxResponseContext = NULL;
				pDriver->linkLayerData->selfIdRecvMemory = NULL;
				pDriver->linkLayerData->stalled = FALSE;

				/* Allocate DMA memory for the self ID receive buffer */
				retVal = fwAllocateDMAMemPool( &pDriver->dmaMemoryPool, 4095, &pDriver->linkLayerData->selfIdRecvMemory );
				pDriver->linkLayerData->selfIdBufferPhysicalAddr = NULL;
				pDriver->linkLayerData->selfIdBufferVirtualAddr = NULL;
				if( retVal != FWSuccess ) break;

				/* compute the block size in bytes required for the receive contexts */
				blockSize = fwGetMaxPayload() + 20;

				retVal = fwCreateLinkContext( pDriver, &pDriver->linkLayerData->asyncRxRequestContext, 3, blockSize );
				if( retVal != FWSuccess ) break;

				retVal = fwCreateLinkContext( pDriver, &pDriver->linkLayerData->asyncRxResponseContext, 3, blockSize );
				if( retVal != FWSuccess ) break;

				/* compute the block size in bytes required for the transmit contexts */
				blockSize = fwGetMaxPayload();
				retVal = fwCreateLinkContext( pDriver, &pDriver->linkLayerData->asyncTxRequestContext, 1, blockSize );
				if( retVal != FWSuccess ) break;

				retVal = fwCreateLinkContext( pDriver, &pDriver->linkLayerData->asyncTxResponseContext, 1, blockSize );
				if( retVal != FWSuccess ) break;

			} while(0);
		}
		else
			retVal = FWSuccess;
	}

	return retVal;
}

FWStatus fwDestroyLinkLayer(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	FWIsoChannel *pChannel;
	FWIsoChannel *pTempChannel;
	FWDescriptorBlock *pBlock;
	FWDescriptorBlock *pTempBlock;

	if( pDriver )
	{
		if( pDriver->linkLayerData != NULL )
		{
			if( pDriver->linkLayerData->asyncRxRequestContext )
			{
				pBlock = pDriver->linkLayerData->asyncRxRequestContext->descriptorBlock;
				while( pBlock )
				{
					fwFreeDMAMemPool( &pDriver->dmaMemoryPool, pBlock->commandList );
					fwFreeDMAMemPool( &pDriver->dmaMemoryPool, pBlock->data );
					pTempBlock = pBlock;
					pBlock = pBlock->next;
					fwFree( pTempBlock );
				}

				if( pDriver->linkLayerData->asyncRxRequestContext->contextSem )
					semDelete( pDriver->linkLayerData->asyncRxRequestContext->contextSem );
				fwFree( pDriver->linkLayerData->asyncRxRequestContext );
			}

			if( pDriver->linkLayerData->asyncRxResponseContext )
			{
				pBlock = pDriver->linkLayerData->asyncRxResponseContext->descriptorBlock;
				while( pBlock )
				{
					fwFreeDMAMemPool( &pDriver->dmaMemoryPool, pBlock->commandList );
					fwFreeDMAMemPool( &pDriver->dmaMemoryPool, pBlock->data );
					pTempBlock = pBlock;
					pBlock = pBlock->next;
					fwFree( pTempBlock );
				}

				if( pDriver->linkLayerData->asyncRxResponseContext->contextSem )
					semDelete( pDriver->linkLayerData->asyncRxResponseContext->contextSem );
				fwFree( pDriver->linkLayerData->asyncRxResponseContext );
			}

			if( pDriver->linkLayerData->asyncTxRequestContext )
			{
				pBlock = pDriver->linkLayerData->asyncTxRequestContext->descriptorBlock;
				while( pBlock )
				{
					fwFreeDMAMemPool( &pDriver->dmaMemoryPool, pBlock->commandList );
					fwFreeDMAMemPool( &pDriver->dmaMemoryPool, pBlock->data );
					pTempBlock = pBlock;
					pBlock = pBlock->next;
					fwFree( pTempBlock );
				}

				if( pDriver->linkLayerData->asyncTxRequestContext->contextSem )
					semDelete( pDriver->linkLayerData->asyncTxRequestContext->contextSem );
				fwFree( pDriver->linkLayerData->asyncTxRequestContext );
			}

			if( pDriver->linkLayerData->asyncTxResponseContext )
			{
				pBlock = pDriver->linkLayerData->asyncTxResponseContext->descriptorBlock;
				while( pBlock )
				{
					fwFreeDMAMemPool( &pDriver->dmaMemoryPool, pBlock->commandList );
					fwFreeDMAMemPool( &pDriver->dmaMemoryPool, pBlock->data );
					pTempBlock = pBlock;
					pBlock = pBlock->next;
					fwFree( pTempBlock );
				}

				if( pDriver->linkLayerData->asyncTxResponseContext->contextSem )
					semDelete( pDriver->linkLayerData->asyncTxResponseContext->contextSem );
				fwFree( pDriver->linkLayerData->asyncTxResponseContext );
			}

			fwFreeDMAMemPool( &pDriver->dmaMemoryPool, pDriver->linkLayerData->selfIdRecvMemory );
			pDriver->linkLayerData->selfIdBufferPhysicalAddr = NULL;
			pDriver->linkLayerData->selfIdBufferVirtualAddr = NULL;

			fwFree( pDriver->linkLayerData );

			pDriver->linkLayerData = NULL;
		}

		retVal = FWSuccess;
	}

	return retVal;
}

FWStatus fwResetLink(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	do
	{
		/* Check for NULL on the pointers. */
		if( pDriver == NULL ) break;
		if( pDriver->linkLayerData == NULL ) break;
		if( pDriver->linkLayerData->asyncRxRequestContext == NULL ) break;
		if( pDriver->linkLayerData->asyncRxResponseContext == NULL ) break;
		if( pDriver->linkLayerData->asyncTxRequestContext == NULL ) break;
		if( pDriver->linkLayerData->asyncTxResponseContext == NULL ) break;
		if( pDriver->linkLayerData->selfIdRecvMemory == NULL ) break;

		pDriver->linkLayerData->initialized = FALSE;

		/* Stop the receive contexts */
		retVal = fwStopAsyncRxRequest( pDriver );
		if( retVal != FWSuccess )
			FWLOGLEVEL5("fwStopAsyncRxRequest() returned:%d\n", retVal );

		retVal = fwStopAsyncRxResponse( pDriver );
		if( retVal != FWSuccess )
			FWLOGLEVEL5("fwStopAsyncRxResponse() returned:%d\n", retVal );

		/* TODO: Stop the isochronous contexts */

		/* Set the hardware soft reset bit */
		pDriver->ohci->hcControlSet = 0x00010000;

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

FWStatus fwInitializeLink(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	UINT32 regVal;
	UINT32 hcControl;
	unsigned char *pAlign;
	unsigned int offset;

	do
	{
		/* Check for NULL on the pointers. */
		if( pDriver == NULL ) break;
		if( pDriver->linkLayerData == NULL ) break;

		pDriver->linkLayerData->initialized = FALSE;

		if( pDriver->linkLayerData->asyncRxRequestContext == NULL ) break;
		if( pDriver->linkLayerData->asyncRxResponseContext == NULL ) break;
		if( pDriver->linkLayerData->asyncTxRequestContext == NULL ) break;
		if( pDriver->linkLayerData->asyncTxResponseContext == NULL ) break;
		if( pDriver->linkLayerData->selfIdRecvMemory == NULL ) break;
		if( pDriver->ohci == NULL ) break;

		hcControl = pDriver->ohci->hcControlClr;

		/* Check if a reset is underway */
		if( hcControl & 0x00010000 )
		{
			retVal = FWResetUnderway;
			break;
		}

		/* Check if the link is enabled */
		if( hcControl & 0x00020000 )
		{
			retVal = FWLinkBusy;
			break;
		}

		/* Initialize OHCI register defaults */
		pDriver->ohci->hcControlClr = 0x60040000;

		/* Enable byte swapping */
		if( FW_BYTE_SWAP_ENABLED )
			pDriver->ohci->hcControlSet = 0x40000000;

		/* set defaults for bus management CSR registers */
		pDriver->ohci->initialBandwidthAvail = 4915;
		pDriver->ohci->initialChannelsAvailHi = 0xFFFFFFFE;
		pDriver->ohci->initialChannelsAvailLo = 0xFFFFFFFF;
		// this is set in the CSR module pDriver->ohci->configROMAddr = 0;

		/* Set the async request and physical filter to disable packets
		 * from being received from all other nodes.
		 */
		pDriver->ohci->asyncRequestFilterHiClr = 0xFFFFFFFF;
		pDriver->ohci->asyncRequestFilterLoClr = 0xFFFFFFFF;
		pDriver->ohci->phyRequestFilterHiClr = 0xFFFFFFFF;
		pDriver->ohci->phyRequestFilterLoClr = 0xFFFFFFFF;

		retVal = fwInitializeLinkContext( pDriver, pDriver->linkLayerData->asyncRxRequestContext );
		if( retVal != FWSuccess ) break;

		retVal = fwInitializeLinkContext( pDriver, pDriver->linkLayerData->asyncRxResponseContext );
		if( retVal != FWSuccess ) break;

		retVal = fwInitializeLinkContext( pDriver, pDriver->linkLayerData->asyncTxRequestContext );
		if( retVal != FWSuccess ) break;

		retVal = fwInitializeLinkContext( pDriver, pDriver->linkLayerData->asyncTxResponseContext );
		if( retVal != FWSuccess ) break;

		/* Initailize the selfID receive buffer. The buffer needs
		 * to be aligned on a physical memory 2K boundry.
		 */
		pAlign = pDriver->linkLayerData->selfIdRecvMemory->physicalAddr + 0x000007FF;
		offset = (unsigned int)pAlign & 0x000007FF;

		pDriver->linkLayerData->selfIdBufferPhysicalAddr = (UINT32*)(pAlign - offset);
		pDriver->linkLayerData->selfIdBufferVirtualAddr = fwPhysicalToVirtualAddress( pDriver->linkLayerData->selfIdBufferPhysicalAddr );

		/* The first register position in the buffer is
		 * must be setup so a false match is not possible.
		 */
		pDriver->linkLayerData->selfIdBufferVirtualAddr[0] = ~pDriver->ohci->selfIdCount;
		pDriver->ohci->selfIdBuffer = (UINT32)pDriver->linkLayerData->selfIdBufferPhysicalAddr;

		/* Set the physical upper bound */
		pDriver->ohci->phyUpperBound = 0;

		/* Initialize the fairness/priority budget control register */
		retVal = fwSetPriorityBudget(pDriver, 0);
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not set default priority budget. Error:%d\n", retVal );
			break;
		}

		retVal = fwSetMaxPriorityBudget(pDriver, 1);
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not set max priority budget. Error:%d\n", retVal );
			break;
		}

		retVal = fwSetCycleLimit(pDriver, 200);
		if( retVal != FWSuccess )
		{
			if( retVal == FWNotImplemented )
			{
				FWLOGLEVEL5("Dual phase rety not implemented. Error:%d\n", retVal );
			}
			else
			{
				FWLOGLEVEL3("Could not set cycle limit. Error:%d\n", retVal );
				break;
			}
		}

		/* Initialize the AT retries */
		retVal = fwSetAsyncRetryLimit(pDriver, 1);
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not set AT retry limit. Error:%d\n", retVal );
			break;
		}

		retVal = fwSetPhysicalRetryLimit(pDriver, 0);
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not set physical retry limit. Error:%d\n", retVal );
			break;
		}

		/* Initialize the link control register */
		pDriver->ohci->linkControlClr = 0x00700640;

		/* Config physical and link enhancements */
		regVal = pDriver->ohci->hcControlClr;

		if( regVal & 0x00800000 )
		{
			/* configure the phyical layer for 1394a operation */
			retVal = fwSetAcceleratedArbitration( pDriver, TRUE );

			if( retVal != FWSuccess )
			{
				FWLOGLEVEL3("Could not set accelerated arbitration. Error:%d\n", retVal );
				break;
			}

			retVal = fwSetEnableMultispeed( pDriver, TRUE );
			if( retVal != FWSuccess )
			{
				FWLOGLEVEL3("Could not enable multispeed. Error:%d\n", retVal );
				break;
			}

			/* Now enable 1394a enhancements on the Link hardware */
			pDriver->ohci->hcControlSet = 0x00400000;
		}
/* This is currently set in the config ROM
		if( pDriver->configROM->selectedConfig == 1 )
		{
			pDriver->ohci->busOptions = pDriver->configROM->config1VirtualAddr[2];
		}
		else if( pDriver->configROM->selectedConfig == 2 )
		{
			pDriver->ohci->busOptions = pDriver->configROM->config2VirtualAddr[2];
		}
*/
		/* Enable the cycle timer and allow self ID packets */
		pDriver->ohci->linkControlSet = 0x00100200;

		/* Initialize the async receive request context. */
		retVal = fwInitializeAsyncRxRequestContext( pDriver );
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not initialize the async receive request context. Error:%d\n", retVal );
			break;
		}

		/* Initialize the async receive request context. */
		retVal = fwInitializeAsyncRxResponseContext( pDriver );
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not initialize the async receive response context. Error:%d\n", retVal );
			break;
		}

		pDriver->linkLayerData->initialized = TRUE;

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

FWStatus fwEnableLink(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	UINT32 hcControl;

	do
	{
		if( pDriver == NULL ) break;
		if( pDriver->ohci == NULL ) break;
		if( pDriver->linkLayerData == NULL ) break;
		if( pDriver->linkLayerData->initialized == FALSE )
		{
			retVal = FWNotInitialized;
			break;
		}

      hcControl = pDriver->ohci->hcControlClr;

		/* Check if a reset is underway */
		if( hcControl & 0x00010000 )
		{
			retVal = FWResetUnderway;
			break;
		}

		/* Determine if the link is already enabled. */
		if( hcControl & 0x00020000 )
		{
			retVal = FWSuccess;
			break;
		}

		/* Enable the link */
		pDriver->ohci->hcControlSet = 0x00020000;

		/* Enable the receive contexts after the link is enabled. */
		retVal = fwStartAsyncRxRequest( pDriver );
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not start the async receive request context. Error:%d\n", retVal );
			break;
		}

		/* Enable the receive contexts after the link is enabled. */
		retVal = fwStartAsyncRxResponse( pDriver );
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not start the async receive response context. Error:%d\n", retVal );
			break;
		}

		/* Issue a bus reset. */
		retVal = fwInitiateBusReset( pDriver );

	} while(0);

	return retVal;
}

FWStatus fwSetCycleMaster(FWDriverData *pDriver, boolean flag)
{
	FWStatus retVal = FWInternalError;
	boolean rootStatus;

	if( pDriver )
	{
		if( pDriver->ohci )
		{
			retVal = fwGetRootNodeStatus( pDriver, &rootStatus );

			if( flag )
			{
				/* If the cycle master is being enabled,
				 * check if we are the root node before
				 * enabling cycle master */
				if( retVal == FWSuccess && rootStatus )
				{
					pDriver->ohci->linkControlSet = 0x00200000;
				}
			}
			else
			{
				pDriver->ohci->linkControlClr = 0x00200000;
			}
		}
	}

	return retVal;
}

FWStatus fwGetCycleMaster(FWDriverData *pDriver, boolean *flag)
{
	FWStatus retVal = FWInternalError;

	*flag = FALSE;

	if( pDriver )
	{
		if( pDriver->ohci )
		{
			if( pDriver->ohci->linkControlClr & 0x00200000 )
			{
				*flag = TRUE;
			}

			retVal = FWSuccess;
		}
	}

	return retVal;
}

FWStatus fwSetCycleTime(FWDriverData *pDriver, UINT32 cycleTime)
{
	FWStatus retVal = FWInternalError;

	if( pDriver )
	{
		if( pDriver->ohci )
		{
			pDriver->ohci->isoCycleTimer = cycleTime;
			retVal = FWSuccess;
		}
	}

	return retVal;
}

FWStatus fwGetCycleTime(FWDriverData *pDriver, UINT32 *cycleTime)
{
	FWStatus retVal = FWInternalError;

	*cycleTime = 0;

	if( pDriver )
	{
		if( pDriver->ohci )
		{
			*cycleTime = pDriver->ohci->isoCycleTimer;
			retVal = FWSuccess;
		}
	}

	return retVal;
}

FWStatus fwSendPhysicalPacket(FWDriverData *pDriver, UINT32 physPacket)
{
	FWStatus retVal = FWInternalError;
	UINT32 contextCtrl;
	UINT32 *cmdDesc;
	FWDescriptorBlock *pBlock;

	/* Start critical section */
	semTake( pDriver->linkLayerData->asyncTxRequestContext->contextSem, WAIT_FOREVER );

	pBlock = pDriver->linkLayerData->asyncTxRequestContext->descriptorBlock;

	/* Find an avaliable descriptor */
	while( pBlock )
	{
		if( pBlock->inuse == FALSE )
			break;

		pBlock = pBlock->next;
	}

	/* End critial section */
	semGive( pDriver->linkLayerData->asyncTxRequestContext->contextSem );

	if( pBlock )
	{
		/* Create an async quadlet write request command descriptor */
		cmdDesc = (UINT32*)pBlock->commandList->virtualAddr;

		cmdDesc[0] = 0x123C000C;
		cmdDesc[1] = 0;
		cmdDesc[2] = 0;
		cmdDesc[3] = 0;

		/* Fill in quadlets 4, 5 and 6 */
		cmdDesc[4] = 0x000000E0;
		cmdDesc[5] = physPacket;
		cmdDesc[6] = ~physPacket;
		cmdDesc[7] = 0;

		pBlock->length = 0;
		pBlock->transactionID = 0;
		pBlock->numCommands = 2;

		/* Flush DMA memory before using in the driver */
		fwFlushDMAMemPool( pBlock->commandList );

		/* Critical section */
		semTake( pDriver->linkLayerData->asyncTxRequestContext->contextSem, WAIT_FOREVER );

		/* Determine if the transaction can be accepted by the link layer */
		contextCtrl = pDriver->ohci->asyncTxRequest.contextControlClr;
		if((contextCtrl & FWContextCtrlActive) == 0 && (contextCtrl & FWContextCtrlRun) == 0 )
		{
			/* Setup and run the async request DMA context program */
			pDriver->ohci->asyncTxRequest.commandPtr = (UINT32)pBlock->commandList->physicalAddr;
			pDriver->ohci->asyncTxRequest.commandPtr |= pBlock->numCommands;
			pDriver->ohci->asyncTxRequest.contextControlSet = FWContextCtrlRun;
			pBlock->inuse = TRUE;
			retVal = FWSuccess;
		}
		else
		{
			/* Abandon the block and return link busy status */
			FWLOGLEVEL9("Async TX request still active for PHY send. Context control reg:0x%08X\n", contextCtrl );
			retVal = FWLinkBusy;
			pBlock->inuse = FALSE;
		}

		semGive( pDriver->linkLayerData->asyncTxRequestContext->contextSem );
	}

	return retVal;
}

FWStatus fwSendAsyncMessage(FWDriverData *pDriver, FWTransaction *transaction)
{
	FWStatus retVal = FWInternalError;
	unsigned short sourceID;
	unsigned long receiveSpace;

	do
	{
		if( pDriver == NULL ) break;
		if( pDriver->ohci == NULL ) break;
		if( pDriver->physicalLayerData == NULL ) break;
		if( pDriver->linkLayerData == NULL ) break;

		if( transaction == NULL )
		{
			retVal = FWInvalidArg;
			break;
		}

		/* Check if the link layer is enabled */
		if( pDriver->linkLayerData->initialized == FALSE || (pDriver->ohci->hcControlClr & 0x00020000) == 0 )
		{
			retVal = FWNotInitialized;
			break;
		}

		/* Determine if the self ID process has completed and we have a valid node ID */
		retVal = fwGetSourceID( pDriver, &sourceID );
		if( retVal != FWSuccess )
		{
			break;
		}

		/* Check the bus generation in the transaction */
		if( transaction->busGeneration != pDriver->physicalLayerData->busGeneration )
		{
			retVal = FWBusGenerationError;
			break;
		}

		/* Determine what context is appropriate for this transaction */
		switch( transaction->transactionCode )
		{
		case FWWriteRequestQuadlet:
			/* Determine if there is space in the async RX response DMA context
			 * to receive the response before sending the request. */
			if( fwIsAsyncResponseBufferAvailable( pDriver, 16 ) )
			{
				retVal = fwSendAsyncRequest( pDriver, transaction );
			}
			else
			{
				pDriver->linkLayerData->stalled = TRUE;
				retVal = FWLinkBusy;
			}
			break;

		case FWWriteRequestBlock:
			/* Determine if there is space in the async RX response DMA context
			 * to receive the response before sending the request. */
			if( fwIsAsyncResponseBufferAvailable( pDriver, 16 ) )
			{
				retVal = fwSendAsyncRequest( pDriver, transaction );
			}
			else
			{
				pDriver->linkLayerData->stalled = TRUE;
				retVal = FWLinkBusy;
			}
			break;

		case FWReadRequestQuadlet:
			/* Determine if there is space in the async RX response DMA context
			 * to receive the response before sending the request. */
			if( fwIsAsyncResponseBufferAvailable( pDriver, 20 ) )
			{
				retVal = fwSendAsyncRequest( pDriver, transaction );
			}
			else
			{
				FWLOGLEVEL9("Async quadlet read request stalled.\n");
				pDriver->linkLayerData->stalled = TRUE;
				retVal = FWLinkBusy;
			}
			break;

		case FWReadRequestBlock:
			/* Determine if there is space in the async RX response DMA context
			 * to receive the response before sending the request. */
			if( fwIsAsyncResponseBufferAvailable( pDriver, 20 + ((transaction->dataLength + 3) & ~3)))
			{
				retVal = fwSendAsyncRequest( pDriver, transaction );
			}
			else
			{
				pDriver->linkLayerData->stalled = TRUE;
				retVal = FWLinkBusy;
			}
			break;

		case FWLockRequest:
			/* Determine if there is space in the async RX response DMA context
			 * to receive the response before sending the request. */
			if( fwIsAsyncResponseBufferAvailable( pDriver, 20 + ((transaction->dataLength + 3) & ~3)))
			{
				retVal = fwSendAsyncRequest( pDriver, transaction );
			}
			else
			{
				pDriver->linkLayerData->stalled = TRUE;
				retVal = FWLinkBusy;
			}
			break;

		case FWReadResponseQuadlet:
		case FWReadResponseBlock:
		case FWWriteResponse:
		case FWLockResponse:
			retVal = fwSendAsyncResponse( pDriver, transaction );
			break;

		case FWCycleStart:
		case FWIsoDataBlock:
			retVal = FWInvalidTransactionType;
			break;

		default:
			retVal = FWInvalidTransactionType;
		}

	} while(0);

	return retVal;
}

FWStatus fwAsyncTxRequestComplete(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	volatile FWDescriptorBlock *pBlock;
	UINT32 contextCtrl;

	if( pDriver )
	{
		contextCtrl = pDriver->ohci->asyncTxRequest.contextControlClr;

		if( contextCtrl & FWContextCtrlActive )
		{
			FWLOGLEVEL3("fwAsyncTxRequestComplete() called while context still active.\n");
		}
		else
		{
			/* Start critical section */
			semTake( pDriver->linkLayerData->asyncTxRequestContext->contextSem, WAIT_FOREVER );

			/* Clear the Run, wake, and dead bits in the context */
			pDriver->ohci->asyncTxRequest.contextControlClr = 0x00009800;

			pBlock = pDriver->linkLayerData->asyncTxRequestContext->descriptorBlock;

			while( pBlock )
			{
				pBlock->inuse = FALSE;
				pBlock = pBlock->next;
			}

			/* End critical section */
			semGive( pDriver->linkLayerData->asyncTxRequestContext->contextSem );

			if( contextCtrl & FWContextCtrlDead )
			{
				FWLOGLEVEL3("The async TX request context dead bit is set.\n");
				retVal = FWContextError;
			}
			else
				retVal = FWSuccess;
		}
	}

	return retVal;
}

FWStatus fwAsyncTxResponseComplete(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	volatile FWDescriptorBlock *pBlock;
	UINT32 contextCtrl;

	if( pDriver )
	{
		contextCtrl = pDriver->ohci->asyncTxResponse.contextControlClr;

		if( contextCtrl & FWContextCtrlActive )
		{
			FWLOGLEVEL3("fwAsyncTxResponseComplete() called while context still active.\n");
		}
		else
		{
			/* Start critical section */
			semTake( pDriver->linkLayerData->asyncTxResponseContext->contextSem, WAIT_FOREVER );

			/* Clear the Run, wake, and dead bits in the context */
			pDriver->ohci->asyncTxResponse.contextControlClr = 0x00009800;

			pBlock = pDriver->linkLayerData->asyncTxResponseContext->descriptorBlock;

			while( pBlock )
			{
				pBlock->inuse = FALSE;
				pBlock = pBlock->next;
			}

			/* End critical section */
			semGive( pDriver->linkLayerData->asyncTxResponseContext->contextSem );

			if( contextCtrl & FWContextCtrlDead )
			{
				FWLOGLEVEL3("The async TX response context dead bit is set.\n");
				retVal = FWContextError;
			}
			else
				retVal = FWSuccess;
		}
	}

	return retVal;
}

FWStatus fwCancelAsyncMessage(FWDriverData *pDriver, FWTransaction *transaction)
{
	FWStatus retVal = FWInternalError;
	FWDescriptorBlock *pBlock = NULL;
	int selectedContext = 0;
	UINT32 contextCtrl;

	if( pDriver )
	{
		if( pDriver->ohci )
		{
			if( pDriver->linkLayerData )
			{
				if( transaction->transactionID != 0 )
				{
					retVal = FWNotFound;

					/* Determine what context is appropriate for this transaction */
					switch( transaction->transactionCode )
					{
					case FWWriteRequestQuadlet:
					case FWWriteRequestBlock:
					case FWReadRequestQuadlet:
					case FWReadRequestBlock:
					case FWLockRequest:
						pBlock = pDriver->linkLayerData->asyncTxRequestContext->descriptorBlock;
						selectedContext = 1;
						break;

					case FWReadResponseQuadlet:
					case FWReadResponseBlock:
					case FWWriteResponse:
					case FWLockResponse:
						pBlock = pDriver->linkLayerData->asyncTxResponseContext->descriptorBlock;
						selectedContext = 2;
						break;

					case FWCycleStart:
					case FWIsoDataBlock:
					default:
						retVal = FWInvalidTransactionType;
					}

					/* Search for the transaction */
					while( pBlock )
					{
						if( pBlock->transactionID == transaction->transactionID && pBlock->transactionID != 0 )
							break;
						else
							pBlock = pBlock->next;
					}

					if( pBlock )
					{
						/* Found the transaction */
						if( selectedContext == 1 )
						{
							/* Critical section */
							semTake( pDriver->linkLayerData->asyncTxRequestContext->contextSem, WAIT_FOREVER );

							/* Determine if the transaction can be accepted by the link layer */
							contextCtrl = pDriver->ohci->asyncTxRequest.contextControlClr;
							if((contextCtrl & FWContextCtrlActive) != 0 && (contextCtrl & FWContextCtrlRun) != 0 )
							{
								/* Stop the async request DMA context */
								pDriver->ohci->asyncTxRequest.contextControlClr = FWContextCtrlRun;
								pBlock->inuse = FALSE;
								retVal = FWSuccess;
							}

							semGive( pDriver->linkLayerData->asyncTxRequestContext->contextSem );
						}
						else if( selectedContext == 2 )
						{
							/* Critical section */
							semTake( pDriver->linkLayerData->asyncTxResponseContext->contextSem, WAIT_FOREVER );

							/* Determine if the transaction can be accepted by the link layer */
							contextCtrl = pDriver->ohci->asyncTxResponse.contextControlClr;
							if((contextCtrl & FWContextCtrlActive) != 0 && (contextCtrl & FWContextCtrlRun) != 0 )
							{
								/* Stop the async response DMA context */
								pDriver->ohci->asyncTxResponse.contextControlClr = FWContextCtrlRun;
								pBlock->inuse = FALSE;
								retVal = FWSuccess;
							}

							semGive( pDriver->linkLayerData->asyncTxResponseContext->contextSem );
						}
					}
				}
			}
		}
	}

	return retVal;
}

FWStatus fwGetGUID(FWDriverData *pDriver, FWGUID *adapterGuid)
{
	FWStatus retVal = FWInvalidArg;

	if( adapterGuid )
	{
		retVal = FWNotInitialized;
		adapterGuid->guidHi = 0;
		adapterGuid->guidLow = 0;

		if( pDriver )
		{
			if( pDriver->ohci )
			{
				adapterGuid->guidHi = pDriver->ohci->guiIdHi;
				adapterGuid->guidLow = pDriver->ohci->guiIdLo;
				retVal = FWSuccess;
			}
		}
	}

	return retVal;
}

FWStatus fwGetNodeID(FWDriverData *pDriver, unsigned short *busNumber, unsigned char *physNodeNumber)
{
	FWStatus retVal = FWInternalError;
	UINT32 nodeInfo;

	*busNumber = 0x3FF;
	*physNodeNumber = 0x3F;

	if( pDriver )
	{
		if( pDriver->ohci )
		{
			nodeInfo = pDriver->ohci->nodeId;

			if( nodeInfo & 0x80000000 )
			{
				nodeInfo &= 0x0000FFFF;
				*physNodeNumber = (unsigned short)(nodeInfo & 0x0000003F);
				*busNumber = (unsigned short)(nodeInfo >> 6);

				retVal = FWSuccess;
			}
			else
			{
				retVal = FWInvalidNodeAddr;
			}
		}
	}

	return retVal;
}

FWStatus fwGetSourceID(FWDriverData *pDriver, unsigned short *sourceID)
{
	FWStatus retVal = FWInternalError;
	UINT32 nodeInfo;

	*sourceID = 0xFFFF;

	if( pDriver )
	{
		if( pDriver->ohci )
		{
			nodeInfo = pDriver->ohci->nodeId;

			if( nodeInfo & 0x80000000 )
			{
				*sourceID = (unsigned short)(nodeInfo & 0x0000FFFF);

				retVal = FWSuccess;
			}
			else
			{
				retVal = FWInvalidNodeAddr;
			}
		}
	}

	return retVal;
}

FWStatus fwSetBusNumber(FWDriverData *pDriver, unsigned short busNumber)
{
	FWStatus retVal = FWInternalError;
	UINT32 nodeInfo;

	if( pDriver )
	{
		if( pDriver->ohci )
		{
			nodeInfo = busNumber;
			pDriver->ohci->nodeId = nodeInfo << 6;

			retVal = FWSuccess;
		}
	}

	return retVal;
}

FWStatus fwSetSplitTransTimeout(FWDriverData *pDriver, int seconds, int fraction)
{
	FWStatus retVal = FWInternalError;

	if( pDriver )
	{
		if( pDriver->linkLayerData )
		{
			pDriver->linkLayerData->splitTransTimeoutSec = seconds & 0x07;
			pDriver->linkLayerData->splitTransTimeoutFrac = fraction & 0x1FFF;

			retVal = FWSuccess;
		}
	}

	return retVal;
}

FWStatus fwSetAsyncRetryLimit(FWDriverData *pDriver, int maxATRetries)
{
	FWStatus retVal = FWInternalError;
	UINT32 atRetries;

	if( pDriver )
	{
		if( pDriver->ohci )
		{
			if( maxATRetries >= 0 && maxATRetries < 16 )
			{
				atRetries = pDriver->ohci->asyncTxRetries & 0xFFFFFF00;

				/* Set the max AT response and request retry */
				atRetries |= maxATRetries | (maxATRetries << 4);

				pDriver->ohci->asyncTxRetries = atRetries;

				retVal = FWSuccess;
			}
			else
			{
				retVal = FWBoundsError;
			}
		}
	}

	return retVal;
}

FWStatus fwGetAsyncRetryLimit(FWDriverData *pDriver, int *maxATRetries)
{
	FWStatus retVal = FWInternalError;

	*maxATRetries = 0;

	if( pDriver )
	{
		if( pDriver->ohci )
		{
			*maxATRetries = pDriver->ohci->asyncTxRetries & 0x0000000F;
			retVal = FWSuccess;
		}
	}

	return retVal;
}

FWStatus fwSetPhysicalRetryLimit(FWDriverData *pDriver, int maxPhyRespRetries)
{
	FWStatus retVal = FWInternalError;
	UINT32 phyRetries;

	if( pDriver )
	{
		if( pDriver->ohci )
		{
			if( maxPhyRespRetries >= 0 && maxPhyRespRetries < 16 )
			{
				phyRetries = pDriver->ohci->asyncTxRetries & 0xFFFFF0FF;

				phyRetries |= maxPhyRespRetries << 8;

				pDriver->ohci->asyncTxRetries = phyRetries;

				retVal = FWSuccess;
			}
			else
			{
				retVal = FWBoundsError;
			}
		}
	}

	return retVal;
}

FWStatus fwGetPhysicalRetryLimit(FWDriverData *pDriver, int *maxPhyRespRetries)
{
	FWStatus retVal = FWInternalError;
	UINT32 phyRetries;

	*maxPhyRespRetries = 0;

	if( pDriver )
	{
		if( pDriver->ohci )
		{
			phyRetries = pDriver->ohci->asyncTxRetries & 0x00000F00;
			phyRetries = phyRetries >> 8;
			*maxPhyRespRetries = (int)phyRetries;

			retVal = FWSuccess;
		}
	}

	return retVal;
}

FWStatus fwSetCycleLimit(FWDriverData *pDriver, unsigned short cycleLimit)
{
	FWStatus retVal = FWInternalError;
	UINT32 atRetries;
	UINT32 testCmp;

	/* Read the ATRetries register (if the cycle limit register cannot be set,
	 * then dual phase retry is not be implemented. */
	if( pDriver )
	{
		if( pDriver->ohci )
		{
			atRetries = pDriver->ohci->asyncTxRetries & 0x0000FFFF;
			atRetries |= (UINT32)cycleLimit << 16;

			pDriver->ohci->asyncTxRetries = atRetries;

			/* Read back the register to determine if dual phase retry is implemented */
			testCmp = pDriver->ohci->asyncTxRetries;

			if( testCmp != atRetries )
			{
				retVal = FWNotImplemented;
			}
			else
			{
				retVal = FWSuccess;
			}
		}
	}

	return retVal;
}

FWStatus fwGetCycleLimit(FWDriverData *pDriver, unsigned short *cycleLimit)
{
	FWStatus retVal = FWInternalError;
	UINT32 atRetries;

	*cycleLimit = 0;

	if( pDriver )
	{
		if( pDriver->ohci )
		{
			atRetries = pDriver->ohci->asyncTxRetries;
			*cycleLimit = (unsigned short)(atRetries >> 16);

			retVal = FWSuccess;
		}
	}

	return retVal;
}

FWStatus fwSetBuiltinCSR(FWDriverData *pDriver, UINT32 compare, UINT32 *swap, UINT32 control)
{
	FWStatus retVal = FWInternalError;

	if( pDriver )
	{
		if( pDriver->ohci )
		{
			if( control >= 0 && control < 5 )
			{
				pDriver->ohci->csrData = *swap;
				pDriver->ohci->csrCompare = compare;
				pDriver->ohci->csrControl = control;

				while(( pDriver->ohci->csrControl & 0x80000000 ) == 0 )
				{
					taskDelay(0);
				}

				*swap = pDriver->ohci->csrData;

				retVal = FWSuccess;
			}
			else
				retVal = FWBoundsError;
		}
	}

	return retVal;
}

FWStatus fwGetBuiltinCSR(FWDriverData *pDriver, UINT32 *value, UINT32 control)
{
	FWStatus retVal = FWInternalError;

	if( pDriver )
	{
		if( pDriver->ohci )
		{
			if( control >= 0 && control < 5 )
			{
				pDriver->ohci->csrData = 0;
				pDriver->ohci->csrCompare = 0;
				pDriver->ohci->csrControl = control;

				while((pDriver->ohci->csrControl & 0x80000000) == 0 )
				{
					taskDelay(0);
				}

				*value = pDriver->ohci->csrData;
				retVal = FWSuccess;
			}
			else
				retVal = FWBoundsError;
		}
	}

	return retVal;
}

FWStatus fwSetPriorityBudget(FWDriverData *pDriver, unsigned char priority)
{
	FWStatus retVal = FWInternalError;

	if( pDriver )
	{
		if( pDriver->coreCSR )
		{
			if( priority <= pDriver->coreCSR->maxPriorityBudget )
			{
				pDriver->ohci->fairnessControl = (UINT32)priority;
				retVal = FWSuccess;
			}
			else
			{
				retVal = FWBoundsError;
			}
		}
	}

	return retVal;
}

FWStatus fwGetPriorityBudget(FWDriverData *pDriver, unsigned char *priority)
{
	FWStatus retVal = FWInternalError;

	if( pDriver )
	{
		if( pDriver->coreCSR )
		{
			*priority = (unsigned char)(pDriver->ohci->fairnessControl & 0x3F);
			retVal = FWSuccess;
		}
	}

	return retVal;
}

FWStatus fwSetMaxPriorityBudget(FWDriverData *pDriver, unsigned char maxPriorityBudget)
{
	FWStatus retVal = FWInternalError;

	if( pDriver )
	{
		if( pDriver->coreCSR )
		{
			if( maxPriorityBudget < 64 )
			{
				pDriver->coreCSR->maxPriorityBudget = maxPriorityBudget;
				retVal = FWSuccess;
			}
			else
			{
				retVal = FWBoundsError;
			}
		}
	}

	return retVal;
}

FWStatus fwGetMaxPriorityBudget(FWDriverData *pDriver, unsigned char *maxPriorityBudget)
{
	FWStatus retVal = FWInternalError;

	*maxPriorityBudget = 0;

	if( pDriver )
	{
		if( pDriver->coreCSR )
		{
			*maxPriorityBudget = pDriver->coreCSR->maxPriorityBudget;
			retVal = FWSuccess;
		}
	}

	return retVal;
}

/*
 * Local function implementation.
 */

static FWStatus fwCreateLinkContext(FWDriverData *pDriver, FWContextProgram **context, int numDescriptorBlocks, unsigned long blockSize)
{
	FWStatus retVal = FWInternalError;
	FWDmaMemoryEntry *dmaMem;
	FWDescriptorBlock	*descBlock = NULL;
	FWDescriptorBlock *prevBlock = NULL;
	int index;

	*context = (FWContextProgram*)fwMalloc( sizeof(FWContextProgram) );

	if( *context )
	{
		(*context)->contextSem = semMCreate( SEM_Q_PRIORITY | SEM_INVERSION_SAFE );

		if( (*context)->contextSem != NULL )
		{
			(*context)->descriptorBlock = NULL;
			(*context)->active = FALSE;

			if( numDescriptorBlocks <= 0 ) numDescriptorBlocks = 1;
			if( numDescriptorBlocks > FWMaxDescriptorBlocks ) numDescriptorBlocks = FWMaxDescriptorBlocks;

			for(index=0; index<numDescriptorBlocks; index++)
			{
				descBlock = (FWDescriptorBlock*)fwMalloc( sizeof(FWDescriptorBlock) );
				if( descBlock == NULL )
				{
					retVal = FWMemAllocateError;
					break;
				}

				if( index == 0 )
					(*context)->descriptorBlock = descBlock;

				descBlock->inuse = FALSE;
				descBlock->length = 0;
				descBlock->dataSize = 0;
				descBlock->commandList = NULL;
				descBlock->data = NULL;
				descBlock->next = NULL;
				descBlock->prev = NULL;
				descBlock->numCommands = 0;

				/* allocate DMA memory for the command descriptors */
				retVal = fwAllocateDMAMemPool( &pDriver->dmaMemoryPool, FWDescriptorCmdSize, &dmaMem );
				if( retVal != FWSuccess )
				{
					break;
				}

				descBlock->commandList = dmaMem;

				/* allocate DMA memory for the data area */
				retVal = fwAllocateDMAMemPool( &pDriver->dmaMemoryPool, blockSize, &dmaMem );
				if( retVal != FWSuccess )
				{
					fwFreeDMAMemPool( &pDriver->dmaMemoryPool, descBlock->commandList );
					descBlock->commandList = NULL;
					break;
				}

				descBlock->dataSize = blockSize;

				descBlock->data = dmaMem;

				descBlock->prev = prevBlock;
				if( prevBlock )
					prevBlock->next = descBlock;

				prevBlock = descBlock;
			}

			retVal = FWSuccess;
		}
		else
		{
			fwFree( *context );
			*context = NULL;
			retVal = FWSemAllocateError;
		}
	}
	else
		retVal = FWMemAllocateError;


	return retVal;
}

static FWStatus fwInitializeLinkContext(FWDriverData *pDriver, FWContextProgram *context)
{
	FWStatus retVal = FWInternalError;
	FWDescriptorBlock *pBlock;

	do
	{
		if( context == NULL ) break;

		context->active = FALSE;
		pBlock = context->descriptorBlock;

		/* determine if a descriptor block is allocated */
		while( pBlock )
		{
			pBlock->length = 0;
			pBlock->numCommands = 0;
			pBlock->inuse = FALSE;

			pBlock = pBlock->next;
		}

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

static FWStatus fwSendAsyncRequest(FWDriverData *pDriver, FWTransaction *transaction)
{
	FWStatus retVal = FWInternalError;
	UINT32 contextCtrl;
	UINT32 *cmdDesc;
	volatile FWDescriptorBlock *pBlock;

	pBlock = pDriver->linkLayerData->asyncTxRequestContext->descriptorBlock;

	/* Find an avaliable descriptor */
	while( pBlock )
	{
		if( pBlock->inuse == FALSE )
		{
			/* Critical section - reserve the descriptor block */
			semTake( pDriver->linkLayerData->asyncTxRequestContext->contextSem, WAIT_FOREVER );
			if( pBlock->inuse == FALSE )
			{
				pBlock->inuse = TRUE;
			}
			/* End critical section */
			semGive( pDriver->linkLayerData->asyncTxRequestContext->contextSem );

			if( pBlock->inuse == TRUE )
				break;
		}

		pBlock = pBlock->next;
	}

	if( pBlock )
	{
		/* Create an async quadlet write request command descriptor */
		cmdDesc = (UINT32*)pBlock->commandList->virtualAddr;

		cmdDesc[1] = 0;
		cmdDesc[2] = 0;
		cmdDesc[3] = 0;

		/* Fill in quadlets 4, 5 and 6 */
		cmdDesc[4] = 0x00800100;
		cmdDesc[4] |= ((UINT32)transaction->transactionLabel & 0x0000003F) << 10;
		cmdDesc[4] |= ((UINT32)transaction->transactionCode & 0x0000000F ) << 4;

		if( transaction->speed == FWS200 )
			cmdDesc[4] |= 0x00010000;
		else if( transaction->speed == FWS400 )
			cmdDesc[4] |= 0x00020000;

		cmdDesc[5] = (UINT32)transaction->destinationID << 16;
		cmdDesc[5] |= transaction->destinationOffset.highOffset;
		cmdDesc[6] = transaction->destinationOffset.lowOffset;

		pBlock->length = 0;
		pBlock->numCommands = 0;
		pBlock->transactionID = 0;

		/* Fill in the first descriptor */
		switch( transaction->transactionCode )
		{
		/* Write request quadlet uses a single output last immediate command */
		case FWWriteRequestQuadlet:
			cmdDesc[0] = 0x123C0010;
			if( transaction->dataLength == 4 )
			{
				cmdDesc[7] = *((UINT32*)transaction->databuffer);
				pBlock->length = 0;
				pBlock->numCommands = 2;
				retVal = FWSuccess;
			}
			else
				retVal = FWTransDataSizeError;
			break;

		/* Write block request uses a single output immediate command
		 * followed by a output last command.
		 */
		case FWWriteRequestBlock:
			cmdDesc[0] = 0x02000010;
			if( transaction->dataLength > 0 && transaction->dataLength <= pBlock->dataSize )
			{
				cmdDesc[7] = (UINT32)transaction->dataLength << 16;
				cmdDesc[8] = 0x103C0000 | transaction->dataLength;
				memcpy( pBlock->data->virtualAddr, transaction->databuffer, transaction->dataLength );
				cmdDesc[9] = (UINT32)pBlock->data->physicalAddr;
				cmdDesc[10] = 0;
				cmdDesc[11] = 0;

				pBlock->length = transaction->dataLength;
				pBlock->numCommands = 3;
				retVal = FWSuccess;
			}
			else
				retVal = FWTransDataSizeError;

			break;

		/* Read request quadlet uses a single output last immediate command */
		case FWReadRequestQuadlet:
			cmdDesc[0] = 0x123C000C;
			pBlock->length = 0;
			pBlock->numCommands = 2;
			retVal = FWSuccess;
			break;

		/* Read request block uses a single output last immediate command */
		case FWReadRequestBlock:
			cmdDesc[0] = 0x123C0010;
			/* In this case, dataLength holds the number of bytes being requested
			 * and not the size of the data in the buffer.
			 */
			if( transaction->dataLength > 0 && transaction->dataLength <= pBlock->dataSize )
			{
				cmdDesc[7] = (UINT32)transaction->dataLength << 16;
				pBlock->length = 0;
				pBlock->numCommands = 2;
				retVal = FWSuccess;
			}
			else
				retVal = FWTransDataSizeError;
			break;

		/* Lock request uses a single output immediate command
		 * followed by a output last command.
		 */
		case FWLockRequest:
			cmdDesc[0] = 0x02000010;
			if( transaction->dataLength > 3 && transaction->dataLength <= 32 )
			{
				cmdDesc[7] = (UINT32)transaction->dataLength << 16;
				cmdDesc[7] |= (UINT32)transaction->extendedCode & 0x0000FFFF;
				cmdDesc[8] = 0x103C0000 | transaction->dataLength;
				memcpy( pBlock->data->virtualAddr, transaction->databuffer, transaction->dataLength );
				cmdDesc[9] = (UINT32)pBlock->data->physicalAddr;
				cmdDesc[10] = 0;
				cmdDesc[11] = 0;

				pBlock->length = transaction->dataLength;
				pBlock->numCommands = 3;
				retVal = FWSuccess;
			}
			else
				retVal = FWTransDataSizeError;

			break;

		case FWReadResponseQuadlet:
		case FWReadResponseBlock:
		case FWWriteResponse:
		case FWLockResponse:
		case FWCycleStart:
		case FWIsoDataBlock:
		case FWPhysicalPacket:
		case FWInvalidTransCode:
			retVal = FWInvalidTransactionType;
			break;
		}

		/* Check if the command descriptor was constructed successfully */
		if( retVal == FWSuccess )
		{
			/* Flush DMA memory before using in the driver */
			fwFlushDMAMemPool( pBlock->commandList );
			fwFlushDMAMemPool( pBlock->data );

			/* Critical section */
			semTake( pDriver->linkLayerData->asyncTxRequestContext->contextSem, WAIT_FOREVER );

			/* Determine if the transaction can be accepted by the link layer */
			contextCtrl = pDriver->ohci->asyncTxRequest.contextControlClr;
			if((contextCtrl & FWContextCtrlActive) == 0 && (contextCtrl & FWContextCtrlRun) == 0 )
			{
				/* Setup and run the async request DMA context */
				pDriver->ohci->asyncTxRequest.commandPtr = (UINT32)pBlock->commandList->physicalAddr;
				pDriver->ohci->asyncTxRequest.commandPtr |= pBlock->numCommands;
				pDriver->ohci->asyncTxRequest.contextControlSet = FWContextCtrlRun;
				pBlock->transactionID = transaction->transactionID;
			}
			else
			{
				/* Abandon the block and return link busy status */
				FWLOGLEVEL9("Async TX request still active. Context control reg:0x%08X\n", contextCtrl );
				pBlock->inuse = FALSE;
				retVal = FWLinkBusy;
			}

			/* End critical section */
			semGive( pDriver->linkLayerData->asyncTxRequestContext->contextSem );
		}
		else
			pBlock->inuse = FALSE;
	}
	else
	{
		retVal = FWLinkBusy;
	}

	return retVal;
}

static FWStatus fwSendAsyncResponse(FWDriverData *pDriver, FWTransaction *transaction)
{
	FWStatus retVal = FWInternalError;
	UINT32 contextCtrl;
	UINT32 *cmdDesc;
	volatile FWDescriptorBlock *pBlock;
	UINT32 transTimeStamp;
	UINT32 splitTimeoutSec;
	UINT32 splitTimeoutCount;

	pBlock = pDriver->linkLayerData->asyncTxResponseContext->descriptorBlock;

	/* Find an avaliable descriptor */
	while( pBlock )
	{
		if( pBlock->inuse == FALSE )
		{
			/* Critical section - reserve the descriptor block */
			semTake( pDriver->linkLayerData->asyncTxResponseContext->contextSem, WAIT_FOREVER );
			if( pBlock->inuse == FALSE )
			{
				pBlock->inuse = TRUE;
			}
			/* End critical section */
			semGive( pDriver->linkLayerData->asyncTxResponseContext->contextSem );

			if( pBlock->inuse == TRUE )
				break;
		}

		pBlock = pBlock->next;
	}

	if( pBlock )
	{
		/* Compute the split transaction timeout value */
		splitTimeoutSec = pDriver->linkLayerData->splitTransTimeoutSec;
		splitTimeoutCount = pDriver->linkLayerData->splitTransTimeoutFrac;

		transTimeStamp = transaction->timeStamp;

		splitTimeoutSec += transTimeStamp >> 13;
		splitTimeoutCount += transTimeStamp & 0x00001FFF;
		splitTimeoutSec += splitTimeoutCount / 8000;
		splitTimeoutCount %= 8000;
		transTimeStamp = ((splitTimeoutSec & 0x00000007) << 13) | splitTimeoutCount;

		/* Create an async quadlet write response command descriptor */
		cmdDesc = (UINT32*)pBlock->commandList->virtualAddr;

		cmdDesc[1] = 0;
		cmdDesc[2] = 0;
		cmdDesc[3] = transTimeStamp;

		/* Fill in quadlets 4, 5 and 6 */
		cmdDesc[4] = 0x00800100;
		cmdDesc[4] |= ((UINT32)transaction->transactionLabel & 0x0000003F) << 10;
		cmdDesc[4] |= ((UINT32)transaction->transactionCode & 0x0000000F ) << 4;

		if( transaction->speed == FWS200 )
			cmdDesc[4] |= 0x00010000;
		else if( transaction->speed == FWS400 )
			cmdDesc[4] |= 0x00020000;

		cmdDesc[5] = (UINT32)transaction->destinationID << 16;
		cmdDesc[5] |= (UINT32)transaction->responseCode << 12;
		cmdDesc[6] = 0;

		pBlock->length = 0;
		pBlock->numCommands = 0;
		pBlock->transactionID = 0;

		/* Fill in the first descriptor */
		switch( transaction->transactionCode )
		{
		/* Read response quadlet uses a single output last immediate command */
		case FWReadResponseQuadlet:
			cmdDesc[0] = 0x123C0010;
			if( transaction->dataLength <= 4 && transaction->dataBufferSize >= 4 )
			{
				cmdDesc[7] = *((UINT32*)transaction->databuffer);
				pBlock->length = 0;
				pBlock->numCommands = 2;
				retVal = FWSuccess;
			}
			else
				retVal = FWTransDataSizeError;
			break;

		/* Read block response uses an output immediate command
		 * followed by a output last command.
		 */
		case FWReadResponseBlock:
			cmdDesc[0] = 0x02000010;
			if( transaction->dataLength <= pBlock->dataSize )
			{
				cmdDesc[7] = (UINT32)transaction->dataLength << 16;
				cmdDesc[8] = 0x103C0000 | transaction->dataLength;
				memcpy( pBlock->data->virtualAddr, transaction->databuffer, transaction->dataLength );
				cmdDesc[9] = (UINT32)pBlock->data->physicalAddr;
				cmdDesc[10] = 0;
				cmdDesc[11] = 0;

				pBlock->length = transaction->dataLength;
				pBlock->numCommands = 3;
				retVal = FWSuccess;
			}
			else
				retVal = FWTransDataSizeError;

			break;

		/* Write response quadlet uses a single output last immediate command */
		case FWWriteResponse:
			cmdDesc[0] = 0x123C000C;
			pBlock->length = 0;
			pBlock->numCommands = 2;
			retVal = FWSuccess;
			break;

		/* Lock response uses a single output immediate command
		 * followed by a output last command.
		 */
		case FWLockResponse:
			cmdDesc[0] = 0x02000010;
			if( transaction->dataLength <= 8 )
			{
				cmdDesc[7] = (UINT32)transaction->dataLength << 16;
				cmdDesc[7] |= (UINT32)transaction->extendedCode & 0x0000FFFF;
				cmdDesc[8] = 0x103C0000 | transaction->dataLength;
				memcpy( pBlock->data->virtualAddr, transaction->databuffer, transaction->dataLength );
				cmdDesc[9] = (UINT32)pBlock->data->physicalAddr;
				cmdDesc[10] = 0;
				cmdDesc[11] = 0;

				pBlock->length = transaction->dataLength;
				pBlock->numCommands = 3;
				retVal = FWSuccess;
			}
			else
				retVal = FWTransDataSizeError;

			break;

		case FWWriteRequestQuadlet:
		case FWWriteRequestBlock:
		case FWReadRequestQuadlet:
		case FWReadRequestBlock:
		case FWLockRequest:
		case FWCycleStart:
		case FWIsoDataBlock:
		case FWPhysicalPacket:
		case FWInvalidTransCode:
			retVal = FWInvalidTransactionType;
			break;
		}

		/* Check if the command descriptor was constructed successfully */
		if( retVal == FWSuccess )
		{
			/* Flush DMA memory before using in the driver */
			fwFlushDMAMemPool( pBlock->commandList );
			fwFlushDMAMemPool( pBlock->data );

			/* Critical section */
			semTake( pDriver->linkLayerData->asyncTxResponseContext->contextSem, WAIT_FOREVER );

			/* Determine if the transaction can be accepted by the link layer */
			contextCtrl = pDriver->ohci->asyncTxResponse.contextControlClr;
			if((contextCtrl & FWContextCtrlActive) == 0 && (contextCtrl & FWContextCtrlRun) == 0 )
			{
				/* Setup and run the async response DMA context */
				pDriver->ohci->asyncTxResponse.commandPtr = (UINT32)pBlock->commandList->physicalAddr;
				pDriver->ohci->asyncTxResponse.commandPtr |= pBlock->numCommands;
				pDriver->ohci->asyncTxResponse.contextControlSet = FWContextCtrlRun;
				pBlock->transactionID = transaction->transactionID;
			}
			else
			{
				/* Abandon the block and return link busy status */
				FWLOGLEVEL9("Async TX response still active. Context control reg:0x%08X\n", contextCtrl );
				retVal = FWLinkBusy;
				pBlock->inuse = FALSE;
			}

			/* End critical section */
			semGive( pDriver->linkLayerData->asyncTxResponseContext->contextSem );
		}
		else
			pBlock->inuse = FALSE;
	}
	else
	{
		retVal = FWLinkBusy;
		FWLOGLEVEL5("No block avaiable for link layer response context.\n");
	}

	return retVal;
}

/*
 * fwReadAsyncRequestMessage()
 *
 * This function reads a transaction from the request asynchronous
 * receive DMA context.
 *
 * Returns:
 *    FWSuccess - Transaction read
 *    FWNotFound - No messages are in the read buffer
 *    FWInternalError - Bad pointer value
 *    FWTransDataSizeError - The context appears corrupt
 *
 */
FWStatus fwReadAsyncRequestMessage(FWDriverData *pDriver, FWTransaction *transaction)
{
	FWStatus retVal = FWInternalError;
	boolean semLocked = FALSE;
	FWReadContextSupport readContext;
	UINT32 branchAddr;
	UINT32 transactionCode;
	UINT32 dataLength;
	UINT32 value;
	UINT32 dataValue;
	UINT32 firstPhysPacket = 0;
	UINT32 secondPhysPacket = 0;
	UINT32 *dataBuffer;
	unsigned int i, j;
	unsigned char* readBuf;
	unsigned char* writeBuf;

	/* Initialize the read context structure */
	readContext.pBlock = NULL;
	readContext.dataPtr = NULL;
	readContext.cmdDesc = NULL;

	do
	{
		if( pDriver == NULL ) break;
		if( pDriver->linkLayerData == NULL ) break;
		if( pDriver->linkLayerData->asyncRxRequestContext == NULL ) break;
		if( pDriver->ohci == NULL ) break;

		readContext.pContext = pDriver->linkLayerData->asyncRxRequestContext;
		readContext.pContextCtrl = &pDriver->ohci->asyncRxRequest;

		/* Critical section */
		semTake( readContext.pContext->contextSem, WAIT_FOREVER );
		semLocked = TRUE;

		/* Get the first block */
		readContext.pBlock = readContext.pContext->firstBlock;

		/* Check for a valid block */
		if( readContext.pBlock == NULL )
		{
			retVal = FWNotInitialized;
			break;
		}

		/* Read the transaction data (if any) */

		/* Invalidate the memory to make sure we are reading current info */
		fwInvalidateDMAMemPool( readContext.pBlock->commandList );

		readContext.cmdDesc = (UINT32*)readContext.pBlock->commandList->virtualAddr;

		/* The reqCount is how much data the buffer will hold. The
		 * resCount is remaining bytes available in the buffer.
		 */
		readContext.reqCount = readContext.cmdDesc[0] & 0x0000FFFF;
		readContext.resCount = readContext.cmdDesc[3] & 0x0000FFFF;

		/* perform a quick sanity check */
		if( readContext.reqCount < readContext.resCount )
		{
			/* Error total count cannot be less than the remaining! */
			retVal = FWTransDataSizeError;
			break;
		}

		/* pBlock->length represents how much data was read from the buffer.
		 * length acts as an index into the data area and tracks where the
		 * last transaction left off.
		 */
		if( readContext.pBlock->length < readContext.reqCount - readContext.resCount )
		{
			/* Invalidate the memory to make sure we are reading current data */
			fwInvalidateDMAMemPool( readContext.pBlock->data );

			readContext.dataPtr = (UINT32*)(readContext.pBlock->data->virtualAddr + readContext.pBlock->length);

			if( (retVal = fwIncrementContextReadBuffer( &readContext, &dataValue )) != FWSuccess ) break;

			transactionCode = (dataValue & 0x000000F0) >> 4;

			/* Decode the first quadlet */
			transaction->transactionCode = (FWTransactionCode)transactionCode;
			transaction->destinationID = dataValue >> 16;
			transaction->transactionLabel = (unsigned char)((dataValue & 0x0000FC00) >> 10);
			transaction->clientHandle = 0;
			transaction->retryCode = (dataValue & 0x00000300) >> 8;
			transaction->dataLength = 0;
			transaction->extendedCode = FWInvalidExtendedCode;

			if( (retVal = fwIncrementContextReadBuffer( &readContext, &dataValue )) != FWSuccess ) break;

			/* Decode the second quadlet */
			transaction->destinationOffset.highOffset = dataValue & 0x0000FFFF;
			transaction->sourceID = (unsigned short)((dataValue >> 16) & 0x0000FFFF);
			if( transactionCode == FWPhysicalPacket )
			{
				firstPhysPacket = dataValue;
			}

			if( (retVal = fwIncrementContextReadBuffer( &readContext, &dataValue )) != FWSuccess ) break;

			/* Third quadlet */
			transaction->destinationOffset.lowOffset = dataValue;
			if( transactionCode == FWPhysicalPacket )
			{
				secondPhysPacket = dataValue;
			}

			if( (retVal = fwIncrementContextReadBuffer( &readContext, &dataValue )) != FWSuccess ) break;

			/* Determine packet's transaction type */
			switch( transactionCode )
			{
			case FWWriteRequestQuadlet:
				if( transaction->dataBufferSize >= 4 )
				{
					*((UINT32*)transaction->databuffer) = dataValue;
					transaction->dataLength = 4;
				}
				else
				{
					if( transaction->databuffer )
						fwFree( transaction->databuffer );

					transaction->dataBufferSize = 0;
					transaction->databuffer = (unsigned char*)fwMalloc( 4 );
					if( transaction->databuffer )
					{
						transaction->dataBufferSize = 4;
						transaction->dataLength = 4;
						*((UINT32*)transaction->databuffer) = dataValue;
					}
				}

				retVal = fwIncrementContextReadBuffer( &readContext, &dataValue );
				break;

			case FWWriteRequestBlock:
				/* Get the data length of the block and round it up to the nearest quadlet */
				dataLength = dataValue >> 16;
				value = (dataLength + 3) & ~3;
				if( dataLength > transaction->dataBufferSize )
				{
					if( transaction->databuffer )
						fwFree( transaction->databuffer );

					transaction->dataBufferSize = 0;
					transaction->databuffer = (unsigned char*)fwMalloc( value );
					if( transaction->databuffer )
					{
						transaction->dataBufferSize = value;
						transaction->dataLength = dataLength;
					}
				}
				else
					transaction->dataLength = dataLength;

				value /= 4;

				if( (retVal = fwIncrementContextReadBuffer( &readContext, &dataValue )) != FWSuccess ) break;

				dataBuffer = (UINT32*)transaction->databuffer;

				/* Read the quadlet data area. Note that we need to read
				 * all the data even if there is not enough transaction
				 * buffer space to accommodate the data.
				 */
				for(i=0; i<value; i++)
				{
					/* Move the data in quadlets if possible */
					if( transaction->dataBufferSize >= (i + 1) * 4 )
					{
						*dataBuffer = dataValue;
						dataBuffer++;
					}
					else if( transaction->dataBufferSize > i * 4 )
					{
						readBuf = (unsigned char*)&dataValue;
						writeBuf = (unsigned char*)dataBuffer;

						for(j=0; j<transaction->dataBufferSize - i * 4; j++)
						{
							*writeBuf = *readBuf;
							writeBuf++;
							readBuf++;
						}
					}

					if( (retVal = fwIncrementContextReadBuffer( &readContext, &dataValue )) != FWSuccess ) break;
				}
				break;

			case FWReadRequestQuadlet:
				/* The quadlet read request doesn't require any data
				 * and the size is always 4 bytes.
				 */
				transaction->dataLength = 4;
				break;

			case FWReadRequestBlock:
				transaction->dataLength = dataValue >> 16;
				retVal = fwIncrementContextReadBuffer( &readContext, &dataValue );
				break;

			case FWLockRequest:
				/* Get the data length of the block and round it up to the nearest quadlet */
				dataLength = dataValue >> 16;
				value = (FWExtendedCode)(dataValue & 0x0000FFFF);

				switch( value )
				{
				case FWMaskSwap:
					transaction->extendedCode = FWMaskSwap;
					break;
				case FWCompareSwap:
					transaction->extendedCode = FWCompareSwap;
					break;
				case FWFetchAdd:
					transaction->extendedCode = FWFetchAdd;
					break;
				case FWLittleAdd:
					transaction->extendedCode = FWLittleAdd;
					break;
				case FWBoundedAdd:
					transaction->extendedCode = FWBoundedAdd;
					break;
				case FWWrapAdd:
					transaction->extendedCode = FWWrapAdd;
					break;
				case FWVendorDependent:
					transaction->extendedCode = FWVendorDependent;
					break;
				case FWInvalidExtendedCode:
				default:
					transaction->extendedCode = FWInvalidExtendedCode;
					break;
				}

				value = (dataLength + 3) & ~3;
				if( dataLength > transaction->dataBufferSize )
				{
					if( transaction->databuffer )
						fwFree( transaction->databuffer );

					transaction->dataBufferSize = 0;
					transaction->databuffer = (unsigned char*)fwMalloc( value );
					if( transaction->databuffer )
					{
						transaction->dataBufferSize = value;
						transaction->dataLength = dataLength;
					}
				}
				else
					transaction->dataLength = dataLength;

				value /= 4;

				if( (retVal = fwIncrementContextReadBuffer( &readContext, &dataValue )) != FWSuccess ) break;

				dataBuffer = (UINT32*)transaction->databuffer;

				/* Read the quadlet data area. Note that we need to read
				 * all the data even if there is not enough transaction
				 * buffer space to accommodate the data.
				 */
				for(i=0; i<value; i++)
				{
					/* Move the data in quadlets if possible */
					if( transaction->dataBufferSize >= (i + 1) * 4 )
					{
						*dataBuffer = dataValue;
						dataBuffer++;
					}
					else if( transaction->dataBufferSize > i * 4 )
					{
						readBuf = (unsigned char*)&dataValue;
						writeBuf = (unsigned char*)dataBuffer;

						for(j=0; j<transaction->dataBufferSize - i * 4; j++)
						{
							*writeBuf = *readBuf;
							writeBuf++;
							readBuf++;
						}
					}

					if( (retVal = fwIncrementContextReadBuffer( &readContext, &dataValue )) != FWSuccess ) break;
				}
				break;

			case FWPhysicalPacket:
				transaction->destinationID = 0xFFFF;
				transaction->transactionLabel = 0xFF;
				transaction->destinationOffset.highOffset = 0xFFFF;
				transaction->destinationOffset.lowOffset = 0xFFFFFFFF;
				if( transaction->dataBufferSize < 8 )
				{
					if( transaction->databuffer )
						fwFree( transaction->databuffer );

					transaction->dataBufferSize = 0;
					transaction->databuffer = (unsigned char*)fwMalloc( 8 );
					if( transaction->databuffer )
					{
						transaction->dataBufferSize = 8;
						transaction->dataLength = 8;
					}
				}
				else
					transaction->dataLength = 8;

				dataBuffer = (UINT32*)transaction->databuffer;
				dataBuffer[0] = firstPhysPacket;
				dataBuffer[1] = secondPhysPacket;
				break;

			/* These transaction codes are not valid in this context */
			case FWWriteResponse:
			case FWReadResponseQuadlet:
			case FWReadResponseBlock:
			case FWCycleStart:
			case FWIsoDataBlock:
			case FWLockResponse:
			case FWInvalidTransCode:
			default:
				transaction->transactionCode = FWInvalidTransCode;
				retVal = FWInvalidTransactionType;
				break;
			}

			/* Check for an error in the switch statement */
			if( retVal != FWSuccess ) break;

			transaction->timeStamp = (unsigned short)(dataValue & 0x0000FFFF);

			transaction->speed = FWSInvalid;
			value = ( dataValue & 0x00E00000 ) >> 21;

			transaction->speed = FWS100;
			if( value == FWS400 )
				transaction->speed = FWS400;
			else if( value == FWS200 )
				transaction->speed = FWS200;

			transaction->xferStatus = (unsigned short)(dataValue >> 16);
		}
		else
		{
			retVal = FWNotFound;
		}

	} while(0);

	if( semLocked )
		semGive( readContext.pContext->contextSem );

	return retVal;
}

/*
 * fwReadAsyncResponseMessage()
 *
 * This function reads a transaction from the response asynchronous
 * receive DMA context.
 *
 * Returns:
 *    FWSuccess - Transaction read
 *    FWNotFound - No messages are in the read buffer
 *    FWInternalError - Bad pointer value
 *    FWTransDataSizeError - The context appears corrupt
 *
 */
FWStatus fwReadAsyncResponseMessage(FWDriverData *pDriver, FWTransaction *transaction)
{
	FWStatus retVal = FWInternalError;
	boolean semLocked = FALSE;
	FWReadContextSupport readContext;
	UINT32 branchAddr;
	UINT32 transactionCode;
	UINT32 dataLength;
	UINT32 value;
	UINT32 dataValue;
	UINT32 firstPhysPacket;
	UINT32 secondPhysPacket;
	UINT32 *dataBuffer;
	unsigned int i, j;
	unsigned char* readBuf;
	unsigned char* writeBuf;

	/* Initialize the read context structure */
	readContext.pBlock = NULL;
	readContext.dataPtr = NULL;
	readContext.cmdDesc = NULL;

	do
	{
		if( pDriver == NULL ) break;
		if( pDriver->linkLayerData == NULL ) break;
		if( pDriver->linkLayerData->asyncRxRequestContext == NULL ) break;
		if( pDriver->ohci == NULL ) break;

		readContext.pContext = pDriver->linkLayerData->asyncRxResponseContext;
		readContext.pContextCtrl = &pDriver->ohci->asyncRxResponse;

		/* Critical section */
		semTake( readContext.pContext->contextSem, WAIT_FOREVER );
		semLocked = TRUE;

		/* Get the first block */
		readContext.pBlock = readContext.pContext->firstBlock;

		/* Check for a valid block */
		if( readContext.pBlock == NULL )
		{
			/* End critical section */
			retVal = FWNotInitialized;
			break;
		}

		/* Read the transaction data (if any) */

		/* Invalidate the memory to make sure we are reading current info */
		fwInvalidateDMAMemPool( readContext.pBlock->commandList );

		readContext.cmdDesc = (UINT32*)readContext.pBlock->commandList->virtualAddr;

		/* The reqCount is how much data the block will hold. The
		 * resCount is remaining bytes available in the buffer.
		 */
		readContext.reqCount = readContext.cmdDesc[0] & 0x0000FFFF;
		readContext.resCount = readContext.cmdDesc[3] & 0x0000FFFF;

		/* perform a quick sanity check */
		if( readContext.reqCount < readContext.resCount )
		{
			/* Error total count cannot be less than the remaining! */
			FWLOGLEVEL3("Warning request count:%d < resCount:%d\n", readContext.reqCount, readContext.resCount );
			retVal = FWTransDataSizeError;
			break;
		}

		/* pBlock->length represents how much data was read from the buffer.
		 * length acts as an index into the data area and tracks where the
		 * last transaction left off.
		 */
		if( readContext.pBlock->length < readContext.reqCount - readContext.resCount )
		{
			/* Invalidate the memory to make sure we are reading current data */
			fwInvalidateDMAMemPool( readContext.pBlock->data );

			readContext.dataPtr = (UINT32*)(readContext.pBlock->data->virtualAddr + readContext.pBlock->length);

			if( (retVal = fwIncrementContextReadBuffer( &readContext, &dataValue )) != FWSuccess ) break;

			transactionCode = (dataValue & 0x000000F0) >> 4;

			/* Decode the first quadlet */
			transaction->transactionCode = (FWTransactionCode)transactionCode;
			transaction->transactionLabel = (unsigned char)((dataValue & 0x0000FC00) >> 10);
			transaction->destinationID = (unsigned short)((dataValue >> 16) & 0x0000FFFF);
			transaction->clientHandle = 0;
			transaction->retryCode = (dataValue & 0x00000300) >> 8;
			transaction->dataLength = 0;
			transaction->extendedCode = FWInvalidExtendedCode;
			transaction->xferStatus = 0;

			if( (retVal = fwIncrementContextReadBuffer( &readContext, &dataValue )) != FWSuccess ) break;

			/* Decode the second quadlet */
			value  = ( dataValue & 0x0000F000 ) >> 12;

			switch( value )
			{
			case FWResponseComplete:
				transaction->responseCode = FWResponseComplete;
				break;
			case FWResponseConflictError:
				transaction->responseCode = FWResponseConflictError;
				break;
			case FWResponseDataError:
				transaction->responseCode = FWResponseDataError;
				break;
			case FWResponseTypeError:
				transaction->responseCode = FWResponseTypeError;
				break;
			case FWResponseAddressError:
				transaction->responseCode = FWResponseAddressError;
				break;
			case FWInvalidResponseCode:
			default:
				transaction->responseCode = FWInvalidResponseCode;
				break;
			}

			transaction->sourceID = (unsigned short)((dataValue >> 16) & 0x0000FFFF);

			if( (retVal = fwIncrementContextReadBuffer( &readContext, &dataValue )) != FWSuccess ) break;

			/* Third quadlet is not used */

			if( (retVal = fwIncrementContextReadBuffer( &readContext, &dataValue )) != FWSuccess ) break;

			/* Determine packet's transaction type */
			switch( transactionCode )
			{
			case FWWriteResponse:
				/* no data in the write response */
				break;

			case FWReadResponseQuadlet:
				if( transaction->dataBufferSize >= 4 )
				{
					*((UINT32*)transaction->databuffer) = dataValue;
					transaction->dataLength = 4;
				}
				else
				{
					if( transaction->databuffer )
						fwFree( transaction->databuffer );

					transaction->dataBufferSize = 0;
					transaction->databuffer = (unsigned char*)fwMalloc( 4 );
					if( transaction->databuffer )
					{
						transaction->dataBufferSize = 4;
						transaction->dataLength = 4;
						*((UINT32*)transaction->databuffer) = dataValue;
					}
				}

				retVal = fwIncrementContextReadBuffer( &readContext, &dataValue );
				break;

			case FWReadResponseBlock:
				/* Get the data length of the block and round it up to the nearest quadlet */
				dataLength = dataValue >> 16;
				value = (dataLength + 3) & ~3;
				if( dataLength > transaction->dataBufferSize )
				{
					if( transaction->databuffer )
						fwFree( transaction->databuffer );

					transaction->dataBufferSize = 0;
					transaction->databuffer = (unsigned char*)fwMalloc( value );
					if( transaction->databuffer )
					{
						transaction->dataBufferSize = value;
						transaction->dataLength = dataLength;
					}
				}
				else
					transaction->dataLength = dataLength;

				value /= 4;

				if( (retVal = fwIncrementContextReadBuffer( &readContext, &dataValue )) != FWSuccess ) break;

				dataBuffer = (UINT32*)transaction->databuffer;

				/* Read the quadlet data area. Note that we need to read
				 * all the data even if there is not enough transaction
				 * buffer space to accommodate the data.
				 */
				for(i=0; i<value; i++)
				{
					/* Move the data in quadlets if possible */
					if( transaction->dataBufferSize >= (i + 1) * 4 )
					{
						*dataBuffer = dataValue;
						dataBuffer++;
					}
					else if( transaction->dataBufferSize > i * 4 )
					{
						readBuf = (unsigned char*)&dataValue;
						writeBuf = (unsigned char*)dataBuffer;

						for(j=0; j<transaction->dataBufferSize - i * 4; j++)
						{
							*writeBuf = *readBuf;
							writeBuf++;
							readBuf++;
						}
					}

					if( (retVal = fwIncrementContextReadBuffer( &readContext, &dataValue )) != FWSuccess ) break;
				}
				break;

			case FWLockResponse:
				/* Get the data length of the block and round it up to the nearest quadlet */
				dataLength = dataValue >> 16;
				value = (FWExtendedCode)(dataValue & 0x0000FFFF);

				switch( value )
				{
				case FWMaskSwap:
					transaction->extendedCode = FWMaskSwap;
					break;
				case FWCompareSwap:
					transaction->extendedCode = FWCompareSwap;
					break;
				case FWFetchAdd:
					transaction->extendedCode = FWFetchAdd;
					break;
				case FWLittleAdd:
					transaction->extendedCode = FWLittleAdd;
					break;
				case FWBoundedAdd:
					transaction->extendedCode = FWBoundedAdd;
					break;
				case FWWrapAdd:
					transaction->extendedCode = FWWrapAdd;
					break;
				case FWVendorDependent:
					transaction->extendedCode = FWVendorDependent;
					break;
				case FWInvalidExtendedCode:
				default:
					transaction->extendedCode = FWInvalidExtendedCode;
					break;
				}

				value = (dataLength + 3) & ~3;
				if( dataLength > transaction->dataBufferSize )
				{
					if( transaction->databuffer )
						fwFree( transaction->databuffer );

					transaction->dataBufferSize = 0;
					transaction->databuffer = (unsigned char*)fwMalloc( value );
					if( transaction->databuffer )
					{
						transaction->dataBufferSize = value;
						transaction->dataLength = dataLength;
					}
				}
				else
					transaction->dataLength = dataLength;

				value /= 4;

				if( (retVal = fwIncrementContextReadBuffer( &readContext, &dataValue )) != FWSuccess ) break;

				dataBuffer = (UINT32*)transaction->databuffer;

				/* Read the quadlet data area. Note that we need to read
				 * all the data even if there is not enough transaction
				 * buffer space to accommodate the data.
				 */
				for(i=0; i<value; i++)
				{
					/* Move the data in quadlets if possible */
					if( transaction->dataBufferSize >= (i + 1) * 4 )
					{
						*dataBuffer = dataValue;
						dataBuffer++;
					}
					else if( transaction->dataBufferSize > i * 4 )
					{
						readBuf = (unsigned char*)&dataValue;
						writeBuf = (unsigned char*)dataBuffer;

						for(j=0; j<transaction->dataBufferSize - i * 4; j++)
						{
							*writeBuf = *readBuf;
							writeBuf++;
							readBuf++;
						}
					}

					if( (retVal = fwIncrementContextReadBuffer( &readContext, &dataValue )) != FWSuccess ) break;
				}
				break;

			/* These transaction codes are not valid in this context */
			case FWCycleStart:
			case FWIsoDataBlock:
			case FWWriteRequestQuadlet:
			case FWWriteRequestBlock:
			case FWReadRequestQuadlet:
			case FWReadRequestBlock:
			case FWLockRequest:
			case FWPhysicalPacket:
			case FWInvalidTransCode:
			default:
				transaction->transactionCode = FWInvalidTransCode;
				retVal = FWInvalidTransactionType;
				break;
			}

			/* Check for an error in the switch statement */
			if( retVal != FWSuccess ) break;

			transaction->timeStamp = (unsigned short)(dataValue & 0x0000FFFF);

			transaction->speed = FWSInvalid;
			value = ( dataValue & 0x00E00000 ) >> 21;

			if( value == FWS400 )
				transaction->speed = FWS400;
			else if( value == FWS200 )
				transaction->speed = FWS200;
			else if( value == FWS100 )
				transaction->speed = FWS100;

			transaction->xferStatus = (unsigned short)(dataValue >> 16);
		}
		else
		{
			retVal = FWNotFound;
		}

	} while(0);

	if( semLocked )
		semGive( readContext.pContext->contextSem );

	return retVal;
}

FWStatus fwEnableAsyncRequests(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	do
	{
		if( pDriver == NULL ) break;
		if( pDriver->linkLayerData == NULL ) break;
		if( pDriver->ohci == NULL ) break;

		if( pDriver->linkLayerData->initialized == FALSE )
		{
			retVal = FWNotInitialized;
			break;
		}

		pDriver->ohci->asyncRequestFilterHiSet = 0x7FFFFFFF;
		pDriver->ohci->asyncRequestFilterLoSet = 0xFFFFFFFF;

	} while(0);

	return retVal;
}

/*
 * fwIncrementContextReadBuffer()
 *
 * This function increments the context read buffer by one quadlet. If the end
 * of the buffer is reached, the routine moves to next buffer in the chain. If
 * the end of data is reached, the buffer pointer is not advanced.
 *
 * Returns:
 *    FWSuccess - data is valid
 *    FWNoMoreData - the buffer is empty
 *    FWInternalError - Bad pointer value
 *    FWDescriptorNotAvailable - no descriptor blocks available (fatal)
 *
 */
static FWStatus fwIncrementContextReadBuffer(FWReadContextSupport *pReadContext, UINT32 *dataValue)
{
	FWStatus retVal = FWInternalError;
	FWDescriptorBlock *pNextBlock = NULL;
	FWDescriptorBlock *pLastBlock = NULL;
	FWDescriptorBlock *pIndexBlock = NULL;
	UINT32 branchAddr;
	UINT32 *cmdDesc;

	if( pReadContext )
	{
		/* Check if we have read all the data from this buffer or if
		 * there is a count problem.
		 */
		if( pReadContext->pBlock->length >= pReadContext->reqCount - pReadContext->resCount )
		{
			/* If resCount is zero move to the next data block.
			 * Otherwise, we are out of data.
			 */
			if( pReadContext->resCount == 0 )
			{
				/* Determine the address of the next block */
				branchAddr = pReadContext->cmdDesc[2] & 0xFFFFFFF0;

				pIndexBlock = pReadContext->pContext->descriptorBlock;

				while( pIndexBlock )
				{
					if( (UINT32)(pIndexBlock->commandList->physicalAddr) == branchAddr )
					{
						pNextBlock = pIndexBlock;
					}

					fwInvalidateDMAMemPool( pIndexBlock->commandList );
					cmdDesc = (UINT32*)pIndexBlock->commandList->virtualAddr;

					/* Check for the last block and don't self link */
					if( cmdDesc[2] == 0 && pReadContext->pBlock->commandList->physicalAddr != pIndexBlock->commandList->physicalAddr )
					{
						pLastBlock = pIndexBlock;
					}

					pIndexBlock = pIndexBlock->next;
				}

				if( pLastBlock )
				{
					/* Attach the current block to the last block */

					/* Update the branch address to 0 */
					pReadContext->cmdDesc[2] = 0;
					fwFlushDMAMemPool( pReadContext->pBlock->commandList );
					pReadContext->pBlock->length = 0;

					/* Set the command descriptor branch address of the last block to this block */
					cmdDesc = (UINT32*)pLastBlock->commandList->virtualAddr;
					cmdDesc[2] = (UINT32)pReadContext->pBlock->commandList->physicalAddr | 1;
					fwFlushDMAMemPool( pLastBlock->commandList );

					/* Set the wake bit to restart processing */
					pReadContext->pContextCtrl->contextControlSet = FWContextCtrlWake;

					if( pNextBlock )
					{
						/* Update the first block */
						pReadContext->pContext->firstBlock = pNextBlock;
						pReadContext->pBlock = pNextBlock;

						/* Obtain information on the new block. */
						fwInvalidateDMAMemPool( pReadContext->pBlock->commandList );
						fwInvalidateDMAMemPool( pReadContext->pBlock->data );

						pReadContext->cmdDesc = (UINT32*)pReadContext->pBlock->commandList->virtualAddr;
						pReadContext->reqCount = pReadContext->cmdDesc[0] & 0x0000FFFF;
						pReadContext->resCount = pReadContext->cmdDesc[3] & 0x0000FFFF;
						pReadContext->dataPtr = (UINT32*)(pReadContext->pBlock->data->virtualAddr);

						if( pReadContext->pBlock->length < pReadContext->reqCount - pReadContext->resCount )
						{
							/* Get the data */
							*dataValue = *pReadContext->dataPtr;

							/* Increment the data pointer and length */
							pReadContext->pBlock->length += 4;
							pReadContext->dataPtr++;
							retVal = FWSuccess;
						}
						else
						{
							retVal = FWNoMoreData;
						}
					}
					else
					{
						retVal = FWDescriptorNotAvailable;
					}
				}
				else
				{
					retVal = FWDescriptorNotAvailable;
				}
			}
			else
			{
				retVal = FWNoMoreData;
			}
		}
		else
		{
			/* Get the data */
			*dataValue = *pReadContext->dataPtr;

			/* Increment the data pointer and length */
			pReadContext->pBlock->length += 4;
			pReadContext->dataPtr++;
			retVal = FWSuccess;
		}
	}

	return retVal;
}


/*
 * fwInitializeAsyncRxRequestContext()
 *
 * This function initializes the asynchronous receive DMA context.
 * The available buffers are used in a series of INPUT_MORE descriptors
 * to hold incoming packets. Note that this function only sets up the
 * context program. It does not start/run the context program.
 *
 * Returns:
 *    FWSuccess
 *    FWLinkBusy - The context is active and must be stopped before initializing
 *    FWInternalError - Bad pointer value
 *    FWDescriptorNotAvailable - no descriptor blocks available
 *
 */
static FWStatus fwInitializeAsyncRxRequestContext(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	FWDescriptorBlock *pBlock = NULL;
	UINT32 contextCtrl;
	UINT32 cmdPtr = 0;
	UINT32 size;
	UINT32 *cmdDesc;

	do
	{
		if( pDriver == NULL ) break;
		if( pDriver->linkLayerData == NULL ) break;
		if( pDriver->linkLayerData->asyncRxRequestContext == NULL ) break;
		if( pDriver->ohci == NULL ) break;

		/* Critical section */
		semTake( pDriver->linkLayerData->asyncRxRequestContext->contextSem, WAIT_FOREVER );

		/* Check that the context is not active or commanded to run */
		contextCtrl = pDriver->ohci->asyncRxRequest.contextControlClr;
		if( (contextCtrl & FWContextCtrlActive) || (contextCtrl & FWContextCtrlRun) )
		{
			/* End critical section */
			semGive( pDriver->linkLayerData->asyncRxRequestContext->contextSem );
			retVal = FWLinkBusy;
			break;
		}

		/* Clear the context control register */
		pDriver->ohci->asyncRxRequest.contextControlClr = 0x000098FF;

		/* Setup the context and input more descriptors */
		pDriver->linkLayerData->asyncRxRequestContext->active = FALSE;
		pBlock = pDriver->linkLayerData->asyncRxRequestContext->descriptorBlock;

		if( pBlock )
		{
			cmdPtr = (UINT32)pBlock->commandList->physicalAddr | 1;
		}

		pDriver->linkLayerData->asyncRxRequestContext->firstBlock = pBlock;

		/* Loop through the available blocks setting up each block as an input more descriptor */
		while( pBlock )
		{
			cmdDesc = (UINT32*)pBlock->commandList->virtualAddr;

			cmdDesc[0] = 0x283C0000;	/* Input more descriptor block */
			size = (UINT32)(pBlock->dataSize & 0x0000FFFC);
			cmdDesc[0] |= size;
			cmdDesc[1] = (UINT32)pBlock->data->physicalAddr;

			if( pBlock->next )
			{
				cmdDesc[2] = (UINT32)pBlock->next->commandList->physicalAddr;
				cmdDesc[2] |= 1;
			}
			else
				cmdDesc[2] = 0;

			cmdDesc[3] = size;

			pBlock->length = 0;
			pBlock->numCommands = 1;
			pBlock->inuse = TRUE;

			/* set the block up with all FF values */
			memset( pBlock->data->virtualAddr, 0xFF, size );

			/* Flush DMA memory before using in the driver */
			fwFlushDMAMemPool( pBlock->commandList );
			fwFlushDMAMemPool( pBlock->data );

			pBlock = pBlock->next;
		}

		/* Setup the context command pointer */
		pDriver->ohci->asyncRxRequest.commandPtr = cmdPtr;

		/* End critical section */
		semGive( pDriver->linkLayerData->asyncRxRequestContext->contextSem );

		if( cmdPtr != 0 )
			retVal = FWSuccess;
		else
			retVal = FWDescriptorNotAvailable;

	} while(0);

	return retVal;
}

/*
 * fwStartAsyncRxRequest()
 *
 * This function starts the asynchronous receive DMA context program.
 * The context should be setup using the initialize call before starting
 * the context.
 *
 * Returns:
 *    FWSuccess
 *    FWLinkBusy - The context is active and must be stopped before initializing
 *    FWInternalError - Bad pointer value
 *    FWNotinitialized - The context does not appear to be initialized
 *
 */
static FWStatus fwStartAsyncRxRequest(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	UINT32 contextCtrl;
	UINT32 cmdPtr = 0;

	do
	{
		if( pDriver == NULL ) break;
		if( pDriver->linkLayerData == NULL ) break;
		if( pDriver->linkLayerData->asyncRxRequestContext == NULL ) break;
		if( pDriver->ohci == NULL ) break;

		/* Critical section */
		semTake( pDriver->linkLayerData->asyncRxRequestContext->contextSem, WAIT_FOREVER );

		/* Check if the context is already running */
		contextCtrl = pDriver->ohci->asyncRxRequest.contextControlClr;
		if( (contextCtrl & FWContextCtrlActive) && (contextCtrl & FWContextCtrlRun) )
		{
			/* End critical section */
			semGive( pDriver->linkLayerData->asyncRxRequestContext->contextSem );
			retVal = FWSuccess;
			break;
		}

		/* The active flag cannot be set before run is set */
		if( contextCtrl & FWContextCtrlActive )
		{
			/* End critical section */
			semGive( pDriver->linkLayerData->asyncRxRequestContext->contextSem );
			retVal = FWLinkBusy;
			break;
		}

		cmdPtr = pDriver->ohci->asyncRxRequest.commandPtr;
		if( (cmdPtr & 0x0000000F) != 1 )
		{
			/* End critical section */
			semGive( pDriver->linkLayerData->asyncRxRequestContext->contextSem );
			retVal = FWNotInitialized;
			break;
		}

		pDriver->linkLayerData->asyncRxRequestContext->active = TRUE;

		/* Set the context control run bit */
		pDriver->ohci->asyncRxRequest.contextControlSet = FWContextCtrlRun;

		/* End critical section */
		semGive( pDriver->linkLayerData->asyncRxRequestContext->contextSem );

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

/*
 * fwStopAsyncRxRequest()
 *
 * This function stops the asynchronous receive DMA context program.
 *
 * Returns:
 *    FWSuccess - if the context is marked to stop
 *
 */
static FWStatus fwStopAsyncRxRequest(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	UINT32 contextCtrl;

	do
	{
		if( pDriver == NULL ) break;
		if( pDriver->linkLayerData == NULL ) break;
		if( pDriver->linkLayerData->asyncRxRequestContext == NULL ) break;
		if( pDriver->ohci == NULL ) break;

		/* Critical section */
		semTake( pDriver->linkLayerData->asyncRxRequestContext->contextSem, WAIT_FOREVER );

		pDriver->linkLayerData->asyncRxRequestContext->active = FALSE;

		/* Clear the context control run bit */
		pDriver->ohci->asyncRxRequest.contextControlClr = FWContextCtrlRun;

		/* Check if the context is already stopped */
		contextCtrl = pDriver->ohci->asyncRxRequest.contextControlClr;
		if( (contextCtrl & FWContextCtrlActive) == 0 )
		{
			/* End critical section */
			semGive( pDriver->linkLayerData->asyncRxRequestContext->contextSem );
			retVal = FWSuccess;
			break;
		}

		/* End critical section */
		semGive( pDriver->linkLayerData->asyncRxRequestContext->contextSem );

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

/*
 * fwInitializeAsyncRxResponseContext()
 *
 * This function initializes the asynchronous receive DMA context.
 * The available buffers are used in a series of INPUT_MORE descriptors
 * to hold incoming packets. Note that this function only sets up the
 * context program. It does not start/run the context program.
 *
 * Returns:
 *    FWSuccess
 *    FWLinkBusy - The context is active and must be stopped before initializing
 *    FWInternalError - Bad pointer value
 *    FWDescriptorNotAvailable - no descriptor blocks available
 *
 */
static FWStatus fwInitializeAsyncRxResponseContext(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	FWDescriptorBlock *pBlock = NULL;
	UINT32 contextCtrl;
	UINT32 cmdPtr = 0;
	UINT32 size;
	UINT32 *cmdDesc;

	do
	{
		if( pDriver == NULL ) break;
		if( pDriver->linkLayerData == NULL ) break;
		if( pDriver->linkLayerData->asyncRxResponseContext == NULL ) break;
		if( pDriver->ohci == NULL ) break;

		/* Critical section */
		semTake( pDriver->linkLayerData->asyncRxResponseContext->contextSem, WAIT_FOREVER );

		/* Check that the context is not active or commanded to run */
		contextCtrl = pDriver->ohci->asyncRxResponse.contextControlClr;
		if( (contextCtrl & FWContextCtrlActive) || (contextCtrl & FWContextCtrlRun) )
		{
			/* End critical section */
			semGive( pDriver->linkLayerData->asyncRxResponseContext->contextSem );
			retVal = FWLinkBusy;
			break;
		}

		/* Clear the context control register */
		pDriver->ohci->asyncRxResponse.contextControlClr = 0x000098FF;

		/* Setup the context and input more descriptors */
		pDriver->linkLayerData->asyncRxResponseContext->active = FALSE;
		pBlock = pDriver->linkLayerData->asyncRxResponseContext->descriptorBlock;

		if( pBlock )
		{
			cmdPtr = (UINT32)pBlock->commandList->physicalAddr | 1;
		}

		pDriver->linkLayerData->asyncRxResponseContext->firstBlock = pBlock;

		/* Loop through the available blocks setting up each block as an input more descriptor */
		while( pBlock )
		{
			cmdDesc = (UINT32*)pBlock->commandList->virtualAddr;

			cmdDesc[0] = 0x283C0000;	/* Input more descriptor block */
			size = (UINT32)(pBlock->dataSize & 0x0000FFFC);
			cmdDesc[0] |= size;
			cmdDesc[1] = (UINT32)pBlock->data->physicalAddr;

			if( pBlock->next )
			{
				cmdDesc[2] = (UINT32)pBlock->next->commandList->physicalAddr;
				cmdDesc[2] |= 1;
			}
			else
				cmdDesc[2] = 0;

			cmdDesc[3] = size;

			/* set the block up with all FF values */
			memset( pBlock->data->virtualAddr, 0xFF, size );

			/* Flush DMA memory before using in the driver */
			fwFlushDMAMemPool( pBlock->commandList );
			fwFlushDMAMemPool( pBlock->data );

			pBlock = pBlock->next;
		}

		/* Setup the context command pointer */
		pDriver->ohci->asyncRxResponse.commandPtr = cmdPtr;

		/* End critical section */
		semGive( pDriver->linkLayerData->asyncRxResponseContext->contextSem );

		if( cmdPtr != 0 )
			retVal = FWSuccess;
		else
			retVal = FWDescriptorNotAvailable;

	} while(0);

	return retVal;
}

/*
 * fwStartAsyncRxResponse()
 *
 * This function starts the asynchronous receive DMA context program.
 * The context should be setup using the initialize call before starting
 * the context.
 *
 * Returns:
 *    FWSuccess
 *    FWLinkBusy - The context is active and must be stopped before initializing
 *    FWInternalError - Bad pointer value
 *    FWNotinitialized - The context does not appear to be initialized
 *
 */
static FWStatus fwStartAsyncRxResponse(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	UINT32 contextCtrl;
	UINT32 cmdPtr = 0;

	do
	{
		if( pDriver == NULL ) break;
		if( pDriver->linkLayerData == NULL ) break;
		if( pDriver->linkLayerData->asyncRxResponseContext == NULL ) break;
		if( pDriver->ohci == NULL ) break;

		/* Critical section */
		semTake( pDriver->linkLayerData->asyncRxResponseContext->contextSem, WAIT_FOREVER );

		/* Check if the context is already running */
		contextCtrl = pDriver->ohci->asyncRxResponse.contextControlClr;
		if( (contextCtrl & FWContextCtrlActive) && (contextCtrl & FWContextCtrlRun) )
		{
			/* End critical section */
			semGive( pDriver->linkLayerData->asyncRxResponseContext->contextSem );
			retVal = FWSuccess;
			break;
		}

		/* The active flag cannot be set before run is set */
		if( contextCtrl & FWContextCtrlActive )
		{
			/* End critical section */
			semGive( pDriver->linkLayerData->asyncRxResponseContext->contextSem );
			retVal = FWLinkBusy;
			break;
		}

		cmdPtr = pDriver->ohci->asyncRxResponse.commandPtr;
		if( (cmdPtr & 0x0000000F) != 1 )
		{
			/* End critical section */
			semGive( pDriver->linkLayerData->asyncRxResponseContext->contextSem );
			retVal = FWNotInitialized;
			break;
		}

		pDriver->linkLayerData->asyncRxResponseContext->active = TRUE;

		/* Set the context control run bit */
		pDriver->ohci->asyncRxResponse.contextControlSet = FWContextCtrlRun;

		/* End critical section */
		semGive( pDriver->linkLayerData->asyncRxResponseContext->contextSem );

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

/*
 * fwStopAsyncRxResponse()
 *
 * This function stops the asynchronous receive DMA context program.
 *
 * Returns:
 *    FWSuccess - if the context is marked to stop
 *
 */
static FWStatus fwStopAsyncRxResponse(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	UINT32 contextCtrl;

	do
	{
		if( pDriver == NULL ) break;
		if( pDriver->linkLayerData == NULL ) break;
		if( pDriver->linkLayerData->asyncRxResponseContext == NULL ) break;
		if( pDriver->ohci == NULL ) break;

		/* Critical section */
		semTake( pDriver->linkLayerData->asyncRxResponseContext->contextSem, WAIT_FOREVER );

		/* Clear the context control run bit */
		pDriver->ohci->asyncRxResponse.contextControlClr = FWContextCtrlRun;

		/* Check if the context is already stopped */
		contextCtrl = pDriver->ohci->asyncRxResponse.contextControlClr;
		if( (contextCtrl & FWContextCtrlActive) == 0 )
		{
			/* End critical section */
			semGive( pDriver->linkLayerData->asyncRxResponseContext->contextSem );
			retVal = FWSuccess;
			break;
		}

		pDriver->linkLayerData->asyncRxResponseContext->active = FALSE;

		/* End critical section */
		semGive( pDriver->linkLayerData->asyncRxResponseContext->contextSem );

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

unsigned long fwGetMaxPayload(void)
{
	return (unsigned long)FWDescriptorDataSize;
}

/*
 * fwIsAsyncResponseBufferAvailable()
 *
 * This function returns true if the async response DMA context
 * has buffer space available for a request.
 *
 * Returns:
 *    TRUE - space is available
 *    FALSE - space is not available
 *
 */
static boolean fwIsAsyncResponseBufferAvailable(FWDriverData *pDriver, unsigned long neededSize)
{
	boolean retVal = FALSE;
	FWDescriptorBlock *pBlock = NULL;
	unsigned long available = 0;
	UINT32 *cmdDesc;

	do
	{
		if( pDriver == NULL ) break;
		if( pDriver->linkLayerData == NULL ) break;
		if( pDriver->linkLayerData->asyncRxResponseContext == NULL ) break;
		if( pDriver->ohci == NULL ) break;

		/* Check for available buffer space */
		pBlock = pDriver->linkLayerData->asyncRxResponseContext->descriptorBlock;

		while( pBlock )
		{
			/* Invalidate the DMA memory before reading the command list */
			fwInvalidateDMAMemPool( pBlock->commandList );

			cmdDesc = (UINT32*)pBlock->commandList->virtualAddr;

			available += cmdDesc[3] & 0x0000FFFF;

			if( available >= neededSize )
			{
				retVal = TRUE;
				break;
			}

			pBlock = pBlock->next;
		}

	} while(0);

	return retVal;
}

/*
 * Testing routines
 */
int fwSelfIdShow(int index);
int fwTestReadQuadlet(int index, int node, UINT32 address);
int fwTestReadBlock(int index, int node, UINT32 address, unsigned long numBytes);
int fwTestWriteQuadlet(int index, int node, UINT32 address, unsigned long value);

/* This routine dumps the self ID buffer */
int fwSelfIdShow(int index)
{
	FWStatus retVal = FWInternalError;
	int count = fwGetAdapterCount();
	UINT32 selfIdCount;
	UINT32 selfId;
	int i, size;
	unsigned short busNumber;
	unsigned char nodeNumber;
	unsigned char phyId;
	unsigned char gapCnt;
	FWSpeed speed;
	boolean contender;
	boolean linkActive;

	if( index >= 0 && index < count )
	{
		FWDriverData *pDriver = fwDriverDataArray[index];

		retVal = fwGetNodeID(pDriver, &busNumber, &nodeNumber);

		if( retVal == FWSuccess )
		{
			printf("Adapter bus:0x%X node:0x%X\n", busNumber, nodeNumber );

			fwInvalidateDMAMemPool( pDriver->linkLayerData->selfIdRecvMemory );

			selfIdCount = pDriver->ohci->selfIdCount;

			if( (selfIdCount & 0x80000000) == 0 )
			{
				size = (int)((selfIdCount >> 2) & 0x000001FF);
				for(i=0; i<size; i++)
				{
//					printf("SelfId receive buffer[%d]:0x%X\n", i, pDriver->linkLayerData->selfIdBufferVirtualAddr[i]);

					if( i > 0 )
					{
						if( i & 1)
						{
							selfId = pDriver->linkLayerData->selfIdBufferVirtualAddr[i];
							if( selfId == ~pDriver->linkLayerData->selfIdBufferVirtualAddr[i+1] )
							{
								if( ( selfId & 0x00800000 ) == 0 )
								{
									phyId = (unsigned char)((selfId >> 24) & 0x0000003F);
									printf("ID:0x%X GapCnt: Speed: Power: \n", phyId);
								}
							}
						}
					}
				}
			}
			else
				printf("A self ID error occured.\n");
		}

	}

	return 0;
}


int fwTestReadQuadlet(int index, int node, UINT32 address)
{
	int count = fwGetAdapterCount();
	FWStatus retVal = FWInternalError;
	int i;
	FWTransaction transaction;
	unsigned char myBuffer[100];
	unsigned short sourceId;

	if( index >= 0 && index < count )
	{
		FWDriverData *pDriver = fwDriverDataArray[index];

		transaction.busGeneration = pDriver->physicalLayerData->busGeneration & 0x000FFFFF;
		transaction.clientHandle = 0;
		transaction.databuffer = myBuffer;
		transaction.dataBufferSize = 64;
		transaction.dataLength = 0;
		transaction.destinationID = (unsigned short)node;
		transaction.destinationOffset.highOffset = 0xFFFF;
		transaction.destinationOffset.lowOffset = address;
		transaction.extendedCode = FWInvalidExtendedCode;
		transaction.responseCode = FWInvalidResponseCode;
		transaction.retryCode = FWRetry1;
		retVal = fwGetSourceID( pDriver, &sourceId );

		if( retVal == FWSuccess )
		{
			transaction.sourceID = sourceId;

			transaction.speed = FWS400;
			transaction.status = FWNotInitialized;
			transaction.timeStamp = 0;
			transaction.transactionCode = FWReadRequestQuadlet;
			transaction.transactionID = 1;
			transaction.transactionLabel = 1;
			transaction.xferStatus = 0;

			retVal = fwSendAsyncMessage( pDriver, &transaction );
		}
		else
		{
			printf("Source ID not valid.\n");
		}
	}
	else
		printf("Invalid adapter index entered.\n");

	return retVal;
}

int fwTestReadBlock(int index, int node, UINT32 address, unsigned long numBytes)
{
	int count = fwGetAdapterCount();
	FWStatus retVal = FWInternalError;
	int i;
	FWTransaction transaction;
	unsigned short sourceId;

	if( index >= 0 && index < count )
	{
		FWDriverData *pDriver = fwDriverDataArray[index];

		transaction.busGeneration = pDriver->physicalLayerData->busGeneration & 0x000FFFFF;
		transaction.clientHandle = 0;
		transaction.databuffer = NULL;
		transaction.dataBufferSize = 0;
		transaction.dataLength = numBytes;
		transaction.destinationID = (unsigned short)node;
		transaction.destinationOffset.highOffset = 0xFFFF;
		transaction.destinationOffset.lowOffset = address;
		transaction.extendedCode = FWInvalidExtendedCode;
		transaction.responseCode = FWInvalidResponseCode;
		transaction.retryCode = FWRetry1;
		retVal = fwGetSourceID( pDriver, &sourceId );

		if( retVal == FWSuccess )
		{
			transaction.sourceID = sourceId;

			transaction.speed = FWS400;
			transaction.status = FWNotInitialized;
			transaction.timeStamp = 0;
			transaction.transactionCode = FWReadRequestBlock;
			transaction.transactionID = 1;
			transaction.transactionLabel = 1;
			transaction.xferStatus = 0;

			retVal = fwSendAsyncMessage( pDriver, &transaction );
		}
		else
		{
			printf("Source ID not valid.\n");
		}
	}
	else
		printf("Invalid adapter index entered.\n");

	return retVal;
}

int fwTestWriteQuadlet(int index, int node, UINT32 address, unsigned long value)
{
	int count = fwGetAdapterCount();
	FWStatus retVal = FWInternalError;
	UINT32 buffer = value;
	int i;

	FWTransaction transaction;
	unsigned short sourceId;

	if( index >= 0 && index < count )
	{
		FWDriverData *pDriver = fwDriverDataArray[index];

		transaction.busGeneration = pDriver->physicalLayerData->busGeneration & 0x000FFFFF;
		transaction.clientHandle = 0;
		transaction.databuffer = (unsigned char*)&buffer;
		transaction.dataBufferSize = 4;
		transaction.dataLength = 4;
		transaction.destinationID = (unsigned short)node;
		transaction.destinationOffset.highOffset = 0xFFFF;
		transaction.destinationOffset.lowOffset = address;
		transaction.extendedCode = FWInvalidExtendedCode;
		transaction.responseCode = FWInvalidResponseCode;
		transaction.retryCode = FWRetry1;
		retVal = fwGetSourceID( pDriver, &sourceId );

		if( retVal == FWSuccess )
		{
			transaction.sourceID = sourceId;

			transaction.speed = FWS400;
			transaction.status = FWNotInitialized;
			transaction.timeStamp = 0;
			transaction.transactionCode = FWWriteRequestQuadlet;
			transaction.transactionID = 2;
			transaction.transactionLabel = 2;
			transaction.xferStatus = 0;

			retVal = fwSendAsyncMessage( pDriver, &transaction );
		}
		else
		{
			printf("Source ID not valid.\n");
		}
	}
	else
		printf("Invalid adapter index entered.\n");

	return retVal;
}

