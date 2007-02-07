
#include <vxWorks.h>
#include "fw_utility.h"
#include "fw_data_map.h"

/* Local functions */
static void fwAssert(int fwAssertion, char* error);
static void fwDataMapLeftRotate(FWDataMap* tree, FWMapDataItem* x);
static void fwDataMapRightRotate(FWDataMap* tree, FWMapDataItem* y);
static void fwDataMapTreeInsertHelp(FWDataMap* tree, FWMapDataItem* z);
static void fwMapDataInorderTreePrint(FWDataMap* tree, FWMapDataItem* x);
static void fwMapDataTreeDestHelper(FWDataMap* tree, FWMapDataItem* x);
static void fwDataMapDeleteFixUp(FWDataMap* tree, FWMapDataItem* x);

/*
 *  FUNCTION:  fwDataMapCreate
 *
 *  INPUTS:  All the inputs are names of functions. compFunc takes two
 *  void pointers to keys and returns 1 if the first arguement is
 *  "greater than" the second.   DestFunc takes a pointer to a key and
 *  destroys it in the appropriate manner when the node containing that
 *  key is deleted.  InfoDestFunc is similiar to DestFunc except it
 *  receives a pointer to the info of a node and destroys it.
 *  printFunc recieves a pointer to the key of a node and prints it.
 *  PrintInfo recieves a pointer to the info of a node and prints it.
 *  If fwDataMapPrint is never called the print functions don't have to be
 *  defined and fwMapNullFunction can be used.
 *
 *  OUTPUT:  This function returns a pointer to the newly created
 *  red-black tree.
 *
 *  Modifies Input: none
 */
FWDataMap* fwDataMapCreate(int  (*compFunc)(const void *key1, const void *key2),
										void (*destFunc)(void *key),
										void (*infoDestFunc)(void*),
										void (*printFunc)(const void*),
										void (*printInfo)(void*))
{
	FWDataMap *newTree = NULL;
	FWMapDataItem *treeRoot;
	FWMapDataItem *treeNil;

	newTree = (FWDataMap*)fwMalloc(sizeof(FWDataMap));

	if( newTree )
	{
		newTree->compare = compFunc;
		newTree->destroyKey = destFunc;
		newTree->printKey = printFunc;
		newTree->printInfo = printInfo;
		newTree->destroyInfo = infoDestFunc;

		treeNil = (FWMapDataItem*)fwMalloc(sizeof(FWMapDataItem));

		if( treeNil )
		{
			/*  see the comment in the FWDataMap structure in red_black_tree.h */
			/*  for information on nil and root */
			newTree->nil = treeNil;
			treeNil->parent = treeNil->left = treeNil->right = treeNil;
			treeNil->red = 0;
			treeNil->key = NULL;

			treeRoot = (FWMapDataItem*)fwMalloc(sizeof(FWMapDataItem));

			if( treeRoot )
			{
				newTree->root = treeRoot;
				treeRoot->parent = treeRoot->left = treeRoot->right = newTree->nil;
				treeRoot->key = NULL;
				treeRoot->red = 0;
			}
			else
			{
				fwFree( treeNil );
				fwFree( newTree );
				newTree = NULL;
			}
		}
		else
		{
			fwFree( newTree );
			newTree = NULL;
		}
	}

	return newTree;
}

/*
 *  FUNCTION:  fwDataMapLeftRotate
 *
 *  INPUTS:  This takes a tree so that it can access the appropriate
 *           root and nil pointers, and the node to rotate on.
 *
 *  OUTPUT:  None
 *
 *  Modifies Input: tree, x
 *
 *  EFFECTS:  Rotates as described in _Introduction_To_Algorithms by
 *            Cormen, Leiserson, Rivest (Chapter 14).  Basically this
 *            makes the parent of x be to the left of x, x the parent of
 *            its parent before the rotation and fixes other pointers
 *            accordingly.
 */
