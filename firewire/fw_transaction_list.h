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

#ifndef _FW_TRANSACTION_LIST_INCLUDE_
#define _FW_TRANSACTION_LIST_INCLUDE_

#include <semLib.h>
#include <firewire/fw_types.h>
#include <firewire/fw_error_codes.h>
#include <firewire/fw_transaction_data.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Create a list allocates memory for a list
 * structure and initializes the structure
 */
FWTransactionList * fwTransactionListCreate(void);

/* List destroy deletes the list and elements of
 * the list if a destFunc is supplied. If destFunc
 * is NULL the items are not deleted.
 */
void fwTransactionListDestroy(FWTransactionList *list);

/* Add an item to the tail of the list.
 */
FWStatus fwTransactionListPushBack(FWTransactionList *list, FWTransaction *transaction);

/* Add an item to the head of the list.
 */
FWStatus fwTransactionListPushFront(FWTransactionList *list, FWTransaction *transaction);

/* Remove an item from the front of the list and
 * return a pointer to the item. If the list is
 * empty, NULL is returned.
 */
FWTransaction *fwTransactionListPopFront(FWTransactionList *list);

/* Remove an item from the back of the list and
 * return a pointer to the item. If the list is
 * empty, NULL is returned.
 */
FWTransaction *fwTransactionListPopBack(FWTransactionList *list);

/* Find an item in the list using the supplied
 * compare function. Returns NULL if not found.
 */
FWTransaction *fwFindTransactionListItem(FWTransactionList *list,
										  const FWTransaction *transaction,
										  int (*compFunc)(const FWTransaction*, const FWTransaction*));

/* Remove an item from the list using the supplied
 * compare function. Returns NULL if the item was
 * not found.
 */
FWTransaction *fwRemoveTransactionListItem(FWTransactionList *list,
											 const FWTransaction *transaction,
											 int (*compFunc)(const FWTransaction*, const FWTransaction*));

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_TRANSACTION_LIST_INCLUDE_ */

