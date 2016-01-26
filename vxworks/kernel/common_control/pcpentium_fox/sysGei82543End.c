/* sysGei82543End.c - Intel PRO1000X/F/XT/XF/MT/MF driver support routine */

/* Copyright 1989-2002 Wind River Systems, Inc.  */

/*
modification history
--------------------
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

#include "end.h"
#include "drv/end/gei82543End.h"

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

#define GEI_SYS_WRITE_FLUSH(unit)                        \
{                                                        \
   UINT32 x;                                            \
   x = GEI_SYS_READ_REG(unit, INTEL_82543GC_STATUS);    \
}

#define GEI_SYS_WRITE_REG_IO(unit, reg, value)    \
{                                            \
  sysOutByte(geiResources[(unit)].ioBase, (reg));    \
  sysOutByte(geiResources[(unit)].ioBase+4, (value));    \
}

#define GEI_SYS_WRITE_REG_ARRAY(unit, reg, offset, value)  \
    ((*(volatile UINT32 *)(PCI_MEMIO2LOCAL(geiResources[(unit)].memBaseLow+(reg)+((offset) <<2) ) ))  = \
    (UINT32)(value))

/* PCI Device IDs */
#define DEV_ID_82542               0x1000
#define DEV_ID_82543GC_FIBER       0x1001
#define DEV_ID_82543GC_COPPER      0x1004
#define DEV_ID_82544EI_COPPER      0x1008
#define DEV_ID_82544EI_FIBER       0x1009
#define DEV_ID_82544GC_COPPER      0x100C
#define DEV_ID_82544GC_LOM         0x100D
#define DEV_ID_82540EM             0x100E
#define DEV_ID_82540EM_LOM         0x1015
#define DEV_ID_82540EP_LOM         0x1016
#define DEV_ID_82540EP             0x1017
#define DEV_ID_82540EP_LP          0x101E
#define DEV_ID_82545EM_COPPER      0x100F
#define DEV_ID_82545EM_FIBER       0x1011
#define DEV_ID_82545GM_COPPER      0x1026
#define DEV_ID_82545GM_FIBER       0x1027
#define DEV_ID_82545GM_SERDES      0x1028
#define DEV_ID_82546EB_COPPER      0x1010
#define DEV_ID_82546EB_FIBER       0x1012
#define DEV_ID_82546EB_QUAD_COPPER 0x101D
#define DEV_ID_82541EI             0x1013
#define DEV_ID_82541EI_MOBILE      0x1018
#define DEV_ID_82541ER             0x1078
#define DEV_ID_82547GI             0x1075
#define DEV_ID_82541PI             0x1076
#define DEV_ID_82541GI_MOBILE      0x1077
#define DEV_ID_82541GI_LF          0x107C
#define DEV_ID_82546GB_COPPER      0x1079
#define DEV_ID_82546GB_FIBER       0x107A
#define DEV_ID_82546GB_SERDES      0x107B
#define DEV_ID_82547EI             0x1019

/* EEPROM/Flash Control */
#define E1000_EECD_SK        0x00000001 /* EEPROM Clock */
#define E1000_EECD_CS        0x00000002 /* EEPROM Chip Select */
#define E1000_EECD_DI        0x00000004 /* EEPROM Data In */
#define E1000_EECD_DO        0x00000008 /* EEPROM Data Out */
#define E1000_EECD_FWE_MASK  0x00000030
#define E1000_EECD_FWE_DIS   0x00000010 /* Disable FLASH writes */
#define E1000_EECD_FWE_EN    0x00000020 /* Enable FLASH writes */
#define E1000_EECD_FWE_SHIFT 4
#define E1000_EECD_REQ       0x00000040 /* EEPROM Access Request */
#define E1000_EECD_GNT       0x00000080 /* EEPROM Access Grant */
#define E1000_EECD_PRES      0x00000100 /* EEPROM Present */
#define E1000_EECD_SIZE      0x00000200 /* EEPROM Size (0=64 word 1=256 word) */
#define E1000_EECD_ADDR_BITS 0x00000400 /* EEPROM Addressing bits based on type
                                         * (0-small, 1-large) */
#define E1000_EECD_TYPE      0x00002000 /* EEPROM Type (1-SPI, 0-Microwire) */
#ifndef E1000_EEPROM_GRANT_ATTEMPTS
#define E1000_EEPROM_GRANT_ATTEMPTS 1000 /* EEPROM # attempts to gain grant */
#endif

/* EEPROM Read */
#define E1000_EERD_START      0x00000001 /* Start Read */
#define E1000_EERD_DONE       0x00000010 /* Read Done */
#define E1000_EERD_ADDR_SHIFT 8
#define E1000_EERD_ADDR_MASK  0x0000FF00 /* Read Address */
#define E1000_EERD_DATA_SHIFT 16
#define E1000_EERD_DATA_MASK  0xFFFF0000 /* Read Data */

/* SPI EEPROM Status Register */
#define EEPROM_STATUS_RDY_SPI  0x01
#define EEPROM_STATUS_WEN_SPI  0x02
#define EEPROM_STATUS_BP0_SPI  0x04
#define EEPROM_STATUS_BP1_SPI  0x08
#define EEPROM_STATUS_WPEN_SPI 0x80

/* EEPROM Commands - Microwire */
#define EEPROM_READ_OPCODE_MICROWIRE  0x6  /* EEPROM read opcode */
#define EEPROM_WRITE_OPCODE_MICROWIRE 0x5  /* EEPROM write opcode */
#define EEPROM_ERASE_OPCODE_MICROWIRE 0x7  /* EEPROM erase opcode */
#define EEPROM_EWEN_OPCODE_MICROWIRE  0x13 /* EEPROM erase/write enable */
#define EEPROM_EWDS_OPCODE_MICROWIRE  0x10 /* EEPROM erast/write disable */

/* EEPROM Commands - SPI */
#define EEPROM_MAX_RETRY_SPI    5000 /* Max wait of 5ms, for RDY signal */
#define EEPROM_READ_OPCODE_SPI  0x3  /* EEPROM read opcode */
#define EEPROM_WRITE_OPCODE_SPI 0x2  /* EEPROM write opcode */
#define EEPROM_A8_OPCODE_SPI    0x8  /* opcode bit-3 = address bit-8 */
#define EEPROM_WREN_OPCODE_SPI  0x6  /* EEPROM set Write Enable latch */
#define EEPROM_WRDI_OPCODE_SPI  0x4  /* EEPROM reset Write Enable latch */
#define EEPROM_RDSR_OPCODE_SPI  0x5  /* EEPROM read Status register */
#define EEPROM_WRSR_OPCODE_SPI  0x1  /* EEPROM write Status register */

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



