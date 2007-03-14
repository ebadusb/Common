/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header: H:/BCT_Development/vxWorks/Common/firewire/rcs/fw_bus_manager.c 1.4 2007/02/20 22:53:01Z wms10235 Exp wms10235 $
 *
 * This file contains the firewire routines for the Bus Manager.
 *
 * $Log: fw_bus_manager.c $
 * Revision 1.4  2007/02/20 22:53:01Z  wms10235
 * IT74 - Updated some of the logging messages
 * Revision 1.3  2007/02/13 22:46:46Z  wms10235
 * IT74 - Changes from driver unit testing
 * Revision 1.2  2007/02/12 16:06:58Z  wms10235
 * IT74 - Add Firewire driver to common
 * Revision 1.1  2007/02/07 15:22:27Z  wms10235
 * Initial revision
 *
 */

#include <vxWorks.h>
#include <stdio.h>
#include <stdlib.h>
#include <semLib.h>
#include <string.h>
#include <intLib.h>
#include <taskLib.h>
#include <sysLib.h>
#include "fw_utility.h"
#include "fw_physical_layer.h"
#include "fw_link_layer.h"
#include "fw_transaction_layer.h"
#include "fw_bus_manager.h"
#include "fw_pci_support.h"
#include "fw_vendor_specific.h"
#include "fw_driver.h"
#include "fw_ohci_hw.h"
#include "fw_client.h"
#include "fw_csr.h"
#include "fw_isochronous.h"

/* Transaction timeout ticks are in 125us increments */
#define FW_TRANSACTION_TIMEOUT_TICKS	1600
/* Bus manager timeout ticks are in OS (vxWorks) ticks */
#define FW_BUS_MANAGER_TIMEOUT_TICKS	3

/* Module level varaibles */
static int fwBusManagerShutdownFlag = 0;
static SEM_ID fwBusManagerNotifySem = NULL;
int fwBusManagerTimeout = WAIT_FOREVER;

/* Local functions */
static FWStatus fwRequestTxCompleteIntHandler(FWDriverData *pDriver);
static FWStatus fwResponseTxCompleteIntHandler(FWDriverData *pDriver);
static FWStatus fwARRQIntHandler(FWDriverData *pDriver);
static FWStatus fwARRSIntHandler(FWDriverData *pDriver);
static FWStatus fwRQPktIntHandler(FWDriverData *pDriver);
static FWStatus fwRSPktIntHandler(FWDriverData *pDriver);
static FWStatus fwIsoTxIntHandler(FWDriverData *pDriver, UINT32 interruptMask);
static FWStatus fwIsoRxIntHandler(FWDriverData *pDriver, UINT32 interruptMask);
static FWStatus fwPostedWriteErrorIntHandler(FWDriverData *pDriver);
static FWStatus fwIntLockRespErrHandler(FWDriverData *pDriver);
static FWStatus fwSelfIdComplete2IntHandler(FWDriverData *pDriver);
static FWStatus fwSelfIdCompleteIntHandler(FWDriverData *pDriver);
static FWStatus fwBusResetIntHandler(FWDriverData *pDriver);
static FWStatus fwRegAccessFailIntHandler(FWDriverData *pDriver);
static FWStatus fwPhysicalLayerIntHandler(FWDriverData *pDriver);
static FWStatus fwCycleSyncIntHandler(FWDriverData *pDriver);
static FWStatus fwCycle64SecIntHandler(FWDriverData *pDriver);
static FWStatus fwCycleLostIntHandler(FWDriverData *pDriver);
static FWStatus fwCycleInconsistentIntHandler(FWDriverData *pDriver);
static FWStatus fwUnrecoverableErrorIntHandler(FWDriverData *pDriver);
static FWStatus fwCycleTooLongIntHandler(FWDriverData *pDriver);
static FWStatus fwPhyRegRecvIntHandler(FWDriverData *pDriver);
static FWStatus fwAckTardyIntHandler(FWDriverData *pDriver);
static FWStatus fwSoftwareIntHandler(FWDriverData *pDriver);
static FWStatus fwVendorSpecificIntHandler(FWDriverData *pDriver);
static FWStatus fwBuildTopologyMap(FWDriverData *pDriver);
static FWStatus fwDetermineIsochronousMgr(FWDriverData *pDriver);
static FWStatus fwBusManagerAsyncTrans(FWDriverData *pDriver, FWBusMgrTransactionType type);
static FWStatus fwProcessBusMgrAsyncTrans(FWDriverData *pDriver);
static FWStatus fwIsoMgrIsCompatibleCompleted(FWDriverData *pDriver, FWBusMgrTransaction *busMgrTrans);
static FWStatus fwCancelBusMgrAsyncTrans(FWDriverData *pDriver);

static FWBusManagerData *fwBusManagerDataCreate(void);
static void fwBusManagerDataDestroy(FWBusManagerData * list);
static FWStatus fwBusManagerDataPushBack(FWBusManagerData *list, FWBusMgrTransaction *busMgrTrans);
static FWStatus fwBusManagerDataPushFront(FWBusManagerData *list, FWBusMgrTransaction *busMgrTrans);
static FWBusMgrTransaction *fwBusManagerDataPopFront(FWBusManagerData *list);
static FWBusMgrTransaction *fwBusManagerDataPopBack(FWBusManagerData *list);
static FWStatus fwBusManagerDataUnlink(FWBusManagerData *list, FWBusMgrTransaction *busMgrTrans);
static FWStatus fwBusManagerDataDelete(FWBusMgrTransaction *busMgrTrans);