static void fwDataMapLeftRotate(FWDataMap* tree, FWMapDataItem* x)
{
	FWMapDataItem* y;
	FWMapDataItem* nil = tree->nil;

	/*  I originally wrote this function to use the sentinel for */
	/*  nil to avoid checking for nil.  However this introduces a */
	/*  very subtle bug because sometimes this function modifies */
	/*  the parent pointer of nil.  This can be a problem if a */
	/*  function which calls fwDataMapLeftRotate also uses the nil sentinel */
	/*  and expects the nil sentinel's parent pointer to be unchanged */
	/*  after calling this function.  For example, when fwDataMapDeleteFixUp */
	/*  calls fwDataMapLeftRotate it expects the parent pointer of nil to be */
	/*  unchanged. */

	y = x->right;
	x->right = y->left;

	if ( y->left != nil ) y->left->parent = x; /* used to use sentinel here */
	/* and do an unconditional assignment instead of testing for nil */

	y->parent = x->parent;

	/* instead of checking if x->parent is the root as in the book, we */
	/* count on the root sentinel to implicitly take care of this case */
	if ( x == x->parent->left )
	{
		x->parent->left = y;
	}
	else
	{
		x->parent->right = y;
	}
	y->left = x;
	x->parent = y;

#ifdef DEBUG_fwAssert
	fwAssert( !tree->nil->red, "nil not red in fwDataMapLeftRotate" );
#endif
}


/*
 *  FUNCTION:  fwDataMapRightRotate
 *
 *  INPUTS:  This takes a tree so that it can access the appropriate
 *           root and nil pointers, and the node to rotate on.
 *
 *  OUTPUT:  None
 *
 *  Modifies Input?: tree, y
 *
 *  EFFECTS:  Rotates as described in _Introduction_To_Algorithms by
 *            Cormen, Leiserson, Rivest (Chapter 14).  Basically this
 *            makes the parent of x be to the left of x, x the parent of
 *            its parent before the rotation and fixes other pointers
 *            accordingly.
 */
static void fwDataMapRightRotate(FWDataMap* tree, FWMapDataItem* y)
{
	FWMapDataItem* x;
	FWMapDataItem* nil = tree->nil;

	/*  I originally wrote this function to use the sentinel for */
	/*  nil to avoid checking for nil.  However this introduces a */
	/*  very subtle bug because sometimes this function modifies */
	/*  the parent pointer of nil.  This can be a problem if a */
	/*  function which calls fwDataMapLeftRotate also uses the nil sentinel */
	/*  and expects the nil sentinel's parent pointer to be unchanged */
	/*  after calling this function.  For example, when fwDataMapDeleteFixUp */
	/*  calls fwDataMapLeftRotate it expects the parent pointer of nil to be */
	/*  unchanged. */

	x = y->left;
	y->left = x->right;

	if( nil != x->right ) x->right->parent = y; /*used to use sentinel here */
	/* and do an unconditional assignment instead of testing for nil */

	/* instead of checking if x->parent is the root as in the book, we */
	/* count on the root sentinel to implicitly take care of this case */
	x->parent = y->parent;
	if ( y == y->parent->left )
	{
		y->parent->left = x;
	}
	else
	{
		y->parent->right = x;
	}
	x->right = y;
	y->parent = x;

#ifdef DEBUG_fwAssert
	fwAssert( !tree->nil->red, "nil not red in fwDataMapRightRotate" );
#endif
}

/*
 *  FUNCTION:  fwDataMapTreeInsertHelp
 *
 *  INPUTS:  tree is the tree to insert into and z is the node to insert
 *
 *  OUTPUT:  none
 *
 *  Modifies Input:  tree, z
 *
 *  EFFECTS:  Inserts z into the tree as if it were a regular binary tree
 *            using the algorithm described in _Introduction_To_Algorithms_
 *            by Cormen et al.  This funciton is only intended to be called
 *            by the fwDataMapInsert function and not by the user
 */
static void fwDataMapTreeInsertHelp(FWDataMap* tree, FWMapDataItem* z)
{
	/*  This function should only be called by InsertRBTree (see above) */
	FWMapDataItem* x;
	FWMapDataItem* y;
	FWMapDataItem* nil = tree->nil;

	z->left = z->right = nil;
	y = tree->root;
	x = tree->root->left;
	while( x != nil )
	{
		y = x;
		if ( 1 == tree->compare( x->key, z->key ))
		{ /* x.key > z.key */
			x = x->left;
		}
		else
		{ /* x,key <= z.key */
			x = x->right;
		}
	}
	z->parent = y;
	if ( (y == tree->root) || (1 == tree->compare( y->key, z->key )) )
	{ /* y.key > z.key */
		y->left = z;
	}
	else
	{
		y->right = z;
	}

#ifdef DEBUG_fwAssert
	fwAssert( !tree->nil->red, "nil not red in fwDataMapTreeInsertHelp" );
#endif
}

