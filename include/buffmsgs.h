/*
 * Copyright (c) 1995, 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Log: BUFFMSGS.H $
 * Revision 1.11  2000/05/25 18:43:27  MS10234
 * Added optimizer to proc message ...
 * Revision 1.10  2000/04/14 19:37:02  MS10234
 * Added OverrideAlarmMsg enum
 * Revision 1.8  2000/03/24 18:08:05  BS04481
 * Change several messages to NOTUSED in the message structure
 * which is used for AN2.  Service Control is not used because 
 * stuffer is not part of the project.  VD-HAL messages not used 
 * because the interface disappears.  TaskManagerCommand no
 * longer used.  Optimizer Command and Status temporarily changed
 * to not used to allow AN2 code to compile without the optimizer
 * code.
 * Revision 1.7  2000/03/07 00:29:00  BD10648
 * Additional messages for resturcturing: HalStatus, HalOrders, Calibration
 * Revision 1.6  1999/08/04 22:51:48  TD10216
 * IT3640
 * Revision 1.5  1999/07/28 21:52:24  TD10216
 * IT4154
 * Revision 1.3  1999/07/23 01:37:15  BS04481
 * Add BasinTempStatus message
 * Revision 1.2  1999/06/30 20:35:41  TD10216
 * IT3908
 * Revision 1.1  1999/05/24 23:26:16  TD10216
 * Initial revision
 * Revision 1.6  1999/05/13 15:33:14  SB07663
 * Readded in tcpgateRegistration for IT3838 and new modifyProduct
 * message for V3.3a PFR
 * Revision 1.5  1999/05/13 15:24:28  MS10234
 * Added message type to log non-display messages
 * Revision 1.3  1999/04/01 16:28:27  SB07663
 * Message additions for V3.3a optimizer
 * Revision 1.2  1999/03/24 21:49:48  BS04481
 * Change hardware version information to provide base version of
 * hardware plus computer type, board rev and FPGA revision byte.
 * Add message support in both drivers to respond to queries for 
 * hardware type.
 * Revision 1.1  1998/10/23 19:39:18  TM02109
 * Initial revision
 * Revision 1.3  1998/10/23 17:42:12  TM02109
 * Must use distinct names in the INT32 and BUFFER messages.
 * Revision 1.2  1998/10/22 22:36:31  TM02109
 * Can not have redefined message names.
 * Revision 1.1  1998/10/22 22:29:13  TM02109
 * Initial revision
 * 
 * Created by WTM
 */

#ifndef BUFFMSGS_H                  // prevent multiple includes
#define BUFFMSGS_H

// NOTE: when changing BUFFER_MESSAGES, also change MsgStructMap below
// or your build may fail or runtime not work correctly
enum BUFFER_MESSAGES
{
   FIRST_BUFFER_MESSAGE=0,        // don't change this entry

//      MESSAGE NAME                  FROM        TO         DESCRIPTION
// 12/02/96 msm
// APOLLOnet II Messages Please leave these in a block
// They are given unique IDs for the TCP/IP processes
//
// The msgs identifed as AN2 -> FS will be sent from an external computing resource
// on a UDP socket directed to the FS of interest ( FS ip, AN2 port ).
// Msgs ideintifed as FS -> AN2 will be xmitterd on a UDP socket from the FS
// the the remote host currently active in the tcp_gate ( remote host ip, remote host port ) .
//
   AN2SetHostMsg,                 // AN2 -> FS set remote host for tcp_gate FS msg output.
   AN2AreYouThereRequestMsg,      // AN2 -> FS ping FS for AN2 port status.
   AN2AreYouThereReplyMsg,        // FS  -> AN2 reply to an2 ping.
   AN2StatusRequestMsg,           // AN2 -> FS Machine status request.
   AN2StatusRequestReplyMsg,      // FS  -> AN2 Machine staus request reply.
   AN2ServiceRequestMsg,          // AN2 -> FS  service request.
   AN2ServiceRequestReplyMsg,     // FS  -> AN2 result of service request.
   AN2SetDonorVitalsMsg,          // AN2 -> FS donor vitals structure.
   AN2SetDonorVitalsReplyMsg,     // FS  -> AN2 donor vitals reception reply.
// when adding AN2 messages, RENAME the highest 'AN2TestX' message
   AN2AutoServiceControlCommand,  // auto service msg from external computer containing control cmd data ( pump flows )
   AN2ManualServiceControlCommand,// manual service msg from external computer containing control cmd data ( pump RPM)
   AN2Test3,                      // undifferentiated an2 msg for future use/testing
   AN2Test2,                      // undifferentiated an2 msg for future use/testing
   AN2Test1,                      // undifferentiated an2 msg for future use/testing
// End of APOLLonet II Messages