int fwBusManagerStartup(void)
{
	int retVal = -1;
	int adapterCount = fwGetAdapterCount();
	FWStatus fwStatus;
	STATUS errorStatus = ERROR;
	int index;

	do
	{
		/* Initialize all the driver data pointers to NULL. */
		for(index=0; index<FIREWIRE_MAX_PCI_ADAPTERS; index++)
		{
			fwDriverDataArray[index] = NULL;
		}

		/* If no adapters were found, just exit. */
		if( adapterCount == 0 )
		{
			printf("No IEEE-1394 adapters were found.\n");
			break;
		}

/* #ifdef DEBUG_BUILD */
		retVal = fwOpenLog( "/machine/tmp/fw.log", 7 );
		if( retVal != 0 )
		{
			printf("Could not open log file.\n");
			break;
		}

		FWLOGLEVEL1("Firewire bus manager starting up. Detected %d adapters.\n", adapterCount);
/* #endif  ifdef DEBUG_BUILD */

		/* Perform one time initialization tasks. */
		fwStatus = fwCreateBusManager();
		if( fwStatus != FWSuccess )
		{
			fwDestroyBusManager();
			retVal = fwStatus;
			break;
		}

		fwBusManagerShutdownFlag = 0;

		for(index=0; index<adapterCount; index++)
		{
			/* Perform chip specific initialization */
			errorStatus = sysFirewireChipSpecificConfig( index );
			if( errorStatus != OK )
			{
				FWLOGLEVEL3("Error setting vendor specific configuration parameters on adapter %d.\n", index);
			}

			/* Install the ISR for each adapter */
			errorStatus = fwInstallInterruptHandler( fwInterruptHandler, index );
			if( errorStatus != OK )
			{
				FWLOGLEVEL3("Error installing interrupt handler for adapter %d.\n", index);
			}
		}

		if( errorStatus != OK )
			break;

		/* Enable interrupts for each adapter */
		for(index=0; index<adapterCount; index++)
		{
			fwStatus = fwEnableInterrupts( fwDriverDataArray[index] );
			if( fwStatus != FWSuccess )
			{
				retVal = fwStatus;
				break;
			}
		}

		/* Initialize the adapters. */
		for(index=0; index<adapterCount; index++)
		{
			fwStatus = fwInitialize( index );
			if( fwStatus != FWSuccess )
			{
				FWLOGLEVEL3("Adapter %d returned initialization error %d.\n", index, fwStatus);
			}
			else
				FWLOGLEVEL1("Adapter %d completed initialization.\n", index);
		}

		/* Process events until instructed to shutdown. */
		while( fwBusManagerShutdownFlag == 0 )
		{
			if( fwBusManagerTimeout == 0 )
			{
				fwBusManagerTimeout = sysClkRateGet() / 10;
				if( fwBusManagerTimeout <= 0 ) fwBusManagerTimeout = 1;
			}

			/* Wait for an event */
			errorStatus = semTake( fwBusManagerNotifySem, fwBusManagerTimeout );

			if( errorStatus != OK && fwBusManagerNotifySem == NULL )
			{
				FWLOGLEVEL3("Bus manager invalid semaphore.\n");
				break;
			}

			if( fwBusManagerShutdownFlag )
			{
				FWLOGLEVEL1("IEEE-1394 driver shutting down.\n");
				retVal = 0;
				break;
			}

			/* Process events */
			fwStatus = fwProcessEvents();
			if( fwStatus != FWSuccess )
			{
//				FWLOGLEVEL3("IEEE-1394 driver could not process events. Error:%d\n", fwStatus);
				retVal = fwStatus;
//				break;
			}

			fwStatus = fwProcessAsyncTransactions();

			if( fwStatus != FWSuccess && fwStatus != FWNoTransactions )
			{
				FWLOGLEVEL3("IEEE-1394 driver could not process async transactions. Error:%d\n", fwStatus);
			}
		}

		/* Reset each adapter */
		for(index=0; index<adapterCount; index++)
		{
			retVal = fwReset( index );

			if( retVal != FWSuccess )
			{
				FWLOGLEVEL5("Resetting adapter %d returned error %d.\n", index, retVal);
			}
		}

		/* Disable interrupts for each adapter */
		for(index=0; index<adapterCount; index++)
		{
			fwStatus = fwDisableInterrupts( fwDriverDataArray[index] );
			if( fwStatus != FWSuccess )
			{
				retVal = fwStatus;
			}
		}

		/* Uninstall the ISR for each adapter */
		for(index=0; index<adapterCount; index++)
		{
			errorStatus = fwUninstallInterruptHandler( fwInterruptHandler, index );
			if( errorStatus != OK )
			{
				retVal = fwStatus;
			}
		}

		fwStatus = fwDestroyBusManager();
		if( fwStatus != FWSuccess )
		{
			retVal = fwStatus;
			break;
		}

	} while(0);

	fwCloseLog();

	return retVal;
}

void fwBusManagerShutdown(void)
{
	fwBusManagerShutdownFlag = 1;
	fwNotifyBusManager();
}

FWStatus fwCreateBusManager(void)
{
	FWStatus retVal = FWInternalError;
	int adapterCount = fwGetAdapterCount();
	int index;

	do
	{
		/* Create the notify semaphore */
		fwBusManagerNotifySem = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
		if( fwBusManagerNotifySem == NULL )
		{
			retVal = FWSemAllocateError;
			break;
		}

		/* Create the driver structures for each adapter */
		for(index=0; index<adapterCount; index++)
		{
			/* Create the driver structure array */
			fwDriverDataArray[index] = (FWDriverData*)fwMalloc( sizeof(FWDriverData) );
			if( fwDriverDataArray[index] == NULL )
			{
				retVal = FWMemAllocateError;
				break;
			}

			/* Initialize the driver data memory */
			memset( fwDriverDataArray[index], 0, sizeof(FWDriverData) );

			/* Create the bus manager state data structure */
			fwDriverDataArray[index]->busManagerData = fwBusManagerDataCreate();
			if( fwDriverDataArray[index]->busManagerData == NULL )
			{
				retVal = FWMemAllocateError;
				break;
			}

			/* Initialize the driver data memory */
			memset( fwDriverDataArray[index]->busManagerData, 0, sizeof(FWBusManagerData) );

			/* Create a client entry for the bus manager */
			retVal = fwCreateClient( &fwDriverDataArray[index]->busManagerData->clientHandle, fwBusManagerNotifySem );
			if( retVal != FWSuccess )
			{
				FWLOGLEVEL3("Could not create a client entry for the bus manager. Error:%d.\n", retVal);
				break;
			}

			/* Create the bus manager state data structure */
			fwDriverDataArray[index]->isochronousMgrData = (FWIsochronousMgrData*)fwMalloc( sizeof(FWIsochronousMgrData) );
			if( fwDriverDataArray[index]->isochronousMgrData == NULL )
			{
				retVal = FWMemAllocateError;
				break;
			}

			/* Initialize the driver data memory */
			fwDriverDataArray[index]->isochronousMgrData->isochronousMgrID = 0xFFFF;
			fwDriverDataArray[index]->isochronousMgrData->isochronousMgrSpeed = FWS100;

			/* Create and setup the other data structures */
			retVal = fwCreateDriverData( fwDriverDataArray[index] );
			if( retVal != FWSuccess )
			{
				break;
			}

			/* Map the hardware to the driver structure */
			fwDriverDataArray[index]->pciAdapterData = &fwAdapterData[index];

			if( fwDriverDataArray[index]->pciAdapterData == NULL )
			{
				retVal = FWInternalError;
				break;
			}

			fwDriverDataArray[index]->ohci = (OhciRegisters*)fwDriverDataArray[index]->pciAdapterData->pOhci;
		}

	} while(0);

	return retVal;
}

