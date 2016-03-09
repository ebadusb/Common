/**
 * Copyright (C) 2016 Terumo BCT, Inc.  All rights reserved.
 *
 * @file   cca_pci_safety.h
 *
 * @author mra1usb
 * @date   Feb 16, 2016
 *
 * @brief  Register map for PCI interface of Safety board introduced with EBox-2016.
 *
 * Reference the Trima/Optia Safety-PFR FPGA Design Specification (Chuck Mangan).
 *
 */

#ifndef CCA_PCI_SAFETY_H_
#define CCA_PCI_SAFETY_H_

/**
 * SafetyP CCA Register Map.
 *
 * Base Address Register (BAR) Offsets for Safety registers
 */
typedef enum
{
   //
   // Software Reset and CCA ID (32-bit, write-only / read-only)
   //
   OUT_SW_RESET             = 0x00, // Write 0xFF00AA55 to force reset
   INP_CCA_ID               = 0x00, // Read to get CCA ID pins cca_id[15:0]
   //
   // Watchdog Pet (write-only) and Status (read-only) registers
   //
   OUTP_WATCHDOG_F0         = 0x04,
   OUTP_WATCHDOG_A5         = 0x08,
   INP_WATCHDOG_STATUS      = 0x20,
   INP_WATCHDOG_CHECK       = 0x24,
   //
   // Counters for Lower Level Air & Fluid Sample & Centrifuge Commutation (8-bit)
   //
   INP_LL_AIR_COUNT         = 0x0C, // Ultra-sonics
   INP_LL_FLUID_COUNT       = 0x10, // Ultra-sonics
   INP_CENTRIFUGE           = 0x14, // Read comm counter
   OUTP_CLR_CENT_FAULT      = 0x14, // Write clears comm fault bit
   //
   // Miscellaneous Status registers (8-bit, read-only)
   //
   INP_VALVES               = 0x04, // valves and cassette positions
   INP_HALLS                = 0x08, // pump hall sensors and platelet valve position
   INP_SWITCHES             = 0x18, // power fail, door, stop/pause buttons, draw/return
   INP_MISC_STATUS          = 0x1C, // power line drop status
   //
   // Power Control/Status for 24V, 64V, Valve and Door Solenoid (8-bit, read/write)
   //
   IOP_POWER                = 0x28

} SafetyP_BAR_Offset;

#define CCA_ID_MASK     (0x00FF)

/**
 * Bit masks for SafetyP CCA registers
 */
typedef enum
{
   //
   // bit masks for INP_VALVES
   //
   NOT_CASSETTE_ERROR_BIT   = 0x00,
   NOT_CASSETTE_RAISED_BIT  = 0x80,
   NOT_CASSETTE_LOWERED_BIT = 0x40,
   NOT_RBC_OPEN_BIT         = 0x20,
   NOT_RBC_COLLECT_BIT      = 0x10,
   NOT_RBC_RETURN_BIT       = 0x08,
   NOT_PLASMA_OPEN_BIT      = 0x04,
   NOT_PLASMA_COLLECT_BIT   = 0x02,
   NOT_PLASMA_RETURN_BIT    = 0x01,
   NOT_VALVE1_CENTER_BIT    = NOT_RBC_OPEN_BIT,
   NOT_VALVE1_LEFT_BIT      = NOT_RBC_COLLECT_BIT,
   NOT_VALVE1_RIGHT_BIT     = NOT_RBC_RETURN_BIT,
   NOT_VALVE2_CENTER_BIT    = NOT_PLASMA_OPEN_BIT,
   NOT_VALVE2_LEFT_BIT      = NOT_PLASMA_COLLECT_BIT,
   NOT_VALVE2_RIGHT_BIT     = NOT_PLASMA_RETURN_BIT,
   //
   // bit masks for INP_HALLS
   //
   INLET_PUMP_HALL_BIT      = 0x80,
   PLATELET_PUMP_HALL_BIT   = 0x40,
   PLASMA_PUMP_HALL_BIT     = 0x20,
   RETURN_PUMP_HALL_BIT     = 0x10,
   AC_PUMP_HALL_BIT         = 0x08,
   NOT_PLATELET_OPEN_BIT    = 0x04,
   NOT_PLATELET_COLLECT_BIT = 0x02,
   NOT_PLATELET_RETURN_BIT  = 0x01,
   PUMP1_HALL_BIT           = AC_PUMP_HALL_BIT,
   PUMP2_HALL_BIT           = INLET_PUMP_HALL_BIT,
   PUMP3_HALL_BIT           = RETURN_PUMP_HALL_BIT,
   PUMP4_HALL_BIT           = PLASMA_PUMP_HALL_BIT,
   PUMP5_HALL_BIT           = PLATELET_PUMP_HALL_BIT,
   NOT_VALVE3_CENTER_BIT    = NOT_PLATELET_OPEN_BIT,
   NOT_VALVE3_LEFT_BIT      = NOT_PLATELET_COLLECT_BIT,
   NOT_VALVE3_RIGHT_BIT     = NOT_PLATELET_RETURN_BIT,
   //
   // bit masks for INP_SWITCHES
   //
   CENT_REVERSE_BIT         = 0x80,
   CENT_FAULT_BIT           = 0x40,
   NOT_POWER_FAIL_BIT       = 0x20,
   DOOR_CLOSED_BIT          = 0x10,
   NOT_STOP_SWITCH_BIT      = 0x08,
   NOT_PAUSE_SWITCH_BIT     = 0x04,
   RETURN_DIR_BIT           = 0x02,
   PUMP3_DIR_BIT            = RETURN_DIR_BIT,
   DOOR_LOCKED_BIT          = 0x01,
   //
   // bit masks for INP_MISC_STATUS
   //
   PLD_BIT                  = 0x02, // power line dropout
   BASIN_DATA_SERIAL_BIT    = 0x01, // obsolete; not implemented
   //
   // bit masks for IOP_POWER
   //
   ALARM_LIGHT_BIT          = 0x20, // alarm light (off=0)
   DOOR_POWER_BIT           = 0x10, // door solenoid
   LED_POWER_BIT            = 0x04, // valve LED
   CENT_POWER_BIT           = 0x02, // 64v power
   PUMP_POWER_BIT           = 0x01, // 24v power
} SafetyP_BitMasks;