/*INTEL 82543 for 82541PI porting from Linux*/
#define MAX_PHY_REG_ADDRESS        0x1F  /* 5 bit address bus (0-0x1F) */
#define MAX_PHY_MULTI_PAGE_REG     0xF   /* Registers equal on all pages */

#define IGP01E1000_PHY_PAGE_SELECT     0x1F /* PHY Page Select Core Register */

#define E1000_MANC     0x05820  /* Management Control - RW */
#define E1000_MANC_ARP_EN        0x00002000 /* Enable ARP Request Filtering */

#define E1000_LEDCTL   0x00E00  /* LED Control - RW */
#define IGP_ACTIVITY_LED_MASK   0xFFFFF0FF
#define IGP_ACTIVITY_LED_ENABLE 0x0300
#define IGP_LED3_MODE           0x07000000

#define E1000_WUC      0x05800  /* Wakeup Control - RW */

#define ETHERNET_IEEE_VLAN_TYPE 0x8100  /* 802.3ac packet */

#define E1000_MTA      0x05200  /* Multicast Table Array - RW Array */

/* Filters */
#define E1000_NUM_UNICAST          16   /* Unicast filter entries */
#define E1000_MC_TBL_SIZE          128  /* Multicast Filter Table (4096 bits) */
#define E1000_VLAN_FILTER_TBL_SIZE 128  /* VLAN Filter Table (4096 bits) */

#define E1000_TXDCTL   0x03828  /* TX Descriptor Control - RW */

/* Transmit Descriptor Control */
#define E1000_TXDCTL_PTHRESH 0x000000FF /* TXDCTL Prefetch Threshold */
#define E1000_TXDCTL_HTHRESH 0x0000FF00 /* TXDCTL Host Threshold */
#define E1000_TXDCTL_WTHRESH 0x00FF0000 /* TXDCTL Writeback Threshold */
#define E1000_TXDCTL_GRAN    0x01000000 /* TXDCTL Granularity */
#define E1000_TXDCTL_LWTHRESH 0xFE000000 /* TXDCTL Low Threshold */
#define E1000_TXDCTL_FULL_TX_DESC_WB 0x01010000 /* GRAN=1, WTHRESH=1 */
#define E1000_TXDCTL_COUNT_DESC 0x00400000 /* Enable the counting of desc.
                                              still to be processed. */

/* MDI Control */
#define E1000_MDIC_DATA_MASK 0x0000FFFF
#define E1000_MDIC_REG_MASK  0x001F0000
#define E1000_MDIC_REG_SHIFT 16
#define E1000_MDIC_PHY_MASK  0x03E00000
#define E1000_MDIC_PHY_SHIFT 21
#define E1000_MDIC_OP_WRITE  0x04000000
#define E1000_MDIC_OP_READ   0x08000000
#define E1000_MDIC_READY     0x10000000
#define E1000_MDIC_INT_EN    0x20000000
#define E1000_MDIC_ERROR     0x40000000
                                              
/* Register Set. (82543, 82544)
 *
 * Registers are defined to be 32 bits and  should be accessed as 32 bit values.
 * These registers are physically located on the NIC, but are mapped into the
 * host memory address space.
 *
 * RW - register is both readable and writable
 * RO - register is read only
 * WO - register is write only
 * R/clr - register is read only and is cleared when read
 * A - register array
 */
