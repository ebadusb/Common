/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains the CSR definitions and routines
 * to update and manage the standard CSRs.
 *
 * $Log: fw_csr.c $
 * Revision 1.1  2007/02/07 15:22:29Z  wms10235
 * Initial revision
 *
 */

#include <vxWorks.h>
#include <string.h>
#include <stdio.h>
#include "fw_utility.h"
#include "fw_link_layer.h"
#include "fw_csr.h"

/* Prototypes for local functions */

/* Local variables */
static UINT32	fwTopologyGenerationCount = 0;
int fwCSRDataMapCompare(const void *key1, const void *key2);
void fwCSRDataMapDestroy(void *key);
void fwCSRDataMapInfoDestroy(void*);
void fwCSRDataMapPrint(const void*);
void fwCSRDataMapPrintInfo(void*);

FWStatus fwCreateCoreCSR(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	do
	{
		if( pDriver == NULL ) break;

		if( pDriver->coreCSR == NULL )
		{
			pDriver->coreCSR = fwMalloc( sizeof(FWCoreCSR) );

			if( pDriver->coreCSR == NULL )
			{
				FWLOGLEVEL3("Failed to allocate memory for the core CSR.\n");
				retVal = FWMemAllocateError;
				break;
			}

			memset( pDriver->coreCSR, 0, sizeof(FWCoreCSR) );
		}

		if( pDriver->coreCSR->topologyMap == NULL )
		{
			pDriver->coreCSR->topologyMap = fwMalloc( sizeof(FWTopologyMap) );
			if( pDriver->coreCSR->topologyMap == NULL )
			{
				FWLOGLEVEL3("Failed to allocate memory for the core CSR topology map.\n");
				retVal = FWMemAllocateError;
				break;
			}

			memset( pDriver->coreCSR->topologyMap, 0, sizeof(FWTopologyMap) );
		}

		if( pDriver->coreCSR->topologyMap->selfIDs == NULL )
		{
			pDriver->coreCSR->topologyMap->selfIDs = fwMalloc( 1024 );
			if( pDriver->coreCSR->topologyMap == NULL )
			{
				FWLOGLEVEL3("Failed to allocate memory for the core CSR selfIds.\n");
				retVal = FWMemAllocateError;
				break;
			}

			memset( pDriver->coreCSR->topologyMap->selfIDs, 0, 1024 );
		}


		if( pDriver->coreCSR->csrSemId == NULL )
		{
			pDriver->coreCSR->csrSemId = semMCreate( SEM_Q_PRIORITY | SEM_INVERSION_SAFE );

			if( pDriver->coreCSR->csrSemId == NULL )
			{
				FWLOGLEVEL3("Failed to allocate a semaphore for the core CSR.\n");
				retVal = FWSemAllocateError;
				break;
			}
		}

		if( pDriver->coreCSR->csrAddressMap == NULL )
		{
			pDriver->coreCSR->csrAddressMap = fwDataMapCreate( fwCSRDataMapCompare,
																				fwCSRDataMapDestroy,
																				fwCSRDataMapInfoDestroy,
																				fwCSRDataMapPrint,
																				fwCSRDataMapPrintInfo );

			if( pDriver->coreCSR->csrAddressMap == NULL )
			{
				FWLOGLEVEL3("Failed to create the CSR address map.\n");
				retVal = FWMemAllocateError;
				break;
			}
		}

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

FWStatus fwDestroyCoreCSR(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	if( pDriver )
	{
		if( pDriver->coreCSR )
		{
			if( pDriver->coreCSR->csrSemId )
			{
				semDelete( pDriver->coreCSR->csrSemId );
				pDriver->coreCSR->csrSemId = NULL;
			}

			if( pDriver->coreCSR->csrAddressMap )
			{
				fwDataMapDestroy( pDriver->coreCSR->csrAddressMap );

				pDriver->coreCSR->csrAddressMap = NULL;
			}

			if( pDriver->coreCSR->topologyMap )
			{
				if( pDriver->coreCSR->topologyMap->selfIDs )
				{
					fwFree( pDriver->coreCSR->topologyMap->selfIDs );
				}

				fwFree( pDriver->coreCSR->topologyMap );
			}

			fwFree( pDriver->coreCSR );

			pDriver->coreCSR = NULL;
		}

		retVal = FWSuccess;
	}

	return retVal;
}

FWStatus fwInitializeCoreCSR(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;

	if( pDriver )
	{
		if( pDriver->coreCSR )
		{
			pDriver->coreCSR->busTime = 0;
			retVal = fwSetSplitTimeoutCSR( pDriver, 0, 800 << 19 );
			retVal = fwSetBusyTimeoutCSR( pDriver, 1 );
			retVal = fwTopologyMapReset( pDriver );
			retVal = fwSetBroadcastChannelCSR( pDriver, 0 );
		}
	}

	return retVal;
}

FWStatus fwTopologyMapComplete(FWDriverData *pDriver, UINT32 generation)
{
	FWStatus retVal = FWInternalError;
	UINT16 tcrc = 0;
	UINT32 nodeIndex = 0;
	UINT32 nodeSelfCount;
	int i;

	if( pDriver )
	{
		if( pDriver->coreCSR )
		{
			if( pDriver->coreCSR->topologyMap )
			{
				semTake( pDriver->coreCSR->csrSemId, WAIT_FOREVER );

				pDriver->coreCSR->topologyMap->nodeCount = 0;

				for(nodeIndex=0; nodeIndex<64; nodeIndex++)
				{
					if( pDriver->coreCSR->topologyMap->nodes[nodeIndex] > 0 )
						pDriver->coreCSR->topologyMap->nodeCount++;
				}

				pDriver->coreCSR->topologyMap->generation = generation;
				pDriver->coreCSR->topologyMap->length = pDriver->coreCSR->topologyMap->selfIDCount + 2;
				nodeSelfCount = (UINT32)pDriver->coreCSR->topologyMap->nodeCount << 16;
				nodeSelfCount |= pDriver->coreCSR->topologyMap->selfIDCount;

				tcrc = fwComputeCRC16(tcrc, pDriver->coreCSR->topologyMap->generation);
				tcrc = fwComputeCRC16(tcrc, nodeSelfCount);

				for(i=0; i<pDriver->coreCSR->topologyMap->selfIDCount; i++)
				{
					tcrc = fwComputeCRC16(tcrc, pDriver->coreCSR->topologyMap->selfIDs[i]);
				}

				pDriver->coreCSR->topologyMap->crc = tcrc;

				semGive( pDriver->coreCSR->csrSemId );

				retVal = FWSuccess;
			}
		}
	}

	return retVal;
}

FWStatus fwTopologyMapAddSelfID(FWDriverData *pDriver, UINT32 selfIDQuadlet)
{
	FWStatus retVal = FWInternalError;
	UINT32 nodeIndex = 0;

	if( pDriver )
	{
		if( pDriver->coreCSR )
		{
			if( pDriver->coreCSR->topologyMap )
			{
				semTake( pDriver->coreCSR->csrSemId, WAIT_FOREVER );

				nodeIndex = selfIDQuadlet >> 24;
				nodeIndex &= 0x0000003F;
				pDriver->coreCSR->topologyMap->nodes[nodeIndex]++;
				pDriver->coreCSR->topologyMap->selfIDs[pDriver->coreCSR->topologyMap->selfIDCount] = selfIDQuadlet;
				pDriver->coreCSR->topologyMap->selfIDCount++;

				semGive( pDriver->coreCSR->csrSemId );

				retVal = FWSuccess;
			}
		}
	}

	return retVal;
}

FWStatus fwTopologyMapReset(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	UINT32 nodeIndex = 0;

	if( pDriver )
	{
		if( pDriver->coreCSR )
		{
			if( pDriver->coreCSR->topologyMap )
			{
				semTake( pDriver->coreCSR->csrSemId, WAIT_FOREVER );

				pDriver->coreCSR->topologyMap->length = 2;
				pDriver->coreCSR->topologyMap->nodeCount = 0;
				pDriver->coreCSR->topologyMap->selfIDCount = 0;
				pDriver->coreCSR->topologyMap->crc = 0;

				for(nodeIndex=0; nodeIndex<64; nodeIndex++)
				{
					pDriver->coreCSR->topologyMap->nodes[nodeIndex] = 0;
				}

				semGive( pDriver->coreCSR->csrSemId );

				retVal = FWSuccess;
			}
		}
	}

	return retVal;
}

FWStatus fwGetTopologyMapData(FWDriverData *pDriver, UINT32 offset, UINT32 *data)
{
	FWStatus retVal = FWInternalError;
	unsigned int index;

	*data = 0;

	do
	{
		if( pDriver == NULL ) break;
      if( pDriver->coreCSR == NULL ) break;
      if( pDriver->coreCSR->topologyMap == NULL ) break;
		if( data == NULL )
		{
			retVal = FWInvalidArg;
			break;
		}

		semTake( pDriver->coreCSR->csrSemId, WAIT_FOREVER );

		if( offset == 0xF0001000 )
		{
			*data = (UINT32)pDriver->coreCSR->topologyMap->length << 16;
			*data |= pDriver->coreCSR->topologyMap->crc;
		}
		else if( offset == 0xF0001004 )
		{
			*data = pDriver->coreCSR->topologyMap->generation;
		}
		else if( offset == 0xF0001008 )
		{
			*data = (UINT32)pDriver->coreCSR->topologyMap->nodeCount << 16;
			*data |= pDriver->coreCSR->topologyMap->selfIDCount;
		}
		else if( offset >= 0xF000100C && offset <= 0xF00013FC )
		{
			index = (offset - 0xF000100C) / 4;
			*data = pDriver->coreCSR->topologyMap->selfIDs[index];
		}
		else
		{
			retVal = FWAddressError;
			break;
		}

		semGive( pDriver->coreCSR->csrSemId );

		retVal = FWSuccess;

	} while(0);

	return retVal;
}

FWStatus fwSetStateClearCSR(FWDriverData *pDriver, UINT32 value)
{
	FWStatus retVal = FWInternalError;
	UINT32 stateCSR;

	if( pDriver )
	{
		if( pDriver->coreCSR )
		{
			stateCSR = pDriver->coreCSR->state;

			pDriver->coreCSR->state &= ~value;

			/* Check if the cycle master is being turned off */
			if( value & stateCSR & 0x00000100 )
			{
				/* The stateCSR shows that the cycle master is on
				 * and this request is indicating it should be turned
				 * off */
				retVal = fwSetCycleMaster( pDriver, FALSE );
			}
			else
				retVal = FWSuccess;
		}
	}

	return retVal;
}

FWStatus fwSetStateSetCSR(FWDriverData *pDriver, UINT32 value)
{
	FWStatus retVal = FWInternalError;
	UINT32 stateCSR;

	if( pDriver )
	{
		if( pDriver->coreCSR )
		{
			stateCSR = pDriver->coreCSR->state;

			pDriver->coreCSR->state |= value;

			/* Check if the cycle master is being turned on */
			if( value & ~stateCSR & 0x00000100 )
			{
				/* The stateCSR shows that the cycle master is off
				 * and this request is indicating it should be turned
				 * on */
				retVal = fwSetCycleMaster( pDriver, TRUE );
			}
			else
				retVal = FWSuccess;
		}
	}

	return retVal;
}

FWStatus fwGetStateCSR(FWDriverData *pDriver, UINT32 *value)
{
	FWStatus retVal = FWInternalError;

	*value = 0;

	if( pDriver )
	{
		if( pDriver->coreCSR )
		{
			*value = pDriver->coreCSR->state;
			retVal = FWSuccess;
		}
	}

	return retVal;
}

FWStatus fwSetNodeIDCSR(FWDriverData *pDriver, UINT32 value)
{
	FWStatus retVal = FWInternalError;
	unsigned short busId = 0;

	if( pDriver )
	{
		if( pDriver->coreCSR )
		{
			busId = value >> 22;

			/* Note only the bus ID portion of the node ID can change. */
			retVal = fwSetBusNumber( pDriver, busId );
		}
	}

	return retVal;
}

FWStatus fwGetNodeIDCSR(FWDriverData *pDriver, UINT32 *value)
{
	FWStatus retVal = FWInternalError;
	unsigned short busNumber = 0;
	unsigned char nodeNumber = 0;

	*value = 0xFFFF0000;

	if( pDriver )
	{
		if( pDriver->coreCSR )
		{
			retVal = fwGetNodeID( pDriver, &busNumber, &nodeNumber );

			*value = ((UINT32)busNumber << 22) | ((UINT32)nodeNumber << 16);
		}
	}

	return retVal;
}

FWStatus fwSetResetStartCSR(FWDriverData *pDriver, UINT32 value)
{
	FWStatus retVal = FWInternalError;

	if( pDriver )
	{
		if( pDriver->coreCSR )
		{
			/* Reset the interface without losing communications. */
			retVal = FWSuccess;
		}
	}

	return retVal;
}

FWStatus fwGetResetStartCSR(FWDriverData *pDriver, UINT32 *value)
{
	FWStatus retVal = FWSuccess;

	/* The reset CSR always reads as 0 */
	*value = 0;

	return retVal;
}

FWStatus fwSetSplitTimeoutCSR(FWDriverData *pDriver, UINT32 splitTimeoutHi, UINT32 splitTimeoutLow)
{
	FWStatus retVal = FWInternalError;

	if( pDriver )
	{
		if( pDriver->coreCSR )
		{
			pDriver->coreCSR->splitTimeoutHi = splitTimeoutHi & 0x00000007;
			pDriver->coreCSR->splitTimeoutLow = splitTimeoutLow & 0xFFF80000;

			/* a minimum value of 800 is required for the split transaction timeout */
			if( pDriver->coreCSR->splitTimeoutHi == 0 && (pDriver->coreCSR->splitTimeoutLow >> 19) < 800 )
			{
				pDriver->coreCSR->splitTimeoutLow = (800 << 19);
			}

			retVal = fwSetSplitTransTimeout( pDriver, (int)pDriver->coreCSR->splitTimeoutHi, (int)(pDriver->coreCSR->splitTimeoutLow >> 19) );
		}
	}

	return retVal;
}

FWStatus fwGetSplitTimeoutCSR(FWDriverData *pDriver, UINT32 *splitTimeoutHi, UINT32 *splitTimeoutLow)
{
	FWStatus retVal = FWInternalError;

	*splitTimeoutHi = 0;
	*splitTimeoutLow = 0;

	if( pDriver )
	{
		if( pDriver->coreCSR )
		{
			*splitTimeoutHi = pDriver->coreCSR->splitTimeoutHi;
			*splitTimeoutLow = pDriver ->coreCSR->splitTimeoutLow;

			retVal = FWSuccess;
		}
	}

	return retVal;
}

FWStatus fwSetCycleTimeCSR(FWDriverData *pDriver, UINT32 value)
{
	FWStatus retVal = FWInvalidArg;

	/* Setting the cycle time is not supported */

	return retVal;
}

FWStatus fwGetCycleTimeCSR(FWDriverData *pDriver, UINT32 *value)
{
	return fwGetCycleTime( pDriver, value );
}

FWStatus fwSetBusTimeCSR(FWDriverData *pDriver, UINT32 value)
{
	FWStatus retVal = FWInternalError;

	if( pDriver )
	{
		if( pDriver->coreCSR )
		{
			pDriver->coreCSR->busTime = value >> 7;

			retVal = fwSetCycleTime( pDriver, value << 25 );
		}
	}

	return retVal;
}

FWStatus fwGetBusTimeCSR(FWDriverData *pDriver, UINT32 *value)
{
	FWStatus retVal = FWInternalError;
	UINT32 cycleTime;

	if( pDriver )
	{
		if( pDriver->coreCSR )
		{
			retVal = fwGetCycleTime( pDriver, &cycleTime );

			if( retVal == FWSuccess )
			{
				*value = (pDriver->coreCSR->busTime << 7) | (cycleTime >> 25);
			}
		}
	}

	return retVal;
}

FWStatus fwSetBusyTimeoutCSR(FWDriverData *pDriver, UINT32 value)
{
	FWStatus retVal = FWInternalError;
	UINT32 busyTimeout = value & 0x0FFFF00F;
	unsigned short cycleLimit;
	int maxATRetries;

	cycleLimit = (unsigned short)( busyTimeout >> 12 );
	maxATRetries = busyTimeout & 0x0000000F;

	fwSetCycleLimit( pDriver, cycleLimit );

	retVal = fwSetAsyncRetryLimit( pDriver, maxATRetries );

	return retVal;
}

FWStatus fwGetBusyTimeoutCSR(FWDriverData *pDriver, UINT32 *value)
{
	FWStatus retVal = FWInternalError;
	UINT32 busyTimeout;
	unsigned short cycleLimit;
	int maxATRetries;

	*value = 0;

	retVal = fwGetCycleLimit( pDriver, &cycleLimit );

	if( retVal == FWSuccess )
	{
		retVal = fwGetAsyncRetryLimit( pDriver, &maxATRetries );

		if( retVal == FWSuccess )
		{
			busyTimeout = (UINT32)cycleLimit << 12;
			busyTimeout |= (UINT32)maxATRetries;
			*value = busyTimeout;
		}
	}

	return retVal;
}

FWStatus fwSetBusMgrIdCSR(FWDriverData *pDriver, UINT32 compare, UINT32 *swap)
{
	return fwSetBuiltinCSR( pDriver, compare, swap, 0 );
}

FWStatus fwGetBusMgrIdCSR(FWDriverData *pDriver, UINT32 *value)
{
	return fwGetBuiltinCSR( pDriver, value, 0 );
}

FWStatus fwSetBandwidthAvailableCSR(FWDriverData *pDriver, UINT32 compare, UINT32 *swap)
{
	return fwSetBuiltinCSR( pDriver, compare, swap, 1 );
}

FWStatus fwGetBandwidthAvailableCSR(FWDriverData *pDriver, UINT32 *value)
{
	return fwGetBuiltinCSR( pDriver, value, 1 );
}

FWStatus fwSetChannelsAvailableHiCSR(FWDriverData *pDriver, UINT32 compare, UINT32 *swap)
{
	return fwSetBuiltinCSR( pDriver, compare, swap, 2 );
}

FWStatus fwGetChannelsAvailableHiCSR(FWDriverData *pDriver, UINT32 *channelsHi)
{
	return fwGetBuiltinCSR( pDriver, channelsHi, 2 );
}

FWStatus fwSetChannelsAvailableLoCSR(FWDriverData *pDriver, UINT32 compare, UINT32 *swap)
{
	return fwSetBuiltinCSR( pDriver, compare, swap, 3 );
}

FWStatus fwGetChannelsAvailableLoCSR(FWDriverData *pDriver, UINT32 *channelsLo)
{
	return fwGetBuiltinCSR( pDriver, channelsLo, 3 );
}

FWStatus fwSetPriorityBudgetCSR(FWDriverData *pDriver, UINT32 priority)
{
	unsigned char fairness = (unsigned char)(priority & 0x3F);

	return fwSetPriorityBudget( pDriver, fairness );
}

FWStatus fwGetPriorityBudgetCSR(FWDriverData *pDriver, UINT32 *priority)
{
	FWStatus retVal;
	unsigned char fairness;
	unsigned char maxFairness;

	*priority = 0;

	retVal = fwGetPriorityBudget( pDriver, &fairness );

	if( retVal == FWSuccess )
	{
		retVal = fwGetMaxPriorityBudget( pDriver, &maxFairness );

		if( retVal == FWSuccess )
		{
			*priority = ((UINT32)maxFairness) << 8;
			*priority += fairness;
		}
	}

	return retVal;
}

FWStatus fwSetBroadcastChannelCSR(FWDriverData *pDriver, UINT32 channel)
{
	FWStatus retVal = FWInternalError;

	if( pDriver )
	{
		if( pDriver->coreCSR )
		{
			pDriver->coreCSR->broadcastChannel = channel & 0x40000000;
			pDriver->coreCSR->broadcastChannel |= 0x8000001F;

			retVal = FWSuccess;
		}
	}

	return retVal;
}

FWStatus fwGetBroadcastChannelCSR(FWDriverData *pDriver, UINT32 *channel)
{
	FWStatus retVal = FWInternalError;

	if( pDriver )
	{
		if( pDriver->coreCSR )
		{
			*channel = pDriver->coreCSR->broadcastChannel;
			retVal = FWSuccess;
		}
	}

	return retVal;
}

FWCSRAddressMap *fwLookupCSRAddress(FWDriverData *pDriver, FWAddressOffset *offset, unsigned long dataLength)
{
	FWCSRAddressMap *retVal = NULL;
	FWCSRAddressMapKey addressMapKey;
	FWMapDataItem *result;

	do
	{
		if( pDriver == NULL ) break;
		if( offset == NULL ) break;
		if( pDriver->coreCSR == NULL ) break;
		if( pDriver->coreCSR->csrAddressMap == NULL ) break;

		addressMapKey.blockSize = dataLength;
		addressMapKey.offsetBegin = *offset;

		result = fwDataMapExactQuery( pDriver->coreCSR->csrAddressMap, (void*)&addressMapKey );

		if( result == NULL ) break;

		retVal = (FWCSRAddressMap*)result->info;

	} while(0);

	return retVal;
}

/*
 * fwValidateCSRMemoryAddress
 *
 * This function validates that the offset and length
 * are fully contained and mapped to memory in the supplied
 * CSR address map structure.
 *
 * Returns:
 *     NULL if the range defined by offset and data length is
 *        not fully mapped in the address map.
 *
 *     Memory address of the first byte defined by offset and
 *     data length.
 */
unsigned char *fwValidateCSRMemoryAddress(FWCSRAddressMap *addressMap, FWAddressOffset *offset, unsigned long dataLength)
{
	unsigned char *retVal = NULL;
	UINT64 actualBegin;
	UINT64 actualEnd;
	UINT64 requestedBegin;
	UINT64 requestedEnd;

	do
	{
		if( offset == NULL ) break;
		if( addressMap == NULL ) break;

		/* Determine if this is a valid address */
		actualBegin = (UINT64)addressMap->offsetBegin.highOffset << 32;
		actualBegin |= addressMap->offsetBegin.lowOffset;
		actualEnd = actualBegin + addressMap->blockSize - 1;

		requestedBegin = (UINT64)offset->highOffset << 32;
		requestedBegin |= offset->lowOffset;
		requestedEnd = requestedBegin + dataLength - 1;

		if( requestedBegin < actualBegin || requestedEnd > actualEnd )
		{
			/* The requested memory is not fully contained within
			 * the actual memory block defined in addressMap.
			 */
			break;
		}

		retVal = addressMap->address + (UINT32)(requestedBegin - actualBegin);

	} while(0);

	return retVal;
}

FWStatus fwLockCSRAddressMap(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	STATUS errStatus = semTake( pDriver->coreCSR->csrSemId, WAIT_FOREVER );

	if( errStatus == OK )
		retVal = FWSuccess;

	return retVal;
}

FWStatus fwUnlockCSRAddressMap(FWDriverData *pDriver)
{
	FWStatus retVal = FWInternalError;
	STATUS errStatus = semGive( pDriver->coreCSR->csrSemId );

	if( errStatus == OK )
		retVal = FWSuccess;

	return retVal;
}

/* The key value for the CSR data map is a FWCSRAddressMapKey
 * structure and the info is a FWCSRAddressMap structure.
 */

/*
 * fwCSRDataMapCompare
 *
 * This function compares two key values in the CSR map.
 * If the memory ranges defined in the keys overlap in
 * any way they are considered equal.
 *
 * Returns:
 *     -1 if the range defined by key1 is less than the range
 *        defined by key2.
 *      0 if the ranges overlap in anyway. Note that the validate
 *        CSR memory function must be used to determine if the
 *        requested range is usable and fully mapped.
 *      1 if the range defined by key1 is greater than the range
 *        defined by key2.
 */
int fwCSRDataMapCompare(const void *key1, const void *key2)
{
	int retVal = 0;
	FWCSRAddressMapKey *address1 = (FWCSRAddressMapKey*)key1;
	FWCSRAddressMapKey *address2 = (FWCSRAddressMapKey*)key2;
	UINT64 begin1;
	UINT64 begin2;
	UINT64 end1;
	UINT64 end2;

	if( address1 && address2 )
	{
		begin1 = (UINT64)address1->offsetBegin.highOffset << 32;
		begin1 |= address1->offsetBegin.lowOffset;
		begin2 = (UINT64)address2->offsetBegin.highOffset << 32;
		begin2 |= address2->offsetBegin.lowOffset;
		end1 = begin1 + address1->blockSize - 1;
		end2 = begin2 + address2->blockSize - 1;

		if( end1 < begin2 )
		{
			/* The range defined by key1 is less than key2 */
			retVal = -1;
		}
		else if( begin1 > end2 )
		{
			/* The range defined by key1 is greater than key2 */
			retVal = 1;
		}
	}

	return retVal;
}

void fwCSRDataMapDestroy(void *key)
{
	fwFree( key );
}

void fwCSRDataMapInfoDestroy(void* info)
{
	FWCSRAddressMap *addrMap = (FWCSRAddressMap*)info;

	if( addrMap )
	{
		fwFree( addrMap->address );
		fwFree( addrMap );
	}
}

void fwCSRDataMapPrint(const void* key)
{
	const FWCSRAddressMapKey *address = (const FWCSRAddressMapKey*)key;

	if( address )
	{
		FWLOGLEVEL9("CSR map key 0x%04X %08X Size:%d\n", address->offsetBegin.highOffset, address->offsetBegin.lowOffset, address->blockSize);
	}
}

void fwCSRDataMapPrintInfo(void* info)
{
	FWCSRAddressMap *addrMap = (FWCSRAddressMap*)info;

	FWLOGLEVEL9("CSR map item 0x%04X %08X Size:%d Align:%d RO:%d Address:0x%08X\n",
					addrMap->offsetBegin.highOffset,
					addrMap->offsetBegin.lowOffset,
					addrMap->blockSize,
					addrMap->alignedOnly,
					addrMap->readOnly,
					addrMap->address );
}

