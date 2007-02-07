/*
 *  Copyright(c) 2007 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains the firewire isochronous communications
 * routines.
 *
 * $Log$
 *
 */

#ifndef _FW_ISOCHRONOUS_LAYER_INCLUDE_
#define _FW_ISOCHRONOUS_LAYER_INCLUDE_

#include <firewire/fw_types.h>
#include <firewire/fw_error_codes.h>
#include <firewire/fw_driver_data.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Create the isochronous layer.
 */
FWStatus fwCreateIsochronousLayer(FWDriverData *pDriver);

/* Create the isochronous layer.
 */
FWStatus fwDestroyIsochronousLayer(FWDriverData *pDriver);

/*	Allocate bandwidth an isochronous channel. The channel
 * is allocated until fwIsoCloseChannel is called.
 */
FWStatus fwIsoAllocateChannel(FWDriverData *pDriver, int channel, int bandwidth);

/*	Release (unallocate) an already allocated isochronous channel.
 */
FWStatus fwIsoReleaseChannel(FWDriverData *pDriver, int channel);

/*	Setup and initialize an isochronous channel to receive data.
 */
FWStatus fwIsoInitRecvChannel(FWDriverData *pDriver, FWIsochronousRecvCmd *command);

/*	Enable isochronous send or receive operations on a channel.
 */
FWStatus fwIsoEnableChannel(FWDriverData *pDriver, int channel);

/*	Disable isochronous send or receive operations on a channel.
 */
FWStatus fwIsoDisableChannel(FWDriverData *pDriver, int channel);

/*	Advance the buffer for a receive context. (Only called by the bus manager.)
 */
FWStatus fwIsoAdvanceRecvBuffer(FWDriverData *pDriver, FWIsoChannel *isoChannel);

/* Read data from an isochronous receive channel.
 */
FWStatus fwIsoReadRecvBuffer(FWDriverData *pDriver, int channel, unsigned char *pBuffer, unsigned long bufferSize, UINT32 *xferStatus);

/* Create a list allocates memory for a list
 * structure and initializes the structure
 */
FWIsoChannelList * fwIsoChannelListCreate(void);

/* List destroy deletes the list and elements of
 * the list if a destFunc is supplied. If destFunc
 * is NULL the items are not deleted.
 */
void fwIsoChannelListDestroy(FWIsoChannelList *list);

/* Add an item to the tail of the list.
 */
FWStatus fwIsoChannelListPushBack(FWIsoChannelList *list, FWIsoChannel *isoChannel);

/* Remove an item from the front of the list and
 * return a pointer to the item. If the list is
 * empty, NULL is returned.
 */
FWIsoChannel *fwIsoChannelListPopFront(FWIsoChannelList *list);

/* Find an item in the list using the supplied
 * compare function. Returns NULL if not found.
 */
FWIsoChannel *fwFindIsoChannelListItem(FWIsoChannelList *list, int channel);

/* Remove an item from the list using the supplied
 * compare function. Returns NULL if the item was
 * not found.
 */
FWIsoChannel *fwRemoveIsoChannelListItem(FWIsoChannelList *list, int channel);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_ISOCHRONOUS_LAYER_INCLUDE_ */

