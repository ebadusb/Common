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
 * Base Address Register (BAR) Offsets for Control registers.
 * Note that some entries have an alias defined in order to retain
 * names similar to those used by both Trima and Optia with their
 * legacy FPGA interfaces.
 */
typedef enum
{
   //
   // Software Reset and CCA ID (32-bit, write-only / read-only)
   //
   CTL3_OUT_CCA_RESET            = 0x00, // Write CTL3_RESET_VALUE to force a reset of the CCA
   CTL3_INP_CCA_ID               = 0x00, // Read to get CCA ID pins cca_id[15:0]
   //
   // Pump Encoders (12-bit, read-only encoder counts)
   //
   CTL3_INPW_INLET_ENCODER       = 0x04,
   CTL3_INPW_PLATELET_ENCODER    = 0x08,
   CTL3_INPW_PLASMA_ENCODER      = 0x0C,
   CTL3_INPW_AC_ENCODER          = 0x10,
   CTL3_INPW_RETURN_ENCODER      = 0x14,
   CTL3_INW_PUMP1_ENCODER        = CTL3_INPW_AC_ENCODER,
   CTL3_INW_PUMP2_ENCODER        = CTL3_INPW_INLET_ENCODER,
   CTL3_INW_PUMP3_ENCODER        = CTL3_INPW_RETURN_ENCODER,
   CTL3_INW_PUMP4_ENCODER        = CTL3_INPW_PLASMA_ENCODER,
   CTL3_INW_PUMP5_ENCODER        = CTL3_INPW_PLATELET_ENCODER,
   //
   // Pump PWMs (8-bit, read/write pump speed control registers)
   //
   CTL3_IOP_INLET_PWM            = 0x18,
   CTL3_IOP_PLATELET_PWM         = 0x1C,
   CTL3_IOP_PLASMA_PWM           = 0x20,
   CTL3_IOP_AC_PWM               = 0x24,
   CTL3_IOP_RETURN_PWM           = 0x28,
   CTL3_IO_PUMP1_PWM             = CTL3_IOP_AC_PWM,
   CTL3_IO_PUMP2_PWM             = CTL3_IOP_INLET_PWM,
   CTL3_IO_PUMP3_PWM             = CTL3_IOP_RETURN_PWM,
   CTL3_IO_PUMP4_PWM             = CTL3_IOP_PLASMA_PWM,
   CTL3_IO_PUMP5_PWM             = CTL3_IOP_PLATELET_PWM,
   //
   // External Status Circuits (8-bit, read-only status registers)
   //
   CTL3_INP_BUTTONS              = 0x2C, // plasma valve, fault bits, front-panel buttons
   CTL3_INP_VALVES               = 0x30, // platelet valve, cassette position, RBC valve
   CTL3_INP_POWER_STATUS         = 0x34, // fans, over temperature warning, power status
   CTL3_IN_BUTTONS               = CTL3_INP_BUTTONS,
   CTL3_IN_VALVES                = CTL3_INP_VALVES,
   CTL3_IN_POWER_STATUS          = CTL3_INP_POWER_STATUS,
   //
   // Power Supply Interface Register (5-bit, RW + RO register)
   //
   CTL3_IOP_PWR_SUPPLY           = 0x38,
   CTL3_IO_POWER_STATUS2         = CTL3_IOP_PWR_SUPPLY,
   //
   // Sound and Spare Digital Output Register (7-bit, read/write)
   //
   CTL3_IOP_SOUND_LEVEL          = 0x3C,
   CTL3_IO_SOUND_LEVEL           = CTL3_IOP_SOUND_LEVEL,
   //
   // Counters for Air & Fluid Sample & Centrifuge Commutation (8-bit, read-only)
   //
   CTL3_INW_LOWER_AIR_AND_FLUID  = 0x40,
   CTL3_INW_UPPER_AIR_AND_FLUID  = 0x48,
   CTL3_INP_CENT_COMM            = 0x50,
   CTL3_IN_CENT_COMM             = CTL3_INP_CENT_COMM,
   //
   // Miscellaneous Status (8-bit, read-only)
   //
   CTL3_INP_ODDS_AND_ENDS        = 0x54, // solenoid, door, AC/repl fluids, commutator, centrifuge, A/D
   CTL3_IN_ODDS_AND_ENDS         = CTL3_INP_ODDS_AND_ENDS,
   //
   // Centrifuge Control (3-bit, read-write)
   //
   CTL3_IOP_CENT_ENABLE          = 0x58, // centrifuge reset and enable bits
   CTL3_IO_CENT_ENABLE           = CTL3_IOP_CENT_ENABLE,
   //
   // Centrifuge Speed Control (12-bit, read/write)
   //
   CTL3_IOPW_CENT_SPEED          = 0x5C,
   CTL3_IOW_CENT_SPEED           = CTL3_IOPW_CENT_SPEED,
   //
   // Watchdog Pet (read/write) and Status (read-only) registers
   //
   CTL3_IO_WATCHDOG1             = 0x60,
   CTL3_IO_WATCHDOG2             = 0x64,
   CTL3_IO_WATCHDOG_F0_REG       = CTL3_IO_WATCHDOG1,
   CTL3_IO_WATCHDOG_A5_REG       = CTL3_IO_WATCHDOG2,
   CTL3_IN_WATCHDOG_STATUS       = 0x80,
   CTL3_IN_WATCHDOG_CHECK        = 0x84,
   //
   // Analog-to-Digital Converter Channel Select and Read registers
   //
   CTL3_IOP_ADC_MUX              = 0x68, // write-only bit7 (start reading); read/write bits 5:0
   CTL3_INPW_ADC_VALUE           = 0x6C, // 12-bit read-only of A/D Converter Reading
   CTL3_IO_ADC_MUX               = CTL3_IOP_ADC_MUX,
   CTL3_INW_ADC_VALUE            = CTL3_INPW_ADC_VALUE,

   //
   // Valve Selection register (6-bit, read/write)
   //
   CTL3_IOP_VALVE                = 0x70,
   CTL3_IO_VALVE                 = CTL3_IOP_VALVE,
   //
   // Pump Control & Status register (4-bit, read/write)
   //
   CTL3_IOP_PUMP_ENABLE          = 0x74,
   CTL3_IO_PUMP_ENABLE           = CTL3_IOP_PUMP_ENABLE,
   //
   // Door Lock Control & Status register (2-bit, read/write)
   //
   CTL3_IOP_DOOR                 = 0x78,
   CTL3_IO_DOOR                  = CTL3_IOP_DOOR,
   //
   // Leak Detector and Alarm (2-bit, read/write)
   //
   CTL3_IOP_ALARM                = 0x7C,
   CTL3_IO_ALARM                 = CTL3_IOP_ALARM

} Control3_BAR_Offset;

