/*
* Copyright (c) 1995, 1996 by Cobe BCT, Inc.    All rights reserved.
*
* $Header: Q:/home1/COMMON_PROJECT/Source/INCLUDE/rcs/AN2_LOG_AGENT.HPP 1.2 1999/08/10 21:57:55 TD10216 Exp BS04481 $
* $Log: AN2_LOG_AGENT.HPP $
* Revision 1.1  1999/08/04 22:36:23  TD10216
* Initial revision
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
#include "msghdr.h"
#include "error.h"
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
