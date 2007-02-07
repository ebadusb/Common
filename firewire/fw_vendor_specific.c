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

#include <vxWorks.h>
#include <stdio.h>
#include <vmLib.h>
#include <drv/pci/pciConfigLib.h>
#include "fw_pci_support.h"
#include "fw_vendor_specific.h"

STATUS sysFirewireChipSpecificConfig(int index)
{
	STATUS retVal = ERROR;

	if( index >= 0 && index < fwGetAdapterCount() )
	{
		switch( fwAdapterData[index].vendorId )
		{
		case FIREWIRE_PCI_TI: /* TI specific setup */
			retVal = sysFirewireTIChipConfig(index);
			break;

		default: /* Default chip config */
			retVal = OK;
			break;
		}
	}

	return retVal;
}

STATUS sysFirewireTIChipConfig(int index)
{
	STATUS retVal = ERROR;
	UINT32 value;

	do
	{
		/* A little bounds checking */
		if( index < 0 || index >= fwGetAdapterCount() )
			break;

		/* TI miscellaneous configuration register */

		value = 0x10; /* Disable target abort. */
		retVal = pciConfigModifyLong( fwAdapterData[index].busNo,
												fwAdapterData[index].deviceNo,
												fwAdapterData[index].funcNo,
												0x0F0,
												value,
												value );
		if( retVal != OK )
		{
			break;
		}

		/* Set bits in the link enhancement register */
		value = 0x86; /* Enable async priority requests, insert idle, and acceleration. */
		retVal = pciConfigModifyLong( fwAdapterData[index].busNo,
												fwAdapterData[index].deviceNo,
												fwAdapterData[index].funcNo,
												0x0F4,
												value,
												value );
	} while(0);

	return retVal;
}

