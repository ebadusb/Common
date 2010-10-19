/*
 *  Copyright(c) 2005 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file defines the firewire PCI hardware initialization routines.
 * These routines detect any OHCI v1.1 compatible firewire cards in
 * the system, map them into memory with minimum configuration, and
 * disables all interrupts. Note that there must be a sufficent number
 * of MMU_DUMMY_ENTRY entries in the sysMmuMapTable
 *
 * $Log: fw_pci_support.h $
 * Revision 1.1  2007/02/07 15:22:37Z  wms10235
 * Initial revision
 *
 */

#ifndef _FW_PCI_SUPPORT_INCLUDE_
#define _FW_PCI_SUPPORT_INCLUDE_

#define FIREWIRE_MAX_PCI_ADAPTERS			4
#define FIREWIRE_PCI_DEVICE_CLASS			0x0C0010
#define FIREWIRE_PCI_MEMORY_MAP_SIZE		0x2000
#define FW_BYTE_SWAP_ENABLED					TRUE			/* byte swap async data */

/* Vendor IDs */
#define FIREWIRE_PCI_TI						0x104C			/* Vendor ID for Texas Instruments */
#define FIREWIRE_PCI_AGERE					0x11C1			/* Vendor ID for Agere Systems */
#define FIREWIRE_PCI_VIA					0x1106			/* Vendor ID for VIA Systems */

#ifdef __cplusplus
extern "C" {
#endif

/* Function pointer for the ISR routine. */
typedef void (*FWIsrRoutine)(int adapterIndex);

typedef struct FWPciAdapterDataStruct
{
	int				vendorId;		/* PCI vendor ID */
	int				deviceId;		/* PCI device ID */
	int				busNo;			/* PCI bus number */
	int				deviceNo;		/* PCI device number */
	int				funcNo;			/* PCI function number */
	unsigned char	irq;				/* PCI interrupt line */
	void*				pOhci;			/* OHCI register base address */
} FWPciAdapterData;

/* Prototypes */
STATUS	sysFirewireHwInit(void);
STATUS	sysFirewirePciDetect(int index);
int		fwGetAdapterCount(void);
STATUS	fwInstallInterruptHandler(FWIsrRoutine isrFunctionPtr, int adapterIndex);
STATUS	fwUninstallInterruptHandler(FWIsrRoutine isrFunctionPtr, int adapterIndex);
void		firewireShow(void);

/* This structure hold the PCI information for an adapter */
extern FWPciAdapterData	fwAdapterData[FIREWIRE_MAX_PCI_ADAPTERS];

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_PCI_SUPPORT_INCLUDE_ */

