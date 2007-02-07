/*
 *  Copyright(c) 2005 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains the firewire OHCI interface level routines.
 *
 * $Log$
 *
 */

#include <vxWorks.h>
#include <stdio.h>
#include <sysLib.h>
#include <taskLib.h>
#include "fw_utility.h"
#include "fw_physical_layer.h"
#include "fw_driver.h"
#include "fw_ohci_hw.h"

/* function prototypes for local functions */

/* Read a physical register. */
static FWStatus fwReadPhysicalRegister(FWDriverData *pDriver, int address, unsigned char *value);

/* Write a physical register. */
static FWStatus fwWritePhysicalRegister(FWDriverData *pDriver, int address, unsigned char value);

/* Helper routine to select the port */
static FWStatus fwSelectPhyPort(FWDriverData *pDriver, int port);

FWStatus fwTestPhysicalRead(int index);
int fwPrintOhciReg(int index, int offset);
int fwSetOhciReg(int index, int offset, unsigned int value);

/* Declare local variables */

static int fwPhyTimeout = 1;		/* Timeout in clock ticks for physical read/writes. */

/* Functions */
FWStatus fwCreatePhysicalLayer(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	if( pDriver )
	{
		if( pDriver->physicalLayerData == NULL )
		{
			do
			{
				pDriver->physicalLayerData = (FWPhysicalData*)fwMalloc( sizeof(FWPhysicalData) );

				if( pDriver->physicalLayerData == NULL )
				{
					retVal = FWMemAllocateError;
					break;
				}

				pDriver->physicalLayerData->busGeneration = 0;
				pDriver->physicalLayerData->busResetUnderway = 0;
				pDriver->physicalLayerData->initialized = 0;
				pDriver->physicalLayerData->isRoot = 0;
				pDriver->physicalLayerData->nodeId = 0xFFFF;
				pDriver->physicalLayerData->numPorts = -1;
				pDriver->physicalLayerData->phyRegStatus = 0;

				pDriver->physicalLayerData->phyRegAccess = semBCreate(SEM_Q_FIFO, SEM_EMPTY);

				if( pDriver->physicalLayerData->phyRegAccess == NULL )
				{
					retVal = FWSemAllocateError;
					break;
				}

				retVal = FWSuccess;

			} while(0);
		}
		else
			retVal = FWSuccess;
	}

	return retVal;
}

FWStatus fwDestroyPhysicalLayer(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	FWPhysicalData *pPhysical = NULL;

	if( pDriver )
	{
		if( pDriver->physicalLayerData != NULL )
		{
			if( pDriver->physicalLayerData->phyRegAccess != NULL )
			{
				semDelete( pDriver->physicalLayerData->phyRegAccess );
				pDriver->physicalLayerData->phyRegAccess = NULL;
			}

			fwFree( pDriver->physicalLayerData );

			pDriver->physicalLayerData = NULL;
		}

		retVal = FWSuccess;
	}

	return retVal;
}

FWStatus fwInitializePhysical(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	unsigned char ports;
	int numPorts = -1;
	boolean flag;

	fwPhyTimeout = sysClkRateGet() / 100;
	if( fwPhyTimeout <= 0 ) fwPhyTimeout = 2;

	do
	{
		if( pDriver == NULL ) break;

		if( pDriver->physicalLayerData == NULL ) break;

		pDriver->physicalLayerData->initialized = FALSE;
		pDriver->physicalLayerData->numPorts = -1;
		pDriver->physicalLayerData->busResetUnderway = FALSE;
		pDriver->physicalLayerData->isRoot = FALSE;
		pDriver->physicalLayerData->nodeId = 0xFFFF;
		pDriver->physicalLayerData->phyRegStatus = 0;
		pDriver->physicalLayerData->busGeneration = 0x0FFFFFFF;

		retVal = fwGetPhysicalEnable( pDriver, &flag );
		if( retVal != FWSuccess ) break;

		/* Enable the SCLK for the physical layer if it is not already enabled. */
		if( !flag )
		{
			retVal = fwSetPhysicalEnable( pDriver );
			if( retVal != FWSuccess ) break;

			/* 10ms must elapse before the OHCI registers can be accessed reliably.
			 * This time is necessary for the PHY SCLK to become stable.
			 */
			taskDelay( fwPhyTimeout );
		}

		retVal = fwGetNumPorts( pDriver, &numPorts );

		if( retVal != FWSuccess ) break;

		pDriver->physicalLayerData->initialized = TRUE;
		pDriver->physicalLayerData->numPorts = numPorts;

	} while(0);

	return retVal;
}

