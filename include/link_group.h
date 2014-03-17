/*
 * $Header$
 * $Log$
 *
 */

#ifndef _LINK_GROUP_INCLUDE
#define _LINK_GROUP_INCLUDE

#include "Mutex.h"
using namespace Bct;

class LinkGroup
{
public:
	// The LinkGroup class maintains a group of linked elements.  These elements
	// are protected by a common semaphore.  Tasks must perform a lock operation
	// on the associated LinkGroup before access to any of the LinkElement members.
	//
	LinkGroup(void);
	virtual ~LinkGroup();
	void lock(void) { _mutex.lock(); }
	void unlock(void) { _mutex.unlock(); }

private:
   Mutex _mutex;
};

#endif /* ifndef _LINK_GROUP_INCLUDE */
