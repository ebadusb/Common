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

#ifndef _FW_DATA_LIST_INCLUDE_
#define _FW_DATA_LIST_INCLUDE_

#include <semLib.h>
#include <firewire/fw_types.h>
#include <firewire/fw_error_codes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FWListItemStruct
{
	void							*listData;
	struct FWListItemStruct	*next;
	struct FWListItemStruct	*prev;

} FWListDataItem;

typedef struct FWDataListStruct
{
	FWListDataItem	*head;	/* Pointer to the list head */
	FWListDataItem *tail;	/* Pointer to the list tail */
	unsigned long	count;	/* number of items inthe list */
	SEM_ID			listSem;	/* Mutex semaphore access */

} FWDataList;

/* Create a list allocates memory for a list
 * structure and initializes the structure
 */
FWDataList * fwListCreate(void);

/* List destroy deletes the list and elements of
 * the list if a destFunc is supplied. If destFunc
 * is NULL the items are not deleted.
 */
void fwListDestroy(FWDataList * list, void destFunc(void *listItem));

/* Add an item to the tail of the list.
 */
FWStatus fwListPushBack(FWDataList *list, void *data);

/* Add an item to the head of the list.
 */
FWStatus fwListPushFront(FWDataList *list, void *data);

/* Remove an item from the front of the list and
 * return a pointer to the item. If the list is
 * empty, NULL is returned.
 */
void *fwListPopFront(FWDataList *list);

/* Remove an item from the back of the list and
 * return a pointer to the item. If the list is
 * empty, NULL is returned.
 */
void *fwListPopBack(FWDataList *list);

/* Find an item in the list using the supplied
 * compare function. Returns NULL if not found.
 */
void *fwFindListItem(FWDataList *list, void *data, int (*compFunc)(const void*, const void*));

/* Remove an item from the list using the supplied
 * compare function. Returns NULL if the item was
 * not found.
 */
void *fwRemoveListItem(FWDataList *list, void *data, int (*compFunc)(const void*, const void*));

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_DATA_LIST_INCLUDE_ */