   ControlHardwareCommands,      // Procedure -> CNTL        Control commands to the hardware
   ControlHardwareStatus,        // CNTL      -> Procedure   Hardware status data
   CurrentProcedureStatus,       // Procedure -> GUI         Run data
   DataLog,                      // any       -> GUI         Data logging information
   GUIConfigToProcedure,         // GUI       -> Procedure   request to set procedure params (i.e. proc time...)
   GUIToProcedureDonorVitals,    // GUI       -> Procedure   GUI donor safety critical information struct
   GUIToProcedureAdjust,         // GUI       -> Procedure   GUI procedure adjustment struct
   GUIToProcedureProductRequest, // GUI       -> Procedure   GUI to procedure product request struct
   HALtoVDStatus,                // Procedure -> Procedure   Internal message HAL->VD
   KeyboardMessage,              // ?         -> ?           for test programs
   KeypadReturnMsg,              // GUI       -> GUI         GUI return information from keypad
   ProcedureToGUIAdjust,         // Procedure -> GUI         Procedure adjustment struct response to GUI
   ProcToSafetyDonorVitals,      // Procedure -> Safety
   EndRunStats,                  // Procedure -> GUI         end of run statistic information
   RequestRanges,                // GUI       -> Procedure
   SafetyHardwareCommands,
   SafetyHardwareStatus,
   SafetyToGuiDonorVitals,       // Safety    -> GUI
   StatusLine,                   // many      -> GUI
   VDtoHALCommands,              // Procedure -> Procedure   Internal message VD->HAL
   AlarmMessage,                 // Anyone -> Procedure
   GUIAlarmResponse,             // GUI -> Procedure
   AlarmDisplayRequest,          // Procedure -> GUI
   TaskManagerCommand,           // Anyone -> Task_Manager
   ScreenInvokeStateless,        // GUI  -> GUI, displays stateless screen
   ScreenInvokeChild,            // GUI  -> GUI, displays child screen
   ScreenSwapStateless,          // GUI  -> GUI, swaps current stateless screen with another
   ScreenSwapChild,              // GUI  -> GUI, swaps current child screen with another
   ScreenInvoke,                 // GUI  -> GUI, displays procedure screen
   SoundCommands,                // Anyone    -> Sound Manager   Command sounds to speaker
   BuffTestMessage1,             // test message 1
   BuffTestMessage2,             // test message 2
   FastUsData,                   // CTL -> Test  Fast ultrasonics data
   FastApsData,                  // CTL -> AcOccMon   Fast Access Pressure Data
   FastPumpData,                 // CTL -> test  Fast pump encoder data
   SafetyDonorMon,
   SafetyResMon,
   SafetyCycleMon,
   GUIChildScreenToGUI,          // GUI       -> GUI data returning from child screens on GUI


// Spectra ProcII Messages, Please leave these in a block
// They are given unique IDs for the TCP/IP processes
   SpectraAcMsg,
   SpectraAsMsg,
   SpectraAwMsg,
   SpectraAxMsg,
   SpectraAyMsg,
   SpectraAzMsg,
   SpectraDsMsg,
   SpectraDyMsg,
   SpectraKMsg,         // Keyboard
   SpectraMcMsg,
   SpectraMsgAck,       // message acknowledge
   SpectraMsgCmd,       // message command
   SpectraPwrFailAck,   // Power fail Acknowledge
   SpectraPwrFailData,  // Power fail Data
   SpectraPwrFailReq,   // Power fail request
   SpectraPmMsg,        // parameter command
   SpectraPmAckMsg,     // parameter acknowledge
   SpectraPsMsg,
   SpectraPuMsg,
   SpectraPxMsg,
   SpectraPyMsg,
   SpectraPzMsg,
   SpectraSsMsg,
   SpectraSxMsg,
   SpectraSync,
   SpectraTest6,
   SpectraTest5,
   SpectraTest4,
   SpectraTest3,
   SpectraTest2,
   SpectraTest1,
   // End of Spectra Messages

