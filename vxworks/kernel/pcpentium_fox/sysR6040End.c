/* sysR6040End.c - system configuration module for R6040 END */

/* Copyright 1984 - 2001 Wind River Systems, Inc. */

/*
modification history
--------------------
                sysR6040End.c based on sysDec21x40End.c
01h,23apr02,pai  Made DEC and GEI END driver config names consistent with
                 other END driver config names.
01g,22oct01,pai  Updated documentation and routines for new device discovery
                 algorithm (SPR# 35716).
01f,09oct01,pai  Corrected variable use in sysDec21x40EndLoad and
                 conditionally compile PCI_DEV_MMU_MSK and PCI_DEV_ADRS_SIZE.
                 Renamed sysDec21x40LanIntEnable to sysLanIntEnable.  Bound
                 pciIntConnect() to driver-specific interrupt connection.
01e,01oct01,pai  Updated macros to those used in T2.2 (Veloce)
01d,02feb01,rcs  ported to pcPentium
01c,15jul99,jkf  switched intvec and intlvl to match dec21x40End init string
01b,29apr99,jkf  merged with T2.
01a,01apr99,jkf  written 
*/


/*
DESCRIPTION
This is the configuration module for the VxWorks rdc6040End (rdc)
END driver.  It has routines for initializing device resources and provides
BSP-specific routines for RDC 6040 Ethernet PCI bus controllers 
found on the system.

NOTE
This module has only been tested with RDC 6040 PCI Ethernet controller.

*/


#if defined(INCLUDE_RDC_R6040_END)

/* includes */

#include "end.h"
#include <drv/end/rdc6040End.h>


/* defines */

/* specify the maximum number of physical devices to configure */

#define R6040_MAX_DEV       (4)

/* BSP specific RDC 6040 ethernet device type constants */

#define RDC_TYPE_6040       (1)  /* RDC R6040 10/100Base-TX */
#define RDC_PCI_VENDOR_ID   VERSARDC_VENDORID
#define RDC_PCI_DEVICE_ID   VERSARDC_DEVICEID
#define VERSARDC_NAME       "rdc"

/* forward declarations */

LOCAL UINT32 sysRdcDevToType (UINT32, UINT32, UINT8);


/* locals */

LOCAL UINT32 rdcUnitsFound = 0;   /* the number of physical units found */

/*
 * This array defines the board-specific PCI resources.  Each table entry
 * stores this information for specific physical devices found on the system
 * bus.  There is a unique END unit associated with each unique physical
 * device recorded in this table.  The END unit number is equivalent to an
 * index into this table.
 */

LOCAL PCI_BOARD_RESOURCE sysRdcPciRsrcs [R6040_MAX_DEV] =
    {
    {NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
    {NONE, NONE, NONE, NONE, NONE, NONE}, NULL
    },

    {NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
    {NONE, NONE, NONE, NONE, NONE, NONE}, NULL
    },

    {NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
    {NONE, NONE, NONE, NONE, NONE, NONE}, NULL
    },

    {NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
    {NONE, NONE, NONE, NONE, NONE, NONE}, NULL
    }
    };

/* imports */

IMPORT STATUS    sysMmuMapAdd (void * address, UINT len,
                               UINT initialStateMask, UINT initialState);

IMPORT END_OBJ * rdcEndLoad (char *);


/******************************************************************************
*
* sysR6040EndLoad - create a load string and load an rdc6040End device
*
* This routine will be invoked by the MUX for the purpose of loading an
* rdc6040End (rdc) device with initial parameters.  This routine is
* constructed as an interface wrapper for the driver load routine.  Thus,
* the arguments and return values are consistent with any xxxEndLoad()
* routine defined for an END driver and the MUX API.
*
* INTERNAL
* The muxDevLoad() operation calls this routine twice.  A zero length
* <pParamStr> parameter string indicates that this is the first time
* through this routine.  The driver load routine should return the
* driver name in <pParamStr>.
*
* On the second pass though this routine, the initialization parameter
* string is constructed.  Note that on the second pass, the <pParamStr>
* consists of a colon-delimeted END device unit number and rudimentary
* initialization string (often empty) constructed from entries in the
* BSP END Device Table such that:
*
*     <pParamStr> = "<unit>:<default initialization string>"
*
* In the process of building the rest of <pParamStr>, the prepended unit
* number must be preserved and passed to the driver load routine.  The
* <default initialization string> portion mentioned above is discarded,
* but future versions of this routine may use it.
*
* The complete rdcEnd driver load string has format:
*
*     <unit:devMemAddr:devIOAddr:pciMemBase:vecNum:intLvl>
*
* RETURNS: An END object pointer, or NULL on error, or 0 and the name of the
* device if the <pParamStr> was NULL.
*
*/

