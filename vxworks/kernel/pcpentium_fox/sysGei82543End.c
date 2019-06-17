/* sysGei82543End.c - Intel PRO1000X/F/XT/XF/MT/MF driver support routine */

/* Copyright 1989-2002 Wind River Systems, Inc.  */

/*
modification history
--------------------
02j,28Jul18,mfr  Add I210 support
01j,07jul02,jln  add 82544GC device ID (spr#79520)
01i,21may02,jln  support 82545/82546 fiber-based adapters (spr 78084)
01h,25apr02,jln  fix short cable errata; support larger EEPROM; add more 
                 BSP specific functions; support 82540/82545/82546 chips; 
                 change the input argument of sys543IntEnable() and 
                 sys543IntDisable() from vector to unit.
01g,23apr02,pai  Made DEC and GEI END driver config names consistent with
                 other END driver config names.
01f,05dec01,jln  save pciBus/pciDevice/pciFunc in pci resource table; 
                 more comments on PCI to PHY address translation
01e,19nov01,pai  Pass the correct CSR memory size to sysMmuMapAdd().
01d,09nov01,pai  Updated documentation and routines for new device discovery
                 algorithm (SPR# 35716).
01c,16Oct01,jln  added sysGei8254xEndLoad; refined device detecting
01b,08Aug01,jln  support jumbo frame and 544_based adapter, more comments
01a,08Jan01,jln  written based on sysNetif.c.
*/

/*
This module is the WRS-supplied configuration module for Intel PRO1000
F/T/XF/XT/MT/MF adapters. It has routines for initializing device resources
and provides BSP-specific gei82543End (gei) END driver routines for
Intel 82540/3/4/5/6 Ethernet PCI bus controllers.

The number of supported devices that can be configured for a particular
system is finite and is specified by the GEI_MAX_DEV configuration
constant.  This value, and the internal data structures using it, can be
modified in this file for specific implementations.

SEE ALSO: ifLib,
.I "RS82543GC GIGABIT ETHERNET CONTROLLER NETWORKING SILICON DEVELOPER'S MANUAL"
*/

#if defined(INCLUDE_GEI8254X_END)

/* namespace collisions */

#undef INTEL_PCI_VENDOR_ID  /* redefined in gei82543End.h (temporary fix) */

/* includes */

#include <end.h>
#include <drv/end/gei82543End.h>

/* defines */

/* specify the maximum number of physical devices to configure */

#define GEI_MAX_DEV      (4)

/* Default RX descriptor  */

#ifndef GEI_RXDES_NUM
#define GEI_RXDES_NUM              (GEI_DEFAULT_RXDES_NUM)
#endif

/* Default TX descriptor  */

#ifndef GEI_TXDES_NUM
#define GEI_TXDES_NUM              (GEI_DEFAULT_TXDES_NUM)
#endif

/* Default User's flags  */

/* ORed the flag of GEI_END_JUMBO_FRAME_SUPPORT if jumbo frame needed */

#ifndef GEI_USR_FLAG
#define GEI_USR_FLAG        (GEI_END_SET_TIMER | \
                             GEI_END_SET_RX_PRIORITY | \
                             GEI_END_FREE_RESOURCE_DELAY \
                            )
#endif /* GEI_USR_FLAG */

/* 
 * If jumbo frame supported, the default 9000 bytes of MTU will be used. 
 * Otherwise 1500 bytes MTU for normal frames. User can set up different 
 * MTU here for jumbo frames. 
 */

#ifndef GEI_JUMBO_MTU_VALUE
#define GEI_JUMBO_MTU_VALUE         (0)  /* Will use default value (9000) */
#endif /* GEI_JUMBO_MTU_VALUE */

/*
 * define the offset value for different arches.
 * For arch like ARM which requires 4-byte alignment for Integer, 
 * use offset of 2
 */

#define GEI_X86_OFFSET_VALUE        (0x0)
#define GEI_ARM_OFFSET_VALUE        (0x2)

#ifndef GEI_OFFSET_VALUE
#define GEI_OFFSET_VALUE            GEI_X86_OFFSET_VALUE 
#endif /* GEI_OFFSET_VALUE */

/*
 * Assuming Little-Endian Arch (i.e. swapping value for the Big-Endian Arch)
 */

#define GEI_SYS_WRITE_REG(unit, reg, value)     \
   ((*(volatile UINT32 *)(PCI_MEMIO2LOCAL(geiResources[(unit)].memBaseLow + reg))) = \
    (UINT32)(value))

#define GEI_SYS_READ_REG(unit, reg)             \
   (*(volatile UINT32 *)(PCI_MEMIO2LOCAL(geiResources[(unit)].memBaseLow + reg)))

/* PCI device ID for Intel 82543/82544 Ethernet */

#define PRO1000_543_PCI_DEVICE_ID_T     (0x1001) /* Copper */
#define PRO1000_543_PCI_DEVICE_ID_FT    (0x1004) /* Fiber / Copper */
#define PRO1000_544_PCI_DEVICE_ID_XT    (0x1008) /* Copper */
#define PRO1000_544_PCI_DEVICE_ID_XF    (0x1009) /* Fiber */
#define PRO1000_544_PCI_DEVICE_ID_GC    (0x100c) /* Copper */
#define PRO1000_540_PCI_DEVICE_ID_XT    (0x100e) /* Copper only */
#define PRO1000_545_PCI_DEVICE_ID_XT    (0x100f) /* Copper */
#define PRO1000_546_PCI_DEVICE_ID_XT    (0x1010) /* Copper */
#define PRO1000_545_PCI_DEVICE_ID_MF    (0x1011) /* Fiber */
#define PRO1000_546_PCI_DEVICE_ID_MF    (0x1012) /* Fiber */
#define PRO1000_541_PCI_DEVICE_ID_XT    (0x1078) /* Copper */
#define PRO1000_546_PCI_DEVICE_ID_XT2   (0x1079) /* Copper - 82546 GB */
#define DEV_ID_82541PI                  (0x1076) /* Copper */

#define PRO1000_573_PCI_DEVICE_ID_E     (0x108B) /* Copper - 82573E */
#define PRO1000_573_PCI_DEVICE_ID_IAMT  (0x108C) /* Copper - 82573E IAMT */
#define PRO1000_571_PCI_DEVICE_ID_EB    (0x105E) /* Copper - 82571EB */
#define PRO1000_574_PCI_DEVICE_ID       (0x10D3) /* Copper - 82574 */
#define INTEL_I210_DEVICE_ID            (0x1533) /* Copper - I210 */

/* device resources */

#define GEI_MEMSIZE_CSR            (0x20000)     /* 128Kb CSR memory size */
#define GEI_MEMSIZE_FLASH          (0x80000)     /* 512Kb Flash memory size */
#define GEI_EEPROM_SZ_64           (64)          /* 64 WORD */   
#define GEI_EEPROM_SZ_256          (256)         /* 256 WORD */   

#define GEI0_SHMEM_BASE             NONE
#define GEI0_SHMEM_SIZE             (0)
#define GEI0_RXDES_NUM              GEI_RXDES_NUM
#define GEI0_TXDES_NUM              GEI_TXDES_NUM
#define GEI0_USR_FLAG               GEI_USR_FLAG

#define GEI1_SHMEM_BASE             NONE
#define GEI1_SHMEM_SIZE             (0)
#define GEI1_RXDES_NUM              GEI_RXDES_NUM
#define GEI1_TXDES_NUM              GEI_TXDES_NUM
#define GEI1_USR_FLAG               GEI_USR_FLAG

#define GEI2_SHMEM_BASE             NONE
#define GEI2_SHMEM_SIZE             (0)
#define GEI2_RXDES_NUM              GEI_RXDES_NUM
#define GEI2_TXDES_NUM              GEI_TXDES_NUM
#define GEI2_USR_FLAG               GEI_USR_FLAG

#define GEI3_SHMEM_BASE             NONE
#define GEI3_SHMEM_SIZE             (0)
#define GEI3_RXDES_NUM              GEI_RXDES_NUM
#define GEI3_TXDES_NUM              GEI_TXDES_NUM
#define GEI3_USR_FLAG               GEI_USR_FLAG

