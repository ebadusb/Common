/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header: H:/BCT_Development/vxWorks/Common/firewire/rcs/fw_transaction_layer.c 1.1 2007/02/07 15:22:39Z wms10235 Exp wms10235 $
 *
 * This file contains the firewire interface routines
 * to access the transaction layer.
 *
 * $Log: fw_transaction_layer.c $
 *
 */

#include <vxWorks.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fw_utility.h"
#include "fw_link_layer.h"
#include "fw_transaction_layer.h"
#include "fw_transaction_list.h"
#include "fw_bus_manager.h"
#include "fw_csr.h"

/* local variables */
static unsigned long fwUniqueTransactionID = 1;

/* local function prototypes */
static unsigned long fwGetTransactionID(void);
static FWStatus fwGetTransactionLabel(FWDriverData *pDriver, unsigned short destinationID, unsigned char *label);
static FWStatus fwQueueTxRequestTransaction(FWDriverData *pDriver, FWTransaction *transaction);
int fwMatchResponseWithRequest(const FWTransaction *cmp1, const FWTransaction *cmp2);
static FWStatus fwProcessAsyncIndication(FWDriverData *pDriver, FWTransaction *transaction);
static FWStatus fwAppSpecificAsyncIndication(FWDriverData *pDriver, FWTransaction *transaction, FWTransaction *responseTrans);
static FWStatus fwProcessPhyIndication(FWDriverData *pDriver, FWTransaction *transaction);
static FWStatus fwValidateManagerCSR(FWDriverData *pDriver, FWTransaction *transaction);
static FWStatus fwWriteManagerCSR(FWDriverData *pDriver, FWTransaction *transaction);
static FWStatus fwReadManagerCSR(FWDriverData *pDriver, FWTransaction *transaction, UINT32 *dataBuffer);
static FWStatus fwLockManagerCSR(FWDriverData *pDriver, FWTransaction *transaction, UINT32 *dataBuffer);
static FWStatus fwFormStandardAsyncResponse(FWDriverData *pDriver, FWTransaction *transaction, FWTransaction *responseTrans);
static FWStatus fwLocalRequest(FWDriverData *pDriver, FWTransaction *transaction);

FWStatus fwCreateTransactionLayer(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	if( pDriver )
	{
		if( pDriver->transactionLayerData == NULL )
		{
			do
			{
				pDriver->transactionLayerData = (FWTransactionData*)fwMalloc( sizeof(FWTransactionData) );

				if( pDriver->transactionLayerData == NULL )
				{
					retVal = FWMemAllocateError;
					break;
				}

				pDriver->transactionLayerData->initialized = FALSE;
				pDriver->transactionLayerData->incommingRequestList = fwTransactionListCreate();
				if( pDriver->transactionLayerData->incommingRequestList == NULL )
				{
					retVal = FWMemAllocateError;
					break;
				}

				pDriver->transactionLayerData->incommingResponseList = fwTransactionListCreate();
				if( pDriver->transactionLayerData->incommingResponseList == NULL )
				{
					retVal = FWMemAllocateError;
					break;
				}

				pDriver->transactionLayerData->outgoingRequestList = fwTransactionListCreate();
				if( pDriver->transactionLayerData->outgoingRequestList == NULL )
				{
					retVal = FWMemAllocateError;
					break;
				}

				pDriver->transactionLayerData->outgoingResponseList = fwTransactionListCreate();
				if( pDriver->transactionLayerData->outgoingResponseList == NULL )
				{
					retVal = FWMemAllocateError;
					break;
				}

				pDriver->transactionLayerData->transactionLabels = NULL;

				pDriver->transactionLayerData->recvResponseTrans = (FWTransaction*)fwMalloc( sizeof(FWTransaction) );
				if( pDriver->transactionLayerData->recvResponseTrans == NULL )
				{
					retVal = FWMemAllocateError;
					break;
				}

				memset( pDriver->transactionLayerData->recvResponseTrans, 0, sizeof(FWTransaction) );

				pDriver->transactionLayerData->recvResponseTrans->databuffer = (unsigned char*)fwMalloc( fwGetMaxPayload() );
				if( pDriver->transactionLayerData->recvResponseTrans->databuffer == NULL )
				{
					retVal = FWMemAllocateError;
					break;
				}

				pDriver->transactionLayerData->recvResponseTrans->dataBufferSize = fwGetMaxPayload();

				pDriver->transactionLayerData->recvRequestTrans = (FWTransaction*)fwMalloc( sizeof(FWTransaction) );
				if( pDriver->transactionLayerData->recvRequestTrans == NULL )
				{
					retVal = FWMemAllocateError;
					break;
				}

				memset( pDriver->transactionLayerData->recvRequestTrans, 0, sizeof(FWTransaction) );

				pDriver->transactionLayerData->recvRequestTrans->databuffer = (unsigned char*)fwMalloc( fwGetMaxPayload() );
				if( pDriver->transactionLayerData->recvRequestTrans->databuffer == NULL )
				{
					retVal = FWMemAllocateError;
					break;
				}

				pDriver->transactionLayerData->recvRequestTrans->dataBufferSize = fwGetMaxPayload();

				pDriver->transactionLayerData->transSemId = semMCreate( SEM_Q_PRIORITY | SEM_INVERSION_SAFE );
				if( pDriver->transactionLayerData->transSemId == NULL )
				{
					retVal = FWSemAllocateError;
					break;
				}

				pDriver->transactionLayerData->lastRecvGeneration = 0xFFFFFFFF;

				retVal = FWSuccess;

			} while(0);
		}
		else
			retVal = FWSuccess;
	}

	return retVal;
}

FWStatus fwDestroyTransactionLayer(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	FWTransactionLabel *pTransLabel = NULL;
	FWTransactionLabel *pTempLabel = NULL;
	FWTransaction *item;
	FWTransaction *tempItem;

	if( pDriver )
	{
		if( pDriver->transactionLayerData != NULL )
		{
			pDriver->transactionLayerData->initialized = FALSE;

			/* Delete the generic receive transaction structures */
			if( pDriver->transactionLayerData->recvResponseTrans )
			{
				if( pDriver->transactionLayerData->recvResponseTrans->databuffer )
				{
					fwFree( pDriver->transactionLayerData->recvResponseTrans->databuffer );
					pDriver->transactionLayerData->recvResponseTrans->databuffer = NULL;
				}

				fwFree( pDriver->transactionLayerData->recvResponseTrans );
				pDriver->transactionLayerData->recvResponseTrans = NULL;
			}

			if( pDriver->transactionLayerData->recvRequestTrans )
			{
				if( pDriver->transactionLayerData->recvRequestTrans->databuffer )
				{
					fwFree( pDriver->transactionLayerData->recvRequestTrans->databuffer );
					pDriver->transactionLayerData->recvRequestTrans->databuffer = NULL;
				}

				fwFree( pDriver->transactionLayerData->recvRequestTrans );
				pDriver->transactionLayerData->recvRequestTrans = NULL;
			}

			/* Delete the transactions in the receive lists */
			item = pDriver->transactionLayerData->incommingRequestList->head;

			while( item )
			{
				tempItem = item;
				item = item->next;

				fwFree( tempItem );
			}

			item = pDriver->transactionLayerData->incommingResponseList->head;

			while( item )
			{
				tempItem = item;
				item = item->next;

				fwFree( tempItem );
			}

			/* Signal the transactions in the transmit lists
			 * and set the status to shutdown.
			 */
			item = pDriver->transactionLayerData->outgoingRequestList->head;

			while( item )
			{
				item->status = FWShutdown;
				if( item->semId )
					semGive( item->semId );
				item = item->next;
			}

			item = pDriver->transactionLayerData->outgoingResponseList->head;

			while( item )
			{
				item->status = FWShutdown;
				if( item->semId )
					semGive( item->semId );
				item = item->next;
			}

			/* Delete the transaction lists */
			fwTransactionListDestroy( pDriver->transactionLayerData->incommingRequestList );
			pDriver->transactionLayerData->incommingRequestList = NULL;
			fwTransactionListDestroy( pDriver->transactionLayerData->incommingResponseList );
			pDriver->transactionLayerData->incommingResponseList = NULL;
			fwTransactionListDestroy( pDriver->transactionLayerData->outgoingRequestList );
			pDriver->transactionLayerData->outgoingRequestList = NULL;
			fwTransactionListDestroy( pDriver->transactionLayerData->outgoingResponseList );
			pDriver->transactionLayerData->outgoingResponseList = NULL;

			if( pDriver->transactionLayerData->transSemId )
			{
				semDelete( pDriver->transactionLayerData->transSemId );
				pDriver->transactionLayerData->transSemId = NULL;
			}

			pTransLabel = pDriver->transactionLayerData->transactionLabels;

			while( pTransLabel != NULL )
			{
				pTempLabel = pTransLabel->next;
				fwFree( pTransLabel );
				pTransLabel = pTempLabel;
			}

			fwFree( pDriver->transactionLayerData );

			pDriver->transactionLayerData = NULL;

			retVal = FWSuccess;
		}
		else
			retVal = FWSuccess;
	}

	return retVal;
}

