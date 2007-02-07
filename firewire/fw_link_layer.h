/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains the firewire level routines to access
 * and manipulate the link layer.
 *
 * $Log$
 *
 */

#ifndef _FW_LINK_LAYER_INCLUDE_
#define _FW_LINK_LAYER_INCLUDE_

#include <vxWorks.h>
#include <firewire/fw_types.h>
#include <firewire/fw_error_codes.h>
#include <firewire/fw_driver_data.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Create the link layer data structures and
 * allocate resources.
 */
FWStatus fwCreateLinkLayer(FWDriverData *pDriver);

/* Free allocated resources.
 */
FWStatus fwDestroyLinkLayer(FWDriverData *pDriver);

/* Reset the link layer to a quiescent state.
 * Any pending communications are aborted.
 */
FWStatus fwResetLink(FWDriverData *pDriver);

/* Initialize the link layer. This function must be called at
 * least once to perform the initialization of the link layer.
 */
FWStatus fwInitializeLink(FWDriverData *pDriver);

/* Enable the link layer. The link layer is enabled
 * to allow packets to be sent and received. A bus
 * reset is issued once the link layer is enabled.
 * This command should be issued when the driver
 * is ready to begin operations.
 */
FWStatus fwEnableLink(FWDriverData *pDriver);

/*	Enable or disable cycle start packets.
 */
FWStatus fwSetCycleMaster(FWDriverData *pDriver, boolean flag);

/*	Get the cycle master enabled or disabled status.
 */
FWStatus fwGetCycleMaster(FWDriverData *pDriver, boolean *flag);

/* Set the cycle time.
 */
FWStatus fwSetCycleTime(FWDriverData *pDriver, UINT32 cycleTime);

/* Get the cycle time.
 */
FWStatus fwGetCycleTime(FWDriverData *pDriver, UINT32 *cycleTime);

/* Send a physical packet onto the bus.
 */
FWStatus fwSendPhysicalPacket(FWDriverData *pDriver, UINT32 physPacket);

/*	Transmit an asynchronous message to another Firewire node.
 */
FWStatus fwSendAsyncMessage(FWDriverData *pDriver, FWTransaction *transaction);

/* Free the request transmit context resouces. This function
 * is called when the transmit has completed.
 */
FWStatus fwAsyncTxRequestComplete(FWDriverData *pDriver);

/* Free the response transmit context resouces. This function
 * is called when the transmit has completed.
 */
FWStatus fwAsyncTxResponseComplete(FWDriverData *pDriver);

/*	Cancel an asynchronous message that was sent to the link layer.
 */
FWStatus fwCancelAsyncMessage(FWDriverData *pDriver, FWTransaction *transaction);

/* Read an asynchronous request message.
 */
FWStatus fwReadAsyncRequestMessage(FWDriverData *pDriver, FWTransaction *transaction);

/* Read an asynchronous response message.
 */
FWStatus fwReadAsyncResponseMessage(FWDriverData *pDriver, FWTransaction *transaction);

/* Set the asynchronous request filter to
 * enable request from all nodes.
 */
FWStatus fwEnableAsyncRequests(FWDriverData *pDriver);

/* Read the local nodes GUID.
 */
FWStatus fwGetGUID(FWDriverData *pDriver, FWGUID *adapterGuid);

/* Get the bus number and physical node number for this link.
 */
FWStatus fwGetNodeID(FWDriverData *pDriver, unsigned short *busNumber, unsigned char *physNodeNumber);

/* Get the source ID (the combination of bus number and node ID)
 */
FWStatus fwGetSourceID(FWDriverData *pDriver, unsigned short *sourceID);

/* Set the bus number for this link.
 * Note that the node number is always
 * set through the self ID process.
 */
FWStatus fwSetBusNumber(FWDriverData *pDriver, unsigned short busNumber);

/* Set the split subaction timeout value. The timeout uses seconds and
 * fractions of seconds (one fraction is 1/8000 of a second).
 */
FWStatus fwSetSplitTransTimeout(FWDriverData *pDriver, int seconds, int fraction);

/* Set the number of times the link layer is allowed to
 * retry async messages for single phase retry operation.
 * 0 to 15 retries are allowed.
 */
FWStatus fwSetAsyncRetryLimit(FWDriverData *pDriver, int maxATRetries);

/* Get the number of times the link layer is allowed to
 * retry async messages for single phase retry operation.
 * 0 to 15 retries are allowed.
 */
FWStatus fwGetAsyncRetryLimit(FWDriverData *pDriver, int *maxATRetries);

/* Set the number of times the link layer is allowed
 * to retry physical response messages. 0 to 15 retries
 * are allowed.
 */
FWStatus fwSetPhysicalRetryLimit(FWDriverData *pDriver, int maxPhyRespRetries);

/* Get the number of times the link layer is allowed
 * to retry physical response messages.
 * 0 to 15 retries are allowed.
 */
FWStatus fwGetPhysicalRetryLimit(FWDriverData *pDriver, int *maxPhyRespRetries);

/* Set the cycle limit for dual phase retry.
 */
FWStatus fwSetCycleLimit(FWDriverData *pDriver, unsigned short cycleLimit);

/* Get the cycle limit for dual phase retry.
 */
FWStatus fwGetCycleLimit(FWDriverData *pDriver, unsigned short *cycleLimit);

/* Used to set hardware CSR registers
 */
FWStatus fwSetBuiltinCSR(FWDriverData *pDriver, UINT32 compare, UINT32 *swap, UINT32 control);

/* Used to get hardware CSR registers
 */
FWStatus fwGetBuiltinCSR(FWDriverData *pDriver, UINT32 *value, UINT32 control);

/* Set the priority budget (also known as fairness control)
 */
FWStatus fwSetPriorityBudget(FWDriverData *pDriver, unsigned char priority);
FWStatus fwGetPriorityBudget(FWDriverData *pDriver, unsigned char *priority);

/* Set the maximum for the priority budget
 */
FWStatus fwSetMaxPriorityBudget(FWDriverData *pDriver, unsigned char maxPriorityBudget);
FWStatus fwGetMaxPriorityBudget(FWDriverData *pDriver, unsigned char *maxPriorityBudget);

/* Get the maximum data payload supported by this link
 * for async block transactions.
 */
unsigned long fwGetMaxPayload(void);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_LINK_LAYER_INCLUDE_ */