FWStatus fwInitiateBusReset(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	do
	{
		retVal = fwSelectPhyPort( pDriver, 0 );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 1, &phyValue );

		if( retVal != FWSuccess ) break;

		retVal = fwWritePhysicalRegister( pDriver, 1, phyValue | 0x40 );

	} while(0);

	return retVal;
}

FWStatus fwSetPhysicalEnable(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	if( pDriver )
	{
		if( pDriver->ohci )
		{
			pDriver->ohci->hcControlSet = 0x80000;

			retVal = FWSuccess;
		}
	}

	return retVal;
}

FWStatus fwGetPhysicalEnable(FWDriverData *pDriver, boolean *flag)
{
	FWStatus retVal = FWInternalError;
	UINT32 regValue = 0;

	*flag = FALSE;

	if( pDriver )
	{
		if( pDriver->ohci )
		{
			regValue = pDriver->ohci->hcControlSet;

			if( regValue & 0x80000 )
			{
				*flag = TRUE;
			}

			retVal = FWSuccess;
		}
	}

	return retVal;
}

FWStatus fwSetPhyLinkActive(FWDriverData *pDriver, boolean flag)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	do
	{
		retVal = fwSelectPhyPort( pDriver, 0 );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 4, &phyValue );

		if( retVal != FWSuccess ) break;

		if( flag )
		{
			if( (phyValue & 0x80) == 0 )
			{
				retVal = fwWritePhysicalRegister( pDriver, 4, phyValue | 0x80 );
			}
		}
		else
		{
			if( (phyValue & 0x80) != 0 )
			{
				retVal = fwWritePhysicalRegister( pDriver, 4, phyValue & 0x7F );
			}
		}

	} while(0);

	return retVal;
}

FWStatus fwGetPhyLinkActive(FWDriverData *pDriver, boolean *flag)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	*flag = FALSE;

	do
	{
		retVal = fwSelectPhyPort( pDriver, 0 );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 4, &phyValue );

		if( retVal != FWSuccess ) break;

		if( phyValue & 0x80 )
		{
			*flag = TRUE;
		}

	} while(0);

	return retVal;
}

FWStatus fwSetGapCount(FWDriverData *pDriver, unsigned char gapCount)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	do
	{
		retVal = fwSelectPhyPort( pDriver, 0 );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 1, &phyValue );

		if( retVal != FWSuccess ) break;

		phyValue |= gapCount & 0x3F;

		retVal = fwWritePhysicalRegister( pDriver, 1, phyValue );

	} while(0);

	return retVal;
}

FWStatus fwGetGapCount(FWDriverData *pDriver, unsigned char *gapCount)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	*gapCount = 0;

	do
	{
		retVal = fwSelectPhyPort( pDriver, 0 );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 1, &phyValue );

		if( retVal != FWSuccess ) break;

		*gapCount = phyValue & 0xC0;

	} while(0);

	return retVal;
}

FWStatus fwSetForceRoot(FWDriverData *pDriver, boolean flag)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	do
	{
		retVal = fwSelectPhyPort( pDriver, 0 );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 1, &phyValue );

		if( retVal != FWSuccess ) break;

		if( flag )
		{
			if( (phyValue & 0x80) == 0 )
			{
				retVal = fwWritePhysicalRegister( pDriver, 1, phyValue | 0x80 );
			}
		}
		else
		{
			if( (phyValue & 0x80) != 0 )
			{
				retVal = fwWritePhysicalRegister( pDriver, 1, phyValue & 0x7F );
			}
		}

	} while(0);

	return retVal;
}