FWStatus fwInitializeTransaction(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	do
	{
		if( pDriver == NULL ) break;
		if( pDriver->transactionLayerData == NULL ) break;
		if( pDriver->ohci == NULL ) break;

		/* If the self ID generation is invalid, reset
		 * the last generation.
		 */
		if( pDriver->ohci->selfIdCount & 0x80000000 )
			pDriver->transactionLayerData->lastRecvGeneration = 0xFFFFFFFF;

		pDriver->transactionLayerData->initialized = TRUE;
		retVal = FWSuccess;

	} while(0);

	return retVal;
}

FWStatus fwResetTransaction(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	FWTransaction *transaction;

	do
	{
		if( pDriver == NULL ) break;
		if( pDriver->transactionLayerData == NULL ) break;
		if( pDriver->transactionLayerData->initialized == FALSE )
		{
			retVal = FWNotInitialized;
			break;
		}
		/* Set initialize to FALSE to prevent further
		 * transaction from being submitted.
		 */
		pDriver->transactionLayerData->initialized = FALSE;

		/* Move all pending transactions to the waiting queue with
		 * a reset status and signal each processes semaphore
		 */
		transaction = fwTransactionListPopFront( pDriver->transactionLayerData->outgoingRequestList );

		while( transaction )
		{
			transaction->status = FWResetOccured;

			FWLOGLEVEL5("Transaction reset terminating transaction ID:%d.\n", transaction->transactionID );

			/* Add the transaction to the TX response queue. */
			retVal = fwTransactionListPushFront( pDriver->transactionLayerData->outgoingResponseList, transaction );

			semGive( transaction->semId );

			transaction = fwTransactionListPopFront( pDriver->transactionLayerData->outgoingRequestList );
		}

		pDriver->transactionLayerData->lastRecvGeneration = 0xFFFFFFFF;

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

FWStatus fwPostAsyncWriteRequest(FWDriverData *pDriver, FWTransaction *transaction)
{
	FWStatus retVal = FWInternalError;

	do
	{
		if( pDriver == NULL ) break;
		if( transaction == NULL ) break;
		if( pDriver->transactionLayerData == NULL ) break;
		if( pDriver->transactionLayerData->initialized == FALSE )
		{
			retVal = FWNotInitialized;
			break;
		}

		/* Fill in the transaction type based on the write request size. */
		if( transaction->dataLength <= 4 )
		{
			transaction->transactionCode = FWWriteRequestQuadlet;
		}
		else
		{
			transaction->transactionCode = FWWriteRequestBlock;
		}

		retVal = fwQueueTxRequestTransaction( pDriver, transaction );

	} while(0);

	return retVal;
}

FWStatus fwPostAsyncReadRequest(FWDriverData *pDriver, FWTransaction *transaction)
{
	FWStatus retVal = FWInternalError;

	do
	{
		if( pDriver == NULL ) break;
		if( transaction == NULL ) break;
		if( pDriver->transactionLayerData == NULL ) break;
		if( pDriver->transactionLayerData->initialized == FALSE )
		{
			retVal = FWNotInitialized;
			break;
		}

		/* Check if any data is being requested. Note that
		 * in this case the data length is the requested
		 * number of bytes to read from another node.
		 */
		if( transaction->dataLength == 0 )
		{
			retVal = FWNoTransactionData;
		}

		/* Fill in the transaction type based on the read request size. */
		if( transaction->dataLength <= 4 )
		{
			transaction->transactionCode = FWReadRequestQuadlet;
		}
		else
		{
			transaction->transactionCode = FWReadRequestBlock;
		}

		retVal = fwQueueTxRequestTransaction( pDriver, transaction );

	} while(0);

	return retVal;
}

FWStatus fwPostAsyncLockRequest(FWDriverData *pDriver, FWTransaction *transaction)
{
	FWStatus retVal = FWInternalError;

	do
	{
		if( pDriver == NULL ) break;
		if( transaction == NULL ) break;
		if( pDriver->transactionLayerData == NULL ) break;
		if( pDriver->transactionLayerData->initialized == FALSE )
		{
			retVal = FWNotInitialized;
			break;
		}

		transaction->transactionCode = FWLockRequest;

		if( transaction->dataLength > 16 )
		{
			FWLOGLEVEL3("Data length too long for lock request.\n");
			retVal = FWLockTransError;
			break;
		}

		if( transaction->extendedCode == FWInvalidExtendedCode )
		{
			FWLOGLEVEL3("Invalid extended transaction code.\n");
			retVal = FWInvalidExtendedTransactionCode;
			break;
		}

		retVal = fwQueueTxRequestTransaction( pDriver, transaction );

	} while(0);

	return retVal;
}

FWStatus fwSendAsyncRequestTransaction(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	FWTransaction *transaction = NULL;
	unsigned short sourceID;

	do
	{
		if( pDriver == NULL ) break;
		if( pDriver->transactionLayerData == NULL ) break;
		if( pDriver->transactionLayerData->initialized == FALSE )
		{
			retVal = FWNotInitialized;
			break;
		}

		transaction = fwTransactionListPopFront( pDriver->transactionLayerData->outgoingRequestList );

		if( transaction == NULL )
		{
			retVal = FWNoTransactions;
			break;
		}

		fwGetSourceID( pDriver, &sourceID );

		while( transaction )
		{
			if( sourceID == transaction->destinationID &&
				 transaction->busGeneration == pDriver->physicalLayerData->busGeneration )
			{
				retVal = fwLocalRequest( pDriver, transaction );

				/* The transaction was accepted by the link layer.
				 * Update the status and move the transaction to the
				 * waiting queue.
				 */
				transaction->status = retVal;

				/* Add the transaction to the TX response queue. */
				retVal = fwTransactionListPushBack( pDriver->transactionLayerData->outgoingResponseList, transaction );

				semGive( transaction->semId );
			}
			else
			{
				retVal = fwSendAsyncMessage( pDriver, transaction );

				if( retVal == FWSuccess )
				{
					/* The transaction was accepted by the link layer.
					 * Update the status and move the transaction to the
					 * waiting queue.
					 */
					transaction->status = retVal;

					/* Add the transaction to the TX response queue. */
					retVal = fwTransactionListPushBack( pDriver->transactionLayerData->outgoingResponseList, transaction );

					if( retVal == FWSuccess )
						transaction->status = FWQueuedWaiting;

					/* Set the pending status to false since an interrupt
					 * will trigger the processing of the next transaction.
					 */
					fwUpdateRequestTXStatus( pDriver, FALSE );
					break;
				}
				else if( retVal == FWLinkBusy )
				{
					/* The link layer is busy processing other transactions. Push
					 * the transaction back on the queue.
					 */
					retVal = fwTransactionListPushFront( pDriver->transactionLayerData->outgoingRequestList, transaction );
					if( retVal != FWSuccess )
					{
						FWLOGLEVEL2("Error: Request TX transaction list is inconsistent.\n");
					}
					break;
				}
				else
				{
					/* Move the transaction to the waiting queue with
					 * the status set and then signal the client that
					 * the transaction has completed in error.
					 */
					transaction->status = retVal;

					FWLOGLEVEL5("Error %d sending request TX transaction %d.\n", retVal, transaction->transactionID );

					/* Add the transaction to the TX response queue. */
					retVal = fwTransactionListPushBack( pDriver->transactionLayerData->outgoingResponseList, transaction );

					semGive( transaction->semId );
				}
			}

			/* Process the next message */
			transaction = fwTransactionListPopFront( pDriver->transactionLayerData->outgoingRequestList );
		}

	} while(0);

	return retVal;
}

FWStatus fwRecvAsyncResponseTransaction(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	FWTransaction *requestTrans;
	FWTransaction *responseTrans;

	do
	{
		if( pDriver == NULL ) break;
		if( pDriver->transactionLayerData == NULL ) break;
		if( pDriver->transactionLayerData->initialized == FALSE )
		{
			retVal = FWNotInitialized;
			break;
		}

		responseTrans = pDriver->transactionLayerData->recvResponseTrans;

		retVal = fwReadAsyncResponseMessage( pDriver, responseTrans );

		if( retVal != FWSuccess )
		{
			if( retVal != FWNotFound )
				FWLOGLEVEL3("fwRecvAsyncResponseTransaction() Read returned:%d\n", retVal );
			break;
		}

		FWLOGLEVEL9("Response received. Trans Label:%d Source ID:%d Dest ID:%d\n",
						responseTrans->transactionLabel,
						responseTrans->sourceID,
						responseTrans->destinationID );

		semTake( pDriver->transactionLayerData->transSemId, WAIT_FOREVER );

		/* Found a response message. Attempt to match it with a request. */
		requestTrans = fwFindTransactionListItem( pDriver->transactionLayerData->outgoingResponseList,
																responseTrans,
																fwMatchResponseWithRequest );

		if( requestTrans )
		{
			FWLOGLEVEL9("Request/response matched. Transaction ID:%d\n", requestTrans->transactionID );
			/* Found the matching request. Populate the transaction
			 * with response data and the status.
			 */
			requestTrans->transactionCode = responseTrans->transactionCode;
			requestTrans->responseCode = responseTrans->responseCode;
			requestTrans->speed = responseTrans->speed;
			requestTrans->extendedCode = responseTrans->extendedCode;
			requestTrans->timeStamp = responseTrans->timeStamp;
			requestTrans->xferStatus = responseTrans->xferStatus;

			FWLOGLEVEL9("Response status. Transaction ID:%d ResponseCode:%d XferStatus:0x%08X\n",
							requestTrans->transactionID,
							requestTrans->responseCode,
							requestTrans->xferStatus );

			if( requestTrans->dataBufferSize < responseTrans->dataLength )
			{
				/* The data is truncated due to the buffer being too small. */
				memcpy( requestTrans->databuffer, responseTrans->databuffer, requestTrans->dataBufferSize );
				requestTrans->dataLength = requestTrans->dataBufferSize;
				requestTrans->status = FWDataTruncated;
			}
			else
			{
				/* Copy all the data into the user buffer. */
				memcpy( requestTrans->databuffer, responseTrans->databuffer, responseTrans->dataLength );
				requestTrans->dataLength = responseTrans->dataLength;
				requestTrans->status = FWSuccess;
			}

			/* Notify the client that the transaction is ready */
			semGive( requestTrans->semId );

			retVal = FWSuccess;
		}

		semGive( pDriver->transactionLayerData->transSemId );

	} while(0);

	return retVal;
}

FWStatus fwPostAsyncResponse(FWDriverData *pDriver, FWTransaction *transaction)
{
	FWStatus retVal = FWInternalError;

	do
	{
		if( pDriver == NULL ) break;
		if( transaction == NULL ) break;
		if( pDriver->transactionLayerData == NULL ) break;
		if( pDriver->transactionLayerData->initialized == FALSE )
		{
			retVal = FWNotInitialized;
			break;
		}

		/* Queue the tranaction by adding it to the end of the response TX list. */
		retVal = fwTransactionListPushBack( pDriver->transactionLayerData->incommingResponseList, transaction );

		FWLOGLEVEL9("Posting response transaction %d to response TX queue.\n", transaction->transactionID );

		if( pDriver->transactionLayerData->incommingResponseList->count == 1 )
		{
			/* Since this transaction is first transaction on the list, notify
			 * the bus manager that new trans actions are ready to be
			 * processed.
			 */
			FWLOGLEVEL9("Notifing bus manager of new response TX queue transactions.\n");
			fwUpdateResponseTXStatus( pDriver, TRUE );
			retVal = fwNotifyBusManager();
		}

	} while(0);

	return retVal;
}

FWStatus fwSendAsyncResponseTransaction(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	FWTransaction *transaction = NULL;

	do
	{
		if( pDriver == NULL ) break;
		if( pDriver->transactionLayerData == NULL ) break;
		if( pDriver->transactionLayerData->initialized == FALSE )
		{
			retVal = FWNotInitialized;
			break;
		}

		transaction = fwTransactionListPopFront( pDriver->transactionLayerData->incommingResponseList );

		if( transaction == NULL )
		{
			retVal = FWNoTransactions;
			break;
		}

		while( transaction )
		{
			retVal = fwSendAsyncMessage( pDriver, transaction );

			FWLOGLEVEL9("Sending response transaction. ID:%d with status:%d.\n", transaction->transactionID, retVal );

			if( retVal == FWSuccess )
			{
				/* The transaction was accepted by the link layer.
				 * Update the status and move the transaction to the
				 * waiting queue.
				 */
				transaction->status = retVal;

				/* Add the transaction to the TX response queue. */
				retVal = fwTransactionListPushBack( pDriver->transactionLayerData->incommingRequestList, transaction );

				if( retVal == FWSuccess )
					transaction->status = FWQueuedWaiting;

				/* Set the pending status to false since an interrupt
				 * will trigger the processing of the next transaction.
				 */
				fwUpdateResponseTXStatus( pDriver, FALSE );
				break;
			}
			else if( retVal == FWLinkBusy )
			{
				/* The link layer is busy processing other transactions. Push
				 * the transaction back on the queue.
				 */
				retVal = fwTransactionListPushFront( pDriver->transactionLayerData->incommingResponseList, transaction );
				if( retVal != FWSuccess )
				{
					FWLOGLEVEL2("Error: Response TX transaction list is inconsistent.\n");
				}
				break;
			}
			else
			{
				/* Move the transaction to the waiting queue with
				 * the status set and then signal the client that
				 * the transaction has completed with an error.
				 */
				transaction->status = retVal;

				FWLOGLEVEL5("Error %d sending response TX transaction %d.\n", retVal, transaction->transactionID );

				/* Add the transaction to the TX response queue. */
				retVal = fwTransactionListPushFront( pDriver->transactionLayerData->incommingRequestList, transaction );

				if( pDriver->transactionLayerData->incommingResponseList->count == 0 )
				{
					fwUpdateResponseTXStatus( pDriver, FALSE );
				}
			}

			/* Process the next message */
			transaction = fwTransactionListPopFront( pDriver->transactionLayerData->incommingResponseList );
		}

		if( pDriver->transactionLayerData->incommingRequestList->count > 0 )
		{
			fwNotifyBusManager();
		}

	} while(0);

	return retVal;
}

FWStatus fwCompleteAsyncResponseTransaction(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	FWTransaction *transaction = NULL;

	do
	{
		if( pDriver == NULL ) break;
		if( pDriver->transactionLayerData == NULL ) break;
		if( pDriver->transactionLayerData->initialized == FALSE )
		{
			retVal = FWNotInitialized;
			break;
		}

		transaction = fwTransactionListPopFront( pDriver->transactionLayerData->incommingRequestList );

		while( transaction )
		{
			FWLOGLEVEL5("Transaction %d is complete with status %d.\n", transaction->transactionID, transaction->status );

			if( transaction->databuffer )
			{
				fwFree( transaction->databuffer );
			}

			fwFree( transaction );

			transaction = fwTransactionListPopFront( pDriver->transactionLayerData->incommingRequestList );
		}

	} while(0);

	return retVal;
}

FWStatus fwRecvAsyncRequestTransaction(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	FWTransaction *requestTrans;
	unsigned short localSourceID;
	int clientHandle = 0;
	UINT32 *quadletData;

	do
	{
		if( pDriver == NULL ) break;
		if( pDriver->transactionLayerData == NULL ) break;
		if( pDriver->transactionLayerData->initialized == FALSE )
		{
			retVal = FWNotInitialized;
			break;
		}

		requestTrans = pDriver->transactionLayerData->recvRequestTrans;

		retVal = fwReadAsyncRequestMessage( pDriver, requestTrans );

		if( retVal != FWSuccess )
		{
			FWLOGLEVEL5("fwRecvAsyncRequestTransaction() Read request msg returned:%d\n", retVal );
			break;
		}

		FWLOGLEVEL9("Async request made. Type:%d Dest Offset:0x%04X %08X\n",
						requestTrans->transactionCode,
						requestTrans->destinationOffset.highOffset,
						requestTrans->destinationOffset.lowOffset );

		switch( requestTrans->transactionCode )
		{
		case FWWriteRequestQuadlet:
		case FWWriteRequestBlock:
		case FWReadRequestQuadlet:
		case FWReadRequestBlock:
		case FWLockRequest:
			/* Check the bus generation (self ID generation) */
			if( pDriver->transactionLayerData->lastRecvGeneration != pDriver->physicalLayerData->busGeneration )
			{
				FWLOGLEVEL3("Received async request has wrong generation. Current:%X Last:%X\n",
								pDriver->physicalLayerData->busGeneration,
								pDriver->transactionLayerData->lastRecvGeneration );
				break;
			}

			retVal = fwGetSourceID( pDriver, &localSourceID );

			if( retVal != FWSuccess )
			{
				FWLOGLEVEL3("fwRecvAsyncRequestTransaction() invalid source ID:%d\n", retVal );
				break;
			}

			if( localSourceID != requestTrans->destinationID )
			{
				/* Message does not appear to be addressed to this node. */
				FWLOGLEVEL3("fwRecvAsyncRequestTransaction() source ID does not match.\n");
				retVal = FWInvalidNodeAddr;
				break;
			}

			/* Update the bus generation */
			requestTrans->busGeneration = pDriver->physicalLayerData->busGeneration;

			retVal = fwProcessAsyncIndication( pDriver, requestTrans );
			break;

		case FWPhysicalPacket:
			FWLOGLEVEL8("fwRecvAsyncRequestTransaction() received physical packet.\n");

			if( (requestTrans->xferStatus & 0x001F) == FWEventBusReset )
			{
				if( requestTrans->dataBufferSize >= 8 )
				{
					quadletData = (UINT32*)requestTrans->databuffer;
					pDriver->transactionLayerData->lastRecvGeneration = (quadletData[1] & 0x00FF0000) >> 16;
					FWLOGLEVEL5("Received async recv bus reset packet. Updating generation to %d.\n", pDriver->transactionLayerData->lastRecvGeneration );
				}
			}

			retVal = FWSuccess;
			break;

		case FWWriteResponse:
		case FWReadResponseQuadlet:
		case FWReadResponseBlock:
		case FWCycleStart:
		case FWIsoDataBlock:
		case FWLockResponse:
		case FWInvalidTransCode:
			retVal = FWInvalidTransactionType;
			break;
		}

	} while(0);

	return retVal;
}

FWStatus fwCancelTransaction(FWDriverData *pDriver, FWTransaction *transaction)
{
	FWStatus retVal = FWInternalError;

	do
	{
		if( pDriver == NULL ) break;
		if( transaction == NULL ) break;
		if( pDriver->transactionLayerData == NULL ) break;
		if( pDriver->transactionLayerData->initialized == FALSE )
		{
			retVal = FWNotInitialized;
			break;
		}

		/* Normally when this function is called the
		 * tranaction will be in the waiting queue.
		 */
		retVal = fwRemoveTransaction( pDriver, transaction );

	} while(0);

	return retVal;
}

FWStatus fwRemoveTransaction(FWDriverData *pDriver, FWTransaction *transaction)
{
	FWStatus retVal = FWInternalError;
	FWTransaction *tempTrans;

	do
	{
		if( pDriver == NULL ) break;
		if( transaction == NULL ) break;
		if( pDriver->transactionLayerData == NULL ) break;
		if( pDriver->transactionLayerData->initialized == FALSE )
		{
			retVal = FWNotInitialized;
			break;
		}

		retVal = FWNotFound;

		/* Normally when this function is called the
		 * tranaction will be in the response TX queue.
		 */
		tempTrans = fwRemoveTransactionListItem( pDriver->transactionLayerData->outgoingResponseList, transaction, NULL );

		if( tempTrans == NULL )
		{
			tempTrans = fwRemoveTransactionListItem( pDriver->transactionLayerData->outgoingRequestList, transaction, NULL );

			if( tempTrans )
			{
				retVal = FWSuccess;
			}
		}
		else
		{
			retVal = FWSuccess;
		}

	} while(0);

	return retVal;
}

FWStatus fwInitializeAsyncRequestTransaction(FWDriverData *pDriver, const FWAsyncTransactionCmd *asyncCmd, FWTransaction *transaction)
{
	FWStatus retVal = FWInternalError;
	unsigned short sourceId;

	do
	{
		retVal = fwGetSourceID( pDriver, &sourceId );

		if( retVal != FWSuccess )
		{
			break;
		}

		if( transaction == NULL )
		{
			retVal = FWInvalidArg;
			break;
		}

		transaction->busGeneration = pDriver->physicalLayerData->busGeneration & 0x000FFFFF;
		transaction->clientHandle = asyncCmd->clientHandle;
		transaction->databuffer = (unsigned char*)asyncCmd->databuffer;
		transaction->dataLength = asyncCmd->dataLength;
		transaction->dataBufferSize = asyncCmd->dataBufferSize;
		transaction->destinationID = asyncCmd->destinationID;
		transaction->destinationOffset = asyncCmd->destinationOffset;
		transaction->extendedCode = FWInvalidExtendedCode;
		transaction->responseCode = FWInvalidResponseCode;
		transaction->retryCode = FWRetry1;
		transaction->sourceID = sourceId;
		transaction->speed = asyncCmd->speed;
		transaction->status = FWQueuedPending;
		transaction->timeStamp = 0;
		transaction->transactionCode = FWInvalidTransCode;
		transaction->xferStatus = 0;
		transaction->semId = NULL;
		transaction->transactionID = fwGetTransactionID();
		retVal = fwGetTransactionLabel( pDriver, asyncCmd->destinationID, &transaction->transactionLabel );
		transaction->prev = NULL;
		transaction->next = NULL;

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

FWStatus fwUpdateRequestTXStatus(FWDriverData *pDriver, boolean newStatus)
{
	FWStatus retVal = FWInternalError;

	do
	{
		if( pDriver == NULL ) break;
		if( pDriver->transactionLayerData == NULL ) break;
		if( pDriver->transactionLayerData->transSemId == NULL ) break;

		semTake( pDriver->transactionLayerData->transSemId, WAIT_FOREVER );

		pDriver->transactionLayerData->newRequestTransaction = newStatus;

		semGive( pDriver->transactionLayerData->transSemId );

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

FWStatus fwUpdateResponseTXStatus(FWDriverData *pDriver, boolean newStatus)
{
	FWStatus retVal = FWInternalError;

	do
	{
		if( pDriver == NULL ) break;
		if( pDriver->transactionLayerData == NULL ) break;
		if( pDriver->transactionLayerData->transSemId == NULL ) break;

		semTake( pDriver->transactionLayerData->transSemId, WAIT_FOREVER );

		pDriver->transactionLayerData->newResponseTransaction = newStatus;

		semGive( pDriver->transactionLayerData->transSemId );

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

/*
 *  Local functions
 */

static unsigned long fwGetTransactionID(void)
{
	if( fwUniqueTransactionID++ > 0xFFFFFFF0 )
		fwUniqueTransactionID = 1;

	return fwUniqueTransactionID;
}

static FWStatus fwGetTransactionLabel(FWDriverData *pDriver, unsigned short destinationID, unsigned char *label)
{
	FWStatus retVal = FWInternalError;
	unsigned short nodeID = destinationID & 0x003F;
	FWTransactionLabel *transLabel;

	*label = 0;

	/* Begin critical section */
	semTake( pDriver->transactionLayerData->transSemId, WAIT_FOREVER );

	transLabel = pDriver->transactionLayerData->transactionLabels;

	while( transLabel )
	{
		if( transLabel->nodeID == nodeID )
		{
			break;
		}

		transLabel = transLabel->next;
	}

	if( transLabel )
	{
		/* Found the transaction label structure for this destination ID */
		transLabel->labelIndex++;

		if( transLabel->labelIndex > 63 )
			transLabel->labelIndex = 0;

		if( transLabel->labelUsed[transLabel->labelIndex] != 0 )
		{
			FWLOGLEVEL5("Warning transaction label is marked as used.\n");
		}

		transLabel->labelUsed[transLabel->labelIndex] = 1;
		*label = transLabel->labelIndex;

		retVal = FWSuccess;
	}
	else
	{
		FWLOGLEVEL9("Creating a new transaction label for node %d.\n", nodeID);

		transLabel = fwMalloc( sizeof(FWTransactionLabel) );

		if( transLabel == NULL )
		{
			retVal = FWMemAllocateError;
		}
		else
		{
			memset( transLabel, 0, sizeof(FWTransactionLabel) );
			transLabel->nodeID = nodeID;

			/* If no list exists, make this the list head. Otherwise,
			 * add this element to the head of the list.
			 */
			if( pDriver->transactionLayerData->transactionLabels != NULL )
			{
				pDriver->transactionLayerData->transactionLabels->prev = transLabel;
				transLabel->next = pDriver->transactionLayerData->transactionLabels;
			}

			pDriver->transactionLayerData->transactionLabels = transLabel;

			transLabel->labelUsed[transLabel->labelIndex] = 1;
			*label = transLabel->labelIndex;

			retVal = FWSuccess;
		}
	}

	/* End critical section */
	semGive( pDriver->transactionLayerData->transSemId );

	return retVal;
}

static FWStatus fwQueueTxRequestTransaction(FWDriverData *pDriver, FWTransaction *transaction)
{
	FWStatus retVal = FWInternalError;

	do
	{
		/* Queue the tranaction by adding it to the end of the request TX list. */
		retVal = fwTransactionListPushBack( pDriver->transactionLayerData->outgoingRequestList, transaction );

		if( pDriver->transactionLayerData->outgoingRequestList->count == 1 )
		{
			/* Since this transaction is first transaction on the list, notify
			 * the bus manager that new trans actions are ready to be
			 * processed.
			 */
			fwUpdateRequestTXStatus( pDriver, TRUE );
			retVal = fwNotifyBusManager();
		}

	} while(0);

	return retVal;
}

int fwMatchResponseWithRequest(const FWTransaction *cmp1, const FWTransaction *cmp2)
{
	int retVal = -1;

	if( cmp1->transactionLabel == cmp2->transactionLabel && cmp1->sourceID == cmp2->destinationID )
	{
		retVal = 0;
	}

	return retVal;
}

static FWStatus fwProcessAsyncIndication(FWDriverData *pDriver, FWTransaction *transaction)
{
	FWStatus retVal = FWInternalError;
	FWResponseCode responseCode = FWResponseTypeError;
	FWTransaction *responseTrans = NULL;
	unsigned long allocateMin;

	do
	{
		/* Allocate a response transaction */

		/* TODO: Pre-allocate a set of transactions to be used for responses.
		 * Also update the fwCompleteAsyncResponseTransaction function to
		 * release the transaction when done.
		 */

		responseTrans = (FWTransaction*)fwMalloc( sizeof(FWTransaction) );

		if( responseTrans == NULL )
		{
			retVal = FWMemAllocateError;
			break;
		}

		*responseTrans = *transaction;

		responseTrans->databuffer = NULL;
		responseTrans->dataBufferSize = 0;
		responseTrans->dataLength = 0;
		responseTrans->destinationID = transaction->sourceID;
		responseTrans->responseCode = responseCode;
		responseTrans->next = NULL;
		responseTrans->prev = NULL;
		responseTrans->status = FWQueuedPending;
		responseTrans->transactionID = fwGetTransactionID();
		responseTrans->xferStatus = 0;

		allocateMin = transaction->dataLength;

		if( transaction->dataLength < 4 )
		{
			allocateMin = 4;
		}

		/* Allocate response buffer */
		responseTrans->databuffer = (unsigned char*)fwMalloc( allocateMin );

		if( responseTrans->databuffer == NULL )
		{
			/* Clean up the transaction */
			fwFree( responseTrans );
			retVal = FWMemAllocateError;
			break;
		}

		responseTrans->dataBufferSize = allocateMin;
		responseTrans->dataLength = 0;

		/* Determine if this is a standard manager CSR */
		retVal = fwValidateManagerCSR( pDriver, transaction );

      if( retVal == FWNotFound )
		{
			/* Check if this is an application specific address
			 * and respond appropriately.
			 */
			retVal = fwAppSpecificAsyncIndication( pDriver, transaction, responseTrans );
		}
		else if( retVal == FWSuccess )
		{
			/* respond to the standard CSR request. */
			retVal = fwFormStandardAsyncResponse( pDriver, transaction, responseTrans );
		}

		if( retVal == FWSuccess )
		{
			responseTrans->responseCode = FWResponseComplete;
		}
		else if( retVal == FWAddressError )
		{
			responseTrans->responseCode = FWResponseAddressError;
			responseTrans->dataLength = 0;
		}
		else if( retVal == FWTypeError )
		{
			responseTrans->responseCode = FWResponseTypeError;
			responseTrans->dataLength = 0;
		}
		else
		{
			FWLOGLEVEL3("Async request for CSR %0x04X %08X returned error %d.\n",
							transaction->destinationOffset.highOffset,
							transaction->destinationOffset.lowOffset,
							retVal );

			fwFree( responseTrans->databuffer );
			fwFree( responseTrans );
			break;
		}

		/* If the CSR does not map to a valid address and this is not a
		 * unified transaction, a response is sent with FWResponseAddressError.
		 * Otherwise, no response is sent. See IEEE-1394 standard section
		 * 7.3.3.2.2
		 */
		if( responseTrans->responseCode == FWResponseAddressError && (transaction->xferStatus & 0x0000001F) != FWAckPending )
		{
			FWLOGLEVEL7("Response aborted for unified transaction. Transaction ID:%d\n", transaction->transactionID );
			fwFree( responseTrans->databuffer );
			fwFree( responseTrans );
			break;
		}
		retVal = fwPostAsyncResponse( pDriver, responseTrans );

	} while(0);

	return retVal;
}

/*	Process an async CSR request to the local node from the
 * local node. This function handles requests that
 * do not require any actual transmission of data
 * across the IEEE-1394 bus.
 */
static FWStatus fwLocalRequest(FWDriverData *pDriver, FWTransaction *transaction)
{
	FWStatus retVal = FWInternalError;
	FWResponseCode responseCode = FWResponseTypeError;
	FWTransaction *responseTrans = NULL;

	FWLOGLEVEL9("Local async request for CSR 0x%04X %08X.\n",
					transaction->destinationOffset.highOffset,
					transaction->destinationOffset.lowOffset );
	do
	{
		/* Determine if this is a standard manager CSR */
		retVal = fwValidateManagerCSR( pDriver, transaction );

		if( retVal == FWNotFound )
		{
			/* Check if this is an application specific address
			 * and respond appropriately.
			 */
			retVal = fwAppSpecificAsyncIndication( pDriver, transaction, transaction );
		}
		else if( retVal == FWSuccess )
		{
			/* respond to the standard CSR request. */
			retVal = fwFormStandardAsyncResponse( pDriver, transaction, transaction );
		}

		if( retVal == FWSuccess )
		{
			transaction->responseCode = FWResponseComplete;
		}
		else if( retVal == FWAddressError )
		{
			transaction->responseCode = FWResponseAddressError;
			transaction->dataLength = 0;
			retVal = FWSuccess;
		}
		else if( retVal == FWTypeError )
		{
			transaction->responseCode = FWResponseTypeError;
			transaction->dataLength = 0;
			retVal = FWSuccess;
		}
		else
		{
			FWLOGLEVEL3("Async request for CSR %0x04X %08X returned error %d.\n",
							transaction->destinationOffset.highOffset,
							transaction->destinationOffset.lowOffset,
							retVal );
			break;
		}

	} while(0);

	return retVal;
}

static FWStatus fwFormStandardAsyncResponse(FWDriverData *pDriver, FWTransaction *transaction, FWTransaction *responseTrans)
{
	FWStatus retVal = FWInternalError;
	UINT32	*dataValue;
	UINT32	*argValue;
	UINT32	*newValue;
	INT64		*data64;
	INT64		*value64;

	do
	{
		/* Perform the action and queue the response */
		switch( transaction->transactionCode )
		{
		case FWWriteRequestQuadlet:
		case FWWriteRequestBlock:
			responseTrans->transactionCode = FWWriteResponse;

			/* Write the manager CSRs */
			retVal = fwWriteManagerCSR( pDriver, transaction );

			/* The response to write transactions does have any data. */
			responseTrans->dataLength = 0;
			break;

		case FWReadRequestQuadlet:
		case FWReadRequestBlock:
			if( transaction->transactionCode == FWReadRequestBlock )
			{
				responseTrans->transactionCode = FWReadResponseBlock;
			}
			else
			{
				responseTrans->transactionCode = FWReadResponseQuadlet;
			}

			if( transaction->dataLength > responseTrans->dataBufferSize )
			{
				FWLOGLEVEL3("Requested data size of %d is greater than the buffer size of %d bytes.\n",
								transaction->dataLength, responseTrans->dataBufferSize );
				retVal = FWTransDataSizeError;
				break;
			}

			/* Read the manager CSRs */
			retVal = fwReadManagerCSR( pDriver, transaction, (UINT32*)responseTrans->databuffer );

			if( retVal == FWSuccess )
			{
				responseTrans->dataLength = transaction->dataLength;
			}
			break;

		case FWLockRequest:
			responseTrans->transactionCode = FWLockResponse;
			if( transaction->extendedCode == FWCompareSwap )
			{
				if( transaction->dataLength == 8 )
				{
					retVal = fwLockManagerCSR( pDriver, transaction, (UINT32*)responseTrans->databuffer );

					if( retVal == FWSuccess )
					{
						responseTrans->dataLength = 4;
					}
				}
				else
				{
					retVal = FWTypeError;
				}
			}
			break;

		case FWWriteResponse:
		case FWReadResponseQuadlet:
		case FWReadResponseBlock:
		case FWCycleStart:
		case FWIsoDataBlock:
		case FWLockResponse:
		case FWPhysicalPacket:
		case FWInvalidTransCode:
			retVal = FWInvalidTransactionType;
			break;
		}

	} while(0);

	return retVal;
}

static FWStatus fwAppSpecificAsyncIndication(FWDriverData *pDriver, FWTransaction *transaction, FWTransaction *responseTrans)
{
	FWStatus retVal = FWInternalError;
	boolean addressMapLocked = FALSE;
	FWCSRAddressMap *addressMap;
	unsigned char *addrPtr;
	unsigned long offset;
	unsigned char tempBuf[16];
	UINT32	*dataValue;
	UINT32	*argValue;
	UINT32	*newValue;
	INT64		*data64;
	INT64		*value64;

	do
	{
		/* Lock the application specific CSR map */
		retVal = fwLockCSRAddressMap( pDriver );

		if( retVal != FWSuccess )
		{
			break;
		}

		addressMapLocked = TRUE;

		/* Look up the data memory address based on the destination information. */
		addressMap = fwLookupCSRAddress( pDriver, &transaction->destinationOffset, transaction->dataLength );

		if( addressMap == NULL )
		{
			FWLOGLEVEL5("CSR address 0x%04X %08X is not mapped to a valid address.\n",
							transaction->destinationOffset.highOffset,
							transaction->destinationOffset.lowOffset );

			retVal = FWAddressError;
		}

		/* Perform the action and update the response transaction */
		switch( transaction->transactionCode )
		{
		case FWWriteRequestQuadlet:
		case FWWriteRequestBlock:
			responseTrans->transactionCode = FWWriteResponse;

			if( addressMap )
			{
				if( addressMap->readOnly )
				{
					retVal = FWTypeError;
				}
				else if( addressMap->alignedOnly && (transaction->destinationOffset.lowOffset & 0x00000003) != 0 )
				{
					retVal = FWTypeError;
				}
				else
				{
					/* Compute the address to the data */
					addrPtr = fwValidateCSRMemoryAddress( addressMap, &transaction->destinationOffset, transaction->dataLength );

					if( addrPtr )
					{
						/* Write the data to the memory */
						memcpy( addrPtr, transaction->databuffer, transaction->dataLength );
						retVal = FWSuccess;
					}
					else
						retVal = FWAddressError;
				}
			}
			break;

		case FWReadRequestQuadlet:
			responseTrans->transactionCode = FWReadResponseQuadlet;

			if( addressMap )
			{
				if( addressMap->alignedOnly && (transaction->destinationOffset.lowOffset & 0x00000003) != 0 )
				{
					retVal = FWTypeError;
				}
				else
				{
					if( transaction->dataLength != 4 )
					{
						FWLOGLEVEL5("Received quadlet read request with data length %d instead of 4.\n", transaction->dataLength );
						retVal = FWTypeError;
					}
					else
					{
						if( responseTrans->dataBufferSize >= 4 )
						{
							responseTrans->dataLength = 4;

							/* Compute the address to the data */
							addrPtr = fwValidateCSRMemoryAddress( addressMap, &transaction->destinationOffset, transaction->dataLength );

							if( addrPtr )
							{
								memcpy( responseTrans->databuffer, addrPtr, responseTrans->dataLength );
								retVal = FWSuccess;
							}
							else
								retVal = FWAddressError;
						}
					}
				}
			}
			break;

		case FWReadRequestBlock:
			responseTrans->transactionCode = FWReadResponseBlock;

			if( addressMap )
			{
				if( addressMap->alignedOnly && (transaction->destinationOffset.lowOffset & 0x00000003) != 0 )
				{
					retVal = FWTypeError;
				}
				else
				{
					if( responseTrans->dataBufferSize >= transaction->dataLength )
					{
						/* Compute the address to the data */
						addrPtr = fwValidateCSRMemoryAddress( addressMap, &transaction->destinationOffset, transaction->dataLength );

						if( addrPtr )
						{
							memcpy( responseTrans->databuffer, addrPtr, transaction->dataLength );
							responseTrans->dataLength = transaction->dataLength;
							retVal = FWSuccess;
						}
						else
							retVal = FWAddressError;
					}
				}
			}
			break;

		case FWLockRequest:
			responseTrans->transactionCode = FWLockResponse;

			if( addressMap )
			{
				if( addressMap->readOnly )
				{
					retVal = FWTypeError;
				}
				else if( addressMap->alignedOnly && (transaction->destinationOffset.lowOffset & 0x00000003) != 0 )
				{
					retVal = FWTypeError;
				}
				else
				{
					if( transaction->dataLength != 4 && transaction->dataLength != 8 && transaction->dataLength != 16 )
					{
						FWLOGLEVEL5("Received lock request with invalid data length %d.\n", transaction->dataLength );
						retVal = FWTypeError;
						break;
					}

					switch( transaction->extendedCode )
					{
					case	FWMaskSwap:
						if( transaction->dataLength == 8 )
						{
							if( responseTrans->dataBufferSize >= 4 )
							{
								responseTrans->dataLength = 4;

								/* Compute the address to the data */
								addrPtr = fwValidateCSRMemoryAddress( addressMap, &transaction->destinationOffset, responseTrans->dataLength );

								if( addrPtr )
								{
									/* Use tempBuf to store the original value since
									 * the transaction and responseTrans pointers can
									 * actually be the same FWTransaction structure.
									 */
									memcpy( tempBuf, addrPtr, responseTrans->dataLength );

									dataValue = (UINT32*)transaction->databuffer;
									argValue = (UINT32*)(transaction->databuffer + 4);
									newValue = (UINT32*)addrPtr;

									*newValue = *dataValue | (*newValue & ~(*argValue));

									memcpy( responseTrans->databuffer, tempBuf, responseTrans->dataLength );

									retVal = FWSuccess;
								}
								else
								{
									retVal = FWAddressError;
								}
							}
							else
							{
								retVal = FWTypeError;
							}
						}
						else if( transaction->dataLength == 16 )
						{
							if( responseTrans->dataBufferSize >= 8 )
							{
								responseTrans->dataLength = 8;

								/* Compute the address to the data */
								addrPtr = fwValidateCSRMemoryAddress( addressMap, &transaction->destinationOffset, responseTrans->dataLength );

								if( addrPtr )
								{
									/* Use tempBuf to store the original value since
									 * the transaction and responseTrans pointers can
									 * actually be the same FWTransaction structure.
									 */
									memcpy( tempBuf, addrPtr, responseTrans->dataLength );

									dataValue = (UINT32*)transaction->databuffer;
									argValue = (UINT32*)(transaction->databuffer + 8);
									newValue = (UINT32*)addrPtr;

									*newValue = *dataValue | (*newValue & ~(*argValue));

									dataValue++;
									argValue++;
									newValue++;

									*newValue = *dataValue | (*newValue & ~(*argValue));

									memcpy( responseTrans->databuffer, tempBuf, responseTrans->dataLength );

									retVal = FWSuccess;
								}
								else
								{
									retVal = FWAddressError;
								}
							}
							else
							{
								retVal = FWTypeError;
							}
						}
						else
						{
							retVal = FWTypeError;
						}
						break;

					case	FWCompareSwap:
						if( transaction->dataLength == 8 )
						{
							if( responseTrans->dataBufferSize >= 4 )
							{
								responseTrans->dataLength = 4;

								/* Compute the address to the data */
								addrPtr = fwValidateCSRMemoryAddress( addressMap, &transaction->destinationOffset, responseTrans->dataLength );

								if( addrPtr )
								{
									/* Use tempBuf to store the original value since
									 * the transaction and responseTrans pointers can
									 * actually be the same FWTransaction structure.
									 */
									memcpy( tempBuf, addrPtr, responseTrans->dataLength );

									dataValue = (UINT32*)transaction->databuffer;
									argValue = (UINT32*)(transaction->databuffer + 4);
									newValue = (UINT32*)addrPtr;

									if( *newValue == *argValue ) *newValue = *dataValue;

									memcpy( responseTrans->databuffer, tempBuf, responseTrans->dataLength );

									retVal = FWSuccess;
								}
								else
								{
									retVal = FWAddressError;
								}
							}
							else
							{
								retVal = FWTypeError;
							}
						}
						else if( transaction->dataLength == 16 )
						{
							if( responseTrans->dataBufferSize >= 8 )
							{
								responseTrans->dataLength = 8;

								/* Compute the address to the data */
								addrPtr = fwValidateCSRMemoryAddress( addressMap, &transaction->destinationOffset, responseTrans->dataLength );

								if( addrPtr )
								{
									/* Use tempBuf to store the original value since
									 * the transaction and responseTrans pointers can
									 * actually be the same FWTransaction structure.
									 */
									memcpy( tempBuf, addrPtr, responseTrans->dataLength );

									dataValue = (UINT32*)transaction->databuffer;
									argValue = (UINT32*)(transaction->databuffer + 8);
									newValue = (UINT32*)addrPtr;

									if( newValue[0] == argValue[0] && newValue[1] == argValue[1] )
									{
										newValue[0] = dataValue[0];
										newValue[1] = dataValue[1];
									}

									memcpy( responseTrans->databuffer, tempBuf, responseTrans->dataLength );

									retVal = FWSuccess;
								}
								else
								{
									retVal = FWAddressError;
								}
							}
							else
							{
								retVal = FWTypeError;
							}
						}
						else
						{
							retVal = FWTypeError;
						}
						break;

					case	FWFetchAdd:
						if( transaction->dataLength == 4 )
						{
							if( responseTrans->dataBufferSize >= 4 )
							{
								responseTrans->dataLength = 4;

								/* Compute the address to the data */
								addrPtr = fwValidateCSRMemoryAddress( addressMap, &transaction->destinationOffset, responseTrans->dataLength );

								if( addrPtr )
								{
									/* Use tempBuf to store the original value since
									 * the transaction and responseTrans pointers can
									 * actually be the same FWTransaction structure.
									 */
									memcpy( tempBuf, addrPtr, responseTrans->dataLength );

									dataValue = (UINT32*)transaction->databuffer;
									newValue = (UINT32*)addrPtr;

									*newValue += *dataValue;
									memcpy( responseTrans->databuffer, tempBuf, responseTrans->dataLength );

									retVal = FWSuccess;
								}
								else
								{
									retVal = FWAddressError;
								}
							}
							else
							{
								retVal = FWTypeError;
							}
						}
						else if( transaction->dataLength == 8 )
						{
							responseTrans->dataLength = 8;

							/* Compute the address to the data */
							addrPtr = fwValidateCSRMemoryAddress( addressMap, &transaction->destinationOffset, responseTrans->dataLength );

							if( addrPtr )
							{
								/* Use tempBuf to store the original value since
								 * the transaction and responseTrans pointers can
								 * actually be the same FWTransaction structure.
								 */
								memcpy( tempBuf, addrPtr, responseTrans->dataLength );

								data64 = (INT64*)transaction->databuffer;
								value64 = (INT64*)addrPtr;

								*value64 += *data64;

								memcpy( responseTrans->databuffer, tempBuf, responseTrans->dataLength );

								retVal = FWSuccess;
							}
							else
							{
								retVal = FWAddressError;
							}
						}
						else
						{
							retVal = FWTypeError;
						}
						break;

					case	FWLittleAdd:
					case	FWBoundedAdd:
					case	FWWrapAdd:
					case	FWVendorDependent:
					case	FWInvalidExtendedCode:
						FWLOGLEVEL5("Received lock request with extended code %d.\n", transaction->extendedCode );
						retVal = FWTypeError;
						break;
					}
				}
			}
			break;

		case FWWriteResponse:
		case FWReadResponseQuadlet:
		case FWReadResponseBlock:
		case FWCycleStart:
		case FWIsoDataBlock:
		case FWLockResponse:
		case FWPhysicalPacket:
		case FWInvalidTransCode:
			retVal = FWInvalidTransactionType;
			break;
		}

	} while(0);

	if( addressMapLocked )
		fwUnlockCSRAddressMap( pDriver );

	return retVal;
}

static FWStatus fwProcessPhyIndication(FWDriverData *pDriver, FWTransaction *transaction)
{
	FWStatus retVal = FWInternalError;
	FWTransaction *responseTrans = NULL;
	FWResponseCode responseCode = FWResponseComplete;
	boolean addressMapLocked = FALSE;
	FWCSRAddressMap *addressMap;
	unsigned char *addrPtr;
	unsigned long offset;
	UINT32	*dataValue;
	UINT32	*argValue;
	UINT32	*newValue;
	INT64		*data64;
	INT64		*value64;

	do
	{
		switch( transaction->transactionCode )
		{
		case FWWriteRequestQuadlet:
		case FWWriteRequestBlock:
		case FWReadRequestQuadlet:
		case FWReadRequestBlock:
		case FWLockRequest:
		case FWWriteResponse:
		case FWReadResponseQuadlet:
		case FWReadResponseBlock:
		case FWCycleStart:
		case FWIsoDataBlock:
		case FWLockResponse:
		case FWPhysicalPacket:
		case FWInvalidTransCode:
			retVal = FWInvalidTransactionType;
			break;
		}

	} while(0);

	if( addressMapLocked )
		fwUnlockCSRAddressMap( pDriver );

	return retVal;
}

static FWStatus fwReadManagerCSR(FWDriverData *pDriver, FWTransaction *transaction, UINT32 *dataBuffer)
{
	FWStatus retVal = FWSuccess;
	int index = 0;
	UINT32 offset;
	UINT32 upperBound;
	UINT32 tempValue;

	offset = transaction->destinationOffset.lowOffset;
	upperBound = offset + transaction->dataLength;

	do
	{
		while( retVal == FWSuccess && offset < upperBound )
		{
			switch( offset )
			{
			case 0xF0000000:	/* State clear CSR */
			case 0xF0000004:
				retVal = fwGetStateCSR( pDriver, &dataBuffer[index] );
				break;

			case 0xF0000008:	/* Node ID CSR */
				retVal = fwGetNodeIDCSR( pDriver, &dataBuffer[index] );
				break;

			case 0xF000000C:	/* Reset start CSR */
				retVal = fwGetResetStartCSR( pDriver, &dataBuffer[index] );
				break;

			case 0xF0000018:	/* Split timeout HI CSR */
				retVal = fwGetSplitTimeoutCSR( pDriver, &dataBuffer[index], &tempValue );
				break;

			case 0xF000001C:	/* Split timeout LO CSR */
				retVal = fwGetSplitTimeoutCSR( pDriver, &tempValue, &dataBuffer[index] );
				break;

			case 0xF0000200:	/* Cycle time CSR */
				retVal = fwGetCycleTimeCSR( pDriver, &dataBuffer[index] );
				break;

			case 0xF0000204:	/* Bus time CSR */
				retVal = fwGetBusTimeCSR( pDriver, &dataBuffer[index] );
				break;

			case 0xF0000210:	/* Busy timeout CSR */
				retVal = fwGetBusyTimeoutCSR( pDriver, &dataBuffer[index] );
				break;

			case 0xF0000214:	/* Priority Budget CSR */
				retVal = fwGetPriorityBudgetCSR( pDriver, &dataBuffer[index] );
				break;

			case 0xF000021C:	/* Bus Manager ID CSR */
				retVal = fwGetBusMgrIdCSR( pDriver, &dataBuffer[index] );
				break;

			case 0xF0000220:	/* Bandwidth available CSR */
				retVal = fwGetBandwidthAvailableCSR( pDriver, &dataBuffer[index] );
				break;

			case 0xF0000224:	/* Channel available Hi CSR */
				retVal = fwGetChannelsAvailableHiCSR( pDriver, &dataBuffer[index] );
				break;

			case 0xF0000228:	/* Channel available Lo CSR */
				retVal = fwGetChannelsAvailableLoCSR( pDriver, &dataBuffer[index] );
				break;

			case 0xF0000234:	/* Broadcast channel CSR */
				retVal = fwGetBroadcastChannelCSR( pDriver, &dataBuffer[index] );
				break;

			default:
				retVal = FWAddressError;
				break;
			}

			if( offset >= 0xF0001000 && offset <= 0xF00013FC )
			{
				retVal = fwGetTopologyMapData( pDriver, offset, &dataBuffer[index] );
			}

			offset += 4;
			index++;
		}

	} while(0);


	return retVal;
}

static FWStatus fwWriteManagerCSR(FWDriverData *pDriver, FWTransaction *transaction)
{
	FWStatus retVal = FWNotFound;
	int index = 0;
	UINT32 offset;
	UINT32 upperBound;
	UINT32 splitLo;
	UINT32 splitHi;
	UINT32 *dataBuffer;

	offset = transaction->destinationOffset.lowOffset;
	upperBound = offset + transaction->dataLength;
	dataBuffer = (UINT32*)transaction->databuffer;

	do
	{
		retVal = fwGetSplitTimeoutCSR( pDriver, &splitHi, &splitLo );

		if( retVal != FWSuccess )
		{
			break;
		}

		while( retVal == FWSuccess && offset < upperBound )
		{
			switch( offset )
			{
			case 0xF0000000:	/* State clear CSR */
				retVal = fwSetStateClearCSR( pDriver, dataBuffer[index] );
				break;

			case 0xF0000004:	/* State set CSR */
				retVal = fwSetStateSetCSR( pDriver, dataBuffer[index] );
				break;

			case 0xF0000008:	/* Node ID CSR */
				retVal = fwSetNodeIDCSR( pDriver, dataBuffer[index] );
				break;

			case 0xF000000C:	/* Reset start CSR */
				retVal = fwSetResetStartCSR( pDriver, dataBuffer[index] );
				break;

			case 0xF0000018:	/* Split timeout HI CSR */
				splitHi = dataBuffer[index];
				break;

			case 0xF000001C:	/* Split timeout LO CSR */
				splitLo = dataBuffer[index];
				break;

			case 0xF0000200:	/* Cycle time CSR */
				retVal = fwSetCycleTimeCSR( pDriver, dataBuffer[index] );
				break;

			case 0xF0000204:	/* Bus time CSR */
				retVal = fwSetBusTimeCSR( pDriver, dataBuffer[index] );
				break;

			case 0xF0000210:	/* Busy timeout CSR */
				retVal = fwSetBusyTimeoutCSR( pDriver, dataBuffer[index] );
				break;

			case 0xF0000214:	/* Priority Budget CSR */
				retVal = fwSetPriorityBudgetCSR( pDriver, dataBuffer[index] );
				break;

			case 0xF0000234:	/* Broadcast channel CSR */
				retVal = fwSetBroadcastChannelCSR( pDriver, dataBuffer[index] );
				break;

			default:
				retVal = FWAddressError;
				break;
			}

			if( offset >= 0xF0001000 && offset <= 0xF00013FC )
			{
				retVal = FWTypeError;
			}

			offset += 4;
			index++;
		}

		if( retVal == FWSuccess )
			retVal = fwSetSplitTimeoutCSR( pDriver, splitHi, splitLo );

	} while(0);

	return retVal;
}

static FWStatus fwLockManagerCSR(FWDriverData *pDriver, FWTransaction *transaction, UINT32 *dataBuffer)
{
	FWStatus retVal = FWTypeError;
	UINT32 *buffer;
	UINT32 compare;
	UINT32 swap;

	buffer = (UINT32*)transaction->databuffer;

	do
	{
		if( transaction->dataLength != 8 )
		{
			retVal = FWTypeError;
			break;
		}

		compare = buffer[0];
		swap = buffer[1];

		/* Note that only quadlet compare swap is supported for the
		 * standard CSR lock operations.
		 */
		switch( transaction->destinationOffset.lowOffset )
		{
		case 0xF000021C:	/* Bus Manager ID CSR */
			retVal = fwSetBusMgrIdCSR( pDriver, compare, &swap );
			*dataBuffer = swap;
			break;

		case 0xF0000220:	/* Bandwidth available CSR */
			retVal = fwSetBandwidthAvailableCSR( pDriver, compare, &swap );
			*dataBuffer = swap;
			break;

		case 0xF0000224:	/* Channel available Hi CSR */
			retVal = fwSetChannelsAvailableHiCSR( pDriver, compare, &swap );
			*dataBuffer = swap;
			break;

		case 0xF0000228:	/* Channel available Lo CSR */
			retVal = fwSetChannelsAvailableLoCSR( pDriver, compare, &swap );
			*dataBuffer = swap;
			break;

		default:
			retVal = FWAddressError;
			break;
		}

	} while(0);

	return retVal;
}

static FWStatus fwValidateManagerCSR(FWDriverData *pDriver, FWTransaction *transaction)
{
	FWStatus retVal = FWNotFound;
	unsigned long len;
	UINT32 offset;
	UINT32 upperBound;

	offset = transaction->destinationOffset.lowOffset & ~3;
	len = transaction->dataLength & ~3;

	do
	{
		/* Determine if the offset is for a standard CSR
		 * and that it is quadlet aligned.
		 */
		if( transaction->destinationOffset.highOffset != 0xFFFF )
		{
			break;
		}

		/* Determine if this is within the standard CSR range. Note that
		 * the bus manager ID, isochronous channels,and bandwidth are handled
		 * by the hardware.
		 */
		if( transaction->destinationOffset.lowOffset < 0xF0000000 || transaction->destinationOffset.lowOffset > 0xF00013FC )
		{
			break;
		}

		/* Access must be made on quadlet aligned boundries */
		if( offset != transaction->destinationOffset.lowOffset )
		{
			/* Request is not aligned */
			retVal = FWTypeError;
			break;
		}

		/* Only full quadlet request are supported */
		if( len != transaction->dataLength )
		{
			/* Request is not aligned */
			retVal = FWTypeError;
			break;
		}

		if( !pDriver->busManagerData->isBusManager && transaction->destinationOffset.lowOffset > 0xF0000234 )
		{
			/* If we are not the bus manager, access to the
			 * topology map is not allowed.
			 */
			retVal = FWAddressError;
			break;
		}

		/* Scan for unimplemented CSRs within the valid range and
		 * invalid access methods.
		 */

		retVal = FWSuccess;
		upperBound = offset + len;

		while( retVal == FWSuccess && offset < upperBound )
		{
			if( offset == 0xF0000000 || offset == 0xF0000004 || offset == 0xF0000008 || offset == 0xF000000C )
			{
				/* State set/clear CSR */
				/* Node ID CSR */
				/* Reset start CSR */
				switch( transaction->transactionCode )
				{
				case FWWriteRequestQuadlet:
				case FWReadRequestQuadlet:
				case FWReadRequestBlock:
					break;

				case FWWriteRequestBlock:
				case FWLockRequest:
				case FWWriteResponse:
				case FWReadResponseQuadlet:
				case FWReadResponseBlock:
				case FWCycleStart:
				case FWIsoDataBlock:
				case FWLockResponse:
				case FWPhysicalPacket:
				case FWInvalidTransCode:
				default:
					retVal = FWTypeError;
				}
			}
			else if( offset == 0xF0000018 || offset == 0xF000001C || offset == 0xF0000200 || offset == 0xF0000204 || offset == 0xF0000210 || offset == 0xF0000214 || offset == 0xF0000234 )
			{
				/* Split timeout HI/LO CSR */
				/* Cycle time CSR */
				/* Bus time CSR */
				/* Busy timeout CSR */
				/* Priority Budget CSR */
				/* Broadcast channel CSR */
				switch( transaction->transactionCode )
				{
				case FWWriteRequestQuadlet:
				case FWReadRequestQuadlet:
				case FWReadRequestBlock:
				case FWWriteRequestBlock:
					break;

				case FWLockRequest:
				case FWWriteResponse:
				case FWReadResponseQuadlet:
				case FWReadResponseBlock:
				case FWCycleStart:
				case FWIsoDataBlock:
				case FWLockResponse:
				case FWPhysicalPacket:
				case FWInvalidTransCode:
				default:
					retVal = FWTypeError;
				}
			}
			else if( offset == 0xF000021C || offset == 0xF0000220 || offset == 0xF0000224 || offset == 0xF0000228 )
			{
				/* OHCI built in addresses can only be accessed via quadlet lock and quadlet read. */
				if( transaction->dataLength == 8 )
				{
					switch( transaction->transactionCode )
					{
					case FWLockRequest:
					case FWReadRequestQuadlet:
						break;

					case FWReadRequestBlock:
					case FWWriteRequestBlock:
					case FWWriteRequestQuadlet:
					case FWWriteResponse:
					case FWReadResponseQuadlet:
					case FWReadResponseBlock:
					case FWCycleStart:
					case FWIsoDataBlock:
					case FWLockResponse:
					case FWPhysicalPacket:
					case FWInvalidTransCode:
					default:
						retVal = FWTypeError;
					}
				}
				else
				{
					retVal = FWTypeError;
				}
			}
			else if( offset >= 0xF0001000 && offset <= 0xF00013FC )
			{
				if( pDriver->busManagerData->isBusManager )
				{
					switch( transaction->transactionCode )
					{
					case FWReadRequestQuadlet:
					case FWReadRequestBlock:
						break;

					case FWWriteRequestBlock:
					case FWWriteRequestQuadlet:
					case FWLockRequest:
					case FWWriteResponse:
					case FWReadResponseQuadlet:
					case FWReadResponseBlock:
					case FWCycleStart:
					case FWIsoDataBlock:
					case FWLockResponse:
					case FWPhysicalPacket:
					case FWInvalidTransCode:
					default:
						retVal = FWTypeError;
					}
				}
				else
				{
					retVal = FWAddressError;
				}
			}
			else
			{
				retVal = FWAddressError;
				break;
			}

			if( retVal != FWSuccess )
			{
				FWLOGLEVEL6("Invalid transaction type request to address 0x%X %08X. Transaction Code:%d.\n",
								transaction->destinationOffset.highOffset,
								offset,
								transaction->transactionCode );
				break;
			}

			offset += 4;
		}

	} while(0);

	return retVal;
}



