/*
 * Copyright (C) 2003 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log$
 */

#include <taskLib.h>
#include "task_start.h"

int taskStart(const char * taskName, int priority, unsigned long stackSize, FUNCPTR entryPoint, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10)
{
	WIND_TCB	* tid = (WIND_TCB *)malloc(sizeof(WIND_TCB));
	unsigned long	paddedStackSize = (stackSize + 0x0fff) & 0xfffff000;
	void * stack = valloc(paddedStackSize+0x1000);
	int  result = ERROR; 

	if ( tid && stack )
	{
		unsigned long stackBaseAddr = ((unsigned long)stack) + paddedStackSize + 0x1000;
		vmBaseStateSet(NULL, stack, 0x1000, VM_STATE_MASK_VALID, VM_STATE_VALID_NOT);

		if ( taskInit(tid, (char *)taskName, priority, VX_FP_TASK, (char *)stackBaseAddr, paddedStackSize, entryPoint, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) == OK )
		{
			taskActivate((int)tid);
			result = (int)tid;
		}
	}

	if ( result == ERROR )
	{
		if ( tid ) free(tid);
		if ( stack ) free(stack);
	}

	return result;
}
