/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains the firewire OHCI interface level routines
 * to access the physical layer.
 *
 * $Log$
 *
 */

#ifndef _FW_PHYSICAL_LAYER_INCLUDE_
#define _FW_PHYSICAL_LAYER_INCLUDE_

#include <firewire/fw_types.h>
#include <firewire/fw_error_codes.h>
#include <firewire/fw_driver_data.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Create the physical layer data structures and
 * allocate resources.
 */
FWStatus fwCreatePhysicalLayer(FWDriverData *pDriver);

/* Free allocated resources.
 */
FWStatus fwDestroyPhysicalLayer(FWDriverData *pDriver);

/* Initialize the physical layer. This function must be
 * called at least once to perform the initialization
 * of the physical layer.
 */
FWStatus fwInitializePhysical(FWDriverData *pDriver);

/* This function begins an IEEE-1394 bus reset cycle.
 * Any data transactions are aborted and a new bus
 * generation is created.
 */
FWStatus fwInitiateBusReset(FWDriverData *pDriver);

/*	Enables the physical layer so data can be sent and
 * received. This function must be called to enable
 * the the physical layer before parameters in the
 * physical layer can be set.
 */
FWStatus fwSetPhysicalEnable(FWDriverData *pDriver);

/*	Get the current setting of the physical enable flag.
 */
FWStatus fwGetPhysicalEnable(FWDriverData *pDriver, boolean *flag);

/* This function is used to enable or disable
 * the link status bit in transmitted packets.
 */
FWStatus fwSetPhyLinkActive(FWDriverData *pDriver, boolean flag);

/* Get the current setting of the physical link active flag.
 */
FWStatus fwGetPhyLinkActive(FWDriverData *pDriver, boolean *flag);

/* Set the communications gap count. Normally this
 * parameter is set by the hardware automatically.
 */
FWStatus fwSetGapCount(FWDriverData *pDriver, unsigned char gapCount);

/* Get the current communications gap count. Normally
 * this parameter is set by the hardware automatically.
 */
FWStatus fwGetGapCount(FWDriverData *pDriver, unsigned char *gapCount);

/* This sets the force root bit used in the self ID
 * process. Note that setting this bit does not
 * necessarily guarantee the node will be the root node.
 */
FWStatus fwSetForceRoot(FWDriverData *pDriver, boolean flag);

/* Get the root node status. Valid after self ID process completes.
 */
FWStatus fwGetRootNodeStatus(FWDriverData *pDriver, boolean *flag);

/* Returns true in flag if the power on the cable is
 * in the normal operating range.
 */
FWStatus fwGetCablePowerStatus(FWDriverData *pDriver, boolean *flag);

/* Return extended register information.
 */
FWStatus fwGetExtendedReg(FWDriverData *pDriver, int port, unsigned char *extendReg);

/* Number of physical ports present on the Firewire adapter.
 */
FWStatus fwGetNumPorts(FWDriverData *pDriver, int *numPorts);

/* Returns the highest physical speed the adapter is capable of.
 */
FWStatus fwGetPhySpeed(FWDriverData *pDriver, int port, FWSpeed *speed);

/* Physical layer repeater delay in nanoseconds.
 */
FWStatus fwGetDelay(FWDriverData *pDriver, int port, unsigned char *delay);

/* Enable or disable the flag that indicates this
 * node is a contender for bus and isochronous
 * resource manager.
 */
FWStatus fwSetContender(FWDriverData *pDriver, boolean flag);

/* Get the enabled/disabled status for the contender flag.
 */
FWStatus fwGetContender(FWDriverData *pDriver, int port, boolean *flag);

/* Returns the physical repeater jitter, which is
 * the difference between the fastest and slowest
 * repeater data delay.
 */
FWStatus fwGetJitter(FWDriverData *pDriver, int port, unsigned char *jitter);

/* Returns the node power class. The power class
 * describes the power requirements for a node.
 */
FWStatus fwGetPowerClass(FWDriverData *pDriver, int port, unsigned char *powerClass);

/* Enable or disable physical port interrupts.
 */
FWStatus fwClearPhyInterrupts(FWDriverData *pDriver, int port, unsigned char phyReg);

/* Get the enabled/disabled status for physical port interrupts.
 */
FWStatus fwGetPhyInterrupts(FWDriverData *pDriver, int port, unsigned char *phyReg);

/* Enable or disable the accelerated arbitration specified
 * in the IEEE-1394a-2000 specification.
 */
FWStatus fwSetAcceleratedArbitration(FWDriverData *pDriver, boolean flag);

/* Get the enabled/disabled status for the accelerated
 * arbitration flag.
 */
FWStatus fwGetAcceleratedArbitration(FWDriverData *pDriver, boolean *flag);

/* Enable or disable the ability to transmit concatenated
 * packets at different speeds.
 */
FWStatus fwSetEnableMultispeed(FWDriverData *pDriver, boolean flag);

/* Get the enabled/disabled status for the multi-speed enable flag.
 */
FWStatus fwGetEnableMultispeed(FWDriverData *pDriver, boolean *flag);

/* Enable or disable a port on the adapter.
 */
FWStatus fwSetPortEnable(FWDriverData *pDriver, int port, boolean flag);

/* Get the port status, enabled or disabled. Returns
 * FWRangeError if an invalid or non-existant port is
 * referenced.
 */
FWStatus fwGetPortEnable(FWDriverData *pDriver, int port, boolean *flag);

/* Returns the child/parent topology flag for the given port number.
 * flag=true if the port is a child port.
 */
FWStatus fwGetChildParent(FWDriverData *pDriver, int port, boolean *flag);

/* Returns true if the port is connected.
 */
FWStatus fwGetConnectStatus(FWDriverData *pDriver, int port, boolean *flag);

/* Returns true in flag if a stable cable bias is detected. Otherwise
 * false is returned.
 */
FWStatus fwGetCableBias(FWDriverData *pDriver, int port, boolean *flag);

/* Returns the highest physical speed that the connected
 * peer device is capable of.
 */
FWStatus fwGetPeerSpeed(FWDriverData *pDriver, int port, FWSpeed *speed);

/* Enable or disable interrupts for the given port number.
 */
FWStatus fwSetPortInterrupt(FWDriverData *pDriver, int port, boolean flag);

/* Returns the enabled or disabled interrupt state for a given port number.
 */
FWStatus fwGetPortInterrupt(FWDriverData *pDriver, int port, boolean *flag);

/* Clears the fault/error flag on a given port.
 */
FWStatus fwClearPortFault(FWDriverData *pDriver, int port);

/* Returns the set or clear status for the given port number.
 */
FWStatus fwGetPortFault(FWDriverData *pDriver, int port, boolean *flag);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_PHYSICAL_LAYER_INCLUDE_ */

