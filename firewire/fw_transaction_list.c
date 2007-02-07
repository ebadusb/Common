/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains routines for a double link list specifically
 * for transaction structures.
 *
 * $Log$
 *
 */

#include <vxWorks.h>
#include "fw_utility.h"
#include "fw_transaction_list.h"

FWTransactionList *fwTransactionListCreate(void)
{
	FWTransactionList * newList = NULL;

	newList = (FWTransactionList*)fwMalloc(sizeof(FWTransactionList));

	if( newList )
	{
		newList->head = newList->tail = NULL;
		newList->count = 0;
		newList->listSem = semMCreate( SEM_Q_PRIORITY | SEM_INVERSION_SAFE );

		if( newList->listSem == NULL )
		{
			fwFree( newList );
			newList = NULL;
		}
	}

	return newList;
}

void fwTransactionListDestroy(FWTransactionList * list)
{
	if( list )
	{
		if( list->listSem )
			semDelete( list->listSem );

		fwFree( list );
	}
}

FWStatus fwTransactionListPushBack(FWTransactionList *list, FWTransaction *transaction)
{
	FWStatus retVal = FWInternalError;

	do
	{
		if( list == NULL ) break;
		if( transaction == NULL ) break;
		if( list->listSem == NULL ) break;

		if( semTake( list->listSem, WAIT_FOREVER ) != OK )
		{
			break;
		}

		if( list->tail )
		{
			transaction->next = NULL;
			transaction->prev = list->tail;
			list->tail->next = transaction;
			list->tail = transaction;
		}
		else
		{
			transaction->next = NULL;
			transaction->prev = NULL;
			list->tail = transaction;
			list->head = transaction;
		}

		list->count++;

		semGive( list->listSem );

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

FWStatus fwTransactionListPushFront(FWTransactionList *list, FWTransaction *transaction)
{
	FWStatus retVal = FWInternalError;

	do
	{
		if( list == NULL ) break;
		if( transaction == NULL ) break;
		if( list->listSem == NULL ) break;

		if( semTake( list->listSem, WAIT_FOREVER ) != OK )
		{
			break;
		}

		if( list->head )
		{
			transaction->prev = NULL;
			transaction->next = list->head;
			list->head->prev = transaction;
			list->head = transaction;
		}
		else
		{
			transaction->next = NULL;
			transaction->prev = NULL;
			list->tail = transaction;
			list->head = transaction;
		}

		list->count++;

		semGive( list->listSem );

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

FWTransaction *fwTransactionListPopFront(FWTransactionList *list)
{
	FWTransaction *transaction = NULL;

	do
	{
		if( list == NULL ) break;
		if( list->listSem == NULL ) break;

		if( semTake( list->listSem, WAIT_FOREVER ) != OK )
		{
			break;
		}

		if( list->head )
		{
			transaction = list->head;
			list->head = list->head->next;

			if( list->head == NULL )
			{
				list->tail = NULL;
			}

			transaction->prev = transaction->next = NULL;

			list->count--;
		}

		semGive( list->listSem );

	} while(0);


	return transaction;
}

FWTransaction *fwTransactionListPopBack(FWTransactionList *list)
{
	FWTransaction *transaction = NULL;

	do
	{
		if( list == NULL ) break;
		if( list->listSem == NULL ) break;

		if( semTake( list->listSem, WAIT_FOREVER ) != OK )
		{
			break;
		}

		if( list->tail )
		{
			transaction = list->tail;
			list->tail = list->tail->prev;

			if( list->tail == NULL )
			{
				list->head = NULL;
			}

			transaction->prev = transaction->next = NULL;

			list->count--;
		}

		semGive( list->listSem );

	} while(0);

	return transaction;
}

FWTransaction *fwFindTransactionListItem(FWTransactionList *list, const FWTransaction *cmpTrans, int (*compFunc)(const FWTransaction*, const FWTransaction*))
{
	FWTransaction *transaction = NULL;

	do
	{
		if( list == NULL ) break;
		if( list->listSem == NULL ) break;

		if( semTake( list->listSem, WAIT_FOREVER ) != OK )
		{
			break;
		}

		transaction = list->head;

		while( transaction )
		{
			if( compFunc )
			{
				if( compFunc( cmpTrans, transaction ) == 0 )
				{
					break;
				}
			}
			else
			{
				if( cmpTrans == transaction )
				{
					break;
				}
			}

			transaction = transaction->next;
		}

		semGive( list->listSem );

	} while(0);

	return transaction;
}

FWTransaction *fwRemoveTransactionListItem(FWTransactionList *list, const FWTransaction *cmpTrans, int (*compFunc)(const FWTransaction*, const FWTransaction*))
{
	FWTransaction *transaction = NULL;
	FWTransaction *pNext;
	FWTransaction *pPrev;

	do
	{
		if( list == NULL ) break;
		if( list->listSem == NULL ) break;

		if( semTake( list->listSem, WAIT_FOREVER ) != OK )
		{
			break;
		}

		transaction = list->head;

		while( transaction )
		{
			if( compFunc )
			{
				if( compFunc( cmpTrans, transaction ) == 0 )
				{
					pNext = transaction->next;
					pPrev = transaction->prev;

					/* unlink the transaction from the list */
					if( pNext )
					{
						pNext->prev = pPrev;
					}
					else
					{
						list->tail = pPrev;
					}

					if( pPrev )
					{
						pPrev->next = pNext;
					}
					else
					{
						list->head = pNext;
					}

					transaction->next = transaction->prev = NULL;

					list->count--;
					break;
				}
			}
			else
			{
				if( cmpTrans == transaction )
				{
					pNext = transaction->next;
					pPrev = transaction->prev;

					/* unlink the transaction from the list */
					if( pNext )
					{
						pNext->prev = pPrev;
					}
					else
					{
						list->tail = pPrev;
					}

					if( pPrev )
					{
						pPrev->next = pNext;
					}
					else
					{
						list->head = pNext;
					}

					transaction->next = transaction->prev = NULL;

					list->count--;
					break;
				}
			}

			transaction = transaction->next;
		}

		semGive( list->listSem );

	} while(0);

	return transaction;
}

