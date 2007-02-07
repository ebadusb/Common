/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header: H:/BCT_Development/vxWorks/Common/firewire/rcs/fw_driver.h 1.2 2007/02/12 16:06:59Z wms10235 Exp wms10235 $
 *
 * This file contains the firewire driver level routines.
 *
 * $Log: fw_driver.h $
 *
 */

#ifndef _FW_DRIVER_INCLUDE_
#define _FW_DRIVER_INCLUDE_

#include <firewire/fw_types.h>
#include <firewire/fw_error_codes.h>
#include <firewire/fw_driver_data.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Create the driver data structures and
 * allocate resources.
 */
FWStatus fwCreateDriverData(FWDriverData *pDriver);

/* Free allocated resources.
 */
FWStatus fwDestroyDriverData(FWDriverData *pDriver);

/* This command performs a bus reset, discards all pending transactions,
 * and disables transmission and reception of packets. Data requests
 * from applications are not accepted.
 */
FWStatus fwReset(int adapter);

/* This command performs a bus reset, discards all pending transactions,
 * and enables transmission and reception of packets. Data requests from
 * applications are accepted when the initialization completes.
 */
FWStatus fwInitialize(int adapter);

/* Enable a given adapter.
 */
FWStatus fwEnableAdapter(int adapter);

/* Determine if the specified device is present on the bus.
 */
FWStatus fwDevicePresent(int adapter, int deviceID);

/* Request that the physical layer send a configuration packet.
 */
FWStatus fwPhysicalConfig(int adapter, UINT32 phyQuadlet);

/* Start a transaction to write data to a node on the IEEE-1394 bus.
 */
FWStatus fwAsyncWrite(int adapter, const FWAsyncTransactionCmd *asyncCmd, int timeout);

/* Start a transaction to read data from a node on the IEEE-1394 bus.
 */
FWStatus fwAsyncRead(int adapter, FWAsyncTransactionCmd *asyncCmd, int timeout);

/* Perform a compare swap lock transaction. The first quadlet in the
 * data is the compare value and the second the swap value.
 */
FWStatus fwCompareSwap(int adapter, FWAsyncTransactionCmd *asyncCmd, int timeout);

/* The bus manager will issue a link request to the specified node.
 * If the device is powered through the cable, power requirements
 * are assessed and allocated if available.
 */
FWStatus fwLinkOnRequest(int adapter, int deviceID, int timeout);

/* Allocate power to a device that is powered through the Firewire cable.
 */
FWStatus fwDevicePowerRequest(int adapter, int deviceID);

/* A task must register as a IEEE-1394 client and obtain a client
 * handle before executing IEEE-1394 commands.
 */
FWStatus fwRegisterClient(int *clientHandle);

/* A task must unregister to free resources when it will
 * nolonger use the IEEE-1394 bus.
 */
FWStatus fwUnregisterClient(int clientHandle);

/*	Allocate bandwidth an isochronous channel. The channel
 * is allocated until fwIsoCloseChannel is called.
 */
FWStatus fwIsoOpenChannel(int adapter, int channel, int bandwidth);

/*	Close an already allocated isochronous channel.
 */
FWStatus fwIsoCloseChannel(int adapter, int channel);

/*	Setup and initialize an isochronous channel to receive data.
 */
FWStatus fwIsoInitReceive(int adapter, FWIsochronousRecvCmd *command);

/*	Start isochronous send or receive operations on a channel.
 */
FWStatus fwIsoStartChannel(int adapter, int channel);

/*	Stop isochronous send or receive operations on a channel.
 */
FWStatus fwIsoStopChannel(int adapter, int channel);

/* Read data from an isochronous receive channel.
 */
FWStatus fwIsoReadRecvData(int adapter, int channel, unsigned char *pBuffer, unsigned long bufferSize, UINT32 *xferStatus);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_DRIVER_INCLUDE_ */