#define E1000_CTRL     0x00000  /* Device Control - RW */
#define E1000_CTRL_DUP 0x00004  /* Device Control Duplicate (Shadow) - RW */
#define E1000_STATUS   0x00008  /* Device Status - RO */
#define E1000_EECD     0x00010  /* EEPROM/Flash Control - RW */
#define E1000_EERD     0x00014  /* EEPROM Read - RW */
#define E1000_CTRL_EXT 0x00018  /* Extended Device Control - RW */
#define E1000_FLA      0x0001C  /* Flash Access - RW */
#define E1000_MDIC     0x00020  /* MDI Control - RW */
#define E1000_SCTL     0x00024  /* SerDes Control - RW */
#define E1000_FCAL     0x00028  /* Flow Control Address Low - RW */
#define E1000_FCAH     0x0002C  /* Flow Control Address High -RW */
#define E1000_FCT      0x00030  /* Flow Control Type - RW */
#define E1000_VET      0x00038  /* VLAN Ether Type - RW */
#define E1000_ICR      0x000C0  /* Interrupt Cause Read - R/clr */
#define E1000_ITR      0x000C4  /* Interrupt Throttling Rate - RW */
#define E1000_ICS      0x000C8  /* Interrupt Cause Set - WO */
#define E1000_IMS      0x000D0  /* Interrupt Mask Set - RW */
#define E1000_IMC      0x000D8  /* Interrupt Mask Clear - WO */
#define E1000_IAM      0x000E0  /* Interrupt Acknowledge Auto Mask */
#define E1000_RCTL     0x00100  /* RX Control - RW */
#define E1000_RDTR1    0x02820  /* RX Delay Timer (1) - RW */
#define E1000_RDBAL1   0x02900  /* RX Descriptor Base Address Low (1) - RW */
#define E1000_RDBAH1   0x02904  /* RX Descriptor Base Address High (1) - RW */
#define E1000_RDLEN1   0x02908  /* RX Descriptor Length (1) - RW */
#define E1000_RDH1     0x02910  /* RX Descriptor Head (1) - RW */
#define E1000_RDT1     0x02918  /* RX Descriptor Tail (1) - RW */
#define E1000_FCTTV    0x00170  /* Flow Control Transmit Timer Value - RW */
#define E1000_TXCW     0x00178  /* TX Configuration Word - RW */
#define E1000_RXCW     0x00180  /* RX Configuration Word - RO */
#define E1000_TCTL     0x00400  /* TX Control - RW */
#define E1000_TIPG     0x00410  /* TX Inter-packet gap -RW */
#define E1000_TBT      0x00448  /* TX Burst Timer - RW */
#define E1000_AIT      0x00458  /* Adaptive Interframe Spacing Throttle - RW */
#define E1000_LEDCTL   0x00E00  /* LED Control - RW */
#define E1000_EXTCNF_CTRL  0x00F00  /* Extended Configuration Control */
#define E1000_EXTCNF_SIZE  0x00F08  /* Extended Configuration Size */
#define E1000_PBA      0x01000  /* Packet Buffer Allocation - RW */
#define E1000_PBS      0x01008  /* Packet Buffer Size */
#define E1000_EEMNGCTL 0x01010  /* MNG EEprom Control */
#define E1000_FLASH_UPDATES 1000
#define E1000_EEARBC   0x01024  /* EEPROM Auto Read Bus Control */
#define E1000_FLASHT   0x01028  /* FLASH Timer Register */
#define E1000_EEWR     0x0102C  /* EEPROM Write Register - RW */
#define E1000_FLSWCTL  0x01030  /* FLASH control register */
#define E1000_FLSWDATA 0x01034  /* FLASH data register */
#define E1000_FLSWCNT  0x01038  /* FLASH Access Counter */
#define E1000_FLOP     0x0103C  /* FLASH Opcode Register */
#define E1000_ERT      0x02008  /* Early Rx Threshold - RW */
#define E1000_FCRTL    0x02160  /* Flow Control Receive Threshold Low - RW */
#define E1000_FCRTH    0x02168  /* Flow Control Receive Threshold High - RW */
#define E1000_PSRCTL   0x02170  /* Packet Split Receive Control - RW */
#define E1000_RDBAL    0x02800  /* RX Descriptor Base Address Low - RW */
#define E1000_RDBAH    0x02804  /* RX Descriptor Base Address High - RW */
#define E1000_RDLEN    0x02808  /* RX Descriptor Length - RW */
#define E1000_RDH      0x02810  /* RX Descriptor Head - RW */
#define E1000_RDT      0x02818  /* RX Descriptor Tail - RW */
#define E1000_RDTR     0x02820  /* RX Delay Timer - RW */
#define E1000_RDBAL0   E1000_RDBAL /* RX Desc Base Address Low (0) - RW */
#define E1000_RDBAH0   E1000_RDBAH /* RX Desc Base Address High (0) - RW */
#define E1000_RDLEN0   E1000_RDLEN /* RX Desc Length (0) - RW */
#define E1000_RDH0     E1000_RDH   /* RX Desc Head (0) - RW */
#define E1000_RDT0     E1000_RDT   /* RX Desc Tail (0) - RW */
#define E1000_RDTR0    E1000_RDTR  /* RX Delay Timer (0) - RW */
#define E1000_RXDCTL   0x02828  /* RX Descriptor Control - RW */
#define E1000_RADV     0x0282C  /* RX Interrupt Absolute Delay Timer - RW */
#define E1000_RSRPD    0x02C00  /* RX Small Packet Detect - RW */
#define E1000_RAID     0x02C08  /* Receive Ack Interrupt Delay - RW */
#define E1000_TXDMAC   0x03000  /* TX DMA Control - RW */
#define E1000_TDFH     0x03410  /* TX Data FIFO Head - RW */
#define E1000_TDFT     0x03418  /* TX Data FIFO Tail - RW */
#define E1000_TDFHS    0x03420  /* TX Data FIFO Head Saved - RW */
#define E1000_TDFTS    0x03428  /* TX Data FIFO Tail Saved - RW */
#define E1000_TDFPC    0x03430  /* TX Data FIFO Packet Count - RW */
#define E1000_TDBAL    0x03800  /* TX Descriptor Base Address Low - RW */
#define E1000_TDBAH    0x03804  /* TX Descriptor Base Address High - RW */
#define E1000_TDLEN    0x03808  /* TX Descriptor Length - RW */
#define E1000_TDH      0x03810  /* TX Descriptor Head - RW */
#define E1000_TDT      0x03818  /* TX Descripotr Tail - RW */
#define E1000_TIDV     0x03820  /* TX Interrupt Delay Value - RW */
#define E1000_TXDCTL   0x03828  /* TX Descriptor Control - RW */
#define E1000_TADV     0x0382C  /* TX Interrupt Absolute Delay Val - RW */
#define E1000_TSPMT    0x03830  /* TCP Segmentation PAD & Min Threshold - RW */
#define E1000_TARC0    0x03840  /* TX Arbitration Count (0) */
#define E1000_TDBAL1   0x03900  /* TX Desc Base Address Low (1) - RW */
#define E1000_TDBAH1   0x03904  /* TX Desc Base Address High (1) - RW */
#define E1000_TDLEN1   0x03908  /* TX Desc Length (1) - RW */
#define E1000_TDH1     0x03910  /* TX Desc Head (1) - RW */
#define E1000_TDT1     0x03918  /* TX Desc Tail (1) - RW */
#define E1000_TXDCTL1  0x03928  /* TX Descriptor Control (1) - RW */
#define E1000_TARC1    0x03940  /* TX Arbitration Count (1) */
#define E1000_CRCERRS  0x04000  /* CRC Error Count - R/clr */
#define E1000_ALGNERRC 0x04004  /* Alignment Error Count - R/clr */
#define E1000_SYMERRS  0x04008  /* Symbol Error Count - R/clr */
#define E1000_RXERRC   0x0400C  /* Receive Error Count - R/clr */
#define E1000_MPC      0x04010  /* Missed Packet Count - R/clr */
#define E1000_SCC      0x04014  /* Single Collision Count - R/clr */
#define E1000_ECOL     0x04018  /* Excessive Collision Count - R/clr */
#define E1000_MCC      0x0401C  /* Multiple Collision Count - R/clr */
#define E1000_LATECOL  0x04020  /* Late Collision Count - R/clr */
#define E1000_COLC     0x04028  /* Collision Count - R/clr */
#define E1000_DC       0x04030  /* Defer Count - R/clr */
#define E1000_TNCRS    0x04034  /* TX-No CRS - R/clr */
#define E1000_SEC      0x04038  /* Sequence Error Count - R/clr */
#define E1000_CEXTERR  0x0403C  /* Carrier Extension Error Count - R/clr */
#define E1000_RLEC     0x04040  /* Receive Length Error Count - R/clr */
#define E1000_XONRXC   0x04048  /* XON RX Count - R/clr */
#define E1000_XONTXC   0x0404C  /* XON TX Count - R/clr */
#define E1000_XOFFRXC  0x04050  /* XOFF RX Count - R/clr */
#define E1000_XOFFTXC  0x04054  /* XOFF TX Count - R/clr */
#define E1000_FCRUC    0x04058  /* Flow Control RX Unsupported Count- R/clr */
#define E1000_PRC64    0x0405C  /* Packets RX (64 bytes) - R/clr */
#define E1000_PRC127   0x04060  /* Packets RX (65-127 bytes) - R/clr */
#define E1000_PRC255   0x04064  /* Packets RX (128-255 bytes) - R/clr */
#define E1000_PRC511   0x04068  /* Packets RX (255-511 bytes) - R/clr */
#define E1000_PRC1023  0x0406C  /* Packets RX (512-1023 bytes) - R/clr */
#define E1000_PRC1522  0x04070  /* Packets RX (1024-1522 bytes) - R/clr */
#define E1000_GPRC     0x04074  /* Good Packets RX Count - R/clr */
#define E1000_BPRC     0x04078  /* Broadcast Packets RX Count - R/clr */
#define E1000_MPRC     0x0407C  /* Multicast Packets RX Count - R/clr */
#define E1000_GPTC     0x04080  /* Good Packets TX Count - R/clr */
#define E1000_GORCL    0x04088  /* Good Octets RX Count Low - R/clr */
#define E1000_GORCH    0x0408C  /* Good Octets RX Count High - R/clr */
#define E1000_GOTCL    0x04090  /* Good Octets TX Count Low - R/clr */
#define E1000_GOTCH    0x04094  /* Good Octets TX Count High - R/clr */
#define E1000_RNBC     0x040A0  /* RX No Buffers Count - R/clr */
#define E1000_RUC      0x040A4  /* RX Undersize Count - R/clr */
#define E1000_RFC      0x040A8  /* RX Fragment Count - R/clr */
#define E1000_ROC      0x040AC  /* RX Oversize Count - R/clr */
#define E1000_RJC      0x040B0  /* RX Jabber Count - R/clr */
#define E1000_MGTPRC   0x040B4  /* Management Packets RX Count - R/clr */
#define E1000_MGTPDC   0x040B8  /* Management Packets Dropped Count - R/clr */
#define E1000_MGTPTC   0x040BC  /* Management Packets TX Count - R/clr */
#define E1000_TORL     0x040C0  /* Total Octets RX Low - R/clr */
#define E1000_TORH     0x040C4  /* Total Octets RX High - R/clr */
#define E1000_TOTL     0x040C8  /* Total Octets TX Low - R/clr */
#define E1000_TOTH     0x040CC  /* Total Octets TX High - R/clr */
#define E1000_TPR      0x040D0  /* Total Packets RX - R/clr */
#define E1000_TPT      0x040D4  /* Total Packets TX - R/clr */
#define E1000_PTC64    0x040D8  /* Packets TX (64 bytes) - R/clr */
#define E1000_PTC127   0x040DC  /* Packets TX (65-127 bytes) - R/clr */
#define E1000_PTC255   0x040E0  /* Packets TX (128-255 bytes) - R/clr */
#define E1000_PTC511   0x040E4  /* Packets TX (256-511 bytes) - R/clr */
#define E1000_PTC1023  0x040E8  /* Packets TX (512-1023 bytes) - R/clr */
#define E1000_PTC1522  0x040EC  /* Packets TX (1024-1522 Bytes) - R/clr */
#define E1000_MPTC     0x040F0  /* Multicast Packets TX Count - R/clr */
#define E1000_BPTC     0x040F4  /* Broadcast Packets TX Count - R/clr */
#define E1000_TSCTC    0x040F8  /* TCP Segmentation Context TX - R/clr */
#define E1000_TSCTFC   0x040FC  /* TCP Segmentation Context TX Fail - R/clr */
#define E1000_IAC      0x04100  /* Interrupt Assertion Count */
#define E1000_ICRXPTC  0x04104  /* Interrupt Cause Rx Packet Timer Expire Count */
#define E1000_ICRXATC  0x04108  /* Interrupt Cause Rx Absolute Timer Expire Count */
#define E1000_ICTXPTC  0x0410C  /* Interrupt Cause Tx Packet Timer Expire Count */
#define E1000_ICTXATC  0x04110  /* Interrupt Cause Tx Absolute Timer Expire Count */
#define E1000_ICTXQEC  0x04118  /* Interrupt Cause Tx Queue Empty Count */
#define E1000_ICTXQMTC 0x0411C  /* Interrupt Cause Tx Queue Minimum Threshold Count */
#define E1000_ICRXDMTC 0x04120  /* Interrupt Cause Rx Descriptor Minimum Threshold Count */
#define E1000_ICRXOC   0x04124  /* Interrupt Cause Receiver Overrun Count */
#define E1000_RXCSUM   0x05000  /* RX Checksum Control - RW */
#define E1000_RFCTL    0x05008  /* Receive Filter Control*/
#define E1000_MTA      0x05200  /* Multicast Table Array - RW Array */
#define E1000_RA       0x05400  /* Receive Address - RW Array */
#define E1000_VFTA     0x05600  /* VLAN Filter Table Array - RW Array */
#define E1000_WUC      0x05800  /* Wakeup Control - RW */
#define E1000_WUFC     0x05808  /* Wakeup Filter Control - RW */
#define E1000_WUS      0x05810  /* Wakeup Status - RO */
#define E1000_MANC     0x05820  /* Management Control - RW */
#define E1000_IPAV     0x05838  /* IP Address Valid - RW */
#define E1000_IP4AT    0x05840  /* IPv4 Address Table - RW Array */
#define E1000_IP6AT    0x05880  /* IPv6 Address Table - RW Array */
#define E1000_WUPL     0x05900  /* Wakeup Packet Length - RW */
#define E1000_WUPM     0x05A00  /* Wakeup Packet Memory - RO A */
#define E1000_FFLT     0x05F00  /* Flexible Filter Length Table - RW Array */
#define E1000_HOST_IF  0x08800  /* Host Interface */
#define E1000_FFMT     0x09000  /* Flexible Filter Mask Table - RW Array */
#define E1000_FFVT     0x09800  /* Flexible Filter Value Table - RW Array */

