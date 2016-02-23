/**
 * This file defines the TerumoBCT CCA PCI hardware initialization routines.
 *
 * These routines detect TerumoBCT CCA cards in the system and maps them
 * into memory.  Note that there must be a sufficient number of
 * MMU_DUMMY_ENTRY entries in the sysMmuMapTable
 *
 */

#ifndef _CCA_PCI_SUPPORT_INCLUDE_
#define _CCA_PCI_SUPPORT_INCLUDE_

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
 */
STATUS sysCCAHwInit (void);

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
UINT8  ccaInByte(UINT8 offset, UINT rsrcIndx, BOOL useBar1);
UINT16 ccaInWord(UINT8 offset, UINT rsrcIndx, BOOL useBar1);

/**
 * Write value to the specified resource. Offset is applied either to BAR1 or BAR0.
 */
void ccaOutByte(UINT8 offset, UINT8  value, UINT rsrcIndx, BOOL useBar1);
void ccaOutWord(UINT8 offset, UINT16 value, UINT rsrcIndx, BOOL useBar1);


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

/**
 * Simple wrapper class to do read/write operations on a CCA PCI resource.
 */
class CcaInOut
{
public:

   /** Constructor for a specified resource for either BAR1 or BAR0 */
   CcaInOut(int index=0, bool useBar1=false) : pBar(0)
   {
      ccaPciResources rsrc;
      if (OK == ccaPciGetResource(index, &rsrc))
         pBar = (UINT8*)(useBar1 ? rsrc.pBAR1 : rsrc.pBAR1);
      else
         abort();
   }

   UINT8  InByte(UINT8 offset) { return pBar[offset]; }
   UINT16 InWord(UINT8 offset) { return *(UINT16*)(pBar+offset); }

   void	OutByte(UINT8 offset, UINT8  value) { pBar[offset] = value; }
   void OutWord(UINT8 offset, UINT16 value) { *(UINT16*)(pBar+offset) = value; }

private:
   UINT8 * pBar;
};

#endif /* __cplusplus */

#endif /* #ifndef _CCA_PCI_SUPPORT_INCLUDE_ */
