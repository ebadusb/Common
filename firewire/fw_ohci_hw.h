/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains the firewire OHCI hardware interface
 * structure definitions for register access and DMA context
 * programs.
 *
 * $Log$
 *
 */

#ifndef _OHCI_HW_INCLUDE_
#define _OHCI_HW_INCLUDE_

#include <vxWorks.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OhciContextStruct
{
	UINT32	contextControlSet;
	UINT32	contextControlClr;
	UINT32	reserved;
	UINT32	commandPtr;
} OhciContext;

typedef struct OhciContextRecvStruct
{
	UINT32	contextControlSet;
	UINT32	contextControlClr;
	UINT32	reserved1;
	UINT32	commandPtr;
	UINT32	contextMatch;
	UINT32	reserved2;
	UINT32	reserved3;
	UINT32	reserved4;
} OhciContextRecv;

/* OHCI Register Map Structure */
typedef struct OhciRegistersStruct
{
	UINT32	version;							/* OHCI version - 0x00 */
	UINT32	guidRom;							/* GUID ROM - 0x04 */
	UINT32	asyncTxRetries;				/* Async transmit retries - 0x08 */
	UINT32	csrData;							/* CSR data - 0x0C */
	UINT32	csrCompare;						/* CSR compare - 0x10 */
	UINT32	csrControl;						/* CSR control - 0x14 */
	UINT32	configRomHeader;				/* Configuration ROM header - 0x18 */
	UINT32	busId;							/* Firewire bus ID - 0x1C */
	UINT32	busOptions;						/* Firewire bus options - 0x20 */
	UINT32	guiIdHi;							/* GUID ID high 32 bits - 0x24 */
	UINT32	guiIdLo;							/* GUID ID low 32 bits - 0x28 */
	UINT32	reserved_2C;					/* reserved register - 0x2C */
	UINT32	reserved_30;					/* reserved register - 0x30 */
	UINT32	configROMAddr;					/* Configuation ROM address - 0x34 */
	UINT32	postedWriteAddrLo;			/* Posted write address low - 0x38 */
	UINT32	postedWriteAddrHi;			/* Posted write address high - 0x3C */
	UINT32	vendorId;						/* Vendor ID - 0x40 */
	UINT32	reserved_44;					/* reserved register - 0x44 */
	UINT32	reserved_48;					/* reserved register - 0x48 */
	UINT32	reserved_4C;					/* reserved register - 0x4C */
	UINT32	hcControlSet;					/* Host controller set - 0x50 */
	UINT32	hcControlClr;					/* Host controller clear - 0x54 */
	UINT32	reserved_58;					/* reserved register - 0x58 */
	UINT32	reserved_5C;					/* reserved register - 0x5C */
	UINT32	reserved_60;					/* reserved register - 0x60 */
	UINT32	selfIdBuffer;					/* Self ID buffer - 0x64 */
	UINT32	selfIdCount;					/* Self ID count - 0x68 */
	UINT32	reserved_6C;					/* reserved register - 0x6C */
	UINT32	irChannelMaskHiSet;			/* Isochronous receive channel set mask high - 0x70 */
	UINT32	irChannelMaskHiClr;			/* Isochronous receive channel clear mask high - 0x74 */
	UINT32	irChannelMaskLoSet;			/* Isochronous receive channel set mask low - 0x78 */
	UINT32	irChannelMaskLoClr;			/* Isochronous receive channel clear mask low - 0x7C */
	UINT32	intEventSet;					/* Interrupt event set - 0x80 */
	UINT32	intEventClr;					/* Interrupt event clear - 0x84 */
	UINT32	intMaskSet;						/* Interrupt mask set - 0x88 */
	UINT32	intMaskClr;						/* Interrupt mask clear - 0x8C */
	UINT32	isoXmitIntEventSet;			/* Isochronous transmit interrupt event set - 0x90 */
	UINT32	isoXmitIntEventClr;			/* Isochronous transmit interrupt event clear - 0x94 */
	UINT32	isoXmitIntMaskSet;			/* Isochronous transmit interrupt mask set - 0x98 */
	UINT32	isoXmitIntMaskClr;			/* Isochronous transmit interrupt mask clear - 0x9C */
	UINT32	isoRecvIntEventSet;			/* Isochronous receive interrupt event set - 0xA0 */
	UINT32	isoRecvIntEventClr;			/* Isochronous receive interrupt event clear - 0xA4 */
	UINT32	isoRecvIntMaskSet;			/* Isochronous receive interrupt mask set - 0xA8 */
	UINT32	isoRecvIntMaskClr;			/* Isochronous receive interrupt mask clear - 0xAC */
	UINT32	initialBandwidthAvail;		/* Initial bandwidth available - 0xB0 */
	UINT32	initialChannelsAvailHi;		/* Initial channels available high - 0xB4 */
	UINT32	initialChannelsAvailLo;		/* Initial channels available low - 0xB4 */
	UINT32	reserved_BC_D8[8];			/* reserved registers 0xBC - 0xD8 */
	UINT32	fairnessControl;        	/* Fairness control - 0xDC */
	UINT32	linkControlSet;				/* Link control set - 0xE0 */
	UINT32	linkControlClr;				/* Link control clear - 0xE4 */
	UINT32	nodeId;							/* Node ID - 0xE8 */
	UINT32	phyControl;						/* PHY control - 0xEC */
	UINT32	isoCycleTimer;					/* Isochronous cycle timer - 0xF0 */
	UINT32	reserved_F4_FC[3];			/* reserved registers 0xF4 - 0xFC */
	UINT32	asyncRequestFilterHiSet;	/* Asynchronous request filter high set - 0x100 */
	UINT32	asyncRequestFilterHiClr;	/* Asynchronous request filter high clear - 0x104 */
	UINT32	asyncRequestFilterLoSet;	/* Asynchronous request filter low set - 0x108 */
	UINT32	asyncRequestFilterLoClr;	/* Asynchronous request filter low clear - 0x10C */
	UINT32	phyRequestFilterHiSet;		/* Physical request filter high set - 0x110 */
	UINT32	phyRequestFilterHiClr;		/* Physical request filter high clear - 0x114 */
	UINT32	phyRequestFilterLoSet;		/* Physical request filter low set - 0x118 */
	UINT32	phyRequestFilterLoClr;		/* Physical request filter low clear - 0x11C */
	UINT32	phyUpperBound;					/* Physical upper bound - 0x120 */
	UINT32	reserved_124_17C[23];		/* reserved registers 0x124 - 0x17C */
	OhciContext	asyncTxRequest;			/* Asynchronous request transmit context 0x180 - 0x18C */
	UINT32	reserved_190_19C[4];			/* reserved registers 0x190 - 0x19C */
	OhciContext	asyncTxResponse;			/* Asynchronous response transmit context 0x1A0 - 0x1AC */
	UINT32	reserved_1B0_1BC[4];			/* reserved registers 1B0h - 0x1BC */
	OhciContext	asyncRxRequest;			/* Asynchronous request receive context 0x1C0 - 0x1CC */
	UINT32	reserved_1D0_1DC[4];			/* reserved registers 0x1DC - 0x1CC */
	OhciContext	asyncRxResponse;			/* Asynchronous response receive context 0x1E0 - 0x1EC */
	UINT32	reserved_1F0_1FF[4];			/* reserved registers 0x1F0 - 0x1FC */
	OhciContext isoXmit[16];				/* Isochronous transmit contexts 0x200 - 0x2FC */
	UINT32	reserved_300_3FC[64];		/* reserved space 0x300 - 0x3FC */
	OhciContextRecv	isoRecv[32];		/* Isochronous receive contexts 0x400 - 0x7FC */
	UINT32	endOhci;							/* used for validation */
} OhciRegisters;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _OHCI_HW_INCLUDE_ */

