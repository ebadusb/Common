/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains the CSR data structure definitions.
 *
 * $Log$
 *
 */

#ifndef _FW_CSR_DATA_INCLUDE_
#define _FW_CSR_DATA_INCLUDE_

#include <vxWorks.h>
#include <firewire/fw_dma_memory.h>
#include <firewire/fw_data_map.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Structure for CSRs */

/* Topology map at offset 0x1000 */
typedef struct FWTopologyMapStruct
{
	UINT16	length;
	UINT16	crc;
	UINT32	generation;
	UINT16	nodeCount;
	UINT16	selfIDCount;
	int		nodes[64];		/* Physical IDs of the nodes on the bus */
	UINT32	*selfIDs;

} FWTopologyMap;

typedef struct FWCoreCSRStruct
{
	UINT32			state;					/* STATE_CLEAR and STATE_SET modify the state CSR */
	UINT32   		splitTimeoutHi;		/* Split timeout CSR hi value */
	UINT32			splitTimeoutLow;		/* Split timeout CSR low value */
	UINT32			busTime;					/* Bus time published by the bus manager */
	UINT32			broadcastChannel;		/* Broadcast channel published by the isochronous mgr */
	unsigned char	maxPriorityBudget;	/* Max priority budget allowed. */
	FWTopologyMap	*topologyMap;			/* Pointer to the topology map */
	FWDataMap		*csrAddressMap;		/* Map that contains the memory address to CSR mapping */
	SEM_ID			csrSemId;				/* Mutex semphore to guard access to the CSR memory areas */

} FWCoreCSR;

/* Structure for config ROM CSR data */
typedef struct FWConfigROMStruct
{
	FWDmaMemoryEntry	*memPtr;						/* Allocated DMA safe memory block used for config ROM (3K to allow 2 1K configROM areas) */
	UINT32				*config1VirtualAddr;		/* Virtual address for 1K of DMA memory for config 1 */
	UINT32				*config1PhysicalAddr;	/* Physical address for 1K of DMA memory aligned on a 1K boundry */
	UINT32				*config2VirtualAddr;		/* Virtual address for 1K of DMA memory for config 2 */
	UINT32				*config2PhysicalAddr;	/* Physical address for 1K of DMA memory aligned on a 1K boundry */
	int					selectedConfig;			/* Selected/active configROM -1=none, 1=config1, 2=config2 */

} FWConfigROM;

/* This structure is used to map a block of CSR registers to
 * a client address where the CSR quadlet data is stored.
 * When an async request for a CSR location is received, the
 * bus manager will lookup the internal address
 */
typedef struct FWCSRAddressMapStruct
{
	FWAddressOffset	offsetBegin;	/* IEEE-1394 address offset of begining of the block */
	unsigned long		blockSize;		/* Number of bytes in the block */
	boolean				alignedOnly;	/* Quadlet aligned access only */
	boolean				readOnly;		/* The CSR is read only */
	unsigned char		*address;		/* Memory location of where the quadlet data for the request is stored. */

} FWCSRAddressMap;

typedef struct FWCSRAddressMapKeyStruct
{
	FWAddressOffset	offsetBegin;	/* IEEE-1394 address offset of begining of the block */
	unsigned long		blockSize;		/* Number of bytes in the block */

} FWCSRAddressMapKey;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_CSR_DATA_INCLUDE_ */