FWStatus fwDestroyBusManager(void)
{
	FWStatus retVal = FWInternalError;
	int adapterCount = fwGetAdapterCount();
	int index;

	do
	{
		/* Destroy the driver structures for each adapter */
		for(index=0; index<adapterCount; index++)
		{
			if( fwDriverDataArray[index] != NULL )
			{
				/* Destroy the physical layer data structures */
				retVal = fwDestroyDriverData( fwDriverDataArray[index] );
				if( retVal != FWSuccess )
				{
					FWLOGLEVEL3("Could not destroy the driver data! Error:%d.\n", retVal);
				}

				/* Delete the client entry for the bus manager */
				retVal = fwDestroyClient( fwDriverDataArray[index]->busManagerData->clientHandle );

				/* Delete the bus manager data */
				fwBusManagerDataDestroy( fwDriverDataArray[index]->busManagerData );

				/* Delete the isochronous manager data */
				fwFree( fwDriverDataArray[index]->isochronousMgrData );

				/* fwFree the driver structure */
				fwFree( fwDriverDataArray[index] );

				fwDriverDataArray[index] = NULL;
			}
		}

		if( fwBusManagerNotifySem )
		{
			semDelete( fwBusManagerNotifySem );
		}

		fwBusManagerNotifySem = NULL;

	} while(0);

	return retVal;
}

FWStatus fwProcessEvents(void)
{
	FWStatus retVal = FWInternalError;
	int adapterCount = fwGetAdapterCount();
	int timeoutKnt = 0;
	int index;
	FWDriverData *pDriver;
	UINT32 interruptMask;
	UINT32 isoRecvInterruptMask;
	UINT32 isoXmitInterruptMask;

	for(index=0; index<adapterCount; index++)
	{
		if( fwDriverDataArray[index] != NULL )
		{
			pDriver = fwDriverDataArray[index];

			retVal = fwGetInterruptMask( pDriver, &interruptMask, &isoXmitInterruptMask, &isoRecvInterruptMask );

			if( retVal == FWSuccess )
			{
				do
				{
					/* Check for interrupts */
					if( interruptMask == 0 && isoXmitInterruptMask == 0 && isoRecvInterruptMask == 0 ) break;

					/* Process interrupts */
					FWLOGLEVEL9("Interrupt received from adapter %d intMask:0x%08X isoXmit:0x%08X isoRecv:0x%08X\n", index, interruptMask, isoXmitInterruptMask, isoRecvInterruptMask);

					if( interruptMask & 0x00000001 )
					{
						retVal = fwRequestTxCompleteIntHandler( pDriver );
					}

					if( interruptMask & 0x00000002 )
					{
						retVal = fwResponseTxCompleteIntHandler( pDriver );
					}

					if( interruptMask & 0x00000004 )
					{
						retVal = fwARRQIntHandler( pDriver );
					}

					if( interruptMask & 0x00000008 )
					{
						retVal = fwARRSIntHandler( pDriver );
					}

					if( interruptMask & 0x00000010 )
					{
						retVal = fwRQPktIntHandler( pDriver );
					}

					if( interruptMask & 0x00000020 )
					{
						retVal = fwRSPktIntHandler( pDriver );
					}

					if( interruptMask & 0x00000040 )
					{
						retVal = fwIsoTxIntHandler( pDriver, isoXmitInterruptMask );
					}

					if( interruptMask & 0x00000080 )
					{
						retVal = fwIsoRxIntHandler( pDriver, isoRecvInterruptMask );
					}

					if( interruptMask & 0x00000100 )
					{
						retVal = fwPostedWriteErrorIntHandler( pDriver );
					}

					if( interruptMask & 0x00000200 )
					{
						retVal = fwIntLockRespErrHandler( pDriver );
					}

					if( interruptMask & 0x00020000 )
					{
						retVal = fwBusResetIntHandler( pDriver );
					}

					if( interruptMask & 0x00010000 )
					{
						retVal = fwSelfIdCompleteIntHandler( pDriver );
					}

					if( interruptMask & 0x00008000 )
					{
						retVal = fwSelfIdComplete2IntHandler( pDriver );
					}

					if( interruptMask & 0x00040000 )
					{
						retVal = fwRegAccessFailIntHandler( pDriver );
					}

					if( interruptMask & 0x00080000 )
					{
						retVal = fwPhysicalLayerIntHandler( pDriver );
					}

					if( interruptMask & 0x00100000 )
					{
						retVal = fwCycleSyncIntHandler( pDriver );
					}

					if( interruptMask & 0x00200000 )
					{
						retVal = fwCycle64SecIntHandler( pDriver );
					}

					if( interruptMask & 0x00400000 )
					{
						retVal = fwCycleLostIntHandler( pDriver );
					}

					if( interruptMask & 0x00800000 )
					{
						retVal = fwCycleInconsistentIntHandler( pDriver );
					}

					if( interruptMask & 0x01000000 )
					{
						retVal = fwUnrecoverableErrorIntHandler( pDriver );
					}

					if( interruptMask & 0x02000000 )
					{
						retVal = fwCycleTooLongIntHandler( pDriver );
					}

					if( interruptMask & 0x04000000 )
					{
						retVal = fwPhyRegRecvIntHandler( pDriver );
					}

					if( interruptMask & 0x08000000 )
					{
						retVal = fwAckTardyIntHandler( pDriver );
					}

					if( interruptMask & 0x20000000 )
					{
						retVal = fwSoftwareIntHandler( pDriver );
					}

					if( interruptMask & 0x40000000 )
					{
						retVal = fwVendorSpecificIntHandler( pDriver );
					}

				} while(0);
			}

			/* Clean up any completed responses */
			retVal = fwCompleteAsyncResponseTransaction( pDriver );

			/* Send next async response (if any) */
			retVal = fwSendAsyncResponseTransaction( pDriver );

			/* Send out any requests */
			retVal = fwSendAsyncRequestTransaction( pDriver );

			/* Send bus manager transactions */
			retVal = fwProcessBusMgrAsyncTrans( pDriver );

			if( pDriver->busManagerData->head != NULL )
			{
				timeoutKnt++;
			}
		}
	}

	if( timeoutKnt == 0 )
	{
		fwBusManagerTimeout = WAIT_FOREVER;
	}

	return retVal;
}

FWStatus fwProcessAsyncTransactions(void)
{
	FWStatus retVal = FWSuccess;
	int adapterCount = fwGetAdapterCount();
	int index;
	FWDriverData *pDriver;

	/* Loop through the adapter list and
	 * submit any pending transactions.
	 */
	for(index=0; index<adapterCount; index++)
	{
		if( fwDriverDataArray[index] != NULL )
		{
			pDriver = fwDriverDataArray[index];

		}
	}



	return retVal;
}

FWStatus fwNotifyBusManager(void)
{
	FWStatus retVal = FWInternalError;

	if( fwBusManagerNotifySem )
	{
		if( semGive( fwBusManagerNotifySem ) == OK )
			retVal = FWSuccess;
	}

	return retVal;
}

FWStatus fwChipSpecificConfig(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	return retVal;
}

FWStatus fwTIChipConfig(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	return retVal;
}

FWStatus fwEnableInterrupts(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	if( pDriver )
	{
		pDriver->ohci->intMaskSet = 0xEF8F83FF;
		retVal = FWSuccess;
	}

	return retVal;
}