END_OBJ * sysR6040EndLoad
    (
    char *    pParamStr,   /* pointer to initialization parameter string */
    void *    unused       /* unused optional argument */
    )
    {
    END_OBJ * pEnd;
    char      paramStr [END_INIT_STR_MAX];

    static const char * const paramTemplate =
        "%d:0x%x:0x%x:0x%x:%d:%d:00-04-BF-93-01-D6";

    /* point to RDC board resource table */

    PCI_BOARD_RESOURCE * const pciRsrc = sysRdcPciRsrcs;



    if (strlen (pParamStr) == 0)
        {
        /* PASS (1)
         * The driver load routine returns the driver name in <pParamStr>.
         */

        pEnd = rdcEndLoad (pParamStr);
        }
    else
        {
        /* PASS (2)
         * The END <unit> number is prepended to <pParamStr>.  Construct
         * the rest of the driver load string based on physical devices
         * discovered in sysR6040PciInit().  When this routine is called
         * to process a particular END <unit> number, use the END <unit> as
         * an index into the PCI "resources" table to build the driver
         * parameter string.
         */

        int    typeIdx;  /* an index into the user flags table */

        char * holder  = NULL;
        int    endUnit = atoi (strtok_r (pParamStr, ":", &holder));


        /* is there a PCI resource associated with this END unit ? */

        if (endUnit >= rdcUnitsFound)
            {
            return NULL;
            }


        /* construct an index into the user flags resource table */

        typeIdx = (pciRsrc[endUnit].boardType);
#if 0
        logMsg("IRQ vector %d IRQ Number %d\n",pciRsrc[endUnit].irqvec,
               pciRsrc[endUnit].irq,0,0,0,0);
#endif
        /* finish off the initialization parameter string */

        sprintf (paramStr, paramTemplate, 
                 endUnit,                     /* END unit number */
                 pciRsrc[endUnit].bar[1],     /* memory-mapped IO base */
                 pciRsrc[endUnit].bar[0],     /* IO address space base */
                 PCI2DRAM_BASE_ADRS,          /* host PCI mem. base */
                 pciRsrc[endUnit].irqvec,     /* IRQ vector */
                 pciRsrc[endUnit].irq        /* IRQ number */
                );

        if ((pEnd = rdcEndLoad (paramStr)) == (END_OBJ *) NULL)
            {
            printf ("Error rdcEndLoad:  failed to load driver.\n");
            }
        }

    return (pEnd);
    }