/* INTEL 82544 INTERNAL PHY */

#define INTEL_82544PHY_OUI_ID                   (0x5043)
#define INTEL_82544PHY_MODEL                    (0x3)

#define INTEL_82544PHY_PHY_SPEC_CTRL_REG        (0x10)
#define INTEL_82544PHY_PHY_SPEC_STAT_REG        (0x11)
#define INTEL_82544PHY_INT_ENABLE_REG           (0x12)
#define INTEL_82544PHY_INT_STATUS_REG           (0x13)
#define INTEL_82544PHY_EXT_PHY_SPEC_CTRL_REG    (0x14)
#define INTEL_82544PHY_RX_ERROR_COUNTER         (0x15)
#define INTEL_82544PHY_PHY_GLOBAL_STAT          (0x17)
#define INTEL_82544PHY_LED_CTRL_REG             (0x18)

#define INTEL_82544PHY_PSCR_ASSERT_CRS_ON_TX    (0x0800)
#define INTEL_82544PHY_EPSCR_TX_CLK_25          (0x0070)

/* Alaska PHY's information */

#define MARVELL_OUI_ID                  (0x5043)
#define MARVELL_ALASKA_88E1000          (0x5)
#define MARVELL_ALASKA_88E1000S         (0x4)
#define ALASKA_PHY_SPEC_CTRL_REG        (0x10)
#define ALASKA_PHY_SPEC_STAT_REG        (0x11)
#define ALASKA_INT_ENABLE_REG           (0x12)
#define ALASKA_INT_STATUS_REG           (0x13)
#define ALASKA_EXT_PHY_SPEC_CTRL_REG    (0x14)
#define ALASKA_RX_ERROR_COUNTER         (0x15)
#define ALASKA_LED_CTRL_REG             (0x18)

#define ALASKA_PSCR_ASSERT_CRS_ON_TX    (0x0800)
#define ALASKA_EPSCR_TX_CLK_25          (0x0070)

#define ALASKA_PSCR_AUTO_X_1000T        (0x0040)
#define ALASKA_PSCR_AUTO_X_MODE         (0x0060)

#define ALASKA_PSSR_DPLX                (0x2000)
#define ALASKA_PSSR_SPEED               (0xC000)
#define ALASKA_PSSR_10MBS               (0x0000)
#define ALASKA_PSSR_100MBS              (0x4000)
#define ALASKA_PSSR_1000MBS             (0x8000)

/************************************************************/
/* EEPROM defines/macros ported from vxWorks 6.9 GEI driver */
/************************************************************/

#define GEI_EECD    0x0010    /* EEPROM/flash control */
#define GEI_EERD    0x0014    /* EEPROM/flash data (!82544) */

/* EEPROM read register (not valid on 82544 or earlier) */

#define GEI_EERD_START      0x00000001
#define GEI_EERD_DONE       0x00000010
#define GEI_EERD_ADDR       0x0000FF00
#define GEI_EERD_DATA       0xFFFF0000
#define GEI_EEADDR(x)           (((x) << 8) & GEI_EERD_ADDR)
#define GEI_EEDATA(x)           (((x) & GEI_EERD_DATA) >> 16)
#define GEI_TIMEOUT              10000
#define le16toh(x)      ((UINT16)(x))

/* 9346 EEPROM commands */

#define GEI_9346_WRITE      0x5
#define GEI_9346_READ       0x6
#define GEI_9346_ERASE      0x7

#define GEI_EECD    0x0010   /* EEPROM/flash control */

/* EEPROM/flash control register (must be used on 82544 or earlier) */

#define GEI_EECD_SK         0x00000001 /* EEPROM clock */
#define GEI_EECD_CS         0x00000002 /* EEPROM chip select */
#define GEI_EECD_DI         0x00000004 /* EEPROM data in */
#define GEI_EECD_DO         0x00000008 /* EEPROM data out */
#define GEI_EECD_EE_REQ     0x00000040 /* Request access (!82544) */
#define GEI_EECD_EE_GNT     0x00000080 /* Grant access (!82544) */

#define CSR_SETBIT_4(unit, offset, val)          \
        GEI_SYS_WRITE_REG(unit, offset, GEI_SYS_READ_REG(unit, offset) | (val))

#define CSR_CLRBIT_4(unit, offset, val)          \
        GEI_SYS_WRITE_REG(unit, offset, GEI_SYS_READ_REG(unit, offset) & (UINT32)(~(val)))

/* typedefs */

typedef struct geiResource        /* GEI_RESOURCE */
    {
    BOOL   adr64;                 /* Indicator for 64-bit support */
    UINT32 boardType;             /* board type 82541 */
    UINT32 memBaseLow;            /* Base Address LOW */
    UINT32 memBaseHigh;           /* Base Address HIGH */
    UINT32 flashBase;             /* Base Address for FLASH */

    UINT16 eepromSize;            /* size in unit of word (16 bit) - 64/256 */ 
    UINT16 eeprom_address_bits;   /* eeprom address_bits  - 82541 */
    UINT16 eeprom_icw1;           /* EEPROM initialization control word 1 */
    UINT16 eeprom_icw2;           /* EEPROM initialization control word 2 */
    UCHAR  enetAddr[6];           /* MAC address for this adaptor */

    UINT32 shMemBase;             /* Share memory address if any */
    UINT32 shMemSize;             /* Share memory size if any */

    UINT32 rxDesNum;              /* RX descriptor for this unit */
    UINT32 txDesNum;              /* TX descriptor for this unit */

    BOOL   useShortCable;         /* TRUE if short cable used for 82544 */
                                     /* by default is FALSE */
    UINT32 usrFlags;              /* user flags for this unit */
    STATUS iniStatus;             /* initialization perform status */

    } GEI_RESOURCE;

/* locals */

LOCAL UINT32 geiUnits = 0;     /* number of GEIs we found */

/* This table defined board extended resources */

LOCAL GEI_RESOURCE geiResources [GEI_MAX_DEV] =
    {
    {FALSE, NONE, NONE, NONE, NONE, GEI_EEPROM_SZ_64, 6, 0, 0, {NONE},
     GEI0_SHMEM_BASE, GEI0_SHMEM_SIZE, GEI0_RXDES_NUM, GEI0_TXDES_NUM, FALSE, 
     GEI0_USR_FLAG, ERROR
    },

    {FALSE, NONE, NONE, NONE, NONE, GEI_EEPROM_SZ_64, 6, 0, 0, {NONE},
     GEI1_SHMEM_BASE, GEI1_SHMEM_SIZE, GEI1_RXDES_NUM, GEI1_TXDES_NUM, FALSE, 
     GEI1_USR_FLAG, ERROR
    },

    {FALSE, NONE, NONE, NONE, NONE, GEI_EEPROM_SZ_64, 6, 0, 0, {NONE},
     GEI2_SHMEM_BASE, GEI2_SHMEM_SIZE, GEI2_RXDES_NUM, GEI2_TXDES_NUM, FALSE, 
     GEI2_USR_FLAG, ERROR
    },

    {FALSE, NONE, NONE, NONE, NONE, GEI_EEPROM_SZ_64, 6, 0, 0, {NONE},
     GEI3_SHMEM_BASE, GEI3_SHMEM_SIZE, GEI3_RXDES_NUM, GEI3_TXDES_NUM, FALSE, 
     GEI3_USR_FLAG, ERROR
    }
    };

/* This table defines board PCI resources */

LOCAL PCI_BOARD_RESOURCE geiPciResources [GEI_MAX_DEV] =
    {
    {NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
    {NONE, NONE, NONE, NONE, NONE, NONE},
     (void * const)(&geiResources[0])
    },

    {NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
    {NONE, NONE, NONE, NONE, NONE, NONE},
     (void * const)(&geiResources[1])
    },

    {NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
    {NONE, NONE, NONE, NONE, NONE, NONE},
     (void * const)(&geiResources[2])
    },

    {NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
    {NONE, NONE, NONE, NONE, NONE, NONE},
     (void * const)(&geiResources[3])
    }
    };

    /*externs*/
IMPORT void     sysUsDelay (int);
IMPORT void     sysMsDelay (int);

/* forward declarations */