FWStatus fwDisableInterrupts(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	if( pDriver )
	{
		pDriver->ohci->intMaskClr = 0xEFFF83FF;
		retVal = FWSuccess;
	}

	return retVal;
}

void fwInterruptHandler(int adapterIndex)
{
	int adapterCount = fwGetAdapterCount();
	FWDriverData *pDriver;
	boolean physAccessReady = FALSE;
	UINT32 interruptMask;
	UINT32 isoRecvInterruptMask;
	UINT32 isoXmitInterruptMask;
	UINT32 clearMask = 0xFFFDFFFF;

	/* Bounds check the adapter index */
	if( adapterIndex >=0 && adapterIndex < adapterCount )
	{
		pDriver = fwDriverDataArray[adapterIndex];

		/* Read the interrupt flags */
		interruptMask = pDriver->ohci->intEventClr;
		isoRecvInterruptMask = pDriver->ohci->isoRecvIntEventClr;
		isoXmitInterruptMask = pDriver->ohci->isoXmitIntEventClr;

		/* Update the driver flags */
		pDriver->interruptMask |= interruptMask;
		pDriver->isoRecvInterruptMask |= isoRecvInterruptMask;
		pDriver->isoXmitInterruptMask |= isoXmitInterruptMask;

		/* Special handling for bus reset interrupt. (OHCI 7.2.3.2) */
		if( interruptMask & 0x00020000 )
		{
			pDriver->ohci->asyncTxRequest.contextControlClr = 0x00008000;
			pDriver->ohci->asyncTxResponse.contextControlClr = 0x00008000;

			if( (pDriver->ohci->asyncTxRequest.contextControlClr & 0x00000400) == 0 &&
				 (pDriver->ohci->asyncTxResponse.contextControlClr & 0x00000400) == 0 )
			{
				clearMask = 0xFFFFFFFF;
			}
		}

		/* Clear the interrupt flags */
		pDriver->ohci->intEventClr = interruptMask & clearMask;
		pDriver->ohci->isoRecvIntEventClr = isoRecvInterruptMask;
		pDriver->ohci->isoXmitIntEventClr = isoXmitInterruptMask;

		if( pDriver->physicalLayerData->phyRegStatus == 0 )
			physAccessReady = TRUE;

		/* Check for physical layer read */
		if( interruptMask & 0x040000 ) /* Physical layer register access failed */
		{
			pDriver->physicalLayerData->phyRegStatus |= 1;
		}
		else if( interruptMask & 0x4000000 ) /* Physical layer register read complete */
		{
			pDriver->physicalLayerData->phyRegStatus |= 2;
		}

		if( physAccessReady && pDriver->physicalLayerData->phyRegAccess )
			semGive( pDriver->physicalLayerData->phyRegAccess );

		fwNotifyBusManager();
	}
}

FWStatus fwGetInterruptMask(FWDriverData *pDriver, UINT32 *interruptMask, UINT32 *isoXmitInterruptMask, UINT32 *isoRecvInterruptMask)
{
	FWStatus retVal = FWInternalError;
	int lockKey;

	if( interruptMask && isoXmitInterruptMask && isoRecvInterruptMask )
	{
		lockKey = intLock();

		*interruptMask = pDriver->interruptMask;
		*isoRecvInterruptMask = pDriver->isoRecvInterruptMask;
		*isoXmitInterruptMask = pDriver->isoXmitInterruptMask;

		pDriver->interruptMask = 0;
		pDriver->isoRecvInterruptMask = 0;
		pDriver->isoXmitInterruptMask = 0;

		intUnlock( lockKey );

		retVal = FWSuccess;
	}

	return retVal;
}

FWStatus fwGetDriverTick(FWDriverData *pDriver, unsigned long *tick)
{
	FWStatus retVal = FWInternalError;
	UINT32 cycleTimer;

	if( tick )
	{
		cycleTimer = pDriver->ohci->isoCycleTimer;

		*tick = (cycleTimer & 0x01FFF000) >> 12;
		*tick += (cycleTimer >> 25) * 8000;

		retVal = FWSuccess;
	}

	return retVal;
}

/*
 *  Local functions
 */

static FWStatus fwRequestTxCompleteIntHandler(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	FWLOGLEVEL9("fwRequestTxCompleteIntHandler called.\n");

	/* Free resources in the link layer
	 * for the request TX context.
	 */
	retVal = fwAsyncTxRequestComplete( pDriver );

	return retVal;
}

static FWStatus fwResponseTxCompleteIntHandler(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	FWLOGLEVEL9("fwResponseTxCompleteIntHandler called.\n");

	/* Free resources in the link layer
	 * for the response TX context.
	 */
	retVal = fwAsyncTxResponseComplete( pDriver );

	/* Clean up any completed responses */
	retVal = fwCompleteAsyncResponseTransaction( pDriver );

	/* Send next async response (if any) */
	retVal = fwSendAsyncResponseTransaction( pDriver );

	return retVal;
}

static FWStatus fwARRQIntHandler(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	FWLOGLEVEL9("fwARRQIntHandler called.\n");

	return retVal;
}

static FWStatus fwARRSIntHandler(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	FWLOGLEVEL9("fwARRSIntHandler called.\n");

	return retVal;
}

static FWStatus fwRQPktIntHandler(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	FWLOGLEVEL9("fwRQPktIntHandler called.\n");

	retVal = fwRecvAsyncRequestTransaction( pDriver );

	if( retVal != FWSuccess )
	{
		FWLOGLEVEL4("Recv async transaction returned %d.\n", retVal );
	}

	return retVal;
}

static FWStatus fwRSPktIntHandler(FWDriverData *pDriver)
{
	FWStatus retVal = FWSuccess;
	FWTransaction transaction;
	int i;
	UINT32 *pQuadlet;

	FWLOGLEVEL9("fwRSPktIntHandler called.\n");

	while( retVal == FWSuccess )
	{
		retVal = fwRecvAsyncResponseTransaction( pDriver );

		if( retVal != FWSuccess && retVal != FWNotFound )
		{
			FWLOGLEVEL9("fwRecvAsyncResponseTransaction returned %d.\n", retVal );
		}
	}

	return retVal;
}

static FWStatus fwIsoTxIntHandler(FWDriverData *pDriver, UINT32 interruptMask)
{
	FWStatus retVal = FWInternalError;

	FWLOGLEVEL9("fwIsoTxIntHandler called.\n");

	return retVal;
}

