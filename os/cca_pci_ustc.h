/**
 * Copyright (C) 2016 Terumo BCT, Inc.  All rights reserved.
 *
 * @file   cca_pci_ustc.h
 *
 * @brief  Register map for PCI interface of Control board introduced with EBox-2016.
 *
 * Reference the Optia USTC FPGA Design Specification (Chuck Mangan).
 *
 */

#ifndef CCA_PCI_USTC_H_
#define CCA_PCI_USTC_H_

/**
 * USTC CCA Register Map.
 *
 * Base Address Register (BAR) Offsets for USTC registers
 */
typedef enum
{
   //
   // Software Reset and CCA ID (32-bit, write-only / read-only)
   //
   OUT_SW_RESET             = 0x00, // Write 0xFF00AA55 to force reset of cca
   INP_CCA_ID               = 0x00, // Read to get CCA ID pins cca_id[15:0] 
   //
   // Configuration and status register (16-bit, read/write)
   //
   IOPW_CFG_STAT            = 0x04, 
   //
   // Session Trigger (16-bit, read/write)
   //
   IOPW_SES_TRIG            = 0x08,
   //
   // Session User Strobe Light Pulse Width (8-bit, read/write)
   //
   IOP_SES_U_PW             = 0x0C,
   //
   // Filler position register (12-bit, read only)
   //
   INPW_FILLER_POS          = 0x10,
   //
   // Filler Rotation Time (15-bit, read only)
   //
   INPW_FILLER_ROT_TIME     = 0x14,
   //
   // Encoder Emulator (16-bit, read/write)
   //
   IOPW_ENC_EMU             = 0x18,
   //
   // Miscellaneous Status (1-bit, reset on read)
   //
   INP_MISC_STATUS          = 0x1C

} USTC_BAR_Offset;



#endif /* CCA_PCI_USTC_H_ */