#define E1000_GCR       0x05B00 /* PCI-Ex Control */
#define E1000_GSCL_1    0x05B10 /* PCI-Ex Statistic Control #1 */
#define E1000_GSCL_2    0x05B14 /* PCI-Ex Statistic Control #2 */
#define E1000_GSCL_3    0x05B18 /* PCI-Ex Statistic Control #3 */
#define E1000_GSCL_4    0x05B1C /* PCI-Ex Statistic Control #4 */
#define E1000_FACTPS    0x05B30 /* Function Active and Power State to MNG */
#define E1000_SWSM      0x05B50 /* SW Semaphore */
#define E1000_FWSM      0x05B54 /* FW Semaphore */
#define E1000_FFLT_DBG  0x05F04 /* Debug Register */
#define E1000_HICR      0x08F00 /* Host Inteface Control */


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

/* typedefs */

typedef enum {
    EEPROM_UNINITIALIZED = 0,
    EEPROM_SPI,
    EEPROM_MICROWIRE,
    NUM_EEPROM_TYPES
} EEPROM_TYPE;

typedef struct eeprom {
    EEPROM_TYPE type;
    UINT16 mac_type;
    UINT16 opcode_bits;
    UINT16 address_bits;
    UINT16 delay_usec;
    UINT16 page_size;
}EEPROM;