static FWStatus fwIsoRxIntHandler(FWDriverData *pDriver, UINT32 interruptMask)
{
	FWStatus retVal = FWSuccess;
	FWIsoChannel *isoChannel;
	unsigned int channel;
	UINT32 channelMask;

	FWLOGLEVEL9("fwIsoRxIntHandler called.\n");

	/* For each channel, signal the client semaphore. */
	for(channel=0; channel<32; channel++)
	{
		channelMask = 1 << channel;

		if( channelMask & interruptMask )
		{
			semTake( pDriver->isoChannelList->listSem, WAIT_FOREVER );

			/* Look up the channel */
			isoChannel = fwFindIsoChannelListItem( pDriver->isoChannelList, channel );

			if( isoChannel )
			{
				retVal = fwIsoAdvanceRecvBuffer( pDriver, isoChannel );

				if( retVal == FWSuccess )
				{
					if( isoChannel->clientSem )
					{
						semGive( isoChannel->clientSem );
					}
				}
				else
				{
					FWLOGLEVEL9("Iso channel %d advance buffer failed. Error:%d\n", channel, retVal );
				}
			}
			else
			{
				FWLOGLEVEL9("Iso channel %d not found. Mask:0x%08X\n", channel, channelMask );
				pDriver->ohci->isoRecvIntMaskClr = channelMask;
			}

			semGive( pDriver->isoChannelList->listSem );
		}
	}

	return retVal;
}

static FWStatus fwPostedWriteErrorIntHandler(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	FWLOGLEVEL9("fwPostedWriteErrorIntHandler called.\n");

	return retVal;
}

static FWStatus fwIntLockRespErrHandler(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	FWLOGLEVEL9("fwIntLockRespErrHandler called.\n");

	return retVal;
}

static FWStatus fwBusResetIntHandler(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	pDriver->ohci->asyncTxRequest.contextControlClr = 0x00008000;
	pDriver->ohci->asyncTxResponse.contextControlClr = 0x00008000;

	if( (pDriver->ohci->asyncTxRequest.contextControlClr & 0x00000400) == 0 &&
		 (pDriver->ohci->asyncTxResponse.contextControlClr & 0x00000400) == 0 )
	{
		pDriver->ohci->intEventClr = 0x00020000;
	}

	FWLOGLEVEL9("fwBusResetIntHandler called.\n");

	retVal = fwSetCycleMaster( pDriver, FALSE );
	pDriver->physicalLayerData->busResetUnderway = TRUE;
	pDriver->physicalLayerData->isRoot = FALSE;

	retVal = fwCancelBusMgrAsyncTrans( pDriver );

	return retVal;
}

static FWStatus fwSelfIdCompleteIntHandler(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	unsigned short sourceID;
	UINT32 selfIdCount;
	int i;

	FWLOGLEVEL9("fwSelfIdCompleteIntHandler called.\n");

	fwTopologyMapReset( pDriver );

	selfIdCount = pDriver->ohci->selfIdCount;

	/* Check for a valid selfID generation */
	if( selfIdCount & 0x80000000 )
	{
		pDriver->physicalLayerData->busGeneration = 0x0FFFFFFF;
	}
	else
	{
		pDriver->physicalLayerData->busGeneration = (selfIdCount & 0x00FF0000) >> 16;

		/* Build the topology Map */
		retVal = fwBuildTopologyMap( pDriver );
	}

	printf("New bus generation %d.\n", pDriver->physicalLayerData->busGeneration);

	retVal = fwGetSourceID( pDriver, &sourceID );
	if( retVal == FWSuccess )
	{
		pDriver->physicalLayerData->nodeId = sourceID;
	}
	else
	{
		pDriver->physicalLayerData->nodeId = 0xFFFF;
	}

	retVal = fwGetRootNodeStatus( pDriver, &pDriver->physicalLayerData->isRoot );

	pDriver->physicalLayerData->busResetUnderway = FALSE;

	/* Set the async request filter to allow packets
	 * to be received from all other nodes once self ID
	 * completes.
	 */
	pDriver->ohci->asyncRequestFilterHiSet = 0x7FFFFFFF;
	pDriver->ohci->asyncRequestFilterLoSet = 0XFFFFFFFF;

	/* Determine what node is the isochronous resource manager. */
	retVal = fwDetermineIsochronousMgr( pDriver );

	/* If this node was the bus manager, it can immediately
	 * attempt to become the bus manager again.
	 */
	if( pDriver->busManagerData->isBusManager )
	{
	}

	return retVal;
}

static FWStatus fwSelfIdComplete2IntHandler(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	FWLOGLEVEL9("fwSelfIdComplete2IntHandler called.\n");

	return retVal;
}

static FWStatus fwRegAccessFailIntHandler(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	FWLOGLEVEL9("fwRegAccessFailIntHandler called.\n");

	return retVal;
}

static FWStatus fwPhysicalLayerIntHandler(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	FWLOGLEVEL9("fwPhysicalLayerIntHandler called.\n");

	return retVal;
}

static FWStatus fwCycleSyncIntHandler(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	FWLOGLEVEL9("fwCycleSyncIntHandler called.\n");

	return retVal;
}

static FWStatus fwCycle64SecIntHandler(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	FWLOGLEVEL9("fwCycle64SecIntHandler called.\n");

	return retVal;
}

static FWStatus fwCycleLostIntHandler(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	FWLOGLEVEL9("fwCycleLostIntHandler called.\n");

	return retVal;
}

static FWStatus fwCycleInconsistentIntHandler(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	FWLOGLEVEL9("fwCycleInconsistentIntHandler called.\n");

	return retVal;
}

static FWStatus fwUnrecoverableErrorIntHandler(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	FWLOGLEVEL9("fwUnrecoverableErrorIntHandler called.\n");

	return retVal;
}

static FWStatus fwCycleTooLongIntHandler(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	FWLOGLEVEL9("fwCycleTooLongIntHandler called.\n");

	return retVal;
}

static FWStatus fwPhyRegRecvIntHandler(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	FWLOGLEVEL9("fwPhyRegRecvIntHandler called.\n");

	return retVal;
}

static FWStatus fwAckTardyIntHandler(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	FWLOGLEVEL9("fwAckTardyIntHandler called.\n");

	return retVal;
}

static FWStatus fwSoftwareIntHandler(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	FWLOGLEVEL9("fwSoftwareIntHandler called.\n");

	return retVal;
}

static FWStatus fwVendorSpecificIntHandler(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	FWLOGLEVEL9("fwVendorSpecificIntHandler called.\n");

	return retVal;
}