   ScreenSwapStackChild,         // GUI       -> GUI Request to clean the child screen stack, then add a new screen.
   ScreenSwapStackStateless,     // GUI       -> GUI Request to clean the stateless screen stack, then add a new screen.
   misc_log,                     // anyone    -> proc Request to add misc data to run_data log
   PowerFailRecoveryData,        // proc<->safety power fail data
   ProcStateSubstate,            // PFR       -> proc change state and substate simultaneously
   MeterDataMsg,                 // Router    -> anyone requesting machine meter data.
   FastFilterAPS,                // control driver -> Proc  Asynchronous APS data for fast alarm

// Spectra ProcII Messages, Please leave these in a block
   SpectraTest7,
   SpectraTest8,
   SpectraTest9,
   SpectraTest10,
   SpectraTest11,
   SpectraTest12,
   SpectraTest13,
   SpectraTest14,
   SpectraTest15,
   SpectraTest16, 
// End of Spectra Messages
   
// NOTE: When adding buffer messages,
// Add to end of list, after this comment
   HardwareVersion,     // drivers -> anyone who requests Hardware version structure
   CreateProduct,           //  anyone -> proc create new product
   OptimizerCommands,       // proc -> proc optimizer task : task command
   OptimizerResults,        // optimizer task -> proc : optimization results
   ProcedureInformation,    // proc->gui procedure information for selection screen
   AlarmLogRequest,              // Procedure -> Procedure
   TcpgateRegistration,     // External->an2inetd->tcp_gate:register for tcpgate msgs
   ModifyProduct,           // anyone -> proc : modify product (see predict_msgs.h)
   BasinTempStatus,         // safety driver -> anyone who want basin temp data
   EverestConfigLog,    // Not Sent: Used by Everest to parse log file
    HalStatusMsg,           // sent by Hal Task
    HalOrdersMsg,           // sent to hal task
    ArcCalibrationMsg,      // Cal.dat message from Archive
    ArcConfigMsg,           // Config.dat message to/from Archive
    ProcDonorMsg,           // Within proc donor message
    OverrideAlarmMsg,
    Opt2ProcMsg,
    VistaDonorMsg,          // Vista to Gui donor message
    AdjustmentsMsg,         // Adjustments section message
    LangUnitMsg,            // Config Lang/Unit section message
    ProcedureConfigMsg,     // Config Procedure section message
    PredictionConfigMsg,    // Config Prediction section message
    ProductTemplatesMsg,    // Config Product Templates section message
    ProductDefinitionMsg,   // Config Lang/Unit section message
    BasicMachineCfgMsg,     // Config Lang/Unit section message

   LAST_BUFFER_MESSAGE           // don't change this entry
};

// Join the buffer message ids with a structure. Make changes to 
// MsgStructMap when BUFFER_MESSAGES are changed.
#define NOTUSED "NotUsed"

typedef struct
{
   int MsgId;        // From BUFFER_MESSAGES above
   char *MsgStructure;  // The name of the structure the message sends
} MsgStructMapEntry;