FWStatus fwGetRootNodeStatus(FWDriverData *pDriver, boolean *flag)
{
	FWStatus retVal = FWInternalError;
	UINT32 rootStatus;

	*flag = FALSE;

	if( pDriver )
	{
		if( pDriver->ohci )
		{
			rootStatus = pDriver->ohci->nodeId;

			if( rootStatus & 0x40000000 )
			{
				*flag = TRUE;
			}

			retVal = FWSuccess;
		}
	}

	return retVal;
}

FWStatus fwGetCablePowerStatus(FWDriverData *pDriver, boolean *flag)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	*flag = FALSE;

	do
	{
		retVal = fwSelectPhyPort( pDriver, 0 );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 0, &phyValue );

		if( retVal != FWSuccess ) break;

		*flag = phyValue & 0x01;

	} while(0);

	return retVal;
}

FWStatus fwGetExtendedReg(FWDriverData *pDriver, int port, unsigned char *extendReg)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	*extendReg = 0;

	do
	{
		retVal = fwSelectPhyPort( pDriver, port );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 2, &phyValue );

		if( retVal != FWSuccess ) break;

		*extendReg = phyValue >> 5;

	} while(0);

	return retVal;
}

FWStatus fwGetNumPorts(FWDriverData *pDriver, int *numPorts)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	*numPorts = -1;

	do
	{
		if( pDriver == NULL ) break;
      if( pDriver->physicalLayerData == NULL || pDriver->ohci == NULL ) break;

		/* select the port */
		retVal = fwWritePhysicalRegister( pDriver, 7, 0 );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 2, &phyValue );

		if( retVal != FWSuccess ) break;

		*numPorts = phyValue & 0x0F;

	} while(0);

	return retVal;
}

FWStatus fwGetPhySpeed(FWDriverData *pDriver, int port, FWSpeed *speed)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	*speed = FWSInvalid;

	do
	{
		retVal = fwSelectPhyPort( pDriver, port );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 3, &phyValue );

		if( retVal != FWSuccess ) break;

		phyValue = phyValue >> 5;

		if( phyValue == 0 )
		{
			*speed = FWS100;
		}
		else if( phyValue == 1 )
		{
			*speed = FWS200;
		}
		else if( phyValue == 2 )
		{
			*speed = FWS400;
		}

	} while(0);

	return retVal;
}

FWStatus fwGetDelay(FWDriverData *pDriver, int port, unsigned char *delay)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	*delay = 0;

	do
	{
		retVal = fwSelectPhyPort( pDriver, port );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 3, &phyValue );

		if( retVal != FWSuccess ) break;

		*delay = phyValue & 0x0F;

	} while(0);

	return retVal;
}

FWStatus fwSetContender(FWDriverData *pDriver, boolean flag)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	do
	{
		retVal = fwSelectPhyPort( pDriver, 0 );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 4, &phyValue );

		if( retVal != FWSuccess ) break;

		if( flag )
		{
			if( (phyValue & 0x40) == 0 )
			{
				retVal = fwWritePhysicalRegister( pDriver, 4, phyValue | 0x40 );
			}
		}
		else
		{
			if( (phyValue & 0x40) != 0 )
			{
				retVal = fwWritePhysicalRegister( pDriver, 4, phyValue & 0xBF );
			}
		}

	} while(0);

	return retVal;
}

FWStatus fwGetContender(FWDriverData *pDriver, int port, boolean *flag)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	*flag = FALSE;

	do
	{
		retVal = fwSelectPhyPort( pDriver, port );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 4, &phyValue );

		if( retVal != FWSuccess ) break;

		if( phyValue & 0x40 )
			*flag = TRUE;

	} while(0);

	return retVal;
}

FWStatus fwGetJitter(FWDriverData *pDriver, int port, unsigned char *jitter)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	*jitter = 0;

	do
	{
		retVal = fwSelectPhyPort( pDriver, port );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 4, &phyValue );

		if( retVal != FWSuccess ) break;

		*jitter = (phyValue >> 3) & 0x07;

	} while(0);

	return retVal;
}

FWStatus fwGetPowerClass(FWDriverData *pDriver, int port, unsigned char *powerClass)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	*powerClass = 0;

	do
	{
		retVal = fwSelectPhyPort( pDriver, port );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 4, &phyValue );

		if( retVal != FWSuccess ) break;

		*powerClass = phyValue & 0x07;

	} while(0);

	return retVal;
}