typedef struct geiResource        /* GEI_RESOURCE */
    {
    BOOL   adr64;                 /* Indicator for 64-bit support */
    UINT32 boardType;             /* board type */
    UINT32 memBaseLow;            /* Base Address LOW */
    UINT32 memBaseHigh;           /* Base Address HIGH */
    UINT32 flashBase;             /* Base Address for FLASH */
    UINT32 ioBase;            /*IO Base Address*/

    UINT16 eepromSize;            /* size in unit of word (16 bit) - 64/256 */ 
    EEPROM eeprom;
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

GEI_RESOURCE geiResources [GEI_MAX_DEV] =
    {
    {FALSE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, GEI_EEPROM_SZ_64, 
     {(EEPROM_TYPE)NONE, (UINT16)NONE, (UINT16)NONE, (UINT16)NONE, (UINT16)NONE, (UINT16)NONE}, 0, 0, {(UCHAR)NONE},
     (UINT32)GEI0_SHMEM_BASE, GEI0_SHMEM_SIZE, GEI0_RXDES_NUM, GEI0_TXDES_NUM, FALSE, 
     GEI0_USR_FLAG, ERROR
    },

    {FALSE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, GEI_EEPROM_SZ_64, 
      {(EEPROM_TYPE)NONE, (UINT16)NONE, (UINT16)NONE, (UINT16)NONE, (UINT16)NONE, (UINT16)NONE}, 0, 0, {(UCHAR)NONE},
     (UINT32)GEI1_SHMEM_BASE, GEI1_SHMEM_SIZE, GEI1_RXDES_NUM, GEI1_TXDES_NUM, FALSE, 
     GEI1_USR_FLAG, ERROR
    },

    {FALSE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, GEI_EEPROM_SZ_64, 
     {(EEPROM_TYPE)NONE, (UINT16)NONE, (UINT16)NONE, (UINT16)NONE, (UINT16)NONE, (UINT16)NONE}, 0, 0, {(UCHAR)NONE},
     (UINT32)GEI2_SHMEM_BASE, GEI2_SHMEM_SIZE, GEI2_RXDES_NUM, GEI2_TXDES_NUM, FALSE, 
     GEI2_USR_FLAG, ERROR
    },

    {FALSE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, GEI_EEPROM_SZ_64, 
     {(EEPROM_TYPE)NONE, (UINT16)NONE, (UINT16)NONE, (UINT16)NONE, (UINT16)NONE, (UINT16)NONE}, 0, 0, {(UCHAR)NONE},
     (UINT32)GEI3_SHMEM_BASE, GEI3_SHMEM_SIZE, GEI3_RXDES_NUM, GEI3_TXDES_NUM, FALSE, 
     GEI3_USR_FLAG, ERROR
    }
    };

/* This table defines board PCI resources */

LOCAL PCI_BOARD_RESOURCE geiPciResources [GEI_MAX_DEV] =
    {
    {(UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT8)NONE, (UINT32)NONE, (UINT8)NONE, (UINT32)NONE,
    {(UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE},
     (void * const)(&geiResources[0])
    },

    {(UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT8)NONE, (UINT32)NONE, (UINT8)NONE, (UINT32)NONE,
    {(UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE},
     (void * const)(&geiResources[1])
    },

    {(UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT8)NONE, (UINT32)NONE, (UINT8)NONE, (UINT32)NONE,
    {(UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE},
     (void * const)(&geiResources[2])
    },

    {(UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT8)NONE, (UINT32)NONE, (UINT8)NONE, (UINT32)NONE,
    {(UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE, (UINT32)NONE},
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

void sysGei541InitParam(int unit, int deviceId);
STATUS sysGei541ReadEeprom(int unit, UINT16 offset, UINT16 words, UINT16 *data);


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

    if(boardType == DEV_ID_82541PI){ 
      geiPciResources[geiUnits].boardType = DEV_ID_82541PI;
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

    /*for 82541PI*/
    if(boardType == DEV_ID_82541PI)
        {
        sysGei541InitParam(geiUnits, boardType);
        }
    else
        {
        /* disable sleep mode */

        pciConfigOutByte (pciBus, pciDevice, pciFunc, PCI_CFG_MODE,
                      SLEEP_MODE_DIS);
        }
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
* sys82543BoardInit - Adaptor initialization for 8254x chip
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

    if(pRsrc->boardType == DEV_ID_82541PI)
        {
        pReso = &geiResources[unit];
        }
    else
        {
        pReso = (GEI_RESOURCE *)(pRsrc->pExtended);
        }

    if (pRsrc->boardType != PRO1000_543_BOARD && 
        pRsrc->boardType != PRO1000_544_BOARD &&
        pRsrc->boardType != PRO1000_546_BOARD &&
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
              lanB = (pRsrc->pciFunc == 1)? TRUE : FALSE;

            else if (pRsrc->pciFunc != 0)
             {
                printf ("Error in detecting 82546 dual port: header type =%2d, 
                         pci func=%2d\n", (UINT32)headerType, (UINT32)(pRsrc->pciFunc));
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
        sysGei541ReadEeprom(unit, EEPROM_ICW1, 1, &(geiResources[unit].eeprom_icw1));

        /* get the initialization control word 2 (ICW2) in EEPROM */
        sysGei541ReadEeprom(unit, EEPROM_ICW2, 1, &(geiResources[unit].eeprom_icw2));
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

    if(pReso->boardType == DEV_ID_82541PI)
        {
        /*pBoard->vector      = pRsrc->irq;*/
        pBoard->regBaseHigh = PCI_MEMIO2LOCAL(pReso->memBaseHigh);
        }
    else
        {    
    pBoard->regBaseHigh = pReso->memBaseHigh;
        }
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
     */

    pBoard->phyDelayRtn = (FUNCPTR) taskDelay;
    pBoard->phyMaxDelay = MII_PHY_DEF_DELAY;
    pBoard->phyDelayParm = 5; 

    /* BSP/adapter specific
     * set the PHY address if you know it, otherwise set to zero
     * INTEL 82540/4/5/6-based adapters have a built-in phy with Addr of 1 
     */

    pBoard->phyAddr = (pRsrc->boardType == PRO1000_544_BOARD || 
                       pRsrc->boardType == PRO1000_546_BOARD)? 1 : 0;

    if (pReso->boardType == DEV_ID_82541PI)
        {
            pBoard->phyAddr = 1;
            /*printf ("DEV_ID_82541PI, phyaddr: %d\n", pBoard->phyAddr);*/
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
     * call back functions perform system physcial memory mapping in the PCI 
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
        pBoard->eeprom_icw1   = geiResources[unit].eeprom_icw1;    
        pBoard->eeprom_icw2   = geiResources[unit].eeprom_icw2;
        }
    else
        {
        pBoard->eeprom_icw1   = pReso->eeprom_icw1;
        pBoard->eeprom_icw2   = pReso->eeprom_icw2;
        }
    /* copy Ether address */

    memcpy (&pBoard->enetAddr[0], &(pReso->enetAddr[0]), 
            ETHER_ADDRESS_SIZE);

    /* we finish adaptor initialization */

    pReso->iniStatus = OK;

    /* enable adaptor interrupt */

    sysIntEnablePIC (pRsrc->irq);

    return (OK);
    }

/*************************************************************************
*
* sysGei82546DynaTimerSetup - setup device internal timer value dynamically
* 
* This routine will be called every 2 seconds by default if GEI_END_SET_TIMER
* flag is set. The available timers to adjust include RDTR(unit of ns), 
* RADV(unit of us), and ITR(unit of 256ns). Based on CPU's and/or tasks' 
* usuage on system, user can tune the device's performace dynamically. 
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
     * 1: CPU usuage on system and/or,
     * 2: specific application task's usuage and/or,
     * 3: RX/TX packet processing per second, and/or 
     * 4: RX/TX interrupt counter per second, and/or
     * 5: RX packet processing for each calling gei82543RxTxIntHandle(),
     * users can choose optimal timer values from a predefined table to 
     * reduce interrupt rates. The statistic of 3,4,and 5 items above 
     * may be obtained from pBoard->devDrvStat. 
     *
     * NOTE:
     * ITR:  Interrupt throttling register (unit of 256ns) 
     *       inter-interupt delay between chip's interrupts   
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
  
    /* retuen TRUE; */

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
                (UINT16*)adr);

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

    /*for 82541GI*/
    for (ix = 0; ix < EEPROM_WORD_SIZE; ix++)
        if(geiResources[unit].boardType == DEV_ID_82541PI)
            {
             /*printf("543checksum for 82541GI,unit:%d\n",unit);*/
            sysGei541ReadEeprom(unit, EEPROM_IA_ADDRESS + ix, 1, &checkSumTmp);
            checkSum += checkSumTmp;
            }
        else
            {
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

    /* Intel Pro1000T adaptor uses Alaska transceiver */

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
    return (OK);
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
    return (OK);
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
        printf ("********* Intel 82540/82545/82546EB based Adapter ********\n");
    else
        printf ("********* UNKNOWN Adaptor ************ \n");
 
    printf ("  CSR PCI Membase address = 0x%x\n", pReso->memBaseLow);
 
    printf ("  Flash PCI Membase address = 0x%x\n", pReso->flashBase);

    printf ("  PCI bus no.= 0x%x, device no.= 0x%x, function no.= 0x%x\n", 
             pRsrc->pciBus, pRsrc->pciDevice, pRsrc->pciFunc);

    printf ("  IRQ = %d\n", pRsrc->irq);  

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
            case PRO1000_545_PCI_DEVICE_ID_XT:
            case PRO1000_546_PCI_DEVICE_ID_XT:
            case PRO1000_545_PCI_DEVICE_ID_MF:
            case PRO1000_546_PCI_DEVICE_ID_MF:
                return (PRO1000_546_BOARD);
            case DEV_ID_82541PI:
                return DEV_ID_82541PI;
            }
        }

    return (BOARD_TYPE_UNKNOWN);
    }


/*******************************************************************************
* sysGei541InitParam - Initialize the eeprom struct.
*
* DESCRIPTION:
*        This function initialize the eeprom struct according to the MAC type.
*
* INPUT:
*        unit     - Device unit number.
*        deviceId - The device ID as defined in PCI configuration.
*
* OUTPUT:
*        None.
*
* RETURN:
*        None.
*
*******************************************************************************/
void sysGei541InitParam(int unit, int deviceId)
{
    EEPROM *eeprom = &geiResources[unit].eeprom;
    
    UINT32 eecd = GEI_SYS_READ_REG(unit, INTEL_82543GC_EECD);
    if (eecd & E1000_EECD_TYPE) 
    {
        eeprom->type = EEPROM_SPI;
        eeprom->opcode_bits = 8;
        eeprom->delay_usec = 1;
        if (eecd & E1000_EECD_ADDR_BITS) 
        {
            eeprom->page_size = 32;
            eeprom->address_bits = 16;
        } 
        else 
        {
            eeprom->page_size = 8;
            eeprom->address_bits = 8;
        }
    } 
    else 
    {
        eeprom->type = EEPROM_MICROWIRE;
        eeprom->opcode_bits = 3;
        eeprom->delay_usec = 50;
        if (eecd & E1000_EECD_ADDR_BITS) 
        {
            eeprom->address_bits = 8;
        } 
        else 
        {
            eeprom->address_bits = 6;
        }
    }
}

/*******************************************************************************
* sysGei541RaiseClk - Raises the EEPROM's clock input.
*
* DESCRIPTION:
*        To generate eeprom read/write, its clock input pins should be toggled. 
*        This function raises the EEPROM's clock input.
*
* INPUT:
*        unit - Device unit number.
*        eecd - The device eeprom structure.
*
* OUTPUT:
*        None.
*
* RETURN:
*        None.
*
*******************************************************************************/
static void sysGei541RaiseClk(int unit, UINT32 *eecd)
{
    EEPROM *eeprom = &geiResources[unit].eeprom;

    /* Raise the clock input to the EEPROM (by setting the SK bit), and then
     * wait <delay> microseconds.
     */
    *eecd = *eecd | E1000_EECD_SK;
    GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, *eecd);
    GEI_SYS_WRITE_FLUSH(unit);
    sysUsDelay(eeprom->delay_usec);
}

/*******************************************************************************
* sysGei541LowerClk - Lowers the EEPROM's clock input.
*
* DESCRIPTION:
*       To generate eeprom read/write, its clock input pins should be toggled. 
*       This function lowers the EEPROM's clock input.
*
* INPUT:
*       unit - Device unit number.
*       eecd - The device eeprom structure.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
static void sysGei541LowerClk(int unit, UINT32 *eecd)
{
    EEPROM *eeprom = &geiResources[unit].eeprom;

    /* Lower the clock input to the EEPROM (by clearing the SK bit), and then
     * wait 50 microseconds.
     */
    *eecd = *eecd & ~E1000_EECD_SK;
    GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, *eecd);
    GEI_SYS_WRITE_FLUSH(unit);
    sysUsDelay(eeprom->delay_usec);
}

/*******************************************************************************
* sysGei541ShiftOutBits - Read data bits out to the EEPROM.
*
* DESCRIPTION:
*       To generate eeprom write, a binary value is written bit by bit. 
*       A value of "1" is written to the eeprom by setting bit "DI" to a "1",
*       and then raising and then lowering the clock.
*       A value of "0" is written to the eeprom by setting bit "DI" to a "0", 
*       and then raising and then lowering the clock.
*
* INPUT:
*       unit  - Device unit number.
*       data  - Data to write.
*       count - Number of bits to write.
*
* OUTPUT:
*      None.
*
* RETURN:
*      None.
*
*******************************************************************************/
static void sysGei541ShiftOutBits(int unit, UINT16 data, UINT16 count)
{
    EEPROM *eeprom = &geiResources[unit].eeprom;
    UINT32 eecd;
    UINT32 mask;

    /* We need to shift "count" bits out to the EEPROM. So, value in the
     * "data" parameter will be shifted out to the EEPROM one bit at a time.
     * In order to do this, "data" must be broken down into bits.
     */
    mask = 0x01 << (count - 1);
    eecd = GEI_SYS_READ_REG(unit, INTEL_82543GC_EECD);

    if (eeprom->type == EEPROM_MICROWIRE) {
        eecd &= ~E1000_EECD_DO;
    } else if (eeprom->type == EEPROM_SPI) {
        eecd |= E1000_EECD_DO;
    }
    do {
        /* A "1" is shifted out to the EEPROM by setting bit "DI" to a "1",
         * and then raising and then lowering the clock (the SK bit controls
         * the clock input to the EEPROM).  A "0" is shifted out to the EEPROM
         * by setting "DI" to "0" and then raising and then lowering the clock.
         */
        eecd &= ~E1000_EECD_DI;

        if(data & mask)
            eecd |= E1000_EECD_DI;

        GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, eecd);
        GEI_SYS_WRITE_FLUSH(unit);

        sysUsDelay(eeprom->delay_usec);

        sysGei541RaiseClk(unit, &eecd);
        sysGei541LowerClk(unit, &eecd);

        mask = mask >> 1;

    } while(mask);

    /* We leave the "DI" bit set to "0" when we leave this routine. */
    eecd &= ~E1000_EECD_DI;
    GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, eecd);
}

/*******************************************************************************
* sysGei541ShiftInBits - Read data bits from the EEPROM
*
* DESCRIPTION:
*       In order to read a register from the EEPROM, we need to shift 'count'
*       bits in from the EEPROM. Bits are "shifted in" by raising the clock
*       input to the EEPROM, and then reading the value of the "DO" bit.  
*
* INPUT:
*       unit  - Device unit number.
*       count - Number of bits to write.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Up to 16 bit read data
*
*******************************************************************************/
static UINT16 sysGei541ShiftInBits(int unit, UINT16 count)
{
    UINT32 eecd;
    UINT32 i;
    UINT16 data;

    /* In order to read a register from the EEPROM, we need to shift 'count'
     * bits in from the EEPROM. Bits are "shifted in" by raising the clock
     * input to the EEPROM (setting the SK bit), and then reading the value of
     * the "DO" bit.  During this "shifting in" process the "DI" bit should
     * always be clear.
     */

    eecd = GEI_SYS_READ_REG(unit, INTEL_82543GC_EECD);

    eecd &= ~(E1000_EECD_DO | E1000_EECD_DI);
    data = 0;

    for(i = 0; i < count; i++) 
    {
        data = data << 1;
        sysGei541RaiseClk(unit, &eecd);

        eecd = GEI_SYS_READ_REG(unit, INTEL_82543GC_EECD);

        eecd &= ~(E1000_EECD_DI);
        if(eecd & E1000_EECD_DO)
            data |= 1;

        sysGei541LowerClk(unit, &eecd);
    }

    return data;
}

/*******************************************************************************
* sysGei541AcquireEeprom - Prepares EEPROM for access
*
* DESCRIPTION:
*       This function prepares the eeprom for accesses:
*       Lowers EEPROM clock. 
*       Clears input pin. 
*       Sets the chip select pin. 
*       This function should be called before issuing a command to the EEPROM.
*
* INPUT:
*       unit  - Device unit number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
static STATUS sysGei541AcquireEeprom(int unit)
{
    EEPROM *eeprom = &geiResources[unit].eeprom;
    UINT32 eecd, i=0;

    eecd = GEI_SYS_READ_REG(unit, INTEL_82543GC_EECD);

    /* Request EEPROM Access */
        eecd |= E1000_EECD_REQ;
        GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, eecd);
        eecd = GEI_SYS_READ_REG(unit, INTEL_82543GC_EECD);
        while((!(eecd & E1000_EECD_GNT)) &&
              (i < E1000_EEPROM_GRANT_ATTEMPTS)) {
            i++;
            sysUsDelay(5);
            eecd = GEI_SYS_READ_REG(unit, INTEL_82543GC_EECD);
        }
        if(!(eecd & E1000_EECD_GNT)) {
            eecd &= ~E1000_EECD_REQ;
            GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, eecd);
            /*mvOsPrintf("Could not acquire EEPROM grant\n");*/
            printf("Could not acquire EEPROM grant\n");
            return ERROR;
        }

    /* Setup EEPROM for Read/Write */

    if (eeprom->type == EEPROM_MICROWIRE) {
        /* Clear SK and DI */
        eecd &= ~(E1000_EECD_DI | E1000_EECD_SK);
        GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, eecd);

        /* Set CS */
        eecd |= E1000_EECD_CS;
        GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, eecd);
    } else if (eeprom->type == EEPROM_SPI) {
        /* Clear SK and CS */
        eecd &= ~(E1000_EECD_CS | E1000_EECD_SK);
        GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, eecd);
        sysUsDelay(1);
    }

    return OK;
}

