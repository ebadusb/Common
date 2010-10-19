/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains the client routines that
 * update or manage client resources.
 *
 * $Log: fw_client.c $
 * Revision 1.1  2007/02/07 15:22:28Z  wms10235
 * Initial revision
 *
 */

#include <vxWorks.h>
#include <string.h>
#include "fw_utility.h"
#include "fw_client.h"

/* Prototypes for local functions */
static void fwClientResourcePushback(FWClientResource *resource);
static void fwClientResourceRemove(FWClientResource *resource);
static FWClientResource *fwFindClientResource(int clientHandle);


/* Local variables */

/* Client linked list */
static FWClientResource *fwClientListHead = NULL;
static FWClientResource *fwClientListTail = NULL;
static SEM_ID	fwClientListSem = NULL;

FWStatus fwCreateClient(int *clientHandle, SEM_ID notifySem)
{
	FWStatus retVal = FWInternalError;
	boolean listSemLocked = FALSE;
	int handle = 1;
	FWClientResource *head = NULL;
	FWClientResource *tail = NULL;
	FWClientResource *fwClientRes;

	do
	{
		fwClientRes = (FWClientResource*)fwMalloc( sizeof(FWClientResource) );

		if( fwClientRes == NULL )
		{
			retVal = FWMemAllocateError;
			break;
		}

		memset( fwClientRes, 0, sizeof(FWClientResource) );

		fwClientRes->asyncSendTrans = (FWTransaction*)fwMalloc( sizeof(FWTransaction) );

		if( fwClientRes->asyncSendTrans == NULL )
		{
			retVal = FWMemAllocateError;
			break;
		}

		memset( fwClientRes->asyncSendTrans, 0, sizeof(FWTransaction) );

		if( notifySem )
		{
			fwClientRes->clientSem = notifySem;
		}
		else
		{
			fwClientRes->clientSem = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
		}

		if( fwClientRes->clientSem == NULL )
		{
			retVal = FWSemAllocateError;
			break;
		}

		if( fwClientListSem == NULL )
		{
			fwClientListSem = semMCreate( SEM_Q_PRIORITY | SEM_INVERSION_SAFE );

			if( fwClientListSem == NULL )
			{
				retVal = FWSemAllocateError;
				break;
			}
		}

		if( semTake( fwClientListSem, WAIT_FOREVER ) != OK )
		{
			retVal = FWInternalError;
			break;
		}

		listSemLocked = TRUE;

		/* Determine a handle for this client */
		if( fwClientListTail )
		{
			handle = fwClientListTail->clientHandle + 1;
		}

		fwClientRes->clientHandle = handle;
		*clientHandle = handle;

		fwClientResourcePushback( fwClientRes );

		retVal = FWSuccess;

	} while(0);

	if( listSemLocked )
		semGive( fwClientListSem );

	return retVal;
}

FWStatus fwDestroyClient(int clientHandle)
{
	FWStatus retVal = FWNotFound;
	FWClientResource *fwClientRes;

	fwClientRes = fwFindClientResource(clientHandle);

	if( fwClientRes )
	{
		/* remove the structure from the list */
		fwClientResourceRemove( fwClientRes );

		/* Delete the client's resources */
		if( fwClientRes->swapBuffer )
		{
			fwFree( fwClientRes->swapBuffer );
			fwClientRes->swapBuffer = NULL;
		}

		if( fwClientRes->asyncSendTrans )
		{
			/* Note don't free the databuffer in the
			 * transaction. It is owned by the client.
			 */
			fwFree( fwClientRes->asyncSendTrans );
			fwClientRes->asyncSendTrans = NULL;
		}

		if( fwClientRes->clientSem )
		{
			semDelete( fwClientRes->clientSem );
			fwClientRes->clientSem = NULL;
		}

		fwFree( fwClientRes );

		retVal = FWSuccess;
	}

	return retVal;
}

FWStatus fwGetClientResource(int clientHandle, FWClientResource **clientResource )
{
	FWStatus retVal = FWInternalError;

	if( clientResource )
	{
		*clientResource = fwFindClientResource( clientHandle );

		if( *clientResource )
		{
			retVal = FWSuccess;
		}
		else
		{
			retVal = FWNotFound;
		}
	}

	return retVal;
}

static void fwClientResourcePushback(FWClientResource *resource)
{
	if( resource )
	{
		if( semTake( fwClientListSem, WAIT_FOREVER ) == OK )
		{
			if( fwClientListTail )
			{
				fwClientListTail->next = resource;
				resource->prev = fwClientListTail;
				resource->next = NULL;
				fwClientListTail = resource;
			}
			else
			{
				fwClientListHead = resource;
				fwClientListTail = resource;
				resource->prev = NULL;
				resource->next = NULL;
			}

			semGive( fwClientListSem );
		}
	}
}

static void fwClientResourceRemove(FWClientResource *resource)
{
	FWClientResource *pNext;
	FWClientResource *pPrev;

	if( resource )
	{
		if( semTake( fwClientListSem, WAIT_FOREVER ) == OK )
		{
			pNext = resource->next;
			pPrev = resource->prev;

			if( pNext == NULL && pPrev == NULL )
			{
				if( resource == fwClientListHead )
				{
					fwClientListHead = NULL;
					fwClientListTail = NULL;
				}
			}
			else
			{
				if( pNext )
				{
					pNext->prev = pPrev;
					resource->next = NULL;
				}
				else
				{
					fwClientListTail = pPrev;
				}

				if( pPrev )
				{
					pPrev->next = pNext;
					resource->prev = NULL;
				}
				else
				{
					fwClientListHead = pNext;
				}
			}

			semGive( fwClientListSem );
		}
	}
}

static FWClientResource *fwFindClientResource(int clientHandle)
{
	FWClientResource *fwClientRes = NULL;

	if( semTake( fwClientListSem, WAIT_FOREVER ) == OK )
	{
		fwClientRes = fwClientListHead;

		while( fwClientRes )
		{
			if( fwClientRes->clientHandle == clientHandle )
			{
				break;
			}

			fwClientRes = fwClientRes->next;
		}

		semGive( fwClientListSem );
	}

	return fwClientRes;
}