/*  Before calling Insert RBTree the node x should have its key set */

/*
*  FUNCTION:  fwDataMapInsert
*
*  INPUTS:  tree is the red-black tree to insert a node which has a key
*           pointed to by key and info pointed to by info.
*
*  OUTPUT:  This function returns a pointer to the newly inserted node
*           which is guarunteed to be valid until this node is deleted.
*           What this means is if another data structure stores this
*           pointer then the tree does not need to be searched when this
*           is to be deleted.
*
*  Modifies Input: tree
*
*  EFFECTS:  Creates a node node which contains the appropriate key and
*            info pointers and inserts it into the tree.
*/
FWMapDataItem * fwDataMapInsert(FWDataMap* tree, void* key, void* info)
{
	FWMapDataItem * dataItem = NULL;
	FWMapDataItem * y;
	FWMapDataItem * x;

	x = (FWMapDataItem*)fwMalloc(sizeof(FWMapDataItem));

#ifdef DEBUG_fwAssert
	fwAssert( x && tree && key && info, "NULL pointer in fwDataMapInsert" );
#endif

	if( x && tree && key && info )
	{
		x->key = key;
		x->info = info;

		fwDataMapTreeInsertHelp( tree, x );
		dataItem = x;
		x->red = 1;
		while( x->parent->red )
		{ /* use sentinel instead of checking for root */
			if ( x->parent == x->parent->parent->left )
			{
				y = x->parent->parent->right;
				if( y->red )
				{
					x->parent->red = 0;
					y->red = 0;
					x->parent->parent->red = 1;
					x = x->parent->parent;
				}
				else
				{
					if( x == x->parent->right )
					{
						x = x->parent;
						fwDataMapLeftRotate( tree, x );
					}
					x->parent->red = 0;
					x->parent->parent->red = 1;
					fwDataMapRightRotate( tree, x->parent->parent );
				}
			}
			else
			{ /* case for x->parent == x->parent->parent->right */
				y = x->parent->parent->left;
				if( y->red )
				{
					x->parent->red = 0;
					y->red = 0;
					x->parent->parent->red = 1;
					x = x->parent->parent;
				}
				else
				{
					if( x == x->parent->left )
					{
						x = x->parent;
						fwDataMapRightRotate( tree, x );
					}
					x->parent->red = 0;
					x->parent->parent->red = 1;
					fwDataMapLeftRotate( tree, x->parent->parent );
				}
			}
		}
		tree->root->left->red = 0;
	}

	return dataItem;
}

/*
 *  FUNCTION:  fwDataMapSuccessor
 *
 *    INPUTS:  tree is the tree in question, and x is the node we want the
 *             the successor of.
 *
 *    OUTPUT:  This function returns the successor of x or NULL if no
 *             successor exists.
 *
 *    Modifies Input: none
 *
 *    Note:  uses the algorithm in _Introduction_To_Algorithms_
 */
FWMapDataItem* fwDataMapSuccessor(FWDataMap* tree, FWMapDataItem* x)
{
	FWMapDataItem* y = NULL;
	FWMapDataItem* nil;
	FWMapDataItem* root;

#ifdef DEBUG_fwAssert
	fwAssert( x && tree, "NULL pointer in fwDataMapSuccessor" );
#endif

	if( tree && x )
	{
		nil = tree->nil;
		root = tree->root;

		if( nil != (y = x->right) )
		{ /* assignment to y is intentional */
			while( y->left != nil )
			{ /* returns the minium of the right subtree of x */
				y = y->left;
			}
		}
		else
		{
			y = x->parent;
			while( x == y->right )
			{ /* sentinel used instead of checking for nil */
				x = y;
				y = y->parent;
			}

			if( y == root )
				y = nil;
		}
	}

	return y;
}

