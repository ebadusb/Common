/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header: H:/BCT_Development/vxWorks/Common/firewire/rcs/fw_driver.c 1.2 2007/02/12 16:06:59Z wms10235 Exp wms10235 $
 *
 * This file contains the firewire driver level routines.
 *
 * $Log: fw_driver.c $
 *
 */

#include <vxWorks.h>
#include <stdio.h>
#include <taskLib.h>
#include "fw_utility.h"
#include "fw_config_rom.h"
#include "fw_csr.h"
#include "fw_physical_layer.h"
#include "fw_link_layer.h"
#include "fw_transaction_layer.h"
#include "fw_driver.h"
#include "fw_ohci_hw.h"
#include "fw_client.h"
#include "fw_isochronous.h"

/* Pointer array for firewire driver data */
FWDriverData *fwDriverDataArray[FIREWIRE_MAX_PCI_ADAPTERS];

/* Local function prototypes */
static FWStatus fwCompleteTransaction(FWDriverData *pDriver, FWTransaction *transaction);
static FWStatus fwGetClientSemaphore(FWDriverData *pDriver, int clientHandle, SEM_ID *clientSem);

FWStatus fwCreateDriverData(FWDriverData *pDriver)
{
	FWStatus retVal = FWNotInitialized;

	do
	{
		if( !pDriver )
		{
			break;
		}

		/* Initialize the driver */
		pDriver->dmaMemoryPool = NULL;

		/* Create the config ROM */
		retVal = fwCreateConfigROM( pDriver );
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not create config ROM. Error:%d\n", retVal );
			break;
		}

		/* Create the core CSR area */
		retVal = fwCreateCoreCSR( pDriver );
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not create core CSRs. Error:%d\n", retVal );
			break;
		}

		/* Create the physical layer data structures */
		retVal = fwCreatePhysicalLayer( pDriver );
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not create the physical layer. Error:%d\n", retVal );
			break;
		}

		/* Create the link layer data structures */
		retVal = fwCreateLinkLayer( pDriver );
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not create the link layer. Error:%d\n", retVal );
			break;
		}

		/* Create the transaction layer data structures */
		retVal = fwCreateTransactionLayer( pDriver );
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not create the transaction layer. Error:%d\n", retVal );
			break;
		}

		/* Create the isochronous channels */
		retVal = fwCreateIsochronousLayer( pDriver );
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not create the isochronous layer. Error:%d\n", retVal );
			break;
		}

	} while(0);

	return retVal;
}

FWStatus fwDestroyDriverData(FWDriverData *pDriver)
{
	FWStatus retVal = FWNotInitialized;
	FWDmaMemoryPool *pDmaPool;
	FWDmaMemoryPool *pTempPool;

	do
	{
		if( !pDriver )
		{
			break;
		}

		/* Destroy the transaction layer data structures */
		retVal = fwDestroyIsochronousLayer( pDriver );
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not destroy the isochronous layer. Error:%d\n", retVal );
		}

		/* Destroy the transaction layer data structures */
		retVal = fwDestroyTransactionLayer( pDriver );
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not destroy the transaction layer. Error:%d\n", retVal );
		}

		/* Destroy the link layer data structures */
		retVal = fwDestroyLinkLayer( pDriver );
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not destroy the link layer. Error:%d\n", retVal );
		}

		/* Destroy the physical layer data structures */
		retVal = fwDestroyPhysicalLayer( pDriver );
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not destroy the physical layer. Error:%d\n", retVal );
		}

		retVal = fwDestroyConfigROM( pDriver );
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not destroy the config ROM. Error:%d\n", retVal );
		}

		/* Destroy the core CSR area */
		retVal = fwDestroyCoreCSR( pDriver );
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not destroy the CSR core. Error:%d\n", retVal );
		}

		pDmaPool = pDriver->dmaMemoryPool;
		while( pDmaPool )
		{
			pTempPool = pDmaPool->next;
			if( pDmaPool->startPtr )
				fwFreeDMA( pDmaPool->startPtr );
			fwFree( pDmaPool );
			pDmaPool = pTempPool;
		}

	} while(0);

	return retVal;
}

