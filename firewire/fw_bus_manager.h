/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains the firewire routines for the Bus Manager.
 *
 * $Log$
 *
 */

#ifndef _FW_BUS_MANAGER_INCLUDE_
#define _FW_BUS_MANAGER_INCLUDE_

#include <firewire/fw_types.h>
#include <firewire/fw_error_codes.h>
#include <firewire/fw_driver_data.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Bus manager task entry point. This function is called
 * to setup and start the firewire driver processing. When
 * called, this function will not return until the
 * function fwBusManagerShutdown() is called.
 */
int fwBusManagerStartup(void);

/* Calling this function will shutdown the bus manager
 * in a graceful manner.
 */
void fwBusManagerShutdown(void);

/* Create the bus manager data structures and
 * allocate resources.
 */
FWStatus fwCreateBusManager(void);

/* Free allocated resources.
 */
FWStatus fwDestroyBusManager(void);

/* Process any pending interrupt events.
 */
FWStatus fwProcessEvents(void);

/* Process any new async transactions.
 */
FWStatus fwProcessAsyncTransactions(void);

/* Notify the bus manager that an event occurred. This routine
 * gives the fwBusManagerSemId binary semaphore.
 */
FWStatus fwNotifyBusManager(void);

/* Determine if there is any vendor specific chip configuration.
 */
FWStatus fwChipSpecificConfig(FWDriverData *pDriver);

/* Vendor specific configuration routine for Texas Instruments IEEE-1394 chips.
 */
FWStatus fwTIChipConfig(FWDriverData *pDriver);

/* This method enables interrupts on the adapter card.
 */
FWStatus fwEnableInterrupts(FWDriverData *pDriver);

/* This method disables interrupts on the adapter card.
 */
FWStatus fwDisableInterrupts(FWDriverData *pDriver);

/* This function is the standard firewire interrupt handler.
 */
void fwInterruptHandler(int adapterIndex);

/* This function returns the current interrupt masks from
 * the interruptMask, isoXmitInterruptMask, and
 * isoRecvInterruptMask.  The masks are then cleared.
 */
FWStatus fwGetInterruptMask(FWDriverData *pDriver, UINT32 *interruptMask, UINT32 *isoXmitInterruptMask, UINT32 *isoRecvInterruptMask);

/* Return the IEEE-1394 cycle clock 64 tick count.
 */
FWStatus fwGetDriverTick(FWDriverData *pDriver, unsigned long *tick);

void fwTestBusManager(void);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_BUS_MANAGER_INCLUDE_ */