/*******************************************************************************
*
* sysR6040PciInit - initialize a RDC6040 PCI ethernet device
*
* This routine performs basic PCI initialization for RDC6040 ethernet device
* supported by the rdc6040End END driver.  If supported,  the device
* memory and I/O addresses are mapped into the local CPU address space and
* an internal board-specific PCI resources table is updated with
* information on the board type, memory address, and IO address.
*
* CAVEATS
* This routine must be performed prior to MMU initialization, usrMmuInit().
* NOTE: Only one RDC 6040 device instance Tested
*
* RETURNS:
* OK, else ERROR when the specified device is not supported, or if the device
* could not be mapped into processor memory.
*/
STATUS sysR6040PciInit
    (
    UINT32  pciBus,      /* store a PCI bus number */
    UINT32  pciDevice,   /* store a PCI device number */
    UINT32  pciFunc,     /* store a PCI function number */
    UINT32  vendorId,    /* store a PCI vendor ID */
    UINT32  deviceId,    /* store a PCI device ID */
    UINT8   revisionId   /* store a PCI revision ID */
    )
    {
    UINT32  boardType;   /* store a BSP-specific board type constant */

    UINT32  ioBase;      /* IO base address (BAR 0) */
    UINT32  memIo32;     /* memory-mapped IO address (BAR 1) */
    UINT8   irq;         /* interrupt line number (IRQ) for device */


    /* number of physical units exceeded the number supported ? */

    if (rdcUnitsFound >= R6040_MAX_DEV)
        {
        return (ERROR);
        }

    if ((boardType = sysRdcDevToType (vendorId, deviceId, revisionId))
        == BOARD_TYPE_UNKNOWN)
        {
        return (ERROR);
        }


    pciConfigInLong  (pciBus, pciDevice, pciFunc,
                      PCI_CFG_BASE_ADDRESS_0, &ioBase);
    pciConfigInLong  (pciBus, pciDevice, pciFunc,
                      PCI_CFG_BASE_ADDRESS_1, &memIo32);

    memIo32 &= PCI_MEMBASE_MASK;
    ioBase  &= PCI_IOBASE_MASK;

    /* map a 4Kb 32-bit non-prefetchable memory IO address decoder */

    if (sysMmuMapAdd ((void *)(memIo32 & PCI_DEV_MMU_MSK),
        PCI_DEV_ADRS_SIZE, VM_STATE_MASK_FOR_ALL, VM_STATE_FOR_PCI) == ERROR)
        {
        return (ERROR);
        }


    /* read the IRQ number and vector and save to the resource table */

    pciConfigInByte (pciBus, pciDevice, pciFunc,
                     PCI_CFG_DEV_INT_LINE, &irq);

    /* update the board-specific resource table */

    sysRdcPciRsrcs[rdcUnitsFound].bar[0]     = ioBase;
    sysRdcPciRsrcs[rdcUnitsFound].bar[1]     = memIo32;
    sysRdcPciRsrcs[rdcUnitsFound].irq        = irq;
    sysRdcPciRsrcs[rdcUnitsFound].irqvec     = INT_NUM_GET (irq);

    sysRdcPciRsrcs[rdcUnitsFound].vendorID   = vendorId;
    sysRdcPciRsrcs[rdcUnitsFound].deviceID   = deviceId;
    sysRdcPciRsrcs[rdcUnitsFound].revisionID = revisionId;
    sysRdcPciRsrcs[rdcUnitsFound].boardType  = boardType;

    /* enable mapped memory and IO decoders */

    pciConfigOutWord (pciBus, pciDevice, pciFunc, PCI_CFG_COMMAND,
                      PCI_CMD_MEM_ENABLE | PCI_CMD_IO_ENABLE |
                      PCI_CMD_MASTER_ENABLE);

    /* disable sleep mode */

    pciConfigOutByte (pciBus, pciDevice, pciFunc, PCI_CFG_MODE,
                      SLEEP_MODE_DIS);


    /* increment number of units initialized */

    ++rdcUnitsFound;
	
    return (OK);
    }

/*******************************************************************************
*
* sysRdcDevToType - convert PCI Vendor and Device IDs to a device type
*
* Given <vendorId>, <deviceId>, and <revisionId> values read from PCI Vendor
* and Device ID registers in PCI configuration space, this routine will
* attempt to map the IDs to a RDC 6040 device type value defined in this
* file.
*
* CAVEATS
* BOARD_TYPE_UNKNOWN is returned for untested device types.
*
* RETURNS:
* A board type value which will be one of
*
* .IP
* RDC_TYPE_6040
*
* BOARD_TYPE_UNKNOWN will be returned if the Device ID does not map to
* a supported board type.
*
* NOMANUAL
*/
LOCAL UINT32 sysRdcDevToType
    (
    UINT32 vendorId,    /* specifies a PCI Vendor ID value */
    UINT32 deviceId,    /* specifies a PCI Device ID value */
    UINT8  revisionId   /* specifies a PCI Revision ID value */
    )
    {
    /* At the moment, we are only supporting vendor DEC (now Intel) */

    if (vendorId == VERSARDC_VENDORID)
        {
        switch (deviceId)
            {
		
			case VERSARDC_DEVICEID_6040:
				return (RDC_TYPE_6040);
            }
        }
    return (BOARD_TYPE_UNKNOWN);
    }

/*******************************************************************************
*
* sysR6040IntEnable - enable RDC6040 ethernet device interrupts
*
* This routine enables RDC6040 interrupts.  This may involve operations on
* interrupt control hardware.
*
* RETURNS: OK or ERROR for invalid arguments.
*/
STATUS sysR6040IntEnable
    (
    int level           /* level number */
    )
    {
    return (sysIntEnablePIC (level));
    }

/*******************************************************************************
*
* sysR6040IntDisable - disable RDC6040 ethernet device interrupts
*
* This routine disables RDC6040 interrupts.  This may involve operations on
* interrupt control hardware.
*
* RETURNS: OK or ERROR for invalid arguments.
*/
STATUS sysR6040IntDisable
    (
    int level           /* level number */
    )
    {
    return (sysIntDisablePIC (level));
    }

#endif /* defined(INCLUDE_RDC_R6040_END) */