#define CCA_ID_MASK     (0x00FF)

/**
 * Bit masks for Control3 CCA registers
 */
typedef enum
{
   //
   // output reset value for CTL3_OUT_CCA_RESET
   //
   CTL3_CCA_RESET_VALUE          = 0xFF00AA55,
   //
   // bit masks for INP_BUTTONS
   //
   CTL3_NOT_PLASMA_OPEN_BIT      = 0x80,
   CTL3_NOT_PLASMA_COLLECT_BIT   = 0x40,
   CTL3_NOT_PLASMA_RETURN_BIT    = 0x20,
   CTL3_PUMP_FAULT_BIT           = 0x10,
   CTL3_CENT_FAULT_BIT           = 0x08,    // diff from C2
   CTL3_NOT_STOP_SWITCH_BIT      = 0x04,    // diff from C2
   CTL3_NOT_PAUSE_SWITCH_BIT     = 0x02,    // diff from C2
   CTL3_VALVE_FAULT_BIT          = 0x01,    // diff from C2
   //
   // bit masks for INP_VALVES
   //
   CTL3_NOT_PLATELET_OPEN_BIT    = 0x80,
   CTL3_NOT_PLATELET_COLLECT_BIT = 0x40,
   CTL3_NOT_PLATELET_RETURN_BIT  = 0x20,
   CTL3_NOT_CASSETTE_RAISED_BIT  = 0x10,
   CTL3_NOT_CASSETTE_LOWERED_BIT = 0x08,
   CTL3_NOT_RBC_OPEN_BIT         = 0x04,
   CTL3_NOT_RBC_COLLECT_BIT      = 0x02,
   CTL3_NOT_RBC_RETURN_BIT       = 0x01,
   //
   // bit masks for INP_POWER_STATUS
   //
   CTL3_NOT_FAN_SENSE1_BIT       = 0x80,    // diff from C2
   CTL3_NOT_FAN_SENSE0_BIT       = 0x40,    // diff from C2
   CTL3_NOT_FAN_SENSE2_BIT       = 0x20,
   CTL3_NOT_POWER_FAIL_MASK      = 0x0F,
   CTL3_NOT_BUF_SS_OTW_BIT       = 0x08,
   CTL3_NOT_BUF_PS_OTW_BIT       = 0x04,
   CTL3_NOT_BUF_PFW_BIT          = 0x02,
   CTL3_NOT_BUF_DC_OK_BIT        = 0x01,
   //
   // bit masks for IOP_PWR_SUPPLY
   //
   CTL3_OVP_TEST_FAIL_BIT        = 0x10,
   CTL3_START_OVP_TEST_BIT       = 0x08,
   CTL3_DISABLE_POWER_BIT        = 0x04,
   CTL3_NOT_SEAL_SAFE_OTW_BIT    = 0x02,
   CTL3_SEAL_SAFE_IN_USE_BIT     = 0x01,
   CTL3_OVP_TEST_FAIL            = CTL3_OVP_TEST_FAIL_BIT,
   CTL3_START_OVP_TEST           = CTL3_START_OVP_TEST_BIT,
   CTL3_NOT_SEAL_SAFE_OVERTEMP   = CTL3_NOT_SEAL_SAFE_OTW_BIT,
   CTL3_SEAL_SAFE_IN_USE         = CTL3_SEAL_SAFE_IN_USE_BIT,
   //
   // bit masks for IOP_SOUND_LEVEL
   //
   CTL3_APC_ENABLE_BIT           = 0x04,
   CTL3_SOUND_LEVEL_MASK         = 0x03,
   //
   // bit masks for INP_ODDS_AND_ENDS
   //
   CTL3_NOT_SOLENOID_FAULT_BIT   = 0x80,
   CTL3_DOOR_UNLOCKED_BIT        = 0x40,
   CTL3_AC_GONE_BIT              = 0x20,
   CTL3_DOOR_CLOSED_BIT          = 0x10,
   CTL3_REPL_FLUID_GONE_BIT      = 0x08,
   CTL3_CENT_COMM_FAULT_BIT      = 0x04,
   CTL3_CENT_REVERSE_BIT         = 0x02,
   CTL3_ADC_BUSY_BIT             = 0x01,
   CTL3_NOT_FLUID_SENSOR1_BIT    = CTL3_AC_GONE_BIT,
   CTL3_NOT_FLUID_SENSOR2_BIT    = CTL3_REPL_FLUID_GONE_BIT,
   //
   // bit masks for IOP_CENT_ENABLE
   //
   CTL3_CENT_CLEAR_COMM_FLAG     = 0x04,
   CTL3_CENT_ENABLE_BIT          = 0x01,
   //
   // bit masks for IOP_ADC_MUX
   //
   CTL3_ADC_START_BIT            = 0x80,
   CTL3_ADC_MUX_MASK             = 0x7F,
   //
   // bit masks for IOP_VALVE
   //
   CTL3_VALVE_RUN_BIT            = 0x20,
   CTL3_VALVE_SELECT_CASSETTE_BIT= 0x10,
   CTL3_VALVE_SELECT_RBC_BIT     = 0x08,
   CTL3_VALVE_SELECT_PLASMA_BIT  = 0x04,
   CTL3_VALVE_SELECT_PLATELET_BIT= 0x02,
   CTL3_VALVE_SELECT_VALVE1_BIT  = CTL3_VALVE_SELECT_RBC_BIT,
   CTL3_VALVE_SELECT_VALVE2_BIT  = CTL3_VALVE_SELECT_PLASMA_BIT,
   CTL3_VALVE_SELECT_VALVE3_BIT  = CTL3_VALVE_SELECT_PLATELET_BIT,
   CTL3_VALVE_CCW_BIT            = 0x00,    // diff from C2
   CTL3_VALVE_CW_BIT             = 0x01,    // diff from C2
   CTL3_VALVE_LEFT_BIT           = 0x00,    // diff from C2
   CTL3_VALVE_RIGHT_BIT          = 0x01,    // diff from C2
   //
   // bit masks for IOP_PUMP_ENABLE
   //
   CTL3_PLATELET_DIR_BIT         = 0x08,    // future
   CTL3_PLASMA_DIR_BIT           = 0x04,    // future
   CTL3_PUMP_ENABLE_BIT          = 0x02,
   CTL3_RETURN_DIR_BIT           = 0x01,
   //
   // bit masks for IOP_DOOR
   //
   CTL3_NOT_DOOR_SOLENOID_BIT    = 0x02,
   CTL3_DOOR_DIRECTION_BIT       = 0x01,
   //
   // bit masks for IOP_ALARM
   //
   CTL3_LEAK_DETECTOR_BIT        = 0x02,
   CTL3_NOT_ALARM_LIGHT_BIT      = 0x01,

} Control3_BitMasks;

#ifdef USE_CTL3_WATCHDOG_IO_VALUES
/**
 * Watchdog I/O values.
 *
 * These are identical to the values for Control 1&2 FPGAs, which Trima and Optia
 * have defined in their respective driver modules. Leave here for reference and/or
 * future needs.
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
#endif /* USE_CTL3_WATCHDOG_IO_VALUES */

#endif /* CCA_PCI_CONTROL_H_ */

/* FORMAT HASH 833ac54ee871d9872903ac3e76360dbc */
