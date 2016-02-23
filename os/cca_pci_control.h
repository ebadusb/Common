/**
 * Copyright (C) 2016 Terumo BCT, Inc.  All rights reserved.
 *
 * @file   cca_pci_control.h
 *
 * @author mra1usb
 * @date   Feb 16, 2016
 *
 * @brief  Register map for PCI interface of Control board introduced with EBox-2016.
 *
 * Reference the Trima/Optia CONTROL3 FPGA Design Specification (Chuck Mangan).
 *
 */

#ifndef CCA_PCI_CONTROL_H_
#define CCA_PCI_CONTROL_H_

/**
 * CONTROL3 CCA Register Map.
 *
 * Base Address Register (BAR) Offsets for Control registers
 */
typedef enum
{
   //
   // Software Reset and CCA ID (32-bit, write-only / read-only)
   //
   OUT_SW_RESET             = 0x00, // Write 0xFF00AA55 to force reset
   INP_CCA_ID               = 0x00, // Read to get CCA ID pins cca_id[15:0]
   //
   // Pump Encoders (12-bit, read-only encoder counts)
   //
   INPW_INLET_ENCODER       = 0x04,
   INPW_PLATELET_ENCODER    = 0x08,
   INPW_PLASMA_ENCODER      = 0x0C,
   INPW_AC_ENCODER          = 0x10,
   INPW_RETURN_ENCODER      = 0x14,
   INW_PUMP1_ENCODER        = INPW_AC_ENCODER,
   INW_PUMP2_ENCODER        = INPW_INLET_ENCODER,
   INW_PUMP3_ENCODER        = INPW_RETURN_ENCODER,
   INW_PUMP4_ENCODER        = INPW_PLASMA_ENCODER,
   INW_PUMP5_ENCODER        = INPW_PLATELET_ENCODER,
   //
   // Pump PWMs (8-bit, read/write pump speed control registers)
   //
   IOP_INLET_PWM            = 0x18,
   IOP_PLATELET_PWM         = 0x1C,
   IOP_PLASMA_PWM           = 0x20,
   IOP_AC_PWM               = 0x24,
   IOP_RETURN_PWM           = 0x28,
   IO_PUMP1_PWM             = IOP_AC_PWM,
   IO_PUMP2_PWM             = IOP_INLET_PWM,
   IO_PUMP3_PWM             = IOP_RETURN_PWM,
   IO_PUMP4_PWM             = IOP_PLASMA_PWM,
   IO_PUMP5_PWM             = IOP_RETURN_PWM,
   //
   // External Status Circuits (8-bit, read-only status registers)
   //
   INP_SWITCH_STATUS        = 0x2C, // plasma valve, fault bits, front-panel buttons
   INP_VALVE_STATUS         = 0x30, // platelet valve, cassette position, RBC valve
   INP_POWER_STATUS         = 0x34, // fans, over temperature warning, power status
   //
   // Power Supply Interface Register (5-bit, RW + RO register)
   //
   IOP_PWR_SUPPLY           = 0x38,
   //
   // Sound and Spare Digital Output Register (7-bit, read/write)
   //
   IOP_SOUND_CONTROL        = 0x3C,
   //
   // Counters for Air & Fluid Sample & Centrifuge Commutation (8-bit, read-only)
   //
   INP_LOWER_AIR_COUNT      = 0x40,
   INP_LOWER_FLUID_COUNT    = 0x44,
   INP_UPPER_AIR_COUNT      = 0x48,
   INP_UPPER_FLUID_COUNT    = 0x4C,
   INP_CENT_COMM            = 0x4C,
   //
   // Miscellaneous Status (8-bit, read-only)
   //
   INP_MISC_STATUS          = 0x54, // solenoid, door, AC/repl fluids, commutator, centrifuge, A/D
   //
   // Centrifuge Control (3-bit, read-write)
   //
   IOP_CENT_ENABLE          = 0x58, // centrifuge reset and enable bits
   //
   // Centrifuge Speed Control (12-bit, read/write)
   //
   IOPW_CENT_SPEED          = 0x5C,
   //
   // Watchdog Pet (write-only) and Status (read-only) registers
   //
   OUT_WATCHDOG1            = 0x60,
   OUT_WATCHDOG2            = 0x64,
   OUT_WATCHDOG_F0_REG      = OUT_WATCHDOG1,
   OUT_WATCHDOG_A5_REG      = OUT_WATCHDOG2,
   INP_WATCHDOG_STATUS      = 0x60,
   INP_WATCHDOG_CHECK       = 0x64,
   //
   // Analog-to-Digital Converter Channel Select and Read registers
   //
   IOP_ADC_MUX              = 0x68, // write-only bit7 (start reading); read/write bits 5:0
   INPW_ADC_VALUE           = 0x6C, // 12-bit read-only of A/D Converter Reading
   //
   // Valve Selection register (6-bit, read/write)
   //
   IOP_VALVE                = 0x70,
   //
   // Pump Control & Status register (4-bit, read/write)
   //
   IOP_PUMP_ENABLE          = 0x74,
   //
   // Door Lock Control & Status register (2-bit, read/write)
   //
   IOP_DOOR                 = 0x78,
   //
   // Leak Detector and Alarm (2-bit, read/write)
   //
   IOP_LEAK_ALARM           = 0x7C

} Control3_BAR_Offset;