FWStatus fwClearPhyInterrupts(FWDriverData *pDriver, int port, unsigned char phyReg)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	do
	{
		retVal = fwSelectPhyPort( pDriver, port );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 5, &phyValue );

		if( retVal != FWSuccess ) break;

		retVal = fwWritePhysicalRegister( pDriver, 5, phyReg );

	} while(0);

	return retVal;
}

FWStatus fwGetPhyInterrupts(FWDriverData *pDriver, int port, unsigned char *phyReg)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	do
	{
		retVal = fwSelectPhyPort( pDriver, port );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 5, &phyValue );

		if( retVal != FWSuccess ) break;

	} while(0);

	return retVal;
}

FWStatus fwSetAcceleratedArbitration(FWDriverData *pDriver, boolean flag)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	do
	{
		retVal = fwSelectPhyPort( pDriver, 0 );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 5, &phyValue );

		if( retVal != FWSuccess ) break;

		if( flag )
		{
			if( (phyValue & 0x02) == 0 )
			{
				retVal = fwWritePhysicalRegister( pDriver, 5, phyValue | 0x02 );
			}
		}
		else
		{
			if( (phyValue & 0x02) != 0 )
			{
				retVal = fwWritePhysicalRegister( pDriver, 5, phyValue & 0xFD );
			}
		}

	} while(0);

	return retVal;
}

FWStatus fwGetAcceleratedArbitration(FWDriverData *pDriver, boolean *flag)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	*flag = FALSE;

	do
	{
		retVal = fwSelectPhyPort( pDriver, 0 );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 5, &phyValue );

		if( retVal != FWSuccess ) break;

		if( (phyValue & 0x02) == 0 )
		{
			*flag = TRUE;
		}

	} while(0);

	return retVal;
}

FWStatus fwSetEnableMultispeed(FWDriverData *pDriver, boolean flag)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	do
	{
		retVal = fwSelectPhyPort( pDriver, 0 );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 5, &phyValue );

		if( retVal != FWSuccess ) break;

		if( flag )
		{
			if( (phyValue & 0x01) == 0 )
			{
				retVal = fwWritePhysicalRegister( pDriver, 5, phyValue | 0x01 );
			}
		}
		else
		{
			if( (phyValue & 0x01) != 0 )
			{
				retVal = fwWritePhysicalRegister( pDriver, 5, phyValue & 0xFE );
			}
		}

	} while(0);

	return retVal;
}

FWStatus fwGetEnableMultispeed(FWDriverData *pDriver, boolean *flag)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	*flag = FALSE;

	do
	{
		retVal = fwSelectPhyPort( pDriver, 0 );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 5, &phyValue );

		if( retVal != FWSuccess ) break;

		if( (phyValue & 0x01) != 0 )
		{
			*flag = TRUE;
		}

	} while(0);

	return retVal;
}

FWStatus fwSetPortEnable(FWDriverData *pDriver, int port, boolean flag)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	do
	{
		retVal = fwSelectPhyPort( pDriver, port );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 8, &phyValue );

		if( retVal != FWSuccess ) break;

		if( flag )
		{
			/* The port is disable when the bit is 1 */
			if( (phyValue & 0x01) != 0 )
			{
				retVal = fwWritePhysicalRegister( pDriver, 8, 0x0 );
			}
		}
		else
		{
			if( (phyValue & 0x01) == 0 )
			{
				retVal = fwWritePhysicalRegister( pDriver, 8, 0x01 );
			}
		}

	} while(0);

	return retVal;
}

FWStatus fwGetPortEnable(FWDriverData *pDriver, int port, boolean *flag)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	*flag = FALSE;

	do
	{
		retVal = fwSelectPhyPort( pDriver, port );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 8, &phyValue );

		if( retVal != FWSuccess ) break;

		if( phyValue & 0x01 )
		{
			*flag = FALSE;
		}
		else
		{
			*flag = TRUE;
		}

	} while(0);

	return retVal;
}