FWStatus fwReset(int adapter)
{
	FWStatus retVal = FWNotInitialized;
	int adapterCount = fwGetAdapterCount();

	do
	{
		if( adapter < 0 || adapter >= adapterCount )
		{
			retVal = FWInvalidAdapter;
			break;
		}

		retVal = fwResetLink( fwDriverDataArray[adapter] );

	} while(0);

	return retVal;
}

FWStatus fwInitialize(int adapter)
{
	FWStatus retVal = FWNotInitialized;
	int adapterCount = fwGetAdapterCount();

	do
	{
		if( adapter < 0 || adapter >= adapterCount )
		{
			retVal = FWInvalidAdapter;
			break;
		}

		/* Initialize the physical layer */
		retVal = fwInitializePhysical( fwDriverDataArray[adapter] );
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not initialize physical layer. Error:%d\n", retVal );
			break;
		}

		/* TODO: Set physical layer options */
		retVal = fwSetContender( fwDriverDataArray[adapter], FALSE );
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not set contender bit. Error:%d\n", retVal );
			break;
		}

		retVal = fwInitializeCoreCSR( fwDriverDataArray[adapter] );
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not initialize core CSR area. Error:%d\n", retVal );
			break;
		}

		retVal = fwInitializeConfigROM( fwDriverDataArray[adapter] );
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not initialize configuration ROM area. Error:%d\n", retVal );
			break;
		}

		retVal = fwInitializeLink( fwDriverDataArray[adapter] );
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not initialize link layer. Error:%d\n", retVal );
			break;
		}

		retVal = fwInitializeTransaction( fwDriverDataArray[adapter] );
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not initialize transaction layer. Error:%d\n", retVal );
			break;
		}

	} while(0);

	return retVal;
}

FWStatus fwAsyncWrite(int adapter, const FWAsyncTransactionCmd *asyncCmd, int timeout)
{
	FWStatus retVal = FWInternalError;
	int adapterCount = fwGetAdapterCount();
	FWClientResource *clientResource = NULL;
	FWDriverData *pDriver = NULL;
	STATUS errStatus = ERROR;

	do
	{
		/* Find the adapter */
		if( adapter < 0 || adapter >= adapterCount )
		{
			retVal = FWInvalidAdapter;
			break;
		}

		pDriver = fwDriverDataArray[adapter];

		/* Obtain the client's resource structure */
		retVal = fwGetClientResource( asyncCmd->clientHandle, &clientResource );

		if( retVal != FWSuccess )
		{
			break;
		}

		/* Check if there is data to send/write. */
		if( asyncCmd->dataLength == 0 )
		{
			retVal = FWNoTransactionData;
			break;
		}

		/* Obtain the client's send transaction structure */

		/* TODO: If the size is greater than the max_rec of
		 * this node or the remote node, the operation may need
		 * to be broken down into multiple transactions.
		 */

		/* Fill in the transaction structure and add it to the pending queue */
		retVal = fwInitializeAsyncRequestTransaction( pDriver, asyncCmd, clientResource->asyncSendTrans );

		if( retVal != FWSuccess )
		{
			break;
		}

		clientResource->asyncSendTrans->semId = clientResource->clientSem;

		retVal = fwPostAsyncWriteRequest( pDriver, clientResource->asyncSendTrans );

		if( retVal != FWSuccess )
		{
			break;
		}

		/* Wait for the transaction to complete */
		errStatus = semTake( clientResource->clientSem, timeout );

		if( errStatus == OK )
		{
			/* The transaction is complete. Remove it from the waiting list. */
			retVal = fwRemoveTransaction( pDriver, clientResource->asyncSendTrans );

			if( retVal == FWSuccess )
			{
				retVal = clientResource->asyncSendTrans->status;

				if( retVal == FWSuccess )
				{
					switch( clientResource->asyncSendTrans->responseCode )
					{
					case	FWResponseComplete:
						break;
					case	FWResponseConflictError:
						retVal = FWResponseError;
						break;
					case	FWResponseDataError:
						retVal = FWRequestDataError;
						break;
					case	FWResponseTypeError:
						retVal = FWTypeError;
						break;
					case	FWResponseAddressError:
						retVal = FWAddressError;
						break;
					case	FWInvalidResponseCode:
						retVal = FWResponseFormatError;
						break;
					}
				}
			}
		}
		else
		{
			/* The transaction timed out. Cancel the transaction. */
			retVal = fwCancelTransaction( pDriver, clientResource->asyncSendTrans );
		}

	} while(0);

	return retVal;
}