/*
 *  FUNCTION:  fwDataMapPredecessor
 *
 *    INPUTS:  tree is the tree in question, and x is the node we want the
 *             the predecessor of.
 *
 *    OUTPUT:  This function returns the predecessor of x or NULL if no
 *             predecessor exists.
 *
 *    Modifies Input: none
 *
 *    Note:  uses the algorithm in _Introduction_To_Algorithms_
 */

FWMapDataItem* fwDataMapPredecessor(FWDataMap* tree, FWMapDataItem* x)
{
	FWMapDataItem* y = NULL;
	FWMapDataItem* nil;
	FWMapDataItem* root;

#ifdef DEBUG_fwAssert
	fwAssert( x && tree, "NULL pointer in fwDataMapPredecessor" );
#endif

	if( x && tree )
	{
		nil = tree->nil;
		root = tree->root;

		if( nil != (y = x->left) )
		{ /* assignment to y is intentional */
			while( y->right != nil )
			{ /* returns the maximum of the left subtree of x */
				y = y->right;
			}
		}
		else
		{
			y = x->parent;
			while( x == y->left )
			{
				if( y == root )
				{
					 y = nil;
					 break;
				}
				x = y;
				y = y->parent;
			}
		}
	}

	return y;
}

/*
 *  FUNCTION:  fwMapDataInorderTreePrint
 *
 *    INPUTS:  tree is the tree to print and x is the current inorder node
 *
 *    OUTPUT:  none
 *
 *    EFFECTS:  This function recursively prints the nodes of the tree
 *              inorder using the PrintKey and PrintInfo functions.
 *
 *    Modifies Input: none
 *
 *    Note:    This function should only be called from fwDataMapPrint
 */

static void fwMapDataInorderTreePrint(FWDataMap* tree, FWMapDataItem* x)
{
	FWMapDataItem* nil=tree->nil;
	FWMapDataItem* root=tree->root;

	if ( x != tree->nil )
	{
		fwMapDataInorderTreePrint(tree,x->left);
		FWLOGLEVEL7("info=");
		tree->printInfo(x->info);
		FWLOGLEVEL7("  key=");
		tree->printKey(x->key);
		FWLOGLEVEL7("  l->key=");

		if( x->left == nil )
		{
			FWLOGLEVEL7("NULL");
		}
		else
		{
			tree->printKey(x->left->key);
		}

		FWLOGLEVEL7("  r->key=");

		if( x->right == nil )
		{
			FWLOGLEVEL7("NULL");
		}
		else
		{
			tree->printKey(x->right->key);
		}

		FWLOGLEVEL7("  p->key=");

		if( x->parent == root )
		{
			FWLOGLEVEL7("NULL");
		}
		else
		{
			tree->printKey(x->parent->key);
		}

		FWLOGLEVEL7("  red=%i\n",x->red);
		fwMapDataInorderTreePrint(tree,x->right);
	}
}

/*
 *  FUNCTION:  fwMapDataTreeDestHelper
 *
 *    INPUTS:  tree is the tree to destroy and x is the current node
 *
 *    OUTPUT:  none
 *
 *    EFFECTS:  This function recursively destroys the nodes of the tree
 *              postorder using the DestroyKey and DestroyInfo functions.
 *
 *    Modifies Input: tree, x
 *
 *    Note:    This function should only be called by fwDataMapDestroy
 */
static void fwMapDataTreeDestHelper(FWDataMap* tree, FWMapDataItem* x)
{
	FWMapDataItem* nil = tree->nil;

	if ( x != nil )
	{
		fwMapDataTreeDestHelper( tree,x->left );
		fwMapDataTreeDestHelper( tree,x->right );
		tree->destroyKey( x->key );
		tree->destroyInfo( x->info );
		fwFree( x );
	}
}


/*
 *  FUNCTION:  fwDataMapDestroy
 *
 *    INPUTS:  tree is the tree to destroy
 *
 *    OUTPUT:  none
 *
 *    EFFECT:  Destroys the key and frees memory
 *
 *    Modifies Input: tree
 *
 */