FWStatus fwGetChildParent(FWDriverData *pDriver, int port, boolean *flag)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	*flag = FALSE;

	do
	{
		retVal = fwSelectPhyPort( pDriver, port );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 8, &phyValue );

		if( retVal != FWSuccess ) break;

		if( phyValue & 0x08 )
		{
			*flag = TRUE;
		}

	} while(0);

	return retVal;
}

FWStatus fwGetConnectStatus(FWDriverData *pDriver, int port, boolean *flag)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	*flag = FALSE;

	do
	{
		retVal = fwSelectPhyPort( pDriver, port );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 8, &phyValue );

		if( retVal != FWSuccess ) break;

		if( phyValue & 0x04 )
		{
			*flag = TRUE;
		}

	} while(0);

	return retVal;
}

FWStatus fwGetCableBias(FWDriverData *pDriver, int port, boolean *flag)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	*flag = FALSE;

	do
	{
		retVal = fwSelectPhyPort( pDriver, port );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 8, &phyValue );

		if( retVal != FWSuccess ) break;

		if( phyValue & 0x02 )
		{
			*flag = TRUE;
		}

	} while(0);

	return retVal;
}

FWStatus fwGetPeerSpeed(FWDriverData *pDriver, int port, FWSpeed *speed)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;
	unsigned char conReg;

	*speed = FWSInvalid;

	do
	{
		retVal = fwSelectPhyPort( pDriver, port );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 8, &conReg );

		if( retVal != FWSuccess ) break;

		if( conReg & 0x04 )
		{
			retVal = fwReadPhysicalRegister( pDriver, 9, &phyValue );

			if( retVal != FWSuccess ) break;

			phyValue = phyValue >> 5;

			if( phyValue == 0x0 )
			{
				*speed = FWS100;
			}
			else if( phyValue == 0x01 )
			{
				*speed = FWS200;
			}
			else if( phyValue == 0x02 )
			{
				*speed = FWS400;
			}
		}

	} while(0);

	return retVal;
}

FWStatus fwSetPortInterrupt(FWDriverData *pDriver, int port, boolean flag)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	do
	{
		retVal = fwSelectPhyPort( pDriver, port );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 9, &phyValue );

		if( retVal != FWSuccess ) break;

		if( flag )
		{
			if( (phyValue & 0x10) == 0 )
			{
				retVal = fwWritePhysicalRegister( pDriver, 9, phyValue | 0x10 );
			}
		}
		else
		{
			if( (phyValue & 0x10) != 0 )
			{
				retVal = fwWritePhysicalRegister( pDriver, 9, 0x0 );
			}
		}

	} while(0);

	return retVal;
}

FWStatus fwGetPortInterrupt(FWDriverData *pDriver, int port, boolean *flag)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	*flag = FALSE;

	do
	{
		retVal = fwSelectPhyPort( pDriver, port );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 9, &phyValue );

		if( retVal != FWSuccess ) break;

		if( phyValue & 0x10 )
		{
			*flag = TRUE;
		}

	} while(0);

	return retVal;
}

FWStatus fwClearPortFault(FWDriverData *pDriver, int port)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	do
	{
		retVal = fwSelectPhyPort( pDriver, port );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 9, &phyValue );

		if( retVal != FWSuccess ) break;

		if( phyValue & 0x08 )
		{
			/* writing a 1 to this bit clears the fault bit */
			retVal = fwWritePhysicalRegister( pDriver, 9, phyValue & 0x18 );
		}

	} while(0);

	return retVal;
}

FWStatus fwGetPortFault(FWDriverData *pDriver, int port, boolean *flag)
{
	FWStatus retVal = FWInternalError;
	unsigned char phyValue;

	*flag = FALSE;

	do
	{
		retVal = fwSelectPhyPort( pDriver, port );

		if( retVal != FWSuccess ) break;

		retVal = fwReadPhysicalRegister( pDriver, 9, &phyValue );

		if( retVal != FWSuccess ) break;

		if( phyValue & 0x08 )
		{
			*flag = TRUE;
		}

	} while(0);

	return retVal;
}