FWStatus fwAsyncRead(int adapter, FWAsyncTransactionCmd *asyncCmd, int timeout)
{
	FWStatus retVal = FWInternalError;
	int adapterCount = fwGetAdapterCount();
	FWClientResource *clientResource = NULL;
	FWDriverData *pDriver = NULL;
	STATUS errStatus = ERROR;

	do
	{
		/* Find the adapter */
		if( adapter < 0 || adapter >= adapterCount )
		{
			retVal = FWInvalidAdapter;
			break;
		}

		pDriver = fwDriverDataArray[adapter];

		/* Obtain the client's resource structure */
		retVal = fwGetClientResource( asyncCmd->clientHandle, &clientResource );

		if( retVal != FWSuccess )
		{
			break;
		}

		/* Check if there is data to send/write. */
		if( asyncCmd->dataLength == 0 )
		{
			retVal = FWNoTransactionData;
			break;
		}

		/* Obtain the client's send transaction structure */

		/* TODO: If the size is greater than the max_rec of
		 * this node or the remote node, the operation may need
		 * to be broken down into multiple transactions.
		 */

		/* Fill in the transaction structure and add it to the pending queue */
		retVal = fwInitializeAsyncRequestTransaction( pDriver, asyncCmd, clientResource->asyncSendTrans );

		if( retVal != FWSuccess )
		{
			break;
		}

		clientResource->asyncSendTrans->semId = clientResource->clientSem;

		retVal = fwPostAsyncReadRequest( pDriver, clientResource->asyncSendTrans );

		if( retVal != FWSuccess )
		{
			break;
		}

		/* Wait for the transaction to complete */
		errStatus = semTake( clientResource->clientSem, timeout );

		if( errStatus == OK )
		{
			/* The transaction is complete. Remove it from the waiting list. */
			retVal = fwRemoveTransaction( pDriver, clientResource->asyncSendTrans );

			asyncCmd->dataLength = clientResource->asyncSendTrans->dataLength;
			asyncCmd->speed = clientResource->asyncSendTrans->speed;

			if( retVal == FWSuccess )
			{
				retVal = clientResource->asyncSendTrans->status;

				if( retVal == FWSuccess )
				{
					switch( clientResource->asyncSendTrans->responseCode )
					{
					case	FWResponseComplete:
						break;
					case	FWResponseConflictError:
						retVal = FWResponseError;
						break;
					case	FWResponseDataError:
						retVal = FWRequestDataError;
						break;
					case	FWResponseTypeError:
						retVal = FWTypeError;
						break;
					case	FWResponseAddressError:
						retVal = FWAddressError;
						break;
					case	FWInvalidResponseCode:
						retVal = FWResponseFormatError;
						break;
					}
				}
			}
		}
		else
		{
			/* The transaction timed out. Cancel the transaction. */
			retVal = fwCancelTransaction( pDriver, clientResource->asyncSendTrans );
			retVal = FWTimeout;
		}

	} while(0);

	return retVal;
}

