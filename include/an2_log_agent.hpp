/*
* Copyright (c) 1995, 1996 by Cobe BCT, Inc.    All rights reserved.
*
* $Header: Q:/home1/COMMON_PROJECT/Source/INCLUDE/rcs/AN2_LOG_AGENT.HPP 1.1 1999/08/04 22:36:23 TD10216 Exp TD10216 $
* $Log: AN2_LOG_AGENT.HPP $
* an2msgs.h
*
* Abstract : 12/04/96
*           Header file for Everest config file logging on Trima
*/
#ifndef _AN2_LOG_AGENT                      /* include guard */
#define AN2_LOG_AGENT

#include <semaphore.h>
#include <dirent.h>

#define AGENT_FILE_PATHS
#include "ap2agent.h"
#include "aSectionfile.hpp"
#include "anAgentMemoryInterface.h"
#include "an2msgs.h"
#include "buffmsgs.h"

class an2_log_agent
{
	private:
		DIR *dirfile;
		struct dirent *entry;
		sem_t *LogSem;
		FILE *fo;
		ConfigDataMsgStruct LogStruct;
		aSectionfile *my_sectionfile;
		char eString[256];

		an2_log_agent(void);
		void WriteLog(void);
		void LogVariable(const char *Section,const char *Variable, 
						 char const *Value);

	public:
		an2_log_agent(FILE *, sem_t *s = NULL);
		~an2_log_agent();
		int LogAgentData(void);
};

#endif		// include (cattle) guard
