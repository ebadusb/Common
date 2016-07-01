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
   CFG_STAT_ADDR            = IOPW_CFG_STAT,
   //
   // Session Trigger (16-bit, read/write)
   //
   IOPW_SES_TRIG            = 0x08,
   SES_TRIG_ADDR            = IOPW_SES_TRIG,
   //
   // Session User Strobe Light Pulse Width (8-bit, read/write)
   //
   IOP_SES_U_PW             = 0x0C,
   SES_U_PW_ADDR            = IOP_SES_U_PW,
   //
   // Filler position register (12-bit, read only)
   //
   INPW_FILLER_POS          = 0x10,
   FILLER_POS_ADDR          = INPW_FILLER_POS,
   //
   // Filler Rotation Time (15-bit, read only)
   //
   INPW_FILLER_ROT_TIME     = 0x14,
   FILLER_ROT_TIME_ADDR     = INPW_FILLER_ROT_TIME,
   //
   // Encoder Emulator (16-bit, read/write)
   //
   IOPW_ENC_EMU             = 0x18,
   ENC_EMU_ADDR             = IOPW_ENC_EMU,
   //
   // Miscellaneous Status (1-bit, reset on read)
   //
   INP_MISC_STATUS          = 0x1C

} USTC_BAR_Offset;


/**
 * Bit masks for USTCP CCA registers
 */
typedef enum
{
   //
   // Reset value for OUT_SW_RESET
   //
   USTC_CCA_RESET_VALUE       = 0xFF00AA55,
   //
   // bit masks for CFG_STAT
   //
   CFG_STAT_USE_ENCODER_VALUE = 0x7070,   // set Encoder bit ON
   CFG_STAT_USE_HALLS_VALUE   = 0x7030,   // set Hall bit ON
   CFG_STAT_DISABLE_VALUE     = 0x7050,   // set User Strobe Pulse OFF
   //
   // bit masks for ENC_EMU
   //
   ENC_EMU_ENABLE_VALUE     = 0x0300,
   ENC_EMU_DISABLE_VALUE    = 0x0100

} USTCP_BitMasks;


#endif /* CCA_PCI_USTC_H_ */

