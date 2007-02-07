/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains the CSR definitions and routines
 * to update and manage the standard CSRs.
 *
 * $Log$
 *
 */

#ifndef _FW_CSR_INCLUDE_
#define _FW_CSR_INCLUDE_

#include <vxWorks.h>
#include <firewire/fw_types.h>
#include <firewire/fw_error_codes.h>
#include <firewire/fw_driver_data.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Creates and initializes the core CSR registers.
 */
FWStatus fwCreateCoreCSR(FWDriverData *pDriver);

/* Destroys the core CSR registers.
 */
FWStatus fwDestroyCoreCSR(FWDriverData *pDriver);

/* Initialize the core CSR registers.
 */
FWStatus fwInitializeCoreCSR(FWDriverData *pDriver);

/* Configure the topology map after the self ID
 * process completes.
 */
FWStatus fwTopologyMapComplete(FWDriverData *pDriver, UINT32 generation);

/* Add a self ID to the topology map.
 */
FWStatus fwTopologyMapAddSelfID(FWDriverData *pDriver, UINT32 selfIDQuadlet);

/* Reset the topology map when a bus reset occurs.
 */
FWStatus fwTopologyMapReset(FWDriverData *pDriver);

/* Get a quadlet from the topology map
 */
FWStatus fwGetTopologyMapData(FWDriverData *pDriver, UINT32 offset, UINT32 *data);

/* Set the state clear register CSR offset 0x000
 */
FWStatus fwSetStateClearCSR(FWDriverData *pDriver, UINT32 value);

/* Set the state set register CSR offset 0x004
 */
FWStatus fwSetStateSetCSR(FWDriverData *pDriver, UINT32 value);

/* Get the state register CSR offset 0x000 or 0x004
 */
FWStatus fwGetStateCSR(FWDriverData *pDriver, UINT32 *value);

/* Node IDs register CSR offset 0x008
 */
FWStatus fwSetNodeIDCSR(FWDriverData *pDriver, UINT32 value);
FWStatus fwGetNodeIDCSR(FWDriverData *pDriver, UINT32 *value);

/* Reset start register CSR offset 0x00C
 */
FWStatus fwSetResetStartCSR(FWDriverData *pDriver, UINT32 value);
FWStatus fwGetResetStartCSR(FWDriverData *pDriver, UINT32 *value);

/* Split timeout register CSR offset 0x018 - 0x01C
 */
FWStatus fwSetSplitTimeoutCSR(FWDriverData *pDriver, UINT32 splitTimeoutHi, UINT32 splitTimeoutLow);
FWStatus fwGetSplitTimeoutCSR(FWDriverData *pDriver, UINT32 *splitTimeoutHi, UINT32 *splitTimeoutLow);

/* Cycle time register CSR offset 0x200
 */
FWStatus fwSetCycleTimeCSR(FWDriverData *pDriver, UINT32 value);
FWStatus fwGetCycleTimeCSR(FWDriverData *pDriver, UINT32 *value);

/* Bus time register CSR offset 0x204
 */
FWStatus fwSetBusTimeCSR(FWDriverData *pDriver, UINT32 value);
FWStatus fwGetBusTimeCSR(FWDriverData *pDriver, UINT32 *value);

/* Busy timeout register CSR offset 0x210
 */
FWStatus fwSetBusyTimeoutCSR(FWDriverData *pDriver, UINT32 value);
FWStatus fwGetBusyTimeoutCSR(FWDriverData *pDriver, UINT32 *value);

/* Bus manager ID register CSR offset 0x21C
 */
FWStatus fwSetBusMgrIdCSR(FWDriverData *pDriver, UINT32 compare, UINT32 *swap);
FWStatus fwGetBusMgrIdCSR(FWDriverData *pDriver, UINT32 *value);

/* Bandwidth available register CSR offset 0x220
 */
FWStatus fwSetBandwidthAvailableCSR(FWDriverData *pDriver, UINT32 compare, UINT32 *swap);
FWStatus fwGetBandwidthAvailableCSR(FWDriverData *pDriver, UINT32 *value);

/* Channels available Hi register CSR offset 0x224
 */
FWStatus fwSetChannelsAvailableHiCSR(FWDriverData *pDriver, UINT32 compare, UINT32 *swap);
FWStatus fwGetChannelsAvailableHiCSR(FWDriverData *pDriver, UINT32 *channelsHi);

/* Channels available Lo register CSR offset 0x228
 */
FWStatus fwSetChannelsAvailableLoCSR(FWDriverData *pDriver, UINT32 compare, UINT32 *swap);
FWStatus fwGetChannelsAvailableLoCSR(FWDriverData *pDriver, UINT32 *channelsLo);

/* Set the priority budget CSR
 */
FWStatus fwSetPriorityBudgetCSR(FWDriverData *pDriver, UINT32 priority);
FWStatus fwGetPriorityBudgetCSR(FWDriverData *pDriver, UINT32 *priority);

/* Set the priority budget CSR
 */
FWStatus fwSetBroadcastChannelCSR(FWDriverData *pDriver, UINT32 channel);
FWStatus fwGetBroadcastChannelCSR(FWDriverData *pDriver, UINT32 *channel);

FWCSRAddressMap *fwLookupCSRAddress(FWDriverData *pDriver, FWAddressOffset *offset, unsigned long dataLength);

unsigned char *fwValidateCSRMemoryAddress(FWCSRAddressMap *addressMap, FWAddressOffset *offset, unsigned long dataLength);

FWStatus fwLockCSRAddressMap(FWDriverData *pDriver);

FWStatus fwUnlockCSRAddressMap(FWDriverData *pDriver);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_CSR_INCLUDE_ */