FWStatus fwCompareSwap(int adapter, FWAsyncTransactionCmd *asyncCmd, int timeout)
{
	FWStatus retVal = FWInternalError;
	int adapterCount = fwGetAdapterCount();
	FWClientResource *clientResource = NULL;
	FWDriverData *pDriver = NULL;
	STATUS errStatus = ERROR;

	do
	{
		/* Find the adapter */
		if( adapter < 0 || adapter >= adapterCount )
		{
			retVal = FWInvalidAdapter;
			break;
		}

		pDriver = fwDriverDataArray[adapter];

		/* Obtain the client's resource structure */
		retVal = fwGetClientResource( asyncCmd->clientHandle, &clientResource );

		if( retVal != FWSuccess )
		{
			break;
		}

		/* Check if there is the correct data size. */
		if( asyncCmd->dataLength != 8 )
		{
			retVal = FWTransDataSizeError;
			break;
		}

		/* Obtain the client's send transaction structure */

		/* Fill in the transaction structure and add it to the pending queue */
		retVal = fwInitializeAsyncRequestTransaction( pDriver, asyncCmd, clientResource->asyncSendTrans );

		if( retVal != FWSuccess )
		{
			break;
		}

		clientResource->asyncSendTrans->semId = clientResource->clientSem;
		clientResource->asyncSendTrans->extendedCode = FWCompareSwap;

		retVal = fwPostAsyncLockRequest( pDriver, clientResource->asyncSendTrans );

		if( retVal != FWSuccess )
		{
			break;
		}

		/* Wait for the transaction to complete */
		errStatus = semTake( clientResource->clientSem, timeout );

		if( errStatus == OK )
		{
			/* The transaction is complete. Remove it from the waiting list. */
			retVal = fwRemoveTransaction( pDriver, clientResource->asyncSendTrans );

			asyncCmd->dataLength = clientResource->asyncSendTrans->dataLength;
			asyncCmd->speed = clientResource->asyncSendTrans->speed;

			if( retVal == FWSuccess )
			{
				retVal = clientResource->asyncSendTrans->status;

				if( retVal == FWSuccess )
				{
					switch( clientResource->asyncSendTrans->responseCode )
					{
					case	FWResponseComplete:
						break;
					case	FWResponseConflictError:
						retVal = FWResponseError;
						break;
					case	FWResponseDataError:
						retVal = FWRequestDataError;
						break;
					case	FWResponseTypeError:
						retVal = FWTypeError;
						break;
					case	FWResponseAddressError:
						retVal = FWAddressError;
						break;
					case	FWInvalidResponseCode:
						retVal = FWResponseFormatError;
						break;
					}
				}
			}
		}
		else
		{
			/* The transaction timed out. Cancel the transaction. */
			retVal = fwCancelTransaction( pDriver, clientResource->asyncSendTrans );
			retVal = FWTimeout;
		}

	} while(0);

	return retVal;
}

FWStatus fwEnableAdapter(int adapter)
{
	FWStatus retVal = FWInternalError;
	int adapterCount = fwGetAdapterCount();
	FWDriverData *pDriver = NULL;

	do
	{
		/* Find the adapter */
		if( adapter < 0 || adapter >= adapterCount )
		{
			retVal = FWInvalidAdapter;
			break;
		}

		pDriver = fwDriverDataArray[adapter];

		retVal = fwSetContender( pDriver, TRUE );
		if( retVal != FWSuccess )
		{
			FWLOGLEVEL3("Could not set contender bit. Error:%d\n", retVal );
			break;
		}

		retVal = fwEnableLink( pDriver );

		if( retVal == FWSuccess )
		{
			pDriver->adapterEnabled = TRUE;
		}

	} while(0);

	return retVal;
}

FWStatus fwDevicePresent(int adapter, int deviceID)
{
	FWStatus retVal = FWInternalError;

	return retVal;
}