static FWStatus fwBuildTopologyMap(FWDriverData *pDriver)
{
	FWStatus retVal = FWSelfIdError;
	UINT32 selfIdCount;
	UINT32 selfId;
	UINT32 countGen = 0;
	int i, size;
	unsigned char phyId;
	unsigned char gapCnt;
	int speed;
	int contender;
	int linkActive;
	int power;

	FWLOGLEVEL7("Building the topology map.\n");

	do
	{
		retVal = fwTopologyMapReset(pDriver);

		if( retVal != FWSuccess )
		{
			break;
		}

		selfIdCount = pDriver->ohci->selfIdCount;

		/* Check for a valid selfID generation */
		if( selfIdCount & 0x80000000 )
		{
			FWLOGLEVEL7("A self ID error occured.\n");
			retVal = FWSelfIdError;
			break;
		}

		/* Get the size in quadlets of the self IDs */
		size = (int)((selfIdCount >> 2) & 0x000001FF) - 1;

		fwInvalidateDMAMemPool( pDriver->linkLayerData->selfIdRecvMemory );

		for(i=0; i<size; i++)
		{
			if( i == 0 )
			{
				countGen = pDriver->linkLayerData->selfIdBufferVirtualAddr[i] & 0x00FF0000;

				/* Compare the generation count to make sure the data is consistent */
				if( countGen != (selfIdCount  & 0x00FF0000) )
				{
					retVal = FWSelfIdError;
					break;
				}
			}
			else
			{
				if( i & 1)
				{
					selfId = pDriver->linkLayerData->selfIdBufferVirtualAddr[i];

					/* Check if the self ID is valid */
					if( selfId == ~pDriver->linkLayerData->selfIdBufferVirtualAddr[i+1] )
					{
						retVal = fwTopologyMapAddSelfID( pDriver, selfId );

						if( ( selfId & 0x00800000 ) == 0 )
						{
							phyId = (unsigned char)((selfId >> 24) & 0x0000003F);
							gapCnt = (unsigned char)((selfId >> 16) & 0x0000003F);
							speed = (int)((selfId >> 14) & 0x00000003);
							linkActive = (int)((selfId >> 22) & 0x00000001);
							contender = (int)((selfId >> 11) & 0x00000001);
							power = (int)((selfId >> 8) & 0x00000007);

							printf("ID:0x%X GapCnt:%d Speed:%d LinkActive:%d Contender:%d Power:%d Initiated:%d More:%d\n",
											phyId, gapCnt, speed, linkActive, contender, power, ((selfId & 2) >> 1), selfId & 1);

							retVal = FWSuccess;
						}
						else
						{
							phyId = (unsigned char)((selfId >> 24) & 0x0000003F);
							FWLOGLEVEL7("Extended ID:0x%X More:%d\n", phyId, selfId & 1 );
						}
					}
					else
					{
						FWLOGLEVEL3("SelfId packet is corrupt.\n");
						retVal = FWSelfIdError;
						break;
					}
				}
			}
		}

		if( retVal != FWSuccess )
		{
			break;
		}

		retVal = fwTopologyMapComplete( pDriver, countGen >> 16 );

	} while(0);

	return retVal;
}

static FWStatus fwDetermineIsochronousMgr(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyId;
	unsigned char prevPhyId = 0;
	unsigned short isoMgrPhyId = 0x3F;
	boolean selfIdGood = TRUE;
	unsigned short busNumber;
	unsigned char nodeNumber;
	int speed = 0;
	int contender;
	int linkActive;
	UINT32 selfId;
	int i;

	if( pDriver->coreCSR->topologyMap->nodeCount > 1 )
	{
		for(i=0; i<pDriver->coreCSR->topologyMap->selfIDCount; i++)
		{
			selfId = pDriver->coreCSR->topologyMap->selfIDs[i];

			phyId = (unsigned char)((selfId >> 24) & 0x0000003F);
			contender = (int)((selfId >> 11) & 0x00000001);
			linkActive = (int)((selfId >> 22) & 0x00000001);

			/* Examine the self ID packets to determine what
			 * node should be the isochronous resource manager.
			 */
			if( ( selfId & 0x00800000 ) == 0 )
			{
				if( i > 0 && prevPhyId + 1 != phyId )
				{
					/* self ID sequence appears to be bad (not sequencial). */
					selfIdGood = FALSE;
					FWLOGLEVEL3("SelfId packet sequence is not sequencial.\n");
				}
				else
				{
					if( contender && linkActive )
					{
						isoMgrPhyId = phyId;
						speed = (int)((selfId >> 14) & 0x00000003);
					}
				}
			}

			prevPhyId = phyId;
		}

		if( selfIdGood )
		{
			retVal = fwGetNodeID(pDriver, &busNumber, &nodeNumber);

			if( retVal == FWSuccess && isoMgrPhyId != 0x3F )
			{
				pDriver->isochronousMgrData->isochronousMgrID = isoMgrPhyId | ( busNumber << 6 );
				pDriver->isochronousMgrData->isochronousMgrSpeed = FWS100;

				if( speed == 1 )
				{
					pDriver->isochronousMgrData->isochronousMgrSpeed = FWS200;
				}
				else if( speed == 2 )
				{
					pDriver->isochronousMgrData->isochronousMgrSpeed = FWS400;
				}

				/* A possible isochronous manager is detected.
				 * Determine if the node is a 1394a class device
				 * by querying CSR offset 0x234 for a non-zero value.
				 * If it is not, set the root hold off and initiate
				 * a bus reset. FWIsoManagerDelay100ms
				 */
				retVal = fwBusManagerAsyncTrans( pDriver, FWIsoManagerDelay100ms );
			}
			else
			{
				FWLOGLEVEL3("Node ID not valid after self ID completed. Error:%d\n", retVal);
				selfIdGood = FALSE;
			}
		}

		if( selfIdGood == FALSE )
		{
			/* Perform a bus reset. */
			retVal = fwInitiateBusReset( pDriver );
		}
	}

	return retVal;
}