/*******************************************************************************
* sysGei541StandbyEeprom - Returns EEPROM to a "standby" state
*
* DESCRIPTION:
*       This function returns the EEPROM to a "standby" state.
*
* INPUT:
*       unit  - Device unit number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
static void sysGei541StandbyEeprom(int unit)
{
    EEPROM *eeprom = &geiResources[unit].eeprom;
    UINT32 eecd;

    eecd = GEI_SYS_READ_REG(unit, INTEL_82543GC_EECD);

    if(eeprom->type == EEPROM_MICROWIRE) {
        eecd &= ~(E1000_EECD_CS | E1000_EECD_SK);
        GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, eecd);
        GEI_SYS_WRITE_FLUSH(unit);
        sysUsDelay(eeprom->delay_usec);

        /* Clock high */
        eecd |= E1000_EECD_SK;
        GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, eecd);
        GEI_SYS_WRITE_FLUSH(unit);
        sysUsDelay(eeprom->delay_usec);

        /* Select EEPROM */
        eecd |= E1000_EECD_CS;
        GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, eecd);
        GEI_SYS_WRITE_FLUSH(unit);
        sysUsDelay(eeprom->delay_usec);

        /* Clock low */
        eecd &= ~E1000_EECD_SK;
        GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, eecd);
        GEI_SYS_WRITE_FLUSH(unit);
        sysUsDelay(eeprom->delay_usec);
    } else if(eeprom->type == EEPROM_SPI) {
        /* Toggle CS to flush commands */
        eecd |= E1000_EECD_CS;
        GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, eecd);
        GEI_SYS_WRITE_FLUSH(unit);
        sysUsDelay(eeprom->delay_usec);
        
        eecd &= ~E1000_EECD_CS;
        GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, eecd);
        GEI_SYS_WRITE_FLUSH(unit);
        sysUsDelay(eeprom->delay_usec);
    }
}