FWStatus fwPhysicalConfig(int adapter, UINT32 phyQuadlet)
{
	FWStatus retVal = FWInternalError;

	return retVal;
}

FWStatus fwLinkOnRequest(int adapter, int deviceID, int timeout)
{
	FWStatus retVal = FWInternalError;

	return retVal;
}

FWStatus fwDevicePowerRequest(int adapter, int deviceID)
{
	FWStatus retVal = FWInternalError;

	return retVal;
}

FWStatus fwRegisterClient(int *clientHandle)
{
	FWStatus retVal = fwCreateClient( clientHandle, NULL );

	return retVal;
}

FWStatus fwUnregisterClient(int clientHandle)
{
	FWStatus retVal = fwDestroyClient( clientHandle );

	return retVal;
}

FWStatus fwIsoOpenChannel(int adapter, int channel, int bandwidth)
{
	FWStatus retVal = FWInternalError;
	int adapterCount = fwGetAdapterCount();

	do
	{
		/* Find the adapter */
		if( adapter < 0 || adapter >= adapterCount )
		{
			retVal = FWInvalidAdapter;
			break;
		}

		retVal = fwIsoAllocateChannel( fwDriverDataArray[adapter], channel, bandwidth );

	} while(0);

	return retVal;
}

FWStatus fwIsoCloseChannel(int adapter, int channel)
{
	FWStatus retVal = FWInternalError;
	int adapterCount = fwGetAdapterCount();

	do
	{
		/* Find the adapter */
		if( adapter < 0 || adapter >= adapterCount )
		{
			retVal = FWInvalidAdapter;
			break;
		}

		retVal = fwIsoReleaseChannel( fwDriverDataArray[adapter], channel );

	} while(0);

	return retVal;
}

FWStatus fwIsoInitReceive(int adapter, FWIsochronousRecvCmd *command)
{
	FWStatus retVal = FWInternalError;
	int adapterCount = fwGetAdapterCount();

	do
	{
		/* Find the adapter */
		if( adapter < 0 || adapter >= adapterCount )
		{
			retVal = FWInvalidAdapter;
			break;
		}

		retVal = fwIsoInitRecvChannel( fwDriverDataArray[adapter], command );

	} while(0);

	return retVal;
}

FWStatus fwIsoStartChannel(int adapter, int channel)
{
	FWStatus retVal = FWInternalError;
	int adapterCount = fwGetAdapterCount();

	do
	{
		/* Find the adapter */
		if( adapter < 0 || adapter >= adapterCount )
		{
			retVal = FWInvalidAdapter;
			break;
		}

		retVal = fwIsoEnableChannel( fwDriverDataArray[adapter], channel );

	} while(0);

	return retVal;
}

FWStatus fwIsoStopChannel(int adapter, int channel)
{
	FWStatus retVal = FWInternalError;
	int adapterCount = fwGetAdapterCount();

	do
	{
		/* Find the adapter */
		if( adapter < 0 || adapter >= adapterCount )
		{
			retVal = FWInvalidAdapter;
			break;
		}

		retVal = fwIsoDisableChannel( fwDriverDataArray[adapter], channel );

	} while(0);

	return retVal;
}

FWStatus fwIsoReadRecvData(int adapter, int channel, unsigned char *pBuffer, unsigned long bufferSize, UINT32 *xferStatus)
{
	FWStatus retVal = FWInternalError;
	int adapterCount = fwGetAdapterCount();

	do
	{
		/* Find the adapter */
		if( adapter < 0 || adapter >= adapterCount )
		{
			retVal = FWInvalidAdapter;
			break;
		}

		if( pBuffer == NULL || bufferSize == 0 || xferStatus == NULL )
		{
			retVal = FWInvalidArg;
			break;
		}

		retVal = fwIsoReadRecvBuffer( fwDriverDataArray[adapter], channel, pBuffer, bufferSize, xferStatus );

	} while(0);

	return retVal;
}


/*
 *  Local functions
 */


