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
 * Provides the I/O routines to interface with either a legacy ISA-based FPGA or the
 * PCI-based CCA boards introduced with E-Box 2016.
 *
 */
#ifndef HW_INTF_H_
#define HW_INTF_H_

/* These typedefs match the input types for sysLib's I/O interface */
typedef ULONG       HwPortId;
typedef int         HwPortReg;
typedef char        HwByte;
typedef short int   HwWord;
typedef long int    HwLong;

/* Sentinel value for GetPortRegister() use */
enum {HwPortReg_INVALID = 0xDEADBEEF};

typedef struct _HwInterfaceImpl
{
   /*
    * Function pointer that decodes a port Id and returns the address to read/write
    * Returns HwPortReg_INVALID if the input can't be decoded
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

   BOOL isValid;
} HwInterfaceImpl;

/**
 * Initialize the interface, providing the necessary implementation functions.
 * Drivers call this after determining which driver interface (ISA or PCI) is applicable.
 * Returns TRUE if successful; otherwise, FALSE;
 */
BOOL hwInitInterface(HwInterfaceImpl *pImpl);

UCHAR  hwInByte(HwPortId portId);
USHORT hwInWord(HwPortId portId);
ULONG  hwInLong(HwPortId portId);

void hwOutByte(HwPortId portId, HwByte data);
void hwOutWord(HwPortId portId, HwWord data);
void hwOutLong(HwPortId portId, HwLong data);

#endif /* HW_INTF_H_ */