#define CCA_ID_MASK     (0x00FF)

/**
 * Bit masks for Control3 CCA registers
 */
typedef enum
{
   //
   // bit masks for INP_SWITCH_STATUS
   //
   NOT_PLASMA_OPEN_BIT      = 0x80,
   NOT_PLASMA_COLLECT_BIT   = 0x40,
   NOT_PLASMA_RETURN_BIT    = 0x20,
   NOT_PUMP_FAULT_BIT       = 0x10,
   NOT_CENT_FAULT_BIT       = 0x08,
   NOT_STOP_SWITCH_BIT      = 0x04,
   NOT_PAUSE_SWITCH_BIT     = 0x02,
   NOT_VALVE_FAULT_BIT      = 0x01,
   //
   // bit masks for INP_VALVE_STATUS
   //
   NOT_PLATELET_OPEN_BIT    = 0x80,
   NOT_PLATELET_COLLECT_BIT = 0x40,
   NOT_PLATELET_RETURN_BIT  = 0x20,
   NOT_CASSETTE_RAISED_BIT  = 0x10,
   NOT_CASSETTE_LOWERED_BIT = 0x08,
   NOT_RBC_OPEN_BIT         = 0x04,
   NOT_RBC_COLLECT_BIT      = 0x02,
   NOT_RBC_RETURN_BIT       = 0x01,
   //
   // bit masks for INP_POWER_STATUS
   //
   NOT_FAN_SENSE1_BIT       = 0x80,
   NOT_FAN_SENSE0_BIT       = 0x40,
   NOT_FAN_SENSE2_BIT       = 0x20,
   NOT_POWER_FAIL_MASK      = 0x0F,
   NOT_BUF_SS_OTW_BIT       = 0x08,
   NOT_BUF_PS_OTW_BIT       = 0x04,
   NOT_BUF_PFW_BIT          = 0x02,
   NOT_BUF_DC_OK_BIT        = 0x01,
   //
   // bit masks for IOP_PWR_SUPPLY
   //
   OVP_TEST_FAIL_BIT        = 0x10,
   START_OVP_TEST_BIT       = 0x08,
   DISABLE_POWER_BIT        = 0x04,
   NOT_SEAL_SAFE_OTW_BIT    = 0x02,
   SEAL_SAFE_IN_USE_BIT     = 0x01,
   //
   // bit masks for IOP_SOUND_CONTROL
   //
   APC_ENABLE_BIT           = 0x04,
   SOUND_LEVEL_MASK         = 0x03,
   //
   // bit masks for INP_MISC_STATUS
   //
   NOT_SOLENOID_FAULT_BIT   = 0x80,
   DOOR_UNLOCKED_BIT        = 0x40,
   AC_FLUID_GONE_BIT        = 0x20,
   DOOR_CLOSED_BIT          = 0x10,
   REPL_FLUID_GONE_BIT      = 0x08,
   CENT_COMM_FAULT_BIT      = 0x04,
   CENT_REVERSE_BIT         = 0x02,
   ADC_BUSY_BIT             = 0x01,
   //
   // bit masks for IOP_CENT_ENABLE
   //
   CENT_CLEAR_COMM_FLAG     = 0x04,
   CENT_ENABLE_BIT          = 0x01,
   //
   // bit masks for IOP_ADC_MUX
   //
   ADC_START_BIT            = 0x80,
   ADC_MUX_MASK             = 0x7F,
   //
   // bit masks for IOP_VALVE
   //
   VALVE_RUN_BIT            = 0x20,
   VALVE_SELECT_CASSETTE_BIT= 0x10,
   VALVE_SELECT_RBC_BIT     = 0x08,
   VALVE_SELECT_PLASMA_BIT  = 0x04,
   VALVE_SELECT_PLATELET_BIT= 0x02,
   VALVE_CCW_BIT            = 0x01,
   VALVE_CW_BIT             = 0x00,
   VALVE_SELECT_VALVE1_BIT  = VALVE_SELECT_RBC_BIT,
   VALVE_SELECT_VALVE2_BIT  = VALVE_SELECT_PLASMA_BIT,
   VALVE_SELECT_VALVE3_BIT  = VALVE_SELECT_PLATELET_BIT,
   VALVE_LEFT_BIT           = VALVE_CCW_BIT,
   VALVE_RIGHT_BIT          = VALVE_CW_BIT,
   //
   // bit masks for IOP_PUMP_ENABLE
   //
   PLATELET_DIR_BIT         = 0x08, // future
   PLASMA_DIR_BIT           = 0x04, // future
   PUMP_ENABLE_BIT          = 0x02,
   RETURN_DIR_BIT           = 0x01,
   //
   // bit masks for IOP_DOOR
   //
   NOT_DOOR_SOLENOID_BIT    = 0x02,
   DOOR_DIRECTION_BIT       = 0x01,
   //
   // bit masks for IOP_LEAK_ALARM
   //
   LEAK_DETECTOR_BIT        = 0x02,
   NOT_ALARM_LIGHT_BIT      = 0x01,

} Control3_BitMasks;


