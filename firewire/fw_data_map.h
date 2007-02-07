/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains routines for a red-black binary tree.
 *
 * $Log$
 *
 */

#ifndef _FW_DATA_MAP_INCLUDE_
#define _FW_DATA_MAP_INCLUDE_

#include <firewire/fw_data_list.h>

#ifdef __cplusplus
extern "C" {
#endif

/*  CONVENTIONS:  All data structures for red-black trees have the prefix
 *                "rb_" to prevent name conflicts.
 *
 *                Function names: Each word in a function name begins with
 *                a capital letter.  An example funcntion name is
 *                CreateRedTree(a,b,c). Furthermore, each function name
 *                should begin with a capital letter to easily distinguish
 *                them from variables.
 *
 *                Variable names: Each word in a variable name begins with
 *                a capital letter EXCEPT the first letter of the variable
 *                name.  For example, int newLongInt.  Global variables have
 *                names beginning with "g".  An example of a global
 *                variable name is gNewtonsConstant.
 */

/* comment out the line below to remove all the debugging assertion */
/* checks from the compiled code.  */
#define DEBUG_ASSERT 1

typedef struct FWMapDataItemStruct
{
	void*									key;
	void*									info;
	int									red;	/* if red=0 then the node is black */
	struct FWMapDataItemStruct*	left;
	struct FWMapDataItemStruct*	right;
	struct FWMapDataItemStruct*	parent;

} FWMapDataItem;

/* Compare(a,b) should return 1 if *a > *b, -1 if *a < *b, and 0 otherwise */
/* Destroy(a) takes a pointer to whatever key might be and frees it accordingly */
typedef struct FWDataMapStruct
{
	int  (*compare)(const void* a, const void* b);
	void (*destroyKey)(void* a);
	void (*destroyInfo)(void* a);
	void (*printKey)(const void* a);
	void (*printInfo)(void* a);

	/*  A sentinel is used for root and for nil.  These sentinels are */
	/*  created when RBTreeCreate is called. root->left should always */
	/*  point to the node which is the root of the tree.  nil points to a */
	/*  node which should always be black but has aribtrary children and */
	/*  parent and no key or info.  The point of using these sentinels is so */
	/*  that the root and nil nodes do not require special cases in the code */
	FWMapDataItem* root;
	FWMapDataItem* nil;

} FWDataMap;

/* Map functions */
FWDataMap* fwDataMapCreate(int  (*compFunc)(const void*, const void*),
										void (*destFunc)(void*),
										void (*infoDestFunc)(void*),
										void (*printFunc)(const void*),
										void (*printInfo)(void*));

FWMapDataItem * fwDataMapInsert(FWDataMap *map, void* key, void* info);
void fwDataMapPrint(FWDataMap *map);
void fwDataMapDelete(FWDataMap *map, FWMapDataItem *item);
void fwDataMapDestroy(FWDataMap *map);
FWMapDataItem *fwDataMapPredecessor(FWDataMap *map, FWMapDataItem *item);
FWMapDataItem *fwDataMapSuccessor(FWDataMap *map, FWMapDataItem *item);
FWMapDataItem *fwDataMapExactQuery(FWDataMap *map, void* key);
FWDataList* fwDataMapEnumerate(FWDataMap* tree, void* low, void* high);
void fwMapNullFunction(void*);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_DATA_MAP_INCLUDE_ */