// Dont use NOTUSED for real messages!
// Change NOTUSED to actual struct if msg is used
#ifdef STRUCTDEFGEN
MsgStructMapEntry MsgStructMap[LAST_BUFFER_MESSAGE+1] =
{
   { FIRST_BUFFER_MESSAGE, NULL },     // Dont change this

   { AN2SetHostMsg, NOTUSED },
   { AN2AreYouThereRequestMsg, "SAN2Broadcast" },
   { AN2AreYouThereReplyMsg, NOTUSED },
   { AN2StatusRequestMsg, NOTUSED },
   { AN2StatusRequestReplyMsg, "SFSAN2Status" },
   { AN2ServiceRequestMsg, NOTUSED },  // SAN2ServiceRequest union
   { AN2ServiceRequestReplyMsg, "SAN2ServiceRequestReply" },
   { AN2SetDonorVitalsMsg, NOTUSED },
   { AN2SetDonorVitalsReplyMsg, NOTUSED },
   { AN2AutoServiceControlCommand, NOTUSED },
   { AN2ManualServiceControlCommand, NOTUSED },
   { AN2Test3, "AP2_Section_Name_Struct" }, 
   { AN2Test2, "AP2_Section_Name_Struct" }, 
   { AN2Test1, NOTUSED },

   { ControlHardwareCommands, "CHwOrders" }, 
   { ControlHardwareStatus, "CHwStates" },
   { CurrentProcedureStatus, "CURRENT_PROCEDURE_STATUS_STRUCT" },
   { DataLog, NOTUSED },
   { GUIConfigToProcedure, "ProcedureInformationStruct" },
   { GUIToProcedureDonorVitals, "SDonorVitals" },
   { GUIToProcedureAdjust, NOTUSED },
   { GUIToProcedureProductRequest, NOTUSED },
   { HALtoVDStatus, NOTUSED },
   { KeyboardMessage, NOTUSED },
   { KeypadReturnMsg, "KEYPAD_RETURN_STRUCT" },
   { ProcedureToGUIAdjust, "ADJUST_SCREEN_CONFIG" },
   { ProcToSafetyDonorVitals, "SDonorVitals" },
   { EndRunStats, "RUN_SUMMARY_STRUCT" },
   { RequestRanges, NOTUSED },
   { SafetyHardwareCommands, "SHwOrders" },
   { SafetyHardwareStatus, "SHwStates" },
   { SafetyToGuiDonorVitals, "SDonorVitals" },
   { StatusLine, NOTUSED },
   { VDtoHALCommands, NOTUSED },
   { AlarmMessage, "Alarm_struct" }, 
   { GUIAlarmResponse, "Alarm_response_struct" },
   { AlarmDisplayRequest, "GUI_Alarm_struct" },
   { TaskManagerCommand, NOTUSED },
   { ScreenInvokeStateless, "SCREEN_INVOKE_STRUCT" },
   { ScreenInvokeChild, "SCREEN_INVOKE_STRUCT" },
   { ScreenSwapStateless, "SCREEN_INVOKE_STRUCT" },
   { ScreenSwapChild, "SCREEN_INVOKE_STRUCT" },
   { ScreenInvoke, "SCREEN_INVOKE_STRUCT" },
   { SoundCommands, "SoundOrders" },
   { BuffTestMessage1, NOTUSED },
   { BuffTestMessage2, NOTUSED },
   { FastUsData, "FASTDATA" },
   { FastApsData, "FASTDATA" },
   { FastPumpData, "FASTDATA" },
   { SafetyDonorMon, NOTUSED },
   { SafetyResMon, NOTUSED },
   { SafetyCycleMon, NOTUSED },
   { GUIChildScreenToGUI, "GUI_LISTBOX_STRUCT" },

   // Spectra ProcII Messages, Please leave these in a block
   // They are given unique IDs for the TCP/IP processes
   { SpectraAcMsg, NOTUSED },
   { SpectraAsMsg, NOTUSED },
   { SpectraAwMsg, NOTUSED },
   { SpectraAxMsg, NOTUSED },
   { SpectraAyMsg, NOTUSED },
   { SpectraAzMsg, NOTUSED },
   { SpectraDsMsg, NOTUSED },
   { SpectraDyMsg, NOTUSED },
   { SpectraKMsg, NOTUSED },
   { SpectraMcMsg, NOTUSED },
   { SpectraMsgAck, NOTUSED },
   { SpectraMsgCmd, NOTUSED },
   { SpectraPwrFailAck, NOTUSED },
   { SpectraPwrFailData, NOTUSED },
   { SpectraPwrFailReq, NOTUSED },
   { SpectraPmMsg,   NOTUSED },
   { SpectraPmAckMsg, NOTUSED },
   { SpectraPsMsg, NOTUSED },
   { SpectraPuMsg, NOTUSED },
   { SpectraPxMsg, NOTUSED },
   { SpectraPyMsg, NOTUSED },
   { SpectraPzMsg, NOTUSED },
   { SpectraSsMsg, NOTUSED },
   { SpectraSxMsg, NOTUSED },
   { SpectraSync, NOTUSED },
   { SpectraTest6, NOTUSED },
   { SpectraTest5, NOTUSED },
   { SpectraTest4, NOTUSED },
   { SpectraTest3, NOTUSED },
   { SpectraTest2, NOTUSED },
   { SpectraTest1, NOTUSED },
   // End of Spectra Messages

   { ScreenSwapStackChild, "SCREEN_INVOKE_STRUCT" },
   { ScreenSwapStackStateless, "SCREEN_INVOKE_STRUCT" },
   { misc_log, "MISC_LOG_STRUCT" },
   { PowerFailRecoveryData, "float" },
   { ProcStateSubstate, "stateSubstateChangeRequest_t" },
   { MeterDataMsg, "Meter_Return_Struct" },
   { FastFilterAPS, "FASTFILTER" },

   // Spectra ProcII Messages, Please leave these in a block
   { SpectraTest7, NOTUSED },
   { SpectraTest8, NOTUSED },
   { SpectraTest9, NOTUSED },
   { SpectraTest10, NOTUSED },
   { SpectraTest11, NOTUSED },
   { SpectraTest12, NOTUSED },
   { SpectraTest13, NOTUSED },
   { SpectraTest14, NOTUSED },
   { SpectraTest15, NOTUSED },
   { SpectraTest16, NOTUSED },
   // End of Spectra Messages
   
   // NOTE: When adding buffer messages,
   // Add to end of list, after this comment
   { HardwareVersion, "hardware" },
   { CreateProduct, "CreateProductStruct" },
   { OptimizerCommands, NOTUSED },
   { OptimizerResults, NOTUSED },
   { ProcedureInformation, "ProcedureInformationStruct" },
   { AlarmLogRequest, "GUI_Alarm_struct" },
   { TcpgateRegistration, "TCPGate_Reg_ReqStruct" },
   { ModifyProduct, "ModifyProductStruct" },
   { BasinTempStatus, "basinTemp" },
   { EverestConfigLog, "ConfigDataItem" },
   { HalStatusMsg , NOTUSED },
   { HalOrdersMsg , NOTUSED },
   { ArcCalibrationMsg, NOTUSED },
   { ArcConfigMsg, NOTUSED },
   { ProcDonorMsg , NOTUSED },
   { OverrideAlarmMsg , NOTUSED },
   { Opt2ProcMsg , NOTUSED },

    // don't change this entry
   { LAST_BUFFER_MESSAGE, NULL }
};
#endif

#endif