void fwDataMapDestroy(FWDataMap* tree)
{
	if( tree )
	{
		fwMapDataTreeDestHelper( tree, tree->root->left );
		fwFree( tree->root );
		fwFree( tree->nil );
		fwFree( tree );
	}
}

/*
 *  FUNCTION:  fwDataMapPrint
 *
 *    INPUTS:  tree is the tree to print
 *
 *    OUTPUT:  none
 *
 *    EFFECT:  This function recursively prints the nodes of the tree
 *             inorder using the PrintKey and PrintInfo functions.
 *
 *    Modifies Input: none
 *
 */
void fwDataMapPrint(FWDataMap* tree)
{
	if( tree )
		fwMapDataInorderTreePrint( tree, tree->root->left );
}


/*
 *  FUNCTION:  fwDataMapExactQuery
 *
 *    INPUTS:  tree is the tree to print and q is a pointer to the key
 *             we are searching for
 *
 *    OUTPUT:  returns the a node with key equal to q.  If there are
 *             multiple nodes with key equal to q this function returns
 *             the one highest in the tree
 *
 *    Modifies Input: none
 *
 */
FWMapDataItem* fwDataMapExactQuery(FWDataMap* tree, void* q)
{
	FWMapDataItem* x = NULL;
	FWMapDataItem* nil;
	int compVal;

	if( tree && q )
	{
		x = tree->root->left;
		nil = tree->nil;

		if( x != nil && x != NULL )
		{
			compVal = tree->compare( x->key, q );

			while ( 0 != compVal )
			{ /*assignemnt*/
				if ( 1 == compVal )
				{ /* x->key > q */
					x = x->left;
				}
				else
				{
					x = x->right;
				}

				if( x == nil )
				{
					x = NULL;
					break;
				}
				else
					compVal=tree->compare( x->key, q );
			}
		}
		else
		{
			x = NULL;
		}
	}

	return x;
}

/*
 *  FUNCTION:  fwDataMapDeleteFixUp
 *
 *    INPUTS:  tree is the tree to fix and x is the child of the spliced
 *             out node in RBTreeDelete.
 *
 *    OUTPUT:  none
 *
 *    EFFECT:  Performs rotations and changes colors to restore red-black
 *             properties after a node is deleted
 *
 *    Modifies Input: tree, x
 *
 *    The algorithm from this function is from _Introduction_To_Algorithms_
 */
static void fwDataMapDeleteFixUp(FWDataMap* tree, FWMapDataItem* x)
{
	FWMapDataItem* root=tree->root->left;
	FWMapDataItem* w;

	while( (!x->red) && (root != x) )
	{
		if( x == x->parent->left )
		{
			w = x->parent->right;
			if( w->red )
			{
				w->red = 0;
				x->parent->red = 1;
				fwDataMapLeftRotate( tree, x->parent );
				w = x->parent->right;
			}
			if( (!w->right->red) && (!w->left->red) )
			{
				w->red = 1;
				x = x->parent;
			}
			else
			{
				if( !w->right->red )
				{
					w->left->red = 0;
					w->red = 1;
					fwDataMapRightRotate( tree, w );
					w = x->parent->right;
				}
				w->red = x->parent->red;
				x->parent->red = 0;
				w->right->red = 0;
				fwDataMapLeftRotate( tree, x->parent );
				x = root; /* this is to exit while loop */
			}
		}
		else
		{ /* the code below is has left and right switched from above */
			w = x->parent->left;
			if( w->red )
			{
				w->red = 0;
				x->parent->red = 1;
				fwDataMapRightRotate( tree, x->parent );
				w = x->parent->left;
			}
			if( (!w->right->red) && (!w->left->red) )
			{
				w->red = 1;
				x = x->parent;
			}
			else
			{
				if( !w->left->red )
				{
					w->right->red = 0;
					w->red = 1;
					fwDataMapLeftRotate( tree, w );
					w = x->parent->left;
				}

				w->red = x->parent->red;
				x->parent->red = 0;
				w->left->red = 0;
				fwDataMapRightRotate( tree, x->parent );
				x = root; /* this is to exit while loop */
			}
		}
	}

	x->red = 0;

#ifdef DEBUG_fwAssert
	fwAssert(!tree->nil->red,"nil not black in fwDataMapDeleteFixUp");
#endif
}