/**
 * Safety Watchdog I/O values.
 *
 */
typedef enum
{
   // Output values
   WATCHDOG1_DATA = 0xF0,   // write to OUTP_WATCHDOG_F0
   WATCHDOG2_DATA = 0xA5,   // write to OUTP_WATCHDOG_A5
   //
   // SafetyP FPGA WatchDog STATUS/STATUS_CHK Bits. The STATUS_CHK
   // bits are the inverted value of the STATUS bits. This is used
   // as an authenticity check to see if the system was was reset from
   // a WatchDog Timeout or a Power-Fail/Power-Up condition. Any byte
   // values other than those below are considered an indeterminant state
   // caused by a Power-Fail/Power-Up condition.
   //
   // The WatchDog Timer has Started and No Errors in the
   // Pet-The-WatchDog-Sequence of 0xF0/0xA5 have been detected.
   //
   WD_STATUS_NO_FAILURE         = 0xF0,
   WD_STATUS_CHK_NO_FAILURE     = 0x0F,
   //
   // The WatchDog Circuit has Timed-Out due to a byte
   // other than the correct 0xF0 byte being written to
   // the OUTP_WATCHDOG_F0 register.
   //
   WD_STATUS_WRONG_F0_BYTE      = 0x33,
   WD_STATUS_CHK_WRONG_F0_BYTE  = 0xCC,
   //
   // The WatchDog Circuit has Timed-Out due to a byte
   // other than the correct 0xA5 byte being written to
   // the OUTP_WATCHDOG_A5 register.
   //
   WD_STATUS_WRONG_A5_BYTE      = 0x55,
   WD_STATUS_CHK_WRONG_A5_BYTE  = 0xAA,
   //
   // The WatchDog Circuit has Timed-Out due to an incorrect
   // Pet The WatchDog sequence. The OUTP_WATCHDOG_A5 register
   // was written before the OUTP_WATCHDOG_F0 register.
   //
   WD_STATUS_A5_WRT_WAITING_F0      = 0x17,
   WD_STATUS_CHK_A5_WRT_WAITING_F0  = 0xE8,
   //
   // The WatchDog Circuit has Timed Out due to an incorrect
   // Pet The WatchDog Sequence. The OUTP_WATCHDOG_F0
   // register was written twice.
   //
   WD_STATUS_F0_WRT_WAITING_A5      = 0x69,
   WD_STATUS_CHK_F0_WRT_WAITING_A5  = 0x96,
   //
   // The correct Pet The WatchDog Sequence 0xF0/0xA5
   // was not seen within ~420ms of the WatchDog being
   // started.  This caused the WatchDog to Fail and reset
   // the Processor. Processor Reset occurs 5 seconds
   // after the Timeout.
   //
   WD_STATUS_TIMEOUT        = 0x0F,
   WD_STATUS_CHK_TIMEOUT    = 0xF0,

} WatchDogIO;

#endif /* CCA_PCI_SAFETY_H_ */

/* FORMAT HASH 833ac54ee871d9872903ac3e76360dbc */
