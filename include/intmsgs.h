/*
 * Copyright (c) 1995, 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: Z:/BCT_Development/Common/INCLUDE/rcs/INTMSGS.H 1.5 2000/04/25 18:14:10 MS10234 Exp MS10234 $
 * $Log: INTMSGS.H $
 * Revision 1.2  1999/07/09 15:24:50  BS04481
 * Add messages for clock setting
 * Revision 1.1  1999/05/24 23:26:36  TD10216
 * Initial revision
 * Revision 1.5  1999/05/05 22:15:42  TW09453
 * Add messages for the Replace Fluid status and the operator set ID
 * screens.
 * Revision 1.4  1999/04/01 16:28:45  SB07663
 * Message additions for V3.3a optimizer
 * Revision 1.3  1999/03/24 21:50:54  BS04481
 * Change hardware version information to provide base version of
 * hardware plus computer type, board rev and FPGA revision byte.
 * Add message support in both drivers to respond to queries for
 * hardware type.
 * Revision 1.2  1999/01/14 23:19:47  BS04481
 * Add msgID for MidCycleSwitch
 * Revision 1.1  1998/10/23 19:39:21  TM02109
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

#ifndef INTMSGS_H                  // prevent multiple includes
#define INTMSGS_H

enum INT32_MESSAGES
{

//      MESSAGE NAME                    FROM        TO          DESCRIPTION
   FIRST_INT32_MESSAGE=150,      // don't change this entry, table length
                                 // max is 1024.  The max value for FIRST is
                                 // 1024 - number of int32 messages.

   ACConnected,                  // GUI       -> Procedure   Operator confirms connection of AC
   ACPrimeCompleted,             // Procedure -> GUI         AC Prime is completed
   AckRunComplete,               // GUI       -> Procedure   Acknowledge run complete condition at end of BLOOD RINSEBACK
   AckTestCompleted,             // GUI       -> Procedure   Acknowledge TestCompleted message
   AirInPlasmaLine,              // GUI       -> Procedure   Operator presses 'Air in Plasma Line' button
   AlarmResponse,                // GUI       -> Procedure   Operator responds to an alarm condition (ABORT/CONTINUE for 2.2)
   AlarmStateChange,             // GUI       -> GUI         Alarm handler to alarm screen state change information
   AnswerGUIAlive,               // GUI       -> Procedure   Respond to a Query GUI Alive message
   AnswerSafetyAlive,            // Safety    -> Procedure   Respond to a Query Safety Alive message
   ButtonPressedID,              // GUI       -> Anyone      Screen/Button ID of the button just pressed by operator
   CancelProcedure,              // GUI       -> Procedure   Operator requests an abort of the procedure
   CentrifugePressureStatus,     // Procedure -> GUI         Centrifuge pressure reading in mmHg
   ClampedForTesting,            // GUI       -> Procedure   Operator confirms that the disposable is clamped and disposable test may start
   ConfirmDisposableLoaded,      // GUI       -> Procedure   Operator has confirmed that the disposable is loaded
   ConfirmDonorDisconnected,     // Procedure -> GUI         Procedure confirms donor disconnected condition
   ConfirmEnableLoadSystem,      // GUI       -> Procedure   GUI confirms that the Enable Load System has been accepted
   ProcConfigUpdateMsg,          // Anyone    -> Procedure   Update configuration from config file.
   DisconnectComplete,           // GUI       -> Procedure   Operator confirms disconnect complete
   DisposableLowered,            // Procedure -> GUI         The disposable is lowered
   DisposableRaised,             // Procedure -> GUI         The disposable is Raised.
   DonorConnected,               // GUI       -> Procedure   Operator confirms donor is connected
   DonorDisconnected,            // GUI       -> Procedure   Operator confirms donor is disconnected
   DoPrediction,                 // Procedure -> Procedure   A parameter has changed, do the prediction again.
   EnableLoadSystem,             // Procedure -> GUI         Procedure has completed startup tests - allows load system button
   EndProcedure,                 // Procedure -> Procedure   Tell the procedure it is time to end.
   FinishedViewingStats,         // GUI       -> Procedure   Operator has completed viewing end run summary
   GUIRequestBeginRun,           // GUI       -> Procedure   GUI requests run transition after AC Prime is completed
   GUIRequestRaiseCassette,      // GUI       -> Procedure   GUI requests the procedure to raise the cassette
   GUIRequestStartDraw,          // GUI       -> Procedure   Operator presses "Start Draw" button indicating blood sample complete and ready for Blood Prime
   InitializeInternalStates,     // GUI       -> GUI
   InletPressureStatus,          // Procedure -> GUI         Inlet pressure reading in mmHg
   LogEvent,                     // Anyone    -> Procedure   Take a data log at the current time.
   NewControlData,               // Procedure -> Procedure   Internal command (algos->VD)
   NewProcData,                  // Procedure -> Procedure   Internal status available (VD->algos)
   ProcAlarm,                    // Any domain-> Procedure   Command procedure to set specified alarm
   ProcState,                    // Any domain-> Procedure   Command procedure to go to specified state
   ProcedureAdjustment,          // GUI       -> Procedure   Operator requests adjustment (pressures, clumping, tingling)
   QueryGUIAlive,                // Procedure -> GUI         Procedure queries GUI for a ready response at power-up
   QuerySafetyAlive,             // Procedure -> Safety      Procedure queries Safety for a ready response at power-up
   RemoveAllChildScreens,        // GUI       -> GUI         remove all child screens from child stack
   RemoveAllStatelessScreens,    // GUI       -> GUI         remove all stateless screens from the stateless stack
   RemoveMostRecentChildScreen,  // GUI       -> GUI         remove the last displayed child from stack
   RemoveMostRecentStatelessScreen, // GUI    -> GUI         remove the last displayed stateless screen from stack
   RequestEndRunStats,           // GUI       -> Procedure   Request procedure to send end run statistics
   RequestProcedureStatus,       // GUI       -> Procedure   Request procedure status to update screen status info
   RequestSafetyPower,           // Procedure -> Safety      Request the change of some Power status.
   RunComplete,                  // Procedure -> GUI         Indicates run is finished (end of Blood Rinseback)
   SafetyTestDone,               // Safety    -> Procedure
   ScreenChangeRequest,          // GUI       -> GUI
   Sound,                        // Safety    -> SafetyHardware (audio)
   SpilloverRecovery,            // GUI       -> Procedure   Operator presses 'Spillover Recovery" button
   SystemStateChange,            // Procedure -> any domain  Procedure updates system state status
   SystemStateChangeRequest,     // GUI       -> Procedure,Safety    GUI requests a change to specified system state
   TaskManAlive,                 // Procedure -> Procedure   Tells when the task manager is alive.
   TestCompleted,                // Procedure -> GUI         Disposable tests have completed (passed)
   IntTestMessage1,              // test message 1
   IntTestMessage2,              // test message 2
   TransitionValidation,         // Safety    -> Procedure
   UpdateProcedureConfig,        // GUI       -> Procedure   Command to procedure indicating set type and AC management scheme (2.2)
   SwitchToPreviousScreen,       // GUI       -> GUI         switch to the previously displayed procedure screen
   RequestSafetyDriverStatus,    // Safety    -> SAFE_DRV    request status
   // NOTE: When adding messages, Add to end of list, after this comment
   UnsuccessfulCassetteLowering, // procedure -> GUI         unable to load cassette
   SafetyServiceMode,            // Service   -> Safety sends this message to the control side once the safety service mode boot-up is complete.
   MovePointerService,             // Service   -> External computer sends this message to move the pointer during the touch screen calibration
   StateChangeComplete,          // proc->proc  state manager to task manager sync for power fail
   RinsebackStatus,              // proc<->GUI is rinseback available handshake
   SendMachineMeterStatus,       // anyone -> meter class in ctl_drv  Request the machine hour meter, centrifuge on time meter, and
                                 //                                   and number of successful runs counter.
   DisposableID,                 // proc->safety current cassette type (enum cast to int)
   DonorDataStatus,              // GUI -> GUI from donor data object to donor information screens
   PFRToOccur,                   // proc->AP2 proc will powerfail recover
   ProcSubstateChange,           // proc->anyone new substate now in effect
   MidCycleSwitch,               // proc->safety indicates a cycle switch initiated by proc code instead
                                 //              of the reservoir sensors
   RequestHardwareVersion,       // anyone -> drivers Request causes drivers to respond with hardware version structure
   ProcToGuiPredictStatus,       // procedure to GUI optimization status information
   GuiToProcPredictRequest,      // GUI to proc request for optimization
   GuiToProcDonorTime,           // GUI to proc operator has changed donor time
   ProcToGuiCassetteInfo,        // proc to GUI sends recommended cassette for currently selected product
   DeleteProduct,                // anyone -> proc delete specified product
   OptimizerAlive,               // optimizer task to predict task, opt task is alive
   RepFluidConnectStatus,        // GUI       -> Procedure: notify proc whether operator connected Rep. Fluid or not
   OperatorSetIdentification,    // GUI       -> Procedure: relay operator's ID of a machine-unreadable set.
   DateTimeSet,                  // anyone -> time setting task.  Data is seconds since 1/1/1970 UTC.
   WatchdogControl,              // time setting task -> drivers.  Temporary override of watchdog.
   ProductSelection,			 // proc->AP2 - Product Selection


   // NOTE: Block of last 255 Int32Msg IDs reserved for Spectra
      // Spectra Int32Msgs go here
   FIRST_SPECTRA_INT32=768,
   DATA_ENTRY_STATE_START_MESSAGE, // indicate the start of data entry
   RAISE_VOLUME_ALARM_MESSAGE,     // volume alarm
   CHECK_CCM_LOADED_MESSAGE,       // is the CCM is loaded
   CHECK_CCM_CELL_LINE_OCCLUSION_MESSAGE, // prime-time occlusion
   RAISE_OPERATOR_RECOVERY_MESSAGE, // pause recovery message
   QNX_WATCHDOG_MESSAGE,
   LAST_INT32_MESSAGE=1023          // don't change this entry
};

#endif