FWStatus fwReadPhysicalRegister(FWDriverData *pDriver, int address, unsigned char *value)
{
	FWStatus retVal = FWInternalError;
	UINT32 phyReadCmd;
	UINT32 phyAddr;
	STATUS errStatus;
	unsigned long regStatus;
	int timeoutKnt = 0;

	/* Physical reads to address 0 are forbidden. Use the NodeID register instead. */
	if( address == 0 )
	{
		phyReadCmd = pDriver->ohci->nodeId;

		*value = (unsigned char)(phyReadCmd << 2);

		if( phyReadCmd & 0x40000000 )
		{
			*value |= 0x02;
		}

		if( phyReadCmd & 0x08000000 )
		{
			*value |= 0x01;
		}

		if( phyReadCmd & 0x80000000 )
		{
			retVal = FWSuccess;
		}
		else
		{
			retVal = FWPhyReadFailed;
		}
	}
	else if( address > 0 && address < 16 )
	{
		phyReadCmd = address << 8;

		phyReadCmd |= 0x8000; /* read command */

		/* Make sure the semaphore and status are cleared */
		errStatus = semTake( pDriver->physicalLayerData->phyRegAccess, NO_WAIT );
		pDriver->physicalLayerData->phyRegStatus = 0;

		FWLOGLEVEL9("Sending read cmd to phys 0x%08X.\n", phyReadCmd);
		/* send the PHY read command */
		pDriver->ohci->phyControl = phyReadCmd;

		/* Wait for the read to occur */
		errStatus = semTake( pDriver->physicalLayerData->phyRegAccess, fwPhyTimeout );

		regStatus = pDriver->physicalLayerData->phyRegStatus;

		if( errStatus != OK )
		{
			FWLOGLEVEL5("Physical layer access timeout occured.\n");
		}

		if( regStatus & 1 )
		{
			FWLOGLEVEL5("Physical layer access failed interrupt detected.\n");
		}

		phyReadCmd = pDriver->ohci->phyControl;

		if( phyReadCmd & 0x80000000 )
		{
			phyAddr = (phyReadCmd & 0x0F000000) >> 24;

			if( phyAddr == address )
			{
				*value = (unsigned char)(phyReadCmd >> 16);
				FWLOGLEVEL5("Physical layer value:0x%X.\n", *value );
				retVal = FWSuccess;
			}
			else
			{
				FWLOGLEVEL5("Physical error. Address does not match 0x%08X request address 0x%08X.\n", phyAddr, address);
				retVal = FWPhyReadFailed;
				*value = 0;
			}
		}
		else
		{
			FWLOGLEVEL5("Physical error. Read complete bit not set 0x%08X.\n", phyReadCmd);
			retVal = FWPhyReadFailed;
			*value = 0;
		}
	}
	else
		retVal = FWBoundsError;

	return retVal;
}

FWStatus fwWritePhysicalRegister(FWDriverData *pDriver, int address, unsigned char value)
{
	FWStatus retVal = FWInternalError;
	UINT32 phyWriteCmd;

	/* Physical writes to address 0 are forbidden. Use the NodeID register instead. */
	if( address == 0 )
	{
		/* the only thing that can be written to address 0 is the bus number */
		phyWriteCmd = value;
		pDriver->ohci->nodeId = phyWriteCmd << 6;

		retVal = FWSuccess;
	}
	else if( address > 0 && address < 16 )
	{
		phyWriteCmd = address << 8;

		phyWriteCmd |= (UINT32)value;

		phyWriteCmd |= 0x4000; /* write command */

		/* send the write command */
		pDriver->ohci->phyControl = phyWriteCmd;

		/* Delay for a very short amount of time */
		taskDelay(0);

		retVal = FWSuccess;
	}
	else
		retVal = FWBoundsError;

	return retVal;
}

FWStatus fwSelectPhyPort(FWDriverData *pDriver, int port)
{
	FWStatus retVal = FWInternalError;

	if( pDriver )
	{
		if( pDriver->physicalLayerData && pDriver->ohci )
		{
			if( port < pDriver->physicalLayerData->numPorts && port >= 0 )
			{
				/* select the port */
				retVal = fwWritePhysicalRegister( pDriver, 7, (unsigned char)port );
			}
			else
				retVal = FWBoundsError;
		}
	}

	return retVal;
}

