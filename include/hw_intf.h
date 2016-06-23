/**
 * Copyright (C) 2016 Terumo BCT, Inc.  All rights reserved.
 *
 * @file   hw_intf.h
 *
 * @brief  Common interface between hardware drivers and hardware FPGA/CCA boards.
 *
 * @author mra1usb
 * @date   May 2, 2016
 *
 * Provides the I/O routines that low-level drivers can use interface with either a
 * legacy ISA-based FPGA or the PCI-based CCA boards introduced with E-Box 2016.
 *
 */
#ifndef HW_INTF_H_
#define HW_INTF_H_

#ifdef __cplusplus
extern "C" {
#endif

/* These typedefs match the input types for sysLib's I/O interface */
typedef ULONG       HwPortId;
typedef int         HwPortReg;
typedef char        HwByte;
typedef short int   HwWord;
typedef long int    HwLong;

/* Sentinel value for GetPortRegister() use */
enum {HwPortReg_NA = 0xFFFFFFFF};

typedef struct _HwInterfaceImpl
{
   /*
    * Function pointer that returns the port register mapped to a portId.
    * Returns HwPortReg_NA if the portId can't be decoded or is not applicable.
    */
   HwPortReg (*GetPortRegister)(HwPortId);

   /* Function pointers to read input from a port register */
   UCHAR  (*InByte)(HwPortReg);
   USHORT (*InWord)(HwPortReg);
   ULONG  (*InLong)(HwPortReg);

   /* Function pointers to write output to a port register */
   void (*OutByte)(HwPortReg, HwByte);
   void (*OutWord)(HwPortReg, HwWord);
   void (*OutLong)(HwPortReg, HwLong);

   const char* name;
   int         numPorts;
} HwInterfaceImpl;

/**
 * Initialize the interface, providing the necessary implementation functions.
 * Drivers call this after determining which driver interface (ISA or PCI) is applicable.
 * Returns TRUE if successful; otherwise, FALSE;
 */
BOOL hwInitInterface(const HwInterfaceImpl* pImpl);

/**
 * Returns the register mapped to portId for the implementation.
 */
HwPortReg hwGetPortRegister(HwPortId portId);

/**
 * Hardware interface routines to read the register mapped to a portId.
 */
UCHAR  hwInByte(HwPortId portId);
USHORT hwInWord(HwPortId portId);
ULONG  hwInLong(HwPortId portId);

/**
 * Hardware output routines to write the register mapped to a portId.
 */
void hwOutByte(HwPortId portId, HwByte data);
void hwOutWord(HwPortId portId, HwWord data);
void hwOutLong(HwPortId portId, HwLong data);

/**
 * Utility for printing the port register mapping.
 * Returns the number of entries in the port map.
 */
int hwShowPortMap(void);

#ifdef __cplusplus
}
#endif

#endif /* HW_INTF_H_ */