LOCAL int       sys543IntEnable  (int unit);
LOCAL int       sys543IntDisable (int unit);
LOCAL int       sys543IntAck     (int unit);
LOCAL STATUS    sys543eepromCheckSum (int unit);
LOCAL UINT16    sys543eepromReadWord (int unit,UINT32);
LOCAL STATUS    sys543EtherAdrGet (int unit);
LOCAL void      sys544PhyPreInit (PHY_INFO *);
LOCAL void      sys543PhySpecRegsInit(PHY_INFO *, UINT8);
LOCAL BOOL      sysGei82546InitTimerSetup (ADAPTOR_INFO * ); 
LOCAL BOOL      sysGei82546DynaTimerSetup (ADAPTOR_INFO * ); 
LOCAL UINT32    sysGeiDevToType (UINT32, UINT32, UINT8);

LOCAL void      sysGei541ReadEeprom(int unit,int off, int cnt,UINT8 *dest);
LOCAL int       sysGeiTaskDelay(int millisecs);

/*****************************************************************************
*
* sys543PciInit - initialize a GEI 8254x PCI ethernet device
*
* This routine performs basic PCI initialization for 8254x ethernet
* devices supported by the gei82543End END driver.  Parameters to this
* routine specify a PCI function, including PCI ID registers, to
* initialize.  If supported,  the device memory and I/O addresses are
* mapped into the local CPU address space and an internal board-specific
* PCI resources table is updated with information on the board type,
* memory address, and IO address.
*
* CAVEATS
* This routine must be performed prior to MMU initialization, usrMmuInit().
* If the number of supported 8254x physical device instances installed
* on the PCI bus exceeds GEI_MAX_DEV, then the extra devices will not be
* initialized in this routine.
*
* RETURNS:
* OK, or ERROR if the specified device is not supported, or if
* the device could not be mapped into the local CPU memory space.
*/
STATUS sys543PciInit
    (
    UINT32         pciBus,      /* store a PCI bus number */
    UINT32         pciDevice,   /* store a PCI device number */
    UINT32         pciFunc,     /* store a PCI function number */
    UINT32         vendorId,    /* store a PCI vendor ID */
    UINT32         deviceId,    /* store a PCI device ID */
    UINT8          revisionId   /* store a PCI revision ID */
    )
    {
    UINT32         boardType;   /* store a BSP-specific board type constant */

    UINT32         memBaseLo;   /* temporary BAR storage */
    UINT32         memBaseHi;
    UINT32         flashBase;
    UINT8          irq;         /* store PCI interrupt line (IRQ) number */

    GEI_RESOURCE * pReso;       /* alias extended resource table */

    /* number of physical units exceeded the number supported ? */

    if (geiUnits >= GEI_MAX_DEV)
        {
        return (ERROR);
        }

    if ((boardType = sysGeiDevToType (vendorId, deviceId, revisionId))
        == BOARD_TYPE_UNKNOWN)
        {
        return (ERROR);
        }

    /* BAR information will be saved in the extended resource table */

    pReso = (GEI_RESOURCE *)(geiPciResources[geiUnits].pExtended);

    /*
     * BAR0: [32:17]: memory base
     *       [16:4] : read as "0";
     *       [3]    : 0 - device is not prefetchable
     *       [2:1]  : 00b - 32-bit address space, or
     *                01b - 64-bit address space
     *       [0]    : 0 - memory map decoded
     *
     * BAR1: if BAR0[2:1] == 00b, optional flash memory base
     *       if BAR0[2:1] == 01b, high portion of memory base 
     *                            for 64-bit address space
     *
     * BAR2: if BAR0[2:1] == 01b, optional flash memory base
     *       if BAR0[2:1] == 00b, behaves as BAR-1 when BAR-0 is
     *                            a 32-bit value
     */

    pciConfigInLong  (pciBus, pciDevice, pciFunc,
                      PCI_CFG_BASE_ADDRESS_0, &memBaseLo);

    pReso->adr64 = ((memBaseLo & BAR0_64_BIT) == BAR0_64_BIT)
                   ? TRUE : FALSE;

    if (pReso->adr64)
        {
        pciConfigInLong  (pciBus, pciDevice, pciFunc,
                          PCI_CFG_BASE_ADDRESS_1, &memBaseHi);

        pciConfigInLong  (pciBus, pciDevice, pciFunc,
                          PCI_CFG_BASE_ADDRESS_2, &flashBase);
        }
    else
        {
        memBaseHi = 0x0;

        pciConfigInLong  (pciBus, pciDevice, pciFunc,
                          PCI_CFG_BASE_ADDRESS_1, &flashBase);
        }

    memBaseLo &= PCI_MEMBASE_MASK;
    flashBase &= PCI_MEMBASE_MASK;

    /* map the memory-mapped IO (CSR) space into host CPU address space */

    if (sysMmuMapAdd ((void *)(PCI_MEMIO2LOCAL(memBaseLo)), GEI_MEMSIZE_CSR,
        VM_STATE_MASK_FOR_ALL, VM_STATE_FOR_PCI) == ERROR)
        {
        return (ERROR);
        }

    /* get the device's interrupt line (IRQ) number */

    pciConfigInByte (pciBus, pciDevice, pciFunc,
                     PCI_CFG_DEV_INT_LINE, &irq);

    /* update the board-specific resource tables */

    pReso->memBaseLow  = memBaseLo;
    pReso->memBaseHigh = memBaseHi;
    pReso->flashBase   = flashBase;

    geiPciResources[geiUnits].irq        = irq;
    geiPciResources[geiUnits].irqvec     = INT_NUM_GET (irq);

    geiPciResources[geiUnits].vendorID   = vendorId;
    geiPciResources[geiUnits].deviceID   = deviceId;
    geiPciResources[geiUnits].revisionID = revisionId;
    geiPciResources[geiUnits].boardType  = boardType;

    if(boardType == DEV_ID_82541PI)
        { 
        geiPciResources[geiUnits].boardType = DEV_ID_82541PI;
        geiResources[geiUnits].eeprom_address_bits = 6;
        geiResources[geiUnits].boardType = DEV_ID_82541PI;
        }
   /* the following support legacy interfaces and data structures */

    geiPciResources[geiUnits].pciBus     = pciBus;
    geiPciResources[geiUnits].pciDevice  = pciDevice;
    geiPciResources[geiUnits].pciFunc    = pciFunc;

    /* enable mapped memory and IO decoders */

    pciConfigOutWord (pciBus, pciDevice, pciFunc, PCI_CFG_COMMAND,
                      PCI_CMD_MEM_ENABLE | PCI_CMD_IO_ENABLE |
                      PCI_CMD_MASTER_ENABLE);

    /* disable sleep mode */

    pciConfigOutByte (pciBus, pciDevice, pciFunc, PCI_CFG_MODE,
                      SLEEP_MODE_DIS);

    ++geiUnits;  /* increment number of units initialized */

    return (OK);
    }

