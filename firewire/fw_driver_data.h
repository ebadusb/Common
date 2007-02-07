/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains the firewire driver data structure. A
 * driver data structure is created for each adapter found.
 *
 * $Log$
 *
 */

#ifndef _FW_DRIVER_DATA_INCLUDE_
#define _FW_DRIVER_DATA_INCLUDE_

#include <firewire/fw_pci_support.h>
#include <firewire/fw_ohci_hw.h>
#include <firewire/fw_csr_data.h>
#include <firewire/fw_physical_data.h>
#include <firewire/fw_link_data.h>
#include <firewire/fw_transaction_data.h>
#include <firewire/fw_isochronous_data.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum FWBusMgrTransactionTypeEnum
{
	FWNullTransaction,			/* This indicates no transactions are pending */
	FWIsoMgrIsCompatible,		/* Quadlet read to CSR 0x234 to determine if isochronous mgr is 1394a compatible */
	FWIsoManagerDelay100ms,		/* Delay approximately 100ms then determine if the isochronous manager is compatible */
	FWWriteBusMgrId				/* Broadcast compare swap to the bus manager ID CSR */

} FWBusMgrTransactionType;

typedef struct FWBusMgrTransactionStruct
{
	FWBusMgrTransactionType	transType;		/* The type of transaction submitted */
	unsigned long				tick;				/* Driver tick count from when the transaction was submitted */
	unsigned long				tickTimeout;	/* Number of tick counts when the transaction times out */
	FWTransaction				*transaction;	/* Transaction structure sent */
	struct FWBusMgrTransactionStruct *next;	/* Pointer to next item in the list */
	struct FWBusMgrTransactionStruct *prev;	/* Pointer to next item in the list */

} FWBusMgrTransaction;

typedef struct FWBusManagerDataStruct
{
	int						clientHandle;		/* Bus manager instance client handle. */
	boolean					isBusManager;		/* Set to TRUE if this node is the bus manager. */
	unsigned long			count;				/* number of bus manager transactions in the list */
	FWBusMgrTransaction	*head;				/* The head of the async transaction list or NULL if empty */
	FWBusMgrTransaction	*tail;				/* The head of the async transaction list or NULL if empty */

} FWBusManagerData;

typedef struct FWIsochronousMgrDataStruct
{
	unsigned short			isochronousMgrID;		/* Set to the node ID of the isochronous manager. 0xFFFF = no manager */
	FWSpeed					isochronousMgrSpeed;	/* The communication speed of the isochronous manager */

} FWIsochronousMgrData;

/* This structure defines the driver data areas
 * for a IEEE-1394 adapter card.
 */
typedef struct FWDriverDataStruct
{
	boolean						adapterEnabled;			/* TRUE if the adapter is enabled */
	FWCoreCSR					*coreCSR;					/* Core CSRs used by the IEEE-1394 standard */
	FWConfigROM					*configROM;					/* Config ROM CSR data areas */
	FWPhysicalData				*physicalLayerData;		/* Physical layer data area */
	FWLinkData					*linkLayerData;			/* Link layer data area */
	FWTransactionData			*transactionLayerData;	/* Transaction layer data area */
	FWPciAdapterData			*pciAdapterData;			/* PCI support data for the adapter */
	volatile OhciRegisters	*ohci;						/* OHCI hardware registers */
	FWDmaMemoryPool			*dmaMemoryPool;			/* Points to the head of the DMA memory pool. Can be NULL. */
	UINT32						interruptMask;				/* Interrupt mask is updated by the ISR routine. */
	UINT32						isoXmitInterruptMask;	/* Interrupt mask for isochronous transmit */
	UINT32						isoRecvInterruptMask;	/* Interrupt mask for isochronous receive */
	FWBusManagerData			*busManagerData;			/* State information for the bus manager */
	FWIsochronousMgrData		*isochronousMgrData;		/* State information for the isochronous resource manager */
	FWIsoChannelList			*isoChannelList;			/* List of isochronous channels */

} FWDriverData;

/* Driver data array.
 */
extern FWDriverData *fwDriverDataArray[FIREWIRE_MAX_PCI_ADAPTERS];

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_DRIVER_DATA_INCLUDE_ */

