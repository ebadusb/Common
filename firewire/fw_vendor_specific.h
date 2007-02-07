/*
 *  Copyright(c) 2005 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains vendor specific hardware initialization routines.
 * Once general initialization is complete. These routines perform
 * additional vendor specific initialization of the hardware.
 *
 * $Log$
 *
 */

#ifndef _FIREWIRE_DEVICE_INCLUDE_
#define _FIREWIRE_DEVICE_INCLUDE_


/* Vendor IDs */
#define FIREWIRE_PCI_TI						0x104C			/* Vendor ID for Texas Instruments */
#define FIREWIRE_PCI_AGERE					0x11C1			/* Vendor ID for Agere Systems */
#define FIREWIRE_PCI_VIA					0x1106			/* Vendor ID for VIA Systems */

#ifdef __cplusplus
extern "C" {
#endif

/* Prototypes */
STATUS sysFirewireChipSpecificConfig(int index);
STATUS sysFirewireTIChipConfig(int index);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FIREWIRE_DEVICE_INCLUDE_ */

