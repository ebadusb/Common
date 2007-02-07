/*
 *  Copyright(c) 2005 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header: H:/BCT_Development/vxWorks/Common/firewire/rcs/fw_types.h 1.2 2007/02/12 16:07:00Z wms10235 Exp wms10235 $
 *
 * This file contains type definitions for the firewire routines.
 *
 * $Log: fw_types.h $
 *
 */

#ifndef _FW_TYPES_INCLUDE_
#define _FW_TYPES_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include <semLib.h>

typedef enum FWSpeedEnum
{
	FWSInvalid,	/* Speed cannot be determined */
	FWS100,	/* 100Mbps */
	FWS200,	/* 200Mbps */
	FWS400	/* 400Mbps */

} FWSpeed;

/* boolean type */
typedef	int	boolean;

/* structure to hold a GUID */
typedef struct FWGUIDStruct
{
	unsigned long	guidHi;
	unsigned long	guidLow;

} FWGUID;

typedef struct FWAddressOffsetStruct
{
    UINT16		highOffset;
    UINT32		lowOffset;

} FWAddressOffset;

typedef struct FWAsyncTransactionCmdStruct
{
	int					clientHandle;			/* A unique integer identifier for the client task. */
	unsigned short		destinationID;			/* Destination ID for the IEEE-1394 device. */
	FWAddressOffset	destinationOffset;	/* Destination offset for the IEEE-1394 device. */
	unsigned long		dataLength;				/* The length in bytes of the data quadlets in the dataBuffer */
	unsigned long		dataBufferSize;		/* Size in bytes of the data buffer. */
	UINT32				*databuffer;			/* Data buffer allocated by client */
	FWSpeed				speed;					/* Desired transfer speed of the request. */

} FWAsyncTransactionCmd;

typedef struct FWIsochronousRecvCmdStruct
{
	int				channel;				/* Isochronous channel */
	unsigned long	totalSize;			/* Required total buffer size in bytes for the isochronous data */
	unsigned short	firstPacketSize;	/* Size in bytes of the first packet */
	unsigned short	sync;					/* Sync code used by the first packet */
	unsigned long	numBuffers;			/* Number of buffers to allocate */
	SEM_ID			clientSem;			/* Semaphore used to notify the isochronous client. */

} FWIsochronousRecvCmd;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_TYPES_INCLUDE_ */