/******************************************************************************
*
* sysGei8254xEndLoad - create load string and load a gei driver.
*
* This routine will be invoked by the MUX for the purpose of loading an
* gei82543End (gei) device with initial parameters.  This routine is
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
* consists of a colon-delimited END device unit number and rudimentary
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
* The complete gei82543End driver load string has format:
*
*     <unit>:<shMemBase>:<shMemSize>:<rxDesNum>:<txDesNum>:<usrFlags>:
*     <offset>:<mtu>
*
* RETURNS: An END object pointer, or NULL on error, or 0 and the name of the
* device if the <pParamStr> was NULL.
*
* SEE ALSO: gei82543EndLoad()
*/
END_OBJ * sysGei8254xEndLoad
    (
    char *    pParamStr,   /* ptr to initialization parameter string */
    void *    unused       /* unused optional argument */
    )
    {
    END_OBJ * pEnd;
    char      paramStr [END_INIT_STR_MAX];

    if (strlen (pParamStr) == 0)
        {
        /* PASS (1)
         * The driver load routine returns the driver name in <pParamStr>.
         */

        pEnd = gei82543EndLoad (pParamStr);
        }
    else
        {
        /* PASS (2)
         * The END <unit> number is prepended to <pParamStr>.  Construct
         * the rest of the driver load string based on physical devices
         * discovered in sys543PciInit().  When this routine is called
         * to process a particular END <unit> number, use the END <unit> as
         * an index into the PCI "resources" table to build the driver
         * parameter string.
         */

        GEI_RESOURCE *  pReso;

        char * holder = NULL;
        int    unit   = atoi (strtok_r (pParamStr, ":", &holder));

        /* is there a PCI resource associated with this END unit ? */

        if (unit >= geiUnits)
            {
            return NULL;
            }

        pReso = (GEI_RESOURCE *)(geiPciResources[unit].pExtended);

        /* finish off the initialization parameter string */

        sprintf (paramStr,"%d:0x%x:0x%x:0x%x:0x%x:0x%x:%d:%d",
                 unit,                          /* END unit number */
                 pReso->shMemBase,              /* share memory base */
                 pReso->shMemSize,              /* share memory size */
                 pReso->rxDesNum,               /* RX Descriptor Number*/
                 pReso->txDesNum,               /* TX Descriptor Number*/
                 pReso->usrFlags,               /* user's flags */
                 GEI_OFFSET_VALUE,              /* offset value */
                 GEI_JUMBO_MTU_VALUE            /* mtu value */
                );

        if ((pEnd = gei82543EndLoad (paramStr)) == (END_OBJ *)NULL)
            {
            printf ("ERROR: sysGei8254xEndLoad fails to load gei %d\n", unit);
            }
        }

    return (pEnd);
    }

/*****************************************************************************
*
* sys82543BoardInit - Adapter initialization for 8254x chip
*
* This routine is expected to perform any adapter-specific or target-specific
* initialization that must be done prior to initializing the 8254x chip.
*
* The 82543 driver calls this routine from the driver load routine before
* any other routines.
*
* RETURNS: OK or ERROR
*/
STATUS sys82543BoardInit
    (
    int            unit,      /* unit number */
    ADAPTOR_INFO * pBoard     /* board information for the GEI driver */
    )
    {
    PCI_BOARD_RESOURCE * pRsrc;
    GEI_RESOURCE *       pReso;
    BOOL                 lanB = FALSE;

    /* sanity check */

    if (unit >= geiUnits)
        return (ERROR);

    pRsrc = &geiPciResources[unit];
    pReso = (GEI_RESOURCE *)(pRsrc->pExtended);

    if (pRsrc->boardType != PRO1000_543_BOARD && 
        pRsrc->boardType != PRO1000_544_BOARD &&
        pRsrc->boardType != PRO1000_546_BOARD &&
        pRsrc->boardType != PRO1000_573_BOARD &&
        pRsrc->boardType != PRO1000_I210_BOARD && 
        pRsrc->boardType != DEV_ID_82541PI) 
         return ERROR;
        
    if (pRsrc->boardType == PRO1000_546_BOARD)
        {
        UINT32 eecd;
        UINT16 devId;        

        if (!((eecd = GEI_SYS_READ_REG(unit, INTEL_82543GC_EECD)) & EECD_PRES_BIT))
            {
            printf ("ERROR: gei unit %d eeprom not presented\n", unit);
            return ERROR;
            }

        if(pReso->boardType != DEV_ID_82541PI)
            {
            pReso->eepromSize = (eecd & EECD_SIZE_BIT)? 256 : 64;
            }

        /* detect if this is one of 82546EB dual ports */
       
        pciConfigInWord (pRsrc->pciBus, pRsrc->pciDevice, pRsrc->pciFunc,
                          PCI_CFG_DEVICE_ID, &devId);
        
        if (devId == PRO1000_546_PCI_DEVICE_ID_XT ||
            devId == PRO1000_546_PCI_DEVICE_ID_MF) 
            {
            UINT8 headerType;

            pciConfigInByte (pRsrc->pciBus, pRsrc->pciDevice, pRsrc->pciFunc,
                             PCI_CFG_HEADER_TYPE, &headerType);

            if (headerType == 0x80)
                {
                lanB = (pRsrc->pciFunc == 1)? TRUE : FALSE;
                }
            else if (pRsrc->pciFunc != 0)
                {
                printf ("Error in detecting 82546 dual port: header type =%2d, "
                         "pci func=%2d\n", (UINT32)headerType, (UINT32)(pRsrc->pciFunc));
                }
            }
        }

    /* perform EEPROM checksum */

    if (sys543eepromCheckSum (unit) != OK)
        {
        printf ("ERROR: gei unit=%d, EEPROM checksum error!\n", unit);
        }

    /* get the Ethernet Address from eeprom */

    if (sys543EtherAdrGet (unit) == OK)
        {
        if (pRsrc->boardType == PRO1000_546_BOARD && lanB == TRUE)
           {
           int   ix;

       /* update LANB address */

           for (ix = 5; ix >= 0; ix--)
               {
               if (pReso->enetAddr[ix] != 0xff)
                    {
                    pReso->enetAddr[ix]++;
                    break;
                    }
                else
                    pReso->enetAddr[ix] = 0;
                }
           }
        }
    else
         printf ("ERROR: gei unit=%d, Invalid Ethernet Address!\n", unit);
 
    if(pReso->boardType == DEV_ID_82541PI)
        {
        /* get the initialization control word 1 (ICW1) in EEPROM */
        sysGei541ReadEeprom(unit, EEPROM_ICW1, 1, 
                    (UINT8 *)&(geiResources[unit].eeprom_icw1));

        /* get the initialization control word 2 (ICW2) in EEPROM */
        sysGei541ReadEeprom(unit, EEPROM_ICW2, 1,
                    (UINT8 *)&(geiResources[unit].eeprom_icw2));

        pReso->eeprom_icw1 = geiResources[unit].eeprom_icw1;
        pReso->eeprom_icw2 = geiResources[unit].eeprom_icw2;
        }
    else
        {
        /* get the initialization control word 1 (ICW1) in EEPROM */

        pReso->eeprom_icw1 = sys543eepromReadWord (unit, EEPROM_ICW1);

        /* get the initialization control word 2 (ICW2) in EEPROM */

        pReso->eeprom_icw2 = sys543eepromReadWord (unit, EEPROM_ICW2);
        }
    /* initializes the board information structure */

    pBoard->boardType   = pRsrc->boardType;
    pBoard->vector      = INT_NUM_GET((pRsrc->irq));

    pBoard->regBaseHigh = pReso->memBaseHigh;
    pBoard->regBaseLow  = PCI_MEMIO2LOCAL(pReso->memBaseLow);
    pBoard->flashBase   = PCI_MEMIO2LOCAL(pReso->flashBase);
    pBoard->adr64       = pReso->adr64;

    pBoard->intEnable   = sys543IntEnable;
    pBoard->intDisable  = sys543IntDisable;
    pBoard->intAck      = sys543IntAck;
  
    /* Intel Copper-based adapter is based on GMII interface */    

    pBoard->phyType     = GEI_PHY_GMII_TYPE;  

    if (pBoard->boardType == PRO1000_544_BOARD && 
                             geiResources[unit].useShortCable)   
        {
        miiPhyOptFuncSet ((FUNCPTR)sys544PhyPreInit);
        }

    pBoard->phySpecInit = sys543PhySpecRegsInit;

    /* BSP specific  
     * delayFunc is BSP specific. We prefer a higher time resolution delay 
     * delayUnit is the time of ns elapsed when calling delayFunc ().
     */

    pBoard->delayFunc     = (FUNCPTR) sysDelay;
    pBoard->delayUnit     = 720; /* In x86, sysDelay() takes about 720ns */

    /* BSP specific
     * phyDelayRtn is used as a delay function for PHY detection, if not set,
     * taskDelay will be used.
     *
     * For Fox, we know the 2nd GEI device (APC board) will be initially un-powered,
     * so reduce the timeout to improve END initialization during kernel initialization.
     * Moreover, when taskDelay() is set as the delay routine, the MII lib will enforce
     * a 5 second delay, with a resolution of 1 second.
     */
    pBoard->phyDelayRtn  = sysGeiTaskDelay;
    pBoard->phyDelayParm = 3; /* 3 ticks => 50 msecs */
    pBoard->phyMaxDelay  = (60*4) / pBoard->phyDelayParm;   /* => 4 seconds */

    /* BSP/adapter specific
     * set the PHY address if you know it, otherwise set to zero
     * INTEL 82540/4/5/6-based adapters have a built-in phy with Addr of 1 
     */

    pBoard->phyAddr = (pRsrc->boardType == PRO1000_544_BOARD || 
                       pRsrc->boardType == PRO1000_546_BOARD || 
                       pRsrc->boardType == PRO1000_573_BOARD ||
                       pRsrc->boardType == PRO1000_I210_BOARD)? 1 : 0;

    if (pReso->boardType == DEV_ID_82541PI)
        {
        pBoard->phyAddr = 1;
        }
      
    /* BSP/adapter specific (for 82540/82545/82546 only)
     * allow users set up the device's internal timer based on their
     * application. sysGeiInitTimerSet() is called when the device
     * starts; sysGeiDynaTimerSet() is called every 2s in tNetTask if
     * GEI_END_SET_TIMER is set. 
     */

    if (pRsrc->boardType == PRO1000_546_BOARD)
        {
        pBoard->sysGeiDynaTimerSetup = sysGei82546DynaTimerSetup;
        pBoard->sysGeiInitTimerSetup = sysGei82546InitTimerSetup; 
        }
    else
        {
        pBoard->sysGeiDynaTimerSetup   = NULL;         /* default */
        pBoard->sysGeiInitTimerSetup   = NULL;         /* default */
        }

    /* BSP specific 
     * call back functions perform system physical memory mapping in the PCI 
     * address space. sysLocalToBus converts a system physical memory address 
     * into the pci address space. sysBusToLocal converts a pci address which 
     * actually reflects a system physical memory back to the system memory 
     * address. The sysBusToLocal here in this driver is NOT used for mapping 
     * PCI device's memory (e.g. PCI device's control/status registers) 
     * to the host address space.  
     */  

    pBoard->sysLocalToBus = NULL;     /* for 1:1 mapping */ 
    pBoard->sysBusToLocal = NULL;     /* for 1:1 mapping */ 

    /* specify the interrupt connect/disconnect routines to be used */

    pBoard->intConnect    = (FUNCPTR) pciIntConnect;
    pBoard->intDisConnect = (FUNCPTR) pciIntDisconnect;

    /* get the ICW1 and ICW2 stored in EEPROM */

    /*for 82541PI*/
    if(pReso->boardType == DEV_ID_82541PI)
        {
        pBoard->eeprom_icw1  = geiResources[unit].eeprom_icw1;
        pBoard->eeprom_icw2  = geiResources[unit].eeprom_icw2;
        }
    else
        {
        pBoard->eeprom_icw1  = pReso->eeprom_icw1;
        pBoard->eeprom_icw2  = pReso->eeprom_icw2;
        }

    /* copy Ether address */

    memcpy (&pBoard->enetAddr[0], &(pReso->enetAddr[0]), 
            ETHER_ADDRESS_SIZE);

    /* we finish adapter initialization */

    pReso->iniStatus = OK;

    return (OK);
    }

