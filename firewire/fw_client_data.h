/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header: H:/BCT_Development/vxWorks/Common/firewire/rcs/fw_client_data.h 1.1 2007/02/07 15:22:28Z wms10235 Exp $
 *
 * This file contains the client data structure definitions.
 *
 * $Log: fw_client_data.h $
 * Revision 1.1  2007/02/07 15:22:28Z  wms10235
 * Initial revision
 *
 */

#ifndef _FW_CLIENT_DATA_INCLUDE_
#define _FW_CLIENT_DATA_INCLUDE_

#include <vxWorks.h>
#include <semLib.h>
#include <firewire/fw_transaction_data.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FWClientResourceStruct
{
	int				clientHandle;		/* Unique client handle */
	SEM_ID			clientSem;			/* Semaphore to signal client when a request or response completes */
	FWTransaction	*asyncSendTrans;	/* Transaction used for async send operations */
	UINT32			*swapBuffer;		/* Buffer used for byte swapping */
	unsigned long	swapBufferSize;	/* Byte swap buffer size in bytes */
	struct FWClientResourceStruct *next;	/* Next structure in the list or NULL */
	struct FWClientResourceStruct *prev;	/* Previous structure in the list or NULL */

} FWClientResource;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_CLIENT_DATA_INCLUDE_ */

