/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains the firewire physical layer data
 * structures and enumerations.
 *
 * $Log$
 *
 */

#ifndef _FW_PHYSICAL_DATA_INCLUDE_
#define _FW_PHYSICAL_DATA_INCLUDE_

#include <semLib.h>
#include <firewire/fw_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Physical layer data structure. This structure stores state
 * information for the physical layer and is maintained
 * by the bus manager. */
typedef struct FWPhysicalDataStruct
{
	int				numPorts;				/* Number of physical ports on the hardware */
	boolean			busResetUnderway;		/* TRUE if a bus reset is underway */
	boolean			isRoot;					/* TRUE if this node is the root node */
	boolean			initialized;			/* TRUE if the PHY is initialized */
	unsigned long	busGeneration;			/* Current bus generation (self ID generation) */
	unsigned short	nodeId;					/* Local node ID (bus number + physical) */
	unsigned long	phyRegStatus;			/* Status used for physical layer reads */
	SEM_ID			phyRegAccess;			/* Semaphore to notify when physical layer access complete */

} FWPhysicalData;


#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_PHYSICAL_DATA_INCLUDE_ */