/*************************************************************************
*
* sysGei82546DynaTimerSetup - setup device internal timer value dynamically
* 
* This routine will be called every 2 seconds by default if GEI_END_SET_TIMER
* flag is set. The available timers to adjust include RDTR(unit of ns), 
* RADV(unit of us), and ITR(unit of 256ns). Based on CPU's and/or tasks' 
* usage on system, user can tune the device's performance dynamically. 
* This routine would be called in the tNetTask context, and is only 
* available for 82540/82545/82546 MACs. Any timer value greater than
* 0xffff won't be used to change corresponding timer register.
*
* RETURNS: TRUE if timer value should change, or FALSE 
*/

LOCAL BOOL sysGei82546DynaTimerSetup 
    (
    ADAPTOR_INFO * pBoard     /* board information for the GEI driver */
    )
    {
    /* user's specific code to decide what value should be used.
     * For example, depending on 
     * 1: CPU usage on system and/or,
     * 2: specific application task's usage and/or,
     * 3: RX/TX packet processing per second, and/or 
     * 4: RX/TX interrupt counter per second, and/or
     * 5: RX packet processing for each calling gei82543RxTxIntHandle(),
     * users can choose optimal timer values from a predefined table to 
     * reduce interrupt rates. The statistic of 3,4,and 5 items above 
     * may be obtained from pBoard->devDrvStat. 
     *
     * NOTE:
     * ITR:  Interrupt throttling register (unit of 256ns) 
     *       inter-interrupt delay between chip's interrupts   
     *
     * RADV: receive interrupt absolute delay timer register (unit of 1.024us) 
     *       a RX interrupt will absolutely occur at this defined value 
     *       after the first packet is received.  
     *
     * RDTR: receive delay timer register (unit of 1.024us)
     *       a RX interrupt will occur if device has not received a subsequent
     *       packet within this defined value.
     */
    
    /* value > 0xffff would not be used for corresponding timer registers */

    /* pBoard->devTimerUpdate.rdtrVal = 0xffffffff; /@ unit of 1.024ns */
    /* pBoard->devTimerUpdate.radvVal = 0xffffffff; /@ unit of 1.024us */
    /* pBoard->devTimerUpdate.itrVal  = 0xffffffff; /@ unit of 256 ns */
    /* pBoard->devTimerUpdate.watchDogIntVal = 2;   /@ 2 second default */
  
    /* return TRUE; */

    return FALSE;
    }

/*************************************************************************
*
* sysGei82546InitTimerSetup - initially setup device internal timer value
* 
* The device's internal timers include RDTR(unit of ns), 
* RADV(unit of us), and ITR(unit of 256ns). The function is 
* called before device starts up, and it is only available for 
* 82540/82545/82546 MACs. Any timer value greater than 0xffff will 
* be discarded.
*
* RETURNS: TRUE if timer value should change, or FALSE 
*/

LOCAL BOOL sysGei82546InitTimerSetup 
    (
    ADAPTOR_INFO * pBoard     /* board information for the GEI driver */
    ) 
    {
    /* value > 0xffff would not change corresponding timer registers */

    /* pBoard->devTimerUpdate.rdtrVal = 0xffffffff; /@ unit of 1.024ns */
    /* pBoard->devTimerUpdate.radvVal = 0xffffffff; /@ unit of 1.024us */
    /* pBoard->devTimerUpdate.itrVal  = 0xffffffff; /@ unit of 256 ns  */
    /* pBoard->devTimerUpdate.watchDogIntVal = 2;   /@ 2 second default */

    /* return TRUE; */

    return FALSE;
    }

/*************************************************************************
*
* sys543eepromReadBits - read bits from EEPROM
*
* This routine reads bit data from EEPROM
*
* RETURNS: value in WORD size
*/
LOCAL UINT16 sys543eepromReadBits
    (
    int      unit,
    int      bitsNum
    )
    {
    int      count;
    UINT32   ix;
    UINT16   val = 0;
    UINT16   reqBit = 0;
    PCI_BOARD_RESOURCE * pRsrc;

    pRsrc = &geiPciResources[unit];

    if (pRsrc->boardType == PRO1000_546_BOARD)
        reqBit = EECD_REQ_BIT;        

    for (ix = 0; ix < bitsNum; ix++)
        {
        /* raise the clk */ 

        GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, 
                                (EECD_CS_BIT | EECD_SK_BIT | reqBit));
        /* wait 2000ns */

        for (count = 0; count < 3; count++)
             sysDelay ();

        val = ( val << 1) | 
          ((GEI_SYS_READ_REG(unit, INTEL_82543GC_EECD) & EECD_DO_BIT)? 1 : 0);

        /* lower the clk */

        GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, (EECD_CS_BIT | reqBit));

        /* wait 2000 ns */

        for (count = 0; count < 3; count++)
             sysDelay ();   
        }      

    return (val);
    }