static FWStatus fwBusManagerAsyncTrans(FWDriverData *pDriver, FWBusMgrTransactionType type)
{
	FWStatus retVal = FWInternalError;
	FWClientResource *clientResource = NULL;
	FWBusMgrTransaction *busMgrTrans;
	FWAsyncTransactionCmd asyncCmd;

	if( pDriver->adapterEnabled )
	{
		switch( type )
		{
		case	FWNullTransaction:
			break;

		case	FWIsoMgrIsCompatible:
			/* Obtain the client's resource structure */
			retVal = fwGetClientResource( pDriver->busManagerData->clientHandle, &clientResource );

			if( retVal != FWSuccess )
			{
				break;
			}

			busMgrTrans = (FWBusMgrTransaction*)fwMalloc( sizeof(FWBusMgrTransaction) );
			if( busMgrTrans == NULL )
			{
				retVal = FWMemAllocateError;
				break;
			}

			busMgrTrans->transaction = (FWTransaction*)fwMalloc( sizeof(FWTransaction) );
			if( busMgrTrans->transaction == NULL )
			{
				fwFree( busMgrTrans );
				retVal = FWMemAllocateError;
				break;
			}

			/* Fill in the command to perform a quadlet read to CSR 0x234 */
			asyncCmd.clientHandle = pDriver->busManagerData->clientHandle;
			asyncCmd.dataBufferSize = 8;
			asyncCmd.dataLength = 4;
			asyncCmd.destinationID = pDriver->isochronousMgrData->isochronousMgrID;
			asyncCmd.destinationOffset.highOffset = 0xFFFF;
			asyncCmd.destinationOffset.lowOffset = 0xF0000234;
			asyncCmd.speed = pDriver->isochronousMgrData->isochronousMgrSpeed;

			asyncCmd.databuffer = (UINT32*)fwMalloc( asyncCmd.dataBufferSize );
			if( asyncCmd.databuffer == NULL )
			{
				fwFree( busMgrTrans->transaction );
				fwFree( busMgrTrans );
				retVal = FWMemAllocateError;
				break;
			}

			/* Fill in the transaction structure and add it to the pending queue */
			retVal = fwInitializeAsyncRequestTransaction( pDriver, &asyncCmd, busMgrTrans->transaction );

			if( retVal != FWSuccess )
			{
				fwFree( asyncCmd.databuffer );
				fwFree( busMgrTrans->transaction );
				fwFree( busMgrTrans );
				break;
			}

			busMgrTrans->transaction->semId = fwBusManagerNotifySem;
			busMgrTrans->tickTimeout = FW_TRANSACTION_TIMEOUT_TICKS;
			busMgrTrans->transType = type;

			retVal = fwGetDriverTick( pDriver, &busMgrTrans->tick );
			if( retVal != FWSuccess )
			{
				fwFree( asyncCmd.databuffer );
				fwFree( busMgrTrans->transaction );
				fwFree( busMgrTrans );
				break;
			}

			retVal = fwBusManagerDataPushBack( pDriver->busManagerData, busMgrTrans );
			if( retVal != FWSuccess )
			{
				fwFree( asyncCmd.databuffer );
				fwFree( busMgrTrans->transaction );
				fwFree( busMgrTrans );
				break;
			}

			retVal = fwPostAsyncReadRequest( pDriver, busMgrTrans->transaction );

			FWLOGLEVEL9("Sending bus manager Iso manager transaction.\n");
			fwBusManagerTimeout = FW_BUS_MANAGER_TIMEOUT_TICKS;
			break;

		case	FWWriteBusMgrId:
			break;

		case	FWIsoManagerDelay100ms:
			/* Obtain the client's resource structure */
			retVal = fwGetClientResource( pDriver->busManagerData->clientHandle, &clientResource );

			if( retVal != FWSuccess )
			{
				break;
			}

			busMgrTrans = (FWBusMgrTransaction*)fwMalloc( sizeof(FWBusMgrTransaction) );
			if( busMgrTrans == NULL )
			{
				retVal = FWMemAllocateError;
				break;
			}

			busMgrTrans->next = NULL;
			busMgrTrans->prev = NULL;
			busMgrTrans->transaction = NULL;

			retVal = fwGetDriverTick( pDriver, &busMgrTrans->tick );
			if( retVal != FWSuccess )
			{
				fwFree( busMgrTrans );
				break;
			}

			busMgrTrans->tickTimeout = 600;
			busMgrTrans->transType = type;

			retVal = fwBusManagerDataPushBack( pDriver->busManagerData, busMgrTrans );

			FWLOGLEVEL9("Sending bus manager delay 100ms transaction.\n");
			fwBusManagerTimeout = FW_BUS_MANAGER_TIMEOUT_TICKS;
			if( retVal != FWSuccess )
			{
				fwFree( busMgrTrans );
				break;
			}
		}
	}
	else
	{
		retVal = FWNotInitialized;
	}

	return retVal;
}

static FWStatus fwProcessBusMgrAsyncTrans(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	int adapterCount = fwGetAdapterCount();
	int totalTransCount = 0;
	unsigned long currentTick;
	unsigned long compareTick;
	FWBusMgrTransaction *busMgrTrans;
	FWBusMgrTransaction *busMgrTransReady;

	retVal = fwGetDriverTick( pDriver, &currentTick );

	busMgrTrans = pDriver->busManagerData->head;

	while( busMgrTrans )
	{
		compareTick = currentTick;

		if( currentTick < busMgrTrans->tick )
		{
			compareTick += 1024000;
		}

		if( busMgrTrans->transaction )
		{
			retVal = busMgrTrans->transaction->status;
		}
		else
		{
			retVal = FWNotFound;
		}

		/* Determine if the transaction is complete or timed out */
		if( retVal == FWSuccess || (compareTick - busMgrTrans->tick) >= busMgrTrans->tickTimeout )
		{
			busMgrTransReady = busMgrTrans;
			busMgrTrans = busMgrTrans->next;

			FWLOGLEVEL9("Checking bus manager transactions. Current tick:%d Trans tick:%d\n", compareTick, busMgrTransReady->tick );

			/* Unlink this transaction from the list */
			fwBusManagerDataUnlink( pDriver->busManagerData, busMgrTransReady );

			/* The transaction is complete. Remove it from the waiting list. */
			if( busMgrTransReady->transaction )
			{
				if( busMgrTransReady->transaction->status == FWSuccess )
				{
					retVal = fwRemoveTransaction( pDriver, busMgrTransReady->transaction );
				}
				else
				{
					/* The transaction timed out. Cancel the transaction. */
					retVal = fwCancelTransaction( pDriver, busMgrTransReady->transaction );
				}
			}

			FWLOGLEVEL9("Processing bus manager transaction type %d.\n", busMgrTransReady->transType);
			switch( busMgrTransReady->transType )
			{
			case	FWNullTransaction:
				break;

			case	FWIsoMgrIsCompatible:
				retVal = fwIsoMgrIsCompatibleCompleted( pDriver, busMgrTransReady );
				break;

			case	FWWriteBusMgrId:
				break;

			case	FWIsoManagerDelay100ms:
				retVal = fwBusManagerAsyncTrans( pDriver, FWIsoMgrIsCompatible );
				break;
			}

			fwBusManagerDataDelete( busMgrTransReady );
		}
		else
		{
			totalTransCount++;
			busMgrTrans = busMgrTrans->next;
		}
	}

	return retVal;
}

static FWStatus fwCancelBusMgrAsyncTrans(FWDriverData *pDriver)
{
	FWStatus retVal = FWSuccess;
	FWBusMgrTransaction *busMgrTrans;

	busMgrTrans = fwBusManagerDataPopFront( pDriver->busManagerData );

	while( busMgrTrans )
	{
		if( busMgrTrans->transaction )
		{
			/* Cancel the transaction. */
			retVal = fwCancelTransaction( pDriver, busMgrTrans->transaction );
		}

		fwBusManagerDataDelete( busMgrTrans );

		busMgrTrans = fwBusManagerDataPopFront( pDriver->busManagerData );
	}

	return retVal;
}