/*
 * Test functions
 *
 */

FWStatus fwTestPhysicalRead(int index)
{
	int i;
	FWStatus retVal = FWInternalError;
	int count = fwGetAdapterCount();
	unsigned char value;

	if( index >= 0 && index < count )
	{
		retVal = fwInitializePhysical( fwDriverDataArray[index] );

		if( retVal == FWSuccess )
		{
			for(i=0; i<6; i++)
			{
				if( i != 6 )
				{
					retVal = fwReadPhysicalRegister(fwDriverDataArray[index], i, &value);

					if( retVal == FWSuccess )
					{
						printf("PHY register:%2d Value:0x%X\n", i, value);
					}
					else
					{
						printf("PHY register:%2d Error:0x%X\n", i, retVal);
					}
				}
			}

			for(i=0; i<fwDriverDataArray[index]->physicalLayerData->numPorts; i++)
			{
				retVal = fwWritePhysicalRegister( fwDriverDataArray[index], 7, i );
				if( retVal == FWSuccess )
				{
					printf("\nPort %d\n", i );

					retVal = fwReadPhysicalRegister(fwDriverDataArray[index], 8, &value);
					if( retVal == FWSuccess )
					{
						printf("PHY register: 8 Value:0x%X\n", value);
					}
					else
					{
						printf("PHY register: 8 Error:0x%X\n", retVal);
					}

					retVal = fwReadPhysicalRegister(fwDriverDataArray[index], 9, &value);
					if( retVal == FWSuccess )
					{
						printf("PHY register: 9 Value:0x%X\n", value);
					}
					else
					{
						printf("PHY register: 9 Error:0x%X\n", retVal);
					}
				}
			}
		}
		else
			printf("Adapter %d could not be initialized. Error:0x%X\n", index, retVal);
	}

	return retVal;
}

int fwPrintOhciReg(int index, int offset)
{
	int count = fwGetAdapterCount();
	UINT32 *pValue = NULL;

	if( index >= 0 && index < count )
	{
		pValue = (UINT32*)(fwDriverDataArray[index]->pciAdapterData->pOhci + offset);

		printf("Adapter:%d offset:0x%X value:0x%X\n", index, offset, *pValue);
	}

	return 0;
}

int fwSetOhciReg(int index, int offset, unsigned int value)
{
	int count = fwGetAdapterCount();
	UINT32 *pValue = NULL;

	if( index >= 0 && index < count )
	{
		pValue = (UINT32*)(fwDriverDataArray[index]->pciAdapterData->pOhci + offset);

		*pValue = value;

		printf("Setting OHCI register. Adapter:%d offset:0x%X value:0x%X\n", index, offset, value);
	}

	return 0;
}

FWStatus fwSetPhyReg(int index, int address, int value);
FWStatus fwGetPhyReg(int index, int port);

FWStatus fwSetPhyReg(int index, int address, int value)
{
	FWStatus retVal = FWInternalError;
	int count = fwGetAdapterCount();
	unsigned char phyValue = (unsigned char)value;

	do
	{
		if( index < 0 || index >= count ) break;

		if( fwDriverDataArray[index]->physicalLayerData == NULL ) break;
		if( fwDriverDataArray[index]->physicalLayerData->initialized == FALSE ) break;

		retVal = fwWritePhysicalRegister( fwDriverDataArray[index], address, phyValue );

		printf("Wrote 0x%X to PHY address %d status 0x%X\n", phyValue, address, retVal );

	} while(0);

	return retVal;
}

FWStatus fwGetPhyReg(int index, int address)
{
	FWStatus retVal = FWInternalError;
	int count = fwGetAdapterCount();
	unsigned char phyValue = 0;

	do
	{
		if( index < 0 || index >= count ) break;

		if( fwDriverDataArray[index]->physicalLayerData == NULL ) break;
		if( fwDriverDataArray[index]->physicalLayerData->initialized == FALSE ) break;

		retVal = fwReadPhysicalRegister( fwDriverDataArray[index], address, &phyValue );

		printf("Read 0x%X from PHY address %d status 0x%X\n", phyValue, address, retVal );

	} while(0);

	return retVal;
}