/*************************************************************************
*
* sys543eepromWriteBits - write bits out to EEPROM
*
* This routine writes bits out to EEPROM
*
* RETURNS: N/A
*/
LOCAL void sys543eepromWriteBits
    (
    int          unit,
    UINT16       value,
    UINT16       bitNum
    )
    {
    int             count;
    volatile UINT16 data;
    UINT16          reqBit = 0;
    PCI_BOARD_RESOURCE * pRsrc;

    pRsrc = &geiPciResources[unit];

    if (pRsrc->boardType == PRO1000_546_BOARD)
        reqBit = EECD_REQ_BIT;        

    if (bitNum == 0)
        return;

    while (bitNum--)
        {
        data = (value & (0x1 << bitNum )) ? EECD_DI_BIT : 0;

        data |=  EECD_CS_BIT;

        /* write the data */

        GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, (data | reqBit));
    
        /* wait 1000ns */

        for (count = 0; count < 2; count++)
            sysDelay ();    

        /* raise the clk */ 

        GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, (data | EECD_SK_BIT | reqBit));

        /* wait 1000ns */

        for (count = 0; count < 2; count++)
            sysDelay ();

        /* lower the clk */

        GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, (data | reqBit));

        /* wait 1000ns */

        for (count = 0; count < 2; count++)
            sysDelay ();   
        }
    }

/*
 * For the 82573, forgo the bitbang access methods and just
 * get the controller to read the EEPROM for us.
 */

LOCAL UINT16 sys573eepromReadWord
    (
    int     unit,
    UINT32  index
    )
    {
    int     count;
    UINT16  val;
    UINT32  tmp;
    PCI_BOARD_RESOURCE * pRsrc;

    pRsrc = &geiPciResources[unit];

    tmp = EERD_START_BIT | (index << 2);

    GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EERD, tmp);

    for (count = 0; count < 10000; count++)
        {
        sysDelay();
        if (GEI_SYS_READ_REG(unit, INTEL_82543GC_EERD) & EERD_573_DONE_BIT)
            break;
        }

    if (count == 10000)
        {
        printf("gei%d: EEPROM read timed out\n", unit);
        return (0);
        }

    val = (GEI_SYS_READ_REG(unit, INTEL_82543GC_EERD) >> 16) & 0xFFFF;

    return (val);
    }

/*************************************************************************
*
* sys543eepromReadWord - Read a word from EEPROM
*
* RETURNS: value in WORD size
*/
LOCAL UINT16 sys543eepromReadWord
    (
    int     unit,
    UINT32  index
    )
    {
    int     count;
    UINT16  val;
    UINT32  tmp;
    PCI_BOARD_RESOURCE * pRsrc;

    pRsrc = &geiPciResources[unit];

    if (pRsrc->boardType == PRO1000_573_BOARD || pRsrc->boardType == PRO1000_I210_BOARD)
        return (sys573eepromReadWord (unit, index));

    if (pRsrc->boardType == PRO1000_546_BOARD)
      {
      int  ix = 0; 
      BOOL accessGet = FALSE;

      tmp = GEI_SYS_READ_REG(unit, INTEL_82543GC_EECD);
      tmp |= EECD_REQ_BIT;     /* request EEPROM access */

      GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, tmp);

      do {
         /* wait 2us */
         for (count = 0; count < 3; count++)
              sysDelay ();

         if ((tmp = GEI_SYS_READ_REG(unit, INTEL_82543GC_EECD)) & EECD_GNT_BIT)
         {
             accessGet = TRUE;
             break;
             }
         } while (ix++ < 500000);

         if (!accessGet) 
         {
             /* timeout in a second */
             printf ("ERROR: timeout to grant access to gei unit %d EEPROM\n", unit);
             return 0;
         }
      }

    if (index >= geiResources[(unit)].eepromSize)
        {
        printf ("ERROR: gei unit %d Invalid index %d to EEPROM\n", unit, index);
        return 0;
        }

    tmp = GEI_SYS_READ_REG(unit, INTEL_82543GC_EECD);

    GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, EECD_CS_BIT);

    /* wait 1000ns */

    for (count = 0; count < 2; count++)
         sysDelay ();

    /* write the opcode out */

    sys543eepromWriteBits (unit, EEPROM_READ_OPCODE, EEPROM_CMD_BITS);

    /* write the index out */
    if (geiResources[(unit)].eepromSize == 64)
        sys543eepromWriteBits (unit, index, 6);

    else if (geiResources[(unit)].eepromSize == 256)
        sys543eepromWriteBits (unit, index, 8);
 
    else /* unsupported, but still try 64 words */ 
       {
        sys543eepromWriteBits (unit, index, 6);
        printf ("ERROR: gei unit %d unsupported EEPROM size\n", unit);
       }

    GEI_SYS_READ_REG(unit, INTEL_82543GC_EECD);
    
    /* read the data */

    val = sys543eepromReadBits (unit, EEPROM_DATA_BITS);

    /* clean up access to EEPROM */      
    if (pRsrc->boardType == PRO1000_546_BOARD)
       tmp &= ~(EECD_DI_BIT | EECD_DO_BIT | EECD_CS_BIT | EECD_REQ_BIT);
    else
       tmp &= ~(EECD_DI_BIT | EECD_DO_BIT | EECD_CS_BIT);

    GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, tmp);

    return val;
    }

/*************************************************************************
*
* sys543EtherAdrGet - Get Ethernet address from EEPROM
*
* This routine get an Ethernet address from EEPROM
*
* RETURNS: OK or ERROR
*/
LOCAL STATUS sys543EtherAdrGet
    (
    int    unit
    )
    {
    UINT32 ix;
    UINT32 iy;
    UINT32 count = 0;
    UINT16 val;
    UCHAR  adr [ETHER_ADDRESS_SIZE];

    if(geiResources[unit].boardType == DEV_ID_82541PI)
        {
        sysGei541ReadEeprom(unit, EEPROM_IA_ADDRESS, 
                (ETHER_ADDRESS_SIZE / sizeof(UINT16)), 
                (UINT8 *)adr);

        for (ix = 0, iy = 0; ix < 6; ix++)
            { 
            geiResources[unit].enetAddr[iy++] = adr[ix] & 0xff;
            }
        }
    else
        {
        GEI_RESOURCE * pReso = (GEI_RESOURCE *)(geiPciResources[unit].pExtended);

        for (ix = 0; ix < ETHER_ADDRESS_SIZE / sizeof(UINT16); ix++) 
            {
            /* get word i from EEPROM */

            val = sys543eepromReadWord (unit, (UINT16)(EEPROM_IA_ADDRESS + ix));

            adr [count++] = (UCHAR)val;

            adr [count++] = (UCHAR) (val >> 8);
            }

        memcpy (&(pReso->enetAddr[0]), adr, ETHER_ADDRESS_SIZE);
        }
    /* check IA is UCAST  */

    if (adr[0] & 0x1)
        return (ERROR);
 
    return OK;
    }

/**************************************************************************
* 
* sys543eepromCheckSum - calculate checksum 
*
* This routine perform EEPROM checksum
*
* RETURNS: N/A
*/
LOCAL STATUS sys543eepromCheckSum
    (
    int    unit
    )
    {
    UINT16 checkSum = 0 ;
    UINT32 ix;
    UINT16 checkSumTmp;

 #ifdef FOX_CHKSUM_SKIP
    return OK;
 #endif /* FOX_BOOTROM_CHKSUM_SKIP */
 
    /*for 82541PI*/
    if(geiResources[unit].boardType == DEV_ID_82541PI)
        {
        for (ix = 0; ix < EEPROM_WORD_SIZE; ix++)
            {
            sysGei541ReadEeprom(unit, EEPROM_IA_ADDRESS + ix,
                                   1, (UINT8 *)&checkSumTmp);
            checkSum += checkSumTmp;
            }
        }
    else
        {
        for (ix = 0; ix < EEPROM_WORD_SIZE; ix++) 
            checkSum += sys543eepromReadWord (unit, ix);
        }
    if (checkSum == (UINT16)EEPROM_SUM)
        return OK;
 
    return ERROR;
    }

