/*
 * $Header$
 * $Log$
 *
 */

#ifndef _LINK_GROUP_INCLUDE
#define _LINK_GROUP_INCLUDE

#include <semLib.h>

class LinkGroup
{
public:
	// The LinkGroup class maintains a group of linked elements.  These elements
	// are protected by a common semaphore.  Tasks must perform a lock operation
	// on the associated LinkGroup before access to any of the LinkElement members.
	//
	LinkGroup(void);
	virtual ~LinkGroup();
	void lock(void) { semTake(_lock, WAIT_FOREVER); }
	void unlock(void) { semGive(_lock); }

private:
	SEM_ID _lock;
};

#endif /* ifndef _LINK_GROUP_INCLUDE */