/**
 * Watchdog I/O values.
 *
 */
typedef enum
{
   // Output values
   WATCHDOG1_DATA = 0xF0,   // write to OUT_WATCHDOG1
   WATCHDOG2_DATA = 0xA5,   // write to OUT_WATCHDOG2
   //
   // Control3 FPGA WatchDog STATUS/STATUS_CHK Bits. The STATUS_CHK
   // bits are the inverted value of the STATUS bits. This is used
   // as an authenticity check to see if the system was was reset from
   // a WatchDog Timeout or a Power-Fail/Power-Up condition. Any byte
   // values other than those below are considered an indeterminant state
   // caused by a Power-Fail/Power-Up condition.
   //
   // The WatchDog Timer has Started and No Errors in the
   // Pet The WatchDog Sequence of 0xF0/0xA5 have been detected.
   //
   WD_STATUS_NO_FAILURE     = 0xF0,
   WD_STATUS_CHK_NO_FAILURE = 0x0F,
   //
   // The WatchDog Circuit has Timed-Out due to a byte
   // other than the correct 0xF0 byte being written to
   // the PET_WATCHDOG_F0 register.
   //
   WD_STATUS_WRONG_F0_BYTE      = 0x33,
   WD_STATUS_CHK_WRONG_F0_BYTE  = 0xCC,
   //
   // The WatchDog Circuit has Timed-Out due to a byte
   // other than the correct 0xA5 byte being written to
   // the PET_WATCHDOG_A5 register.
   //
   WD_STATUS_WRONG_A5_BYTE      = 0x55,
   WD_STATUS_CHK_WRONG_A5_BYTE  = 0xAA,
   //
   // The WatchDog Circuit has Timed-Out due to an incorrect
   // Pet The WatchDog sequence. The PET_WATCHDOG_A5 register
   // was written before the PET_WATCHDOG_F0 register.
   //
   WD_STATUS_A5_WRT_WAITING_F0      = 0x17,
   WD_STATUS_CHK_A5_WRT_WAITING_F0  = 0xE8,
   //
   // The WatchDog Circuit has Timed Out due to an incorrect
   // Pet The WatchDog Sequence. The PET_WATCHDOG_F0
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

#endif /* CCA_PCI_CONTROL_H_ */

/* FORMAT HASH 833ac54ee871d9872903ac3e76360dbc */
