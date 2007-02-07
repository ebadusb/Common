/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains routines for a double link list.
 *
 * $Log$
 *
 */

#include <vxWorks.h>
#include "fw_utility.h"
#include "fw_data_list.h"

FWDataList *fwListCreate(void)
{
	FWDataList * newList = NULL;

	newList = (FWDataList*)fwMalloc(sizeof(FWDataList));

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

void fwListDestroy(FWDataList * list, void destFunc(void *info))
{
	FWListDataItem *item;
	FWListDataItem *tempItem;

	if( list )
	{
		item = list->head;

		while( item )
		{
			tempItem = item;
			item = item->next;

			if( destFunc )
				destFunc( tempItem->listData );

			fwFree( tempItem );
		}

		semDelete( list->listSem );
		fwFree( list );
	}
}


FWStatus fwListPushBack(FWDataList *list, void *newInfo)
{
	FWStatus retVal = FWInternalError;
	FWListDataItem *dataItem;

	do
	{
		if( list == NULL ) break;
		if( newInfo == NULL ) break;
		if( list->listSem == NULL ) break;

		dataItem = (FWListDataItem*)fwMalloc(sizeof(FWListDataItem));

		if( dataItem == NULL )
		{
			retVal = FWMemAllocateError;
			break;
		}

		if( semTake( list->listSem, WAIT_FOREVER ) != OK )
		{
			break;
		}

		dataItem->listData = newInfo;

		if( list->tail )
		{
			dataItem->next = NULL;
			dataItem->prev = list->tail;
			list->tail->next = dataItem;
			list->tail = dataItem;
		}
		else
		{
			dataItem->next = NULL;
			dataItem->prev = NULL;
			list->tail = dataItem;
			list->head = dataItem;
		}

		list->count++;

		semGive( list->listSem );

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

FWStatus fwListPushFront(FWDataList *list, void *newInfo)
{
	FWStatus retVal = FWInternalError;
	FWListDataItem *dataItem;

	do
	{
		if( list == NULL ) break;
		if( newInfo == NULL ) break;
		if( list->listSem == NULL ) break;

		dataItem = (FWListDataItem*)fwMalloc(sizeof(FWListDataItem));

		if( dataItem == NULL )
		{
			retVal = FWMemAllocateError;
			break;
		}

		if( semTake( list->listSem, WAIT_FOREVER ) != OK )
		{
			break;
		}

		dataItem->listData = newInfo;

		if( list->head )
		{
			dataItem->prev = NULL;
			dataItem->next = list->head;
			list->head->prev = dataItem;
			list->head = dataItem;
		}
		else
		{
			dataItem->next = NULL;
			dataItem->prev = NULL;
			list->tail = dataItem;
			list->head = dataItem;
		}

		list->count++;

		semGive( list->listSem );

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

void *fwListPopFront(FWDataList *list)
{
	void *popInfo = NULL;
	FWListDataItem *dataItem;

	do
	{
		if( list == NULL ) break;
		if( list->head == NULL ) break;
		if( list->listSem == NULL ) break;

		if( semTake( list->listSem, WAIT_FOREVER ) != OK )
		{
			break;
		}

		popInfo = list->head->listData;
		dataItem = list->head;
		list->head = list->head->next;
		fwFree( dataItem );
		if( list->head == NULL )
		{
			list->tail = NULL;
		}

		list->count--;

		semGive( list->listSem );

	} while(0);

	return popInfo;
}

void *fwListPopBack(FWDataList *list)
{
	void *popInfo = NULL;
	FWListDataItem *dataItem;

	do
	{
		if( list == NULL ) break;
		if( list->tail == NULL ) break;
		if( list->listSem == NULL ) break;

		if( semTake( list->listSem, WAIT_FOREVER ) != OK )
		{
			break;
		}

		popInfo = list->tail->listData;
		dataItem = list->tail;
		list->tail = list->tail->prev;
		fwFree( dataItem );
		if( list->tail == NULL )
		{
			list->head = NULL;
		}

		list->count--;

		semGive( list->listSem );

	} while(0);

	return popInfo;
}

void *fwFindListItem(FWDataList *list, void *data, int (*compFunc)(const void*, const void*))
{
	void *info = NULL;
	FWListDataItem *dataItem;

	do
	{
		if( list == NULL ) break;
		if( list->head == NULL ) break;
		if( list->listSem == NULL ) break;

		if( semTake( list->listSem, WAIT_FOREVER ) != OK )
		{
			break;
		}

		dataItem = list->head;

		while( dataItem )
		{
			if( compFunc)
			{
				if( compFunc( data, dataItem->listData ) == 0 )
				{
					info = dataItem->listData;
					break;
				}
			}
			else
			{
				if( data == dataItem->listData )
				{
					info = dataItem->listData;
					break;
				}
			}

			dataItem = dataItem->next;
		}

		semGive( list->listSem );

	} while(0);

	return info;
}

void *fwRemoveListItem(FWDataList *list, void *data, int (*compFunc)(const void*, const void*))
{
	void *info = NULL;
	FWListDataItem *dataItem;
	FWListDataItem *pNext;
	FWListDataItem *pPrev;

	do
	{
		if( list == NULL ) break;
		if( list->head == NULL ) break;
		if( list->listSem == NULL ) break;

		if( semTake( list->listSem, WAIT_FOREVER ) != OK )
		{
			break;
		}

		dataItem = list->head;

		while( dataItem )
		{
			if( compFunc( data, dataItem->listData ) == 0 )
			{
				pNext = dataItem->next;
				pPrev = dataItem->prev;

				/* unlink the data item from the list */
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

				info = dataItem->listData;

				list->count--;

				fwFree( dataItem );

				break;
			}

			dataItem = dataItem->next;
		}

		semGive( list->listSem );

	} while(0);

	return info;
}