static FWStatus fwIsoMgrIsCompatibleCompleted(FWDriverData *pDriver, FWBusMgrTransaction *busMgrTrans)
{
	FWStatus retVal = FWInternalError;
	UINT32 mgrResponse;

	FWLOGLEVEL9("IsoMgr complete called. Status:%d Response:%d Len:%d\n",
					busMgrTrans->transaction->status,
					busMgrTrans->transaction->responseCode,
					busMgrTrans->transaction->dataLength );

	do
	{
		/* Check the status of the transaction */
		if( busMgrTrans->transaction->status == FWSuccess )
		{
			if( busMgrTrans->transaction->responseCode == FWResponseComplete )
			{
				if( busMgrTrans->transaction->dataLength == 4 )
				{
					if( FW_BYTE_SWAP_ENABLED )
					{
						mgrResponse = fwByteSwap32( *((UINT32*)busMgrTrans->transaction->databuffer) );
					}
					else
					{
						mgrResponse = *((UINT32*)busMgrTrans->transaction->databuffer);
					}

					if( mgrResponse != 0 )
					{
						/* Isochronous manager responded appropriately
						 * for a 1394a-2000 compliant manager.
						 */
						FWLOGLEVEL7("Node ID 0x%04X is the isochronous resource manager.\n",
										pDriver->isochronousMgrData->isochronousMgrID);
						retVal = FWSuccess;

						/* Check if this node is the acting isochronous manager */
						if( pDriver->isochronousMgrData->isochronousMgrID == pDriver->physicalLayerData->nodeId &&
							 pDriver->physicalLayerData->nodeId != 0xFFFF )
						{
							/* determine the cycle master */

							/* If root, enable the cycle master on this node. */
							if( pDriver->physicalLayerData->isRoot )
							{
								FWLOGLEVEL7("Enabling cycle master.\n");
								retVal = fwSetCycleMaster( pDriver, TRUE );
								break;
							}
							else
							{
								/* Determine if the root node is cycle master capable */
							}
						}
						else
						{
							/* Another node is the isochronous manager. Reset this node's
							 * force root flag for improved stability on the bus.
							 */
							retVal = fwSetForceRoot( pDriver, FALSE );
							break;
						}
					}
				}
			}
		}

		/* The isochronous manager did not response within the timeout
		 * or did not respond appropriately. If there is more than one
		 * node on the bus, set the root hold off bit and initiate a
		 * bus reset.
		 */

		if( pDriver->coreCSR->topologyMap->nodeCount <= 1 )
		{
			FWLOGLEVEL7("Node count is less than or equal to one.\n");
			retVal = fwSetForceRoot( pDriver, FALSE );
			break;
		}

		retVal = fwSetForceRoot( pDriver, TRUE );

		retVal = fwInitiateBusReset( pDriver );

	} while(0);

	return retVal;
}

static FWBusManagerData *fwBusManagerDataCreate(void)
{
	FWBusManagerData * newList = NULL;

	newList = (FWBusManagerData*)fwMalloc(sizeof(FWBusManagerData));

	if( newList )
	{
		newList->head = newList->tail = NULL;
		newList->count = 0;
	}

	return newList;
}

static void fwBusManagerDataDestroy(FWBusManagerData * list)
{
	FWBusMgrTransaction *trans;
	FWBusMgrTransaction *temp;

	if( list )
	{
		trans = list->head;

		while( trans )
		{
			if( trans->transaction )
			{
				if( trans->transaction->databuffer )
				{
					fwFree( trans->transaction->databuffer );
					trans->transaction->databuffer = NULL;
				}
				fwFree( trans->transaction );
				trans->transaction = NULL;
			}

			temp = trans;
			trans = trans->next;

			fwFree( temp );
		}

		fwFree( list );
	}
}

static FWStatus fwBusManagerDataPushBack(FWBusManagerData *list, FWBusMgrTransaction *busMgrTrans)
{
	FWStatus retVal = FWInternalError;

	do
	{
		if( list == NULL ) break;
		if( busMgrTrans == NULL ) break;

		if( list->tail )
		{
			busMgrTrans->next = NULL;
			busMgrTrans->prev = list->tail;
			list->tail->next = busMgrTrans;
			list->tail = busMgrTrans;
		}
		else
		{
			busMgrTrans->next = NULL;
			busMgrTrans->prev = NULL;
			list->tail = busMgrTrans;
			list->head = busMgrTrans;
		}

		list->count++;

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

static FWStatus fwBusManagerDataPushFront(FWBusManagerData *list, FWBusMgrTransaction *busMgrTrans)
{
	FWStatus retVal = FWInternalError;

	do
	{
		if( list == NULL ) break;
		if( busMgrTrans == NULL ) break;

		if( list->head )
		{
			busMgrTrans->prev = NULL;
			busMgrTrans->next = list->head;
			list->head->prev = busMgrTrans;
			list->head = busMgrTrans;
		}
		else
		{
			busMgrTrans->next = NULL;
			busMgrTrans->prev = NULL;
			list->tail = busMgrTrans;
			list->head = busMgrTrans;
		}

		list->count++;

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

static FWBusMgrTransaction *fwBusManagerDataPopFront(FWBusManagerData *list)
{
	FWBusMgrTransaction *busMgrTrans = NULL;

	do
	{
		if( list == NULL ) break;

		if( list->head )
		{
			busMgrTrans = list->head;
			list->head = list->head->next;

			if( list->head == NULL )
			{
				list->tail = NULL;
			}

			busMgrTrans->prev = busMgrTrans->next = NULL;

			list->count--;
		}

	} while(0);

	return busMgrTrans;
}

static FWBusMgrTransaction *fwBusManagerDataPopBack(FWBusManagerData *list)
{
	FWBusMgrTransaction *busMgrTrans = NULL;

	do
	{
		if( list == NULL ) break;

		if( list->tail )
		{
			busMgrTrans = list->tail;
			list->tail = list->tail->prev;

			if( list->tail == NULL )
			{
				list->head = NULL;
			}

			busMgrTrans->prev = busMgrTrans->next = NULL;

			list->count--;
		}

	} while(0);

	return busMgrTrans;
}

static FWStatus fwBusManagerDataUnlink(FWBusManagerData *list, FWBusMgrTransaction *busMgrTrans)
{
	FWStatus retVal = FWInternalError;
	FWBusMgrTransaction *pNext;
	FWBusMgrTransaction *pPrev;

	pNext = busMgrTrans->next;
	pPrev = busMgrTrans->prev;

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

	busMgrTrans->next = busMgrTrans->prev = NULL;

	retVal = FWSuccess;

	return retVal;
}

static FWStatus fwBusManagerDataDelete(FWBusMgrTransaction *busMgrTrans)
{
	FWStatus retVal = FWInternalError;

	if( busMgrTrans->transaction )
	{
		if( busMgrTrans->transaction->databuffer )
		{
			fwFree( busMgrTrans->transaction->databuffer );
		}
		fwFree( busMgrTrans->transaction );
	}

	fwFree( busMgrTrans );

	retVal = FWSuccess;

	return retVal;
}

int fwPrintTick(int adapter);

int fwPrintTick(int adapter)
{
	int retVal = -1;
	int adapterCount = fwGetAdapterCount();
	unsigned long currentTick;

	/* Loop through the adapter list and
	 * submit any pending transactions.
	 */
	if(adapter >=0 && adapter < adapterCount )
	{
		if( fwDriverDataArray[adapter] != NULL )
		{
			retVal = fwGetDriverTick( fwDriverDataArray[adapter], &currentTick );
			printf("Tick:%d\n", currentTick );
		}
	}

	return retVal;
}