/***************************************************************************
*
* sys544PhyPreInit -- Init 82544's specific PHY regs before link setup
*
* This routine initializes some 82544's PHY regs before set up link
* Basically, it fixed short cable/backplane problem, Errata 21
*
* RETURN: N/A
*/
LOCAL void sys544PhyPreInit
    (
    PHY_INFO * pPhyInfo     /* PHY's info structure pointer */
    )
    {
    UINT16 regVal;          /* register value */
    UINT16 phyId1;          /* phy Id 1 */
    UINT16 phyId2;          /* phy ID 2 */
    UINT32 retVal;          /* return value */
    UINT32 phyOui = 0;      /* PHY's manufacture ID */
    UINT32 phyMode;         /* PHY mode number */
    UINT8  phyAddr;         /* PHY's bus number */

    phyAddr = pPhyInfo->phyAddr; 

    MII_READ (phyAddr, MII_PHY_ID1_REG, &phyId1, retVal);
   
    MII_READ (phyAddr, MII_PHY_ID2_REG, &phyId2, retVal);

    phyOui =  phyId1 << 6 | phyId2 >> 10;

    phyMode = (phyId2 & MII_ID2_MODE_MASK) >> 4;

    if (phyOui == INTEL_82544PHY_OUI_ID && (phyMode == INTEL_82544PHY_MODEL))
        {
        regVal = 0x0004; 

        MII_WRITE (phyAddr, 29, 0x4, retVal);

        MII_READ (phyAddr, 30, &regVal, retVal);

        regVal |= 0x0200; 

        MII_WRITE (phyAddr, 30, regVal, retVal);
        }
    }

/**************************************************************************
*
* sys543PhySpecRegsInit - Initialize PHY specific registers
*
* This routine initialize PHY specific registers
*
* RETURN: N/A
*/
LOCAL void sys543PhySpecRegsInit
    (
    PHY_INFO * pPhyInfo,    /* PHY's info structure pointer */
    UINT8      phyAddr      /* PHY's bus number */
    )
    {
    UINT16 regVal;          /* register value */
    UINT16 phyId1;          /* phy Id 1 */
    UINT16 phyId2;          /* phy ID 2 */
    UINT32 retVal;          /* return value */
    UINT32 phyOui = 0;      /* PHY's manufacture ID */
    UINT32 phyMode;         /* PHY mode number */

    /* Intel Pro1000T adapter uses Alaska transceiver */

    /* read device ID to check Alaska chip available */

    MII_READ (phyAddr, MII_PHY_ID1_REG, &phyId1, retVal);
    
    MII_READ (phyAddr, MII_PHY_ID2_REG, &phyId2, retVal);
       
    phyOui =  phyId1 << 6 | phyId2 >> 10;

    phyMode = (phyId2 & MII_ID2_MODE_MASK) >> 4;

    if (phyOui == MARVELL_OUI_ID && (phyMode == MARVELL_ALASKA_88E1000 || 
                                     phyMode == MARVELL_ALASKA_88E1000S))
        {
         /* This is actually a Marvell Alaska 1000T transceiver */

         /* disable PHY's interrupt */         

         MII_READ (phyAddr, ALASKA_INT_ENABLE_REG, &regVal, retVal);
         regVal = 0;
         MII_WRITE (phyAddr, ALASKA_INT_ENABLE_REG, regVal, retVal);

         /* CRS assert on transmit */

         MII_READ (phyAddr, ALASKA_PHY_SPEC_CTRL_REG, &regVal, retVal);
         regVal |= ALASKA_PSCR_ASSERT_CRS_ON_TX;
         MII_WRITE (phyAddr, ALASKA_PHY_SPEC_CTRL_REG, regVal, retVal);

        /* set the clock rate when operate in 1000T mode */

         MII_READ (phyAddr, ALASKA_EXT_PHY_SPEC_CTRL_REG, &regVal, retVal);
         regVal |= ALASKA_EPSCR_TX_CLK_25;
         MII_WRITE (phyAddr, ALASKA_EXT_PHY_SPEC_CTRL_REG, regVal, retVal);
        }

    else if (phyOui == INTEL_82544PHY_OUI_ID && 
            (phyMode == INTEL_82544PHY_MODEL))
        {
         /* This is INTEL 82544GC/EI internal PHY */

         /* disable PHY's interrupt */         

         MII_READ (phyAddr, INTEL_82544PHY_INT_ENABLE_REG, &regVal, retVal);
         regVal = 0;
         MII_WRITE (phyAddr, INTEL_82544PHY_INT_ENABLE_REG, regVal, retVal);

         /* CRS assert on transmit */

         MII_READ (phyAddr, INTEL_82544PHY_PHY_SPEC_CTRL_REG, 
                   &regVal, retVal);
         regVal |= INTEL_82544PHY_PSCR_ASSERT_CRS_ON_TX;
         MII_WRITE (phyAddr, INTEL_82544PHY_PHY_SPEC_CTRL_REG, 
                   regVal, retVal);

        /* set the TX_CLK rate when operate in 1000T mode */

         MII_READ (phyAddr, INTEL_82544PHY_EXT_PHY_SPEC_CTRL_REG, 
                   &regVal, retVal);
         regVal |= INTEL_82544PHY_EPSCR_TX_CLK_25;
         MII_WRITE (phyAddr, INTEL_82544PHY_EXT_PHY_SPEC_CTRL_REG, 
                   regVal, retVal);

        /* INTEL PHY's bug fixing ... */
 
        MII_WRITE (phyAddr, 29, 0x5, retVal);

        MII_READ (phyAddr, 30, &regVal, retVal);
        regVal |= 0x100;

        MII_WRITE (phyAddr, 30, regVal, retVal);       
        }        

     /* other PHYS .... */
     }

/*****************************************************************************
*
* sys543IntAck - acknowledge an 8254x interrupt
*
* This routine performs any 8254x interrupt acknowledge that may be
* required.  This typically involves an operation to some interrupt
* control hardware.
*
* This routine gets called from the 82543 driver's interrupt handler.
*
* This routine assumes that the PCI configuration information has already
* been setup.
*
* RETURNS: OK, or ERROR if the interrupt could not be acknowledged.
*/
LOCAL STATUS sys543IntAck
    (
    int    unit        /* unit number */
    )
    {
    return (OK);
    }

/*****************************************************************************
*
* sys543IntEnable - enable 8254x chip interrupts
*
* This routine enables 8254x interrupts.  This may involve operations on
* interrupt control hardware.
*
* The 8254x driver calls this routine throughout normal operation to terminate
* critical sections of code.
*
* This routine assumes that the PCI configuration information has already
* been setup.
*
* RETURNS: OK, or ERROR if interrupts could not be enabled.
*/
LOCAL STATUS sys543IntEnable
    (
    int    unit        /* unit number */
    )
    {
    return ((unit >= geiUnits) ? ERROR :
            (sysIntEnablePIC(geiPciResources[unit].irq)));
    }

/*****************************************************************************
*
* sys543IntDisable - disable 8254x chip interrupts
*
* This routine disables 8254x interrupts.  This may involve operations on
* interrupt control hardware.
*
* The 8254x driver calls this routine throughout normal operation to enter
* critical sections of code.
*
* This routine assumes that the PCI configuration information has already
* been setup.
*
* RETURNS: OK, or ERROR if interrupts could not be disabled.
*/
LOCAL STATUS sys543IntDisable
    (
    int    unit        /* unit number */
    )
    {
    return ((unit >= geiUnits) ? ERROR :
            (sysIntDisablePIC(geiPciResources[unit].irq)));
    }

/*****************************************************************************
*
* sys543Show - shows 8254x chip configuration 
*
* This routine shows (Intel Pro 1000F/T/XT/XF) adapter configuration 
*
* RETURNS: N/A
*/
void sys543Show
    (
    int    unit        /* unit number */
    )
    {
    int    ix;

    PCI_BOARD_RESOURCE * pRsrc;
    GEI_RESOURCE *       pReso;

    if (unit >= geiUnits)
        {
        printf ("gei device %d is not available\n", unit);
        return;
        }
 
    pRsrc = &geiPciResources [unit];
    pReso = (GEI_RESOURCE *)(pRsrc->pExtended);

    if (pRsrc->boardType == PRO1000_543_BOARD)
        printf ("********* Intel PRO1000 82543GC Based Adapter ***********\n");
    else if (pRsrc->boardType == PRO1000_544_BOARD)
        printf ("********* Intel PRO1000 82544GC/EI based Adapter ********\n");
    else if (pRsrc->boardType == PRO1000_546_BOARD)
        printf ("********* Intel 82540/82541/82545/82546 based Adapter ********\n");
    else if (pRsrc->boardType == PRO1000_I210_BOARD)
        printf ("********* Intel I210 based Adapter ********\n");
    else if (pRsrc->boardType == DEV_ID_82541PI)
        printf ("********* Intel 82541PI Adapter ********\n");
    else
        printf ("********* UNKNOWN Adapter ************ \n");
 
    printf ("  CSR PCI Membase address = 0x%x\n", pReso->memBaseLow);
 
    printf ("  Flash PCI Membase address = 0x%x\n", pReso->flashBase);

    printf ("  PCI bus no.= 0x%x, device no.= 0x%x, function no.= 0x%x, IRQ = %d\n",
             pRsrc->pciBus, pRsrc->pciDevice, pRsrc->pciFunc, pRsrc->irq);

    if (pReso->iniStatus == ERROR)
        return;

    printf ("  Adapter Ethernet Address");

    for (ix = 0; ix < 6; ix ++)
        printf (":%2.2X", (UINT32)pReso->enetAddr[ix]);

    printf ("\n");

    printf ("  EEPROM Initialization Control Word 1 = 0x%4.4X\n", 
            pReso->eeprom_icw1);

    printf ("  EEPROM Initialization Control Word 2 = 0x%4.4X\n", 
            pReso->eeprom_icw2);

    printf ("*********************************************\n");
    }

/*******************************************************************************
*
* sysGeiDevToType - convert PCI Vendor and Device IDs to a device type
*
* Given <vendorId>, <deviceId>, and <revisionId> values read from PCI Vendor
* and Device ID registers in PCI configuration space, this routine will
* attempt to map the IDs to an 8254x device type value.
*
* RETURNS:
* A board type value which will be one of
*
* .IP
* PRO1000_543_BOARD
* .IP
* PRO1000_544_BOARD
* .IP
* PRO1000_546_BOARD
* .LP
*
* BOARD_TYPE_UNKNOWN will be returned if the Device ID does not map to
* a supported board type.
*
* NOMANUAL
*/
LOCAL UINT32 sysGeiDevToType
    (
    UINT32 vendorId,    /* specifies a PCI Vendor ID value */
    UINT32 deviceId,    /* specifies a PCI Device ID value */
    UINT8  revisionId   /* specifies a PCI Revision ID value */
    )
    {
    /* At the moment, we are only supporting vendor Intel */

    if (vendorId == PRO1000_PCI_VENDOR_ID)
        {
        switch (deviceId)
            {
            case PRO1000_543_PCI_DEVICE_ID_T:
            case PRO1000_543_PCI_DEVICE_ID_FT:
                return (PRO1000_543_BOARD);

            case PRO1000_544_PCI_DEVICE_ID_XT:
            case PRO1000_544_PCI_DEVICE_ID_XF:
            case PRO1000_544_PCI_DEVICE_ID_GC:
                return (PRO1000_544_BOARD);
            
            case PRO1000_540_PCI_DEVICE_ID_XT:
            case PRO1000_541_PCI_DEVICE_ID_XT:
            case PRO1000_545_PCI_DEVICE_ID_XT:
            case PRO1000_546_PCI_DEVICE_ID_XT:
            case PRO1000_546_PCI_DEVICE_ID_XT2:
            case PRO1000_545_PCI_DEVICE_ID_MF:
            case PRO1000_546_PCI_DEVICE_ID_MF:
                return (PRO1000_546_BOARD);
            case PRO1000_573_PCI_DEVICE_ID_E:
            case PRO1000_573_PCI_DEVICE_ID_IAMT:
            case PRO1000_571_PCI_DEVICE_ID_EB:
            case PRO1000_574_PCI_DEVICE_ID: /*Ampro*/
                return (PRO1000_573_BOARD);
            case INTEL_I210_DEVICE_ID:
                printf("id = %x\n", deviceId);
                return (PRO1000_I210_BOARD);
            case DEV_ID_82541PI:
                return DEV_ID_82541PI;
            }
        }

    return (BOARD_TYPE_UNKNOWN);
    }

/****************************************************************************
* EEPROM read functiond ported from vxWorks 6.9 GEI driver
*
* geiEeAddrSet - select a word in the EEPROM
*
* This is a helper function used by geiBitBangEeWordGet(), which clocks a
* sequence of bits through to the serial EEPROM attached to the Intel
* chip which contains a read command and specifies what word we
* want to access.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
void geiEeAddrSet(int unit, UINT32 addr)
    {
    UINT32 d;
    int i;

    d = addr | (GEI_9346_READ << geiResources[unit].eeprom_address_bits);

    for (i = 1 << (geiResources[unit].eeprom_address_bits + 3); i; i >>= 1)
        {
        if (d & i)
            CSR_SETBIT_4(unit, GEI_EECD, GEI_EECD_DI);
        else
            CSR_CLRBIT_4(unit, GEI_EECD, GEI_EECD_DI);
        sysUsDelay (50);
        CSR_SETBIT_4(unit, GEI_EECD, GEI_EECD_SK);
        CSR_CLRBIT_4(unit, GEI_EECD, GEI_EECD_SK);
        }

    return;
    }

/*****************************************************************************
*
* geiBitBangEeWordGet - read a word from the EEPROM using bitbang I/O
*
* This routine reads a single 16 bit word from a specified address
* within the EEPROM attached to the PRO/1000 controller and returns
* it to the caller. After clocking in a read command and the address
* we want to access, we read back the 16 bit datum stored there.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

void geiBitBangEeWordGet(int unit, UINT32 addr, UINT16 *dest)
    {
    int i;
    UINT16 word = 0;

    /*
     * Some devices require us to acquire access to the EEPROM
     * interface before we can do a read access. On devices that
     * don't support this (82543, 82544), this has no effect.
     */

    CSR_SETBIT_4(unit, GEI_EECD, GEI_EECD_EE_REQ);

    for (i = 0; i < GEI_TIMEOUT; i++)
        if (GEI_SYS_READ_REG(unit, GEI_EECD) & GEI_EECD_EE_GNT)
            break;

    GEI_SYS_WRITE_REG(unit, GEI_EECD, GEI_EECD_EE_REQ|GEI_EECD_CS);

    geiEeAddrSet (unit, addr);

    GEI_SYS_WRITE_REG(unit, GEI_EECD, GEI_EECD_EE_REQ|GEI_EECD_CS);

    for (i = 0x8000; i; i >>= 1)
        {
        CSR_SETBIT_4(unit, GEI_EECD, GEI_EECD_SK);
        sysUsDelay (50);
        if (GEI_SYS_READ_REG(unit, GEI_EECD) & GEI_EECD_DO)
            word = (UINT16) (word | i);
        CSR_CLRBIT_4(unit, GEI_EECD, GEI_EECD_SK);
        }

    GEI_SYS_WRITE_REG(unit, GEI_EECD, 0);

    *dest = word;
    return;
    }

/*****************************************************************************
*
* sysGei541ReadEeprom - read a sequence of words from the EEPROM
*
* This is the top-level EEPROM access function. It will read a
* sequence of words from the specified address into a supplied
* destination buffer. This is used mainly to read the station
* address.
*
* available EEPROM access methods: direct 'bitbang' I/O
* via the EECD register.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

LOCAL void sysGei541ReadEeprom(int unit,int off, int cnt,UINT8 *dest)
    {
    int i;
    UINT16 word, *ptr;

    for (i = 0; i < cnt; i++)
        {
        geiBitBangEeWordGet (unit, (UINT32)(off + i), &word);
        ptr = (UINT16 *)(dest + (i * 2));
        *ptr = le16toh(word);
        }
    return;
    }


/*******************************************************************************
*
* sysGeiTaskDelay - suspend task for specified clock ticks
*
* For use by phyDelayRtn, as an alternative to taskDelay() so that we can tune
* the phyMaxDelay below the 5 second minimum that miiLib will enforce iff taskDelay()
* is used for phyDelayRtn. It is implemented in terms of taskDelay().
*
*/
LOCAL int sysGeiTaskDelay(int ticks)
{
   return taskDelay(ticks);
}

#endif /* INCLUDE_GEI8254X_END */