/*******************************************************************************
* sysGei541ReleaseEeprom - Terminates a command.
*
* DESCRIPTION:
*       This function Terminates a command by inverting the EEPROM's 
*       chip select pin.
*
* INPUT:
*       unit  - Device unit number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
static void sysGei541ReleaseEeprom(int unit)
{
    EEPROM *eeprom = &geiResources[unit].eeprom;
    UINT32 eecd;

    eecd = GEI_SYS_READ_REG(unit, INTEL_82543GC_EECD);

    if (eeprom->type == EEPROM_SPI) {
        eecd |= E1000_EECD_CS;  /* Pull CS high */
        eecd &= ~E1000_EECD_SK; /* Lower SCK */

        GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, eecd);

        sysUsDelay(eeprom->delay_usec);
    } else if(eeprom->type == EEPROM_MICROWIRE) {
        /* cleanup eeprom */

        /* CS on Microwire is active-high */
        eecd &= ~(E1000_EECD_CS | E1000_EECD_DI);

        GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, eecd);

        /* Rising edge of clock */
        eecd |= E1000_EECD_SK;
        GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, eecd);
        GEI_SYS_WRITE_FLUSH(unit);
        sysUsDelay(eeprom->delay_usec);

        /* Falling edge of clock */
        eecd &= ~E1000_EECD_SK;
        GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, eecd);
        GEI_SYS_WRITE_FLUSH(unit);
        sysUsDelay(eeprom->delay_usec);
    }

    /* Stop requesting EEPROM access */
    eecd &= ~E1000_EECD_REQ;
    GEI_SYS_WRITE_REG(unit, INTEL_82543GC_EECD, eecd);
}

/******************************************************************************
 * Reads a 16 bit word from the EEPROM.
 *
 * hw - Struct containing variables accessed by shared code
 * offset - offset of  word in the EEPROM to read
 * data - word read from the EEPROM
 * words - number of words to read
 *****************************************************************************/
/*******************************************************************************
* sysGei541ReadEeprom - Reads a 16 bit word from the EEPROM.
*
* DESCRIPTION:
*       This function reads 16 bit word from the EEPROM.
*
* INPUT:
*       unit   - Device unit number.
*       offset - offset within the eeprom.
*       words  - Number of words to read.
*
* OUTPUT:
*       data   - Pointer to buffer that holds the read result.
*
* RETURN:
*       OK if reading succeded.
*
*******************************************************************************/
STATUS sysGei541ReadEeprom(int unit, UINT16 offset, UINT16 words, UINT16 *data)
{
    EEPROM *eeprom = &geiResources[unit].eeprom;
    UINT32 i = 0;

    /* Prepare the EEPROM for reading  */
    if(sysGei541AcquireEeprom(unit) != OK)
        return ERROR;

    for (i = 0; i < words; i++)
	{
        /* Send the READ command (opcode + addr)  */
        sysGei541ShiftOutBits(unit, EEPROM_READ_OPCODE_MICROWIRE,
                                 eeprom->opcode_bits);
        sysGei541ShiftOutBits(unit, (unsigned short)(offset + i),
                                eeprom->address_bits);

        /* Read the data.  For microwire, each word requires the overhead
         * of eeprom setup and tear-down. */
        data[i] = sysGei541ShiftInBits(unit, 16);
        sysGei541StandbyEeprom(unit);
    }

    /* End this read operation */
    sysGei541ReleaseEeprom(unit);
    return OK;
}

void sys543geiread(int unit, int reg){
  UINT32 eecd;
  
  eecd = GEI_SYS_READ_REG(unit, reg);

  printf("unit:%d,offset:%d,value:%8.8X\n", unit, reg, eecd);
}

void sys543geiwrite(int unit, int reg, UINT32 value){
    GEI_SYS_WRITE_REG(unit, reg, value);

}

void sys543geiphyread(int unit, int phyReg){

    int             count = 6;    /* counter */
    volatile UINT32 mdicRegVal;

    UINT16 data;
    
    mdicRegVal = (MDI_READ_BIT | 1 << MDI_PHY_SHIFT | 
                  phyReg << MDI_REG_SHIFT);
 
    GEI_SYS_WRITE_REG(unit, INTEL_82543GC_MDI, mdicRegVal);

    sysMsDelay (32); /* wait 32 microseconds */

    while (count--)                  /* wait max 96 microseconds */
        {
        mdicRegVal = GEI_SYS_READ_REG(unit, INTEL_82543GC_MDI);

        if (mdicRegVal & MDI_READY_BIT)
            break;
        
        sysMsDelay (16);
        }
     
    if ((mdicRegVal & (MDI_READY_BIT | MDI_ERR_BIT)) != MDI_READY_BIT)
        {
        /*DRV_LOG (DRV_DEBUG_LOAD, "Error: MII read PhyAddr=%d, phyReg=%d\n...",
               phyAddr, phyReg, 3, 4, 5, 6);*/
        }

    /**data = (UINT16) mdicRegVal;*/
        data = (UINT16)mdicRegVal;
    printf("unit:%d, offset:%d, value:%x\n", unit, phyReg, mdicRegVal);
        
}

#endif /* INCLUDE_GEI8254X_END */
