/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains the firewire transaction layer data structures.
 *
 * $Log$
 *
 */

#ifndef _FW_TRANSACTION_DATA_INCLUDE_
#define _FW_TRANSACTION_DATA_INCLUDE_

#include <firewire/fw_types.h>
#include <firewire/fw_error_codes.h>
#include <firewire/fw_data_list.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum FWTransactionCodeEnum
{
	FWWriteRequestQuadlet = 0x0,
	FWWriteRequestBlock = 0x1,
	FWWriteResponse = 0x2,
	FWReadRequestQuadlet = 0x4,
	FWReadRequestBlock = 0x5,
	FWReadResponseQuadlet = 0x6,
	FWReadResponseBlock = 0x7,
	FWCycleStart = 0x8,
	FWLockRequest = 0x9,
	FWIsoDataBlock = 0xA,
	FWLockResponse = 0xB,
	FWPhysicalPacket = 0xE,
	FWInvalidTransCode = 0xF

} FWTransactionCode;

typedef enum FWExtendedCodeEnum
{
	FWInvalidExtendedCode = 0x0,
	FWMaskSwap = 0x1,
	FWCompareSwap = 0x2,
	FWFetchAdd = 0x3,
	FWLittleAdd = 0x4,
	FWBoundedAdd = 0x5,
	FWWrapAdd = 0x6,
	FWVendorDependent = 0x7

} FWExtendedCode;

typedef enum FWResponseCodeEnum
{
	FWResponseComplete = 0,
	FWResponseConflictError = 4,
	FWResponseDataError = 5,
	FWResponseTypeError = 6,
	FWResponseAddressError = 7,
	FWInvalidResponseCode = 0xF

} FWResponseCode;

typedef enum FWRetryCodeEnum
{
	FWRetry1 = 0,
	FWRetryX = 1,
	FWRetryA = 2,
	FWRetryB = 3

} FWRetryCode;

typedef enum FWPacketEventCodeEnum
{
	FWEventNoStatus = 0x00,
	FWEventLongPacket = 0x02,
	FWEventMissingAck = 0x03,
	FWEventUnderrun = 0x04,
	FWEventOverrrun = 0x05,
	FWEventDescriptorRead = 0x06,
	FWEventDataRead = 0x07,
	FWEventDataWrite = 0x08,
	FWEventBusReset = 0x09,
	FWEventTimeout = 0x0A,
	FWEventTransCodeError = 0x0B,
	FWEventUnknown = 0x0E,
	FWEventFlushed = 0x0F,
	FWAckComplete = 0x11,
	FWAckPending = 0x12,
	FWAckBusyX = 0x14,
	FWAckBusyA = 0x15,
	FWAckBusyB = 0x16,
	FWAckTardy = 0x1B,
	FWAckDataError = 0x1D,
	FWAckTypeError = 0x1E

} FWPacketEventCode;

typedef struct FWTransactionStruct
{
	unsigned long		transactionID;			/* Transaction ID or serial number that uniquely identifies the transaction */
	int					clientHandle;			/* A unique integer identifier for the client task. */
	FWStatus				status;					/* Transaction status. */
	unsigned short		sourceID;				/* Source ID for the IEEE-1394 device. */
	unsigned short		destinationID;			/* Destination ID for the IEEE-1394 device. */
	FWAddressOffset	destinationOffset;	/* Destination offset for the IEEE-1394 device. */
	FWTransactionCode	transactionCode;		/* Type of transaction to be performed. */
	FWExtendedCode		extendedCode;			/* Extended transaction codes are used with lock commands. */
	FWRetryCode			retryCode;				/* Type of retry processing to use for the transaction. */
	unsigned short		xferStatus;				/* Transfer status reported from the context control register */
	unsigned char		transactionLabel;		/* 6-bit transaction label. */
	FWResponseCode		responseCode;			/* Filled in when response is sent for a request. */
	unsigned short		timeStamp;				/* Transaction timestamp see OHCI v1.1 section 7.1.5.3 */
	unsigned long		dataLength;				/* The length of the data in bytes in the dataBuffer */
	unsigned long		dataBufferSize;		/* Size in bytes of the data buffer. */
	unsigned char*		databuffer;				/* Data buffer allocated by client */
	FWSpeed				speed;					/* Desired transfer speed of the request or received speed. */
   unsigned long		busGeneration;			/* Bus generation number when request was issued. */
	SEM_ID				semId;					/* Semaphore for the transaction. */
	struct FWTransactionStruct *next;		/* Pointer to next transaction structure or NULL if this is the tail of the list. */
	struct FWTransactionStruct *prev;		/* Pointer to previous transaction structure or NULL if this is the head of the list. */

} FWTransaction;

typedef struct FWTransactionListStruct
{
	FWTransaction	*head;	/* Pointer to the list head */
	FWTransaction	*tail;	/* Pointer to the list tail */
	unsigned long	count;	/* number of items inthe list */
	SEM_ID			listSem;	/* Mutex semaphore access */

} FWTransactionList;

typedef struct FWTransactionLabelStruct
{
	unsigned short	nodeID;						/* Node ID for the IEEE-1394 device. */
	unsigned char	labelIndex;					/* Index to next available label */
	char				labelUsed[64];				/* Label used flag 0=available, 1=used */
	struct FWTransactionLabelStruct *next;	/* Pointer to next label structure or NULL if this is the tail of the list. */
	struct FWTransactionLabelStruct *prev;	/* Pointer to previous label structure or NULL if this is the head of the list. */

} FWTransactionLabel;

/* This structure contains the FIFO queues for incomming
 * and outgoing async messages. For outgoing message, the
 * requests are queued in the request TX list until they
 * are transmited. Once sent, they transaction is moved to
 * the response TX queue where the coresponding response
 * message will update the status of the transaction.
 * Incomming messages are queued in the request RX list.
 * The bus manager will service the request and place a
 * response in the response RX list.
 */
typedef struct FWTransactionDataStruct
{
	boolean					initialized;				/* Non-zero value if the transaction layer is initialized */
	FWTransactionLabel	*transactionLabels;		/* Transaction label list for outgoing messages. */
	FWTransactionList		*outgoingRequestList;	/* Outgoing async transaction request queue. */
	FWTransactionList		*outgoingResponseList;	/* Outgoing async transaction response queue. */
	FWTransactionList		*incommingRequestList;	/* Incomming async transaction request queue. */
	FWTransactionList		*incommingResponseList;	/* Incomming async transaction response queue. */
	boolean					newRequestTransaction;	/* Non zero if a new request TX transaction is on the queue */
	boolean					newResponseTransaction;	/* Non zero if a new response TX transaction is on the queue */
	FWTransaction			*recvResponseTrans;		/* Generic transaction used to receive response messages */
	FWTransaction			*recvRequestTrans;		/* Generic transaction used to receive request messages */
	unsigned long			lastRecvGeneration;		/* Bus generation for incomming async requests */
	SEM_ID					transSemId;					/* Semaphore to synchronize access to the transaction layer. */

} FWTransactionData;

typedef struct FWRequestCSRStruct
{
	FWAddressOffset	lowerBound;				/* Lower bound of the address offset range */
	FWAddressOffset	upperBound;				/* Upper bound of the address offset range */
	struct FWRequestCSRStruct *next;			/* Next element in the list or NULL if the list tail */
	struct FWRequestCSRStruct *prev;			/* Previous element in the list or NULL if the list head */

} FWRequestCSR;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_TRANSACTION_DATA_INCLUDE_ */

