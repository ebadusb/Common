/*
 * Copyright (c) 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * TITLE:      glob_sem.hpp
 *
 * AUTHOR:     Tate Moore
 *
 * ABSTRACT:   This header file defines the global semaphore classes.
 *
 * DOCUMENTS
 * Requirements: 
 * Test:
 *
 */

#ifndef AGENT_SEM_H  //  Prevents multiple compilations.
#define AGENT_SEM_H

#ifndef RUN_WINDOWS

#include "glob_sem.hpp"

typedef struct
{
    // Shared Data Memory semaphore
// - MUST BE FIRST VARIABLE IN STRUCT LIST!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// *****************************************************************************
    sem_t data_semaphore;
// *****************************************************************************

// *****************************************************************************
//
//    INPUT DATA - from low-level control software to algorithms
//
// *****************************************************************************
} AGENT_SEMAPHORE_STRUCT;

class AgentSemaphore : public GlobalSemaphore<AGENT_SEMAPHORE_STRUCT>
{
	public:

	AgentSemaphore(const char *name, int semaphore_already_locked = 0) : 
                  GlobalSemaphore<AGENT_SEMAPHORE_STRUCT>(name, semaphore_already_locked) {};
};

#else

// This is used to hide the semaphore stuff in windows for testing...
class AgentSemaphore
{
	public:

	AgentSemaphore(const char * name)	{};
	inline int AccessMemory() { return 0; };
	inline int LockMemory(void) { return 0; };
	inline int FreeMemory(void) { return 0; };
};

#endif

#endif