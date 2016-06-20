/**
 * This file defines the TerumoBCT CCA PCI hardware interface routines.
 *
 * These routines detect TerumoBCT CCA cards in the system and maps them
 * into memory.  Note that there must be a sufficient number of
 * MMU_DUMMY_ENTRY entries in the sysMmuMapTable
 *
 */

#ifndef _CCA_PCI_INTF_INCLUDE_
#define _CCA_PCI_INTF_INCLUDE_

/*
 * Resources: Safety_P, Ctrl3, USTC_P, STCPCI cards.
 * There is a maximum of 2 CCAs per processor
 */
#define CCA_MAX_PCI_RESOURCES         2
#define CCA_PCI_DEVICE_CLASS          0x000000  /* BEFORE_STD */

/*
 * Memory map sizes for CCAs must be page-aligned.  Accordingly sysMmuMapAdd
 * will round up these lengths to a multiple of VM_PAGE_SIZE bytes
 */
#define CCA_PCI_SAFETYP_MMAP_SIZE     0x0040   /* 64  bytes   BAR 0 */
#define CCA_PCI_SAFETYP_PFR_MMAP_SIZE 0x7D000  /* 512 Kbytes  BAR 1 */
#define CCA_PCI_USTCP_MMAP_SIZE       0x0020   /* 32  bytes   BAR 0 */
#define CCA_PCI_USTCP_PFR_MMAP_SIZE   0x7D000  /* 512 Kbytes  BAR 1 */
#define CCA_PCI_CTRL3_MMAP_SIZE       0x0080   /* 128 bytes */
#define CCA_PCI_STCPCI_MMAP_SIZE      0x0100   /* 256 bytes */

/* Vendor IDs */
#define CCA_PCI_VID                   0x11AA   /* Vendor ID for TerumoBCT CCA cards */

/* Device IDs */
#define CCA_PCI_CTRL3_DEV_ID          0x20
#define CCA_PCI_SAFETYP_DEV_ID        0x21
#define CCA_PCI_STCPCI_DEV_ID         0x22
#define CCA_PCI_USTCP_DEV_ID          0x23

/* CCA Detect Error Codes */
#define CCA_DEVICE_OK                     0x00
#define CCA_DEVICE_CLASS_NOT_FOUND        0x01
#define CCA_DEVICE_BAR0_FAIL              0x02
#define CCA_DEVICE_BAR1_FAIL              0x03
#define CCA_DEVICE_MEM_MAP_FAIL           0x04
#define CCA_DEVICE_ENABLE_DECODER_FAIL    0x05
#define CCA_DEVICE_VENDID_FAIL            0x06
#define CCA_DEVICE_DEVID_FAIL             0x07
#define CCA_DEVICE_NOT_FOUND              0x08
#define CCA_DEVICE_FOUND                  0x09
#define CCA_DEVICE_INIT_STATE             0x0A

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ccaPciResourcesStruct
{
   int           vendorId;          /* PCI vendor ID */
   int           deviceId;          /* PCI device ID */
   int           subsystemId;       /* PCI subsystem ID */
   int           revisionId;        /* PCI revision ID */
   int           busNo;             /* PCI bus number */
   int           deviceNo;          /* PCI device number */
   int           funcNo;            /* PCI function number */
   unsigned char ipin;              /* PCI interrupt pin -- should always be zero for our CCAs */
   void*         pBAR0;             /* pointer to register base address */
   void*         pBAR1;             /* pointer to register base address */
   int           statusCode;        /* log code if unable to detect or configure */
} ccaPciResources;

/* Prototypes */

/**
 * Initializes the CCA Hardware.
 * Returns OK on success, ERROR upon failure or if no resources were found.
 */
STATUS sysCCAHwInit (void);

/**
 * Returns number of CCA resources available and initialized.
 */
unsigned int ccaPciResourcesAvailable ();

/**
 * Displays a list of the of detected CCA resources.
 */
void ccaPciShow (void);

/**
 * Get a copy of the specified PCI resource.
 */
STATUS ccaPciGetResource (UINT rsrcIndx, ccaPciResources* pResource);

/**
 * Read data from the specified resource. Offset is applied either to BAR1 or BAR0.
 */
UINT32 ccaPciIn32 (UINT8 offset, UINT rsrcIndx, BOOL useBar1);

/**
 * Write data to the specified resource. Offset is applied either to BAR1 or BAR0.
 */
void ccaPciOut32 (UINT8 offset, UINT32 value, UINT rsrcIndx, BOOL useBar1);

/* These typedefs match the input types for sysLib's I/O interface */
typedef int CcaIoPort;
typedef char CcaByte;
typedef short int CcaWord;
typedef long int CcaLong;


#define CCA_RSRC_MASK   0x01    /* CCA_MAX_PCI_RESOURCES - 1 */
#define CCA_BAR_MASK    0x01
#define CCA_OFFSET_MASK 0xFF

/**
 * Helper macros to compose a CcaIoPort and/or extract its components
 */
#define CCA_IO_PORT(rsrc, bar, offset)      \
   (((rsrc & CCA_RSRC_MASK) << 16)|((bar & CCA_BAR_MASK) << 8)|(offset & 0xFF))
#define CCA_IO_PORT_RSRC_INDX(ccaIoPort)    ((ccaIoPort >> 16) & CCA_RSRC_MASK)
#define CCA_IO_PORT_BAR_INDX(ccaIoPort)     ((ccaIoPort >>  8) & CCA_BAR_MASK)
#define CCA_IO_PORT_OFFSET(ccaIoPort)       (ccaIoPort & CCA_OFFSET_MASK)

/**
 * I/O functions to read/Write data to/from a specified CCA resource, where:
 * - barIdWithOffset = CCA_IO_PORT(rsrcIndx, barIndx, barOffset)
 *
 * - rsrcIndx  : index to one of the CCA resources [0, CCA_MAX_PCI_RESOURCES-1]
 * - barIndx   : selects base address register [0, 1]
 * - barOffset : offset added to the base address register [0, 0xFF]
 *
 * These functions match the prototypes of the sysLib interface:
 * sysInByte(), sysInWord(), sysInLong(), sysOutByte(), sysOutWord(), sysOutLong()
 *
 * See the helper macro CCA_IO_PORT().
 *
 */
UCHAR  ccaInByte (CcaIoPort barIdWithOffset);
USHORT ccaInWord (CcaIoPort barIdWithOffset);
ULONG  ccaInLong (CcaIoPort barIdWithOffset);
void   ccaOutByte (CcaIoPort barIdWithOffset, CcaByte data);
void   ccaOutWord (CcaIoPort barIdWithOffset, CcaWord data);
void   ccaOutLong (CcaIoPort barIdWithOffset, CcaLong data);


#ifdef __cplusplus
}
#endif

#endif /* _CCA_PCI_INTF_INCLUDE_ */
