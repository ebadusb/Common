/*
 * $Header$
 * $Log$
 *
 */

#include <vxWorks.h>
#include "link_group.h"

LinkGroup::LinkGroup(void)
{
	_lock = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE);
}

LinkGroup::~LinkGroup()
{
	semDelete(_lock);
}

