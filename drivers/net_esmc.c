/*
 * $Header$
 * $Log$
 *
 */

#include <vxWorks.h>
#include <iosLib.h>
#include <memDrv.h>
#include <stdio.h>
#include <string.h>
#include <symLib.h>
#include <sysSymTbl.h>

#include "load_driver.h"

const char * net_esmc_init(const char * args);

typedef STATUS AttachFunc
    (
    int unit,     /* unit number */
    int ioAddr,   /* address of esmc's shared memory */
    int intVec,   /* interrupt vector to connect to */
    int intLevel, /* interrupt level */
    int config,   /* 0: Autodetect 1: AUI 2: BNC 3: RJ45 */
    int mode      /* 0: rx in interrupt 1: rx in task(netTask) */
    );

extern char binArrayStart;
extern char binArrayEnd;

const char * net_esmc_init(const char * args)
{
	static char	deviceName[8] = "\0";
	AttachFunc * attachFunc;
   SYM_TYPE symType;

	int	unitNum;
	int	ioPort;
	int	interruptVector;
	int	interruptNumber;
	int	connectorType;
	int	receiveMode;
	const char * argStart = strchr(args, ':');

	if ( *deviceName )
	{
		// This routine should only be called once.
		//
		fprintf(stderr, "network device %s has already been attached\n", deviceName);
		return NULL;
	}

	// Scan device options from the command line
	//
	if ( !argStart ||
        sscanf(argStart, ":%d %d %d %d %d %d:", &unitNum, &ioPort, &interruptVector, &interruptNumber, &connectorType, &receiveMode) != 6 )
	{
		fprintf(stderr, "net_esmc command line invalid: \"%s\"\n", args);
		return NULL;
	}
	
	// We can't have two copies of a network driver loaded.  The required
	// network driver is attached to this file, but is only loaded if it
	// is not already present.
	// 
   if ( symFindByName(sysSymTbl, "_esmcattach", (char **)&attachFunc, &symType) != OK )
	{
		loadDriverFromMemory(__FILE__, &binArrayStart, &binArrayEnd-&binArrayStart);
	}

   if ( symFindByName(sysSymTbl, "_esmcattach", (char **)&attachFunc, &symType) == OK )
	{
		if ( unitNum < 10 &&
		     (*attachFunc)(unitNum, ioPort, interruptVector, interruptNumber, connectorType, receiveMode) == OK )
		{
			sprintf(deviceName, "esmc%d", unitNum);
		}
		else if ( unitNum >= 10 )
		{
			fprintf(stderr, "%s: invalid unit number %d\n", __FILE__, unitNum);
		}
		else
		{
			fprintf(stderr, "%s: esmcattach(%d %x %x %x %d %d) failed errno=%d\n", __FILE__,
                  unitNum, ioPort, interruptVector, interruptNumber, connectorType, receiveMode, errno);
		}
	}
	else
	{
		fprintf(stderr, "%s: no attach function\n", __FILE__);
	}

	return (*deviceName) ? deviceName : NULL;
}

