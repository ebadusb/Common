/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains the firewire OHCI interface level routines
 * to access the transaction layer.
 *
 * $Log$
 *
 */

#ifndef _FW_TRANSACTION_LAYER_INCLUDE_
#define _FW_TRANSACTION_LAYER_INCLUDE_

#include <firewire/fw_types.h>
#include <firewire/fw_error_codes.h>
#include <firewire/fw_driver_data.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Create the transaction layer data structures and
 * allocate resources.
 */
FWStatus fwCreateTransactionLayer(FWDriverData *pDriver);

/* Free allocated resources.
 */
FWStatus fwDestroyTransactionLayer(FWDriverData *pDriver);

/* This function must be called at least once to initialize
 * the transaction layer. Any pending transactions are discarded
 * when this function is called. When the initialization is
 * complete, the transaction layer will accept transactions
 * from clients.
 */
FWStatus fwInitializeTransaction(FWDriverData *pDriver);

/* Reset the transaction layer. Pending transactions are
 * discarded and new transactions are not accepted until
 * fwTransactionInitialize is called.
 */
FWStatus fwResetTransaction(FWDriverData *pDriver);

/* Post a transaction to the async TX request queue to write
 * data to the destination node on the IEEE-1394 bus.
 */
FWStatus fwPostAsyncWriteRequest(FWDriverData *pDriver, FWTransaction *transaction);

/* Post a transaction to the async TX request queue to read
 * data from the destination node on the IEEE-1394 bus.
 */
FWStatus fwPostAsyncReadRequest(FWDriverData *pDriver, FWTransaction *transaction);

/* Post a transaction to the async TX request queue to perform
 * a lock operation on the destination node.
 */
FWStatus fwPostAsyncLockRequest(FWDriverData *pDriver, FWTransaction *transaction);

/* Send the transaction at the head of the TX request queue
 * to the link layer to be transmitted. If the link layer
 * accepts the transaction, the transaction is moved from
 * the TX request queue to the TX response queue.
 */
FWStatus fwSendAsyncRequestTransaction(FWDriverData *pDriver);

/* Read an async RX response from the link layer and match
 * the response to the request.
 */
FWStatus fwRecvAsyncResponseTransaction(FWDriverData *pDriver);

/* Post a transaction to the async TX response queue to respond
 * to a request made by another node.
 */
FWStatus fwPostAsyncResponse(FWDriverData *pDriver, FWTransaction *transaction);

/* Send an async response transaction to the link layer. If the
 * link layer ready, the async message will be transmitted onto
 * the IEEE-1394 bus.
 */
FWStatus fwSendAsyncResponseTransaction(FWDriverData *pDriver);

/* Once an async response is transmitted. This function is used
 * to check the transaction status and free any resources used
 * by the transaction.
 */
FWStatus fwCompleteAsyncResponseTransaction(FWDriverData *pDriver);

/* Process an incomming async requests from other nodes. This
 * function is called in response to the async request packet
 * interrupt. Note however that this function is not called
 * within the interrupt.
 */
FWStatus fwRecvAsyncRequestTransaction(FWDriverData *pDriver);

/* Cancels a pending transaction.
 */
FWStatus fwCancelTransaction(FWDriverData *pDriver, FWTransaction *transaction);

/* Remove a transaction from the list.
 */
FWStatus fwRemoveTransaction(FWDriverData *pDriver, FWTransaction *transaction);

/* Initialize a transaction for use in a async
 * request transaction.
 */
FWStatus fwInitializeAsyncRequestTransaction(FWDriverData *pDriver, const FWAsyncTransactionCmd *asyncCmd, FWTransaction *transaction);

/* Update the transaction status for the outgoing
 * request queue. When the status is TRUE, there
 * are outbound requests waiting to be processed.
 */
FWStatus fwUpdateRequestTXStatus(FWDriverData *pDriver, boolean newStatus);

/* Update the transaction status for the incomming
 * response queue. When the status is TRUE, there
 * are outbound responses waiting to be processed.
 */
FWStatus fwUpdateResponseTXStatus(FWDriverData *pDriver, boolean newStatus);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_TRANSACTION_LAYER_INCLUDE_ */

