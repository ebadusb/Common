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

const char * net_ene_init(const char * args);

typedef STATUS AttachFunc
    (
    int unit,   /* unit number */
    int ioAddr, /* address of ene's shared memory */
    int ivec,   /* interrupt vector to connect to */
    int ilevel  /* interrupt level */
    );

extern char binArrayStart;
extern char binArrayEnd;

const char * net_ene_init(const char * args)
{
	static char	deviceName[8] = "\0";
	AttachFunc * attachFunc;
   SYM_TYPE symType;

	int	unitNum;
	int	ioPort;
	int	interruptVector;
	int	interruptNumber;
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
        sscanf(argStart, ":%d %d %d %d:", &unitNum, &ioPort, &interruptVector, &interruptNumber) != 4 )
	{
		fprintf(stderr, "net_ene command line invalid: \"%s\"\n", args);
		return NULL;
	}
	
	// We can't have two copies of a network driver loaded.  The required
	// network driver is attached to this file, but is only loaded if it
	// is not already present.
	// 
   if ( symFindByName(sysSymTbl, "_eneattach", (char **)&attachFunc, &symType) != OK )
	{
		loadDriverFromMemory(__FILE__, &binArrayStart, &binArrayEnd-&binArrayStart);
	}

   if ( symFindByName(sysSymTbl, "_eneattach", (char **)&attachFunc, &symType) == OK )
	{
		if ( unitNum < 10 &&
		     (*attachFunc)(unitNum, ioPort, interruptVector, interruptNumber) == OK )
		{
			sprintf(deviceName, "ene%d", unitNum);
		}
		else if ( unitNum >= 10 )
		{
			fprintf(stderr, "%s: invalid unit number %d\n", __FILE__, unitNum);
		}
		else
		{
			fprintf(stderr, "%s: eneattach(%d %x %x %x) failed errno=%d\n", __FILE__,
                  unitNum, ioPort, interruptVector, interruptNumber, errno);
		}
	}
	else
	{
		fprintf(stderr, "%s: no attach function\n", __FILE__);
	}

	return (*deviceName) ? deviceName : NULL;
}