/*
 *  FUNCTION:  fwDataMapDelete
 *
 *    INPUTS:  tree is the tree to delete node z from
 *
 *    OUTPUT:  none
 *
 *    EFFECT:  Deletes z from tree and frees the key and info of z
 *             using DestoryKey and DestoryInfo.  Then calls
 *             fwDataMapDeleteFixUp to restore red-black properties
 *
 *    Modifies Input: tree, z
 *
 *    The algorithm from this function is from _Introduction_To_Algorithms_
 */

void fwDataMapDelete(FWDataMap* tree, FWMapDataItem* z)
{
	FWMapDataItem* y;
	FWMapDataItem* x;
	FWMapDataItem* nil;
	FWMapDataItem* root;

#ifdef DEBUG_fwAssert
	fwAssert( tree && z,"Null pointer in fwDataMapDelete");
#endif

	if( tree && z )
	{
		nil = tree->nil;
		root = tree->root;

		y = ((z->left == nil) || (z->right == nil)) ? z : fwDataMapSuccessor( tree, z );
		x = (y->left == nil) ? y->right : y->left;
		if( root == (x->parent = y->parent) ) /* assignment of y->p to x->p is intentional */
		{
			root->left = x;
		}
		else
		{
			if( y == y->parent->left )
			{
				y->parent->left = x;
			}
			else
			{
				y->parent->right = x;
			}
		}

		if( y != z ) /* y should not be nil in this case */
		{

#ifdef DEBUG_fwAssert
			fwAssert( (y!=tree->nil),"y is nil in fwDataMapDelete\n");
#endif
			/* y is the node to splice out and x is its child */

			if( !y->red ) fwDataMapDeleteFixUp( tree, x );

			tree->destroyKey( z->key );
			tree->destroyInfo( z->info );

			y->left = z->left;
			y->right = z->right;
			y->parent = z->parent;
			y->red = z->red;
			z->left->parent = z->right->parent = y;
			if ( z == z->parent->left )
			{
				z->parent->left = y;
			}
			else
			{
				z->parent->right = y;
			}

			fwFree( z );
		}
		else
		{
			tree->destroyKey( y->key );
			tree->destroyInfo( y->info );
			if ( !(y->red) ) fwDataMapDeleteFixUp( tree, x );
			fwFree( y );
		}

#ifdef DEBUG_fwAssert
		fwAssert(!tree->nil->red,"nil not black in fwDataMapDelete");
#endif
	}
}


/*
 *  FUNCTION:  fwDataMapEnumerate
 *
 *    INPUTS:  tree is the tree to look for keys >= low
 *             and <= high with respect to the Compare function
 *
 *    OUTPUT:  stack containing pointers to the nodes between [low,high]
 *
 *    Modifies Input: none
 */
FWDataList* fwDataMapEnumerate(FWDataMap* tree, void* low, void* high)
{
	FWDataList* enumResultList = NULL;
	FWMapDataItem* nil=tree->nil;
	FWMapDataItem* x=tree->root->left;
	FWMapDataItem* lastBest=nil;

	if( tree && low && high )
	{
		nil = tree->nil;
		x = tree->root->left;
		lastBest = nil;

		enumResultList = fwListCreate();

		if( enumResultList )
		{
			while( nil != x )
			{
				if( 1 == (tree->compare( x->key, high )) )
				{ /* x->key > high */
					x = x->left;
				}
				else
				{
					lastBest = x;
					x = x->right;
				}
			}

			while( (lastBest != nil) && ( 1 != tree->compare( low, lastBest->key )) )
			{
				fwListPushBack( enumResultList, lastBest );
				lastBest = fwDataMapPredecessor( tree, lastBest );
			}
		}
	}

	return enumResultList;
}

static void fwAssert(int fwAssertion, char* error)
{
	if ( !fwAssertion )
	{
		FWLOGLEVEL3("fwAssertion Failed: %s\n",error);
	}
}

/*  fwMapNullFunction does nothing it is included so that it can be passed */
/*  as a function to fwDataMapCreate when no other suitable function has */
/*  been defined */

void fwMapNullFunction(void * junk)
{
}
