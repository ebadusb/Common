/*
 * Copyright (c) 1995, 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: Z:/BCT_Development/Common/INCLUDE/rcs/MSG.HPP 1.2 1999/05/31 20:34:57 BS04481 Exp MS10234 $
 * $Log: MSG.HPP $
 * Revision 1.1  1999/05/24 23:26:40  TD10216
 * Initial revision
 * Revision 1.22  1998/10/23 19:38:45  TM02109
 * Modified to allow for inclusion of the buffmsgs.h and intmsgs.h files
 * without getting the entire array of class definitions.
 * Revision 1.21  1998/10/22 22:28:21  TM02109
 * Modifications to allow for inclusion of just the messages and 
 * header information for objects that do not need the entire
 * messaging system.
 * Revision 1.20  1998/09/30 21:51:45  SB07663
 * ITs 3496, 3373, 3376: Allow 'end run' button in the donor
 * disconnect system state if the operator has not yet confirmed that
 * the donor is disconnected.
 * Revision 1.19  1998/09/22 20:30:35  CF10242
 * Added Spectra INT32 message IDs to assigned area.
 * Revision 1.18  1998/09/22 18:28:49  bs04481
 * Reserve last 255 Int32 message IDs for Spectra
 * Revision 1.17  1998/07/24 20:52:41  TD10216
 * IT3430 - AP2 on Trima for 3.2
 * Revision 1.16  1998/07/02 19:42:15  MC03015
 * addition of the DonorDataStatus message for the donor data object
 * Revision 1.15  1998/04/23 14:57:10  SB07663
 * Added disposable ID message for DRBC testing
 * Revision 1.14  1998/01/30 21:05:50  BF02858
 * - re-rev'd files after Mike had to back rev them because of build problem
 *   Problem was that router built before gui and the info for "how to make"
 *   cfg_file was not in the router.mak file.  Fixed problem by creating a new
 *   copy of cfg_file in /router renamed as meter_io.cpp/hpp.
 * Revision 1.12  1998/01/29 17:24:37  BF02858
 * - code for software-based hour meter.  The meter object is created by
 *   the ctl_drv.  It accumulates total time on, centrifuge on time, and number
 *   of runs.  The data is displayed on the service screen.
 * Revision 1.11  1997/11/21 14:58:06  SB07663
 * Added messaging support for power fail recovery
 * Revision 1.9  1997/06/03 22:00:00  SB07663
 * Added power fail INT32 message
 * Revision 1.8  1997/05/31 21:15:51  Barry_Fulkerson
 * - added Move mouse pointer service message.  Used only during the service
 *   mode for calibrating the touch screen
 * Revision 1.7  1997/05/13 21:17:48  Barry_Fulkerson
 * - added new message for service mode
 * Revision 1.6  1997/05/05 15:27:54  SB07663
 * Added new UnsuccessfulCassetteLowering INT32 message
 * to allow proc/GUI sync at unsuccessful cassette loading due to
 * stamp recognition
 * Revision 1.5  1997/03/24 21:00:59  SS03309
 * Changes message numbering tables to allow for fixed
 * AN2 message numbers and for easier updates
 * Revision 1.4  1996/10/26 04:08:13  MC03015
 * Addition of AlarmStateChange message
 * Revision 1.3  1996/10/10 20:39:04  tm02109
 * Added ProcUpdateConfigMsg to allow people to call and update
 * the current configuration information into the common data store.
 * Revision 1.2  1996/07/24 19:49:50  SS03309
 * fix MKS
 * Revision 1.2  1996/07/22 14:43:24  SS03309
 * Revision 1.25  1996/06/28 16:43:15  SS03309
 * Comment from June 19 Design Review
 * Revision 1.24  1996/06/27 15:40:06  SS03309
 * Fixed comments
 *
 * TITLE:      msg.hpp, Focussed System message router - message classes.
 *
 * ABSTRACT:   These classes support message routing in the Focussed System.
 * Each message type and ID number are defined here.
 *
 * DOCUMENTS
 * Requirements:     I:\ieee1498\SSS2.DOC
 * Test:             I:\ieee1498\STD2.DOC
 *
 * EXAMPLE:
 *    Three steps are needed to send/receive messages
 *       1.  Create the dispatcher
 *       2.  Create needed messages
 *       3.  Execute the dispatch loop
 *
 *    in start up module:
 *
 *    #include <sys/trace.h>
 *    #include <sys/tracecod.h>
 *    #include "dispatch.hpp"
 *    #include "ftrace.h"
 *    #include "msg.hpp"
 *    #include "phdspat.hpp"                    // if using photon
 *
 * extern dispatcher* dispatch = 0;             // public dec
 *
 *    main()
 *    {
 *       dispatch = new dispatcher();        // create message dispatcher
 *            or if using Photon
 *       dispatch = new photonDispatcher();  // create photon msg dispatcher
 *       ...
 *       msg1 = new focusFloatMsg( focusFloatMsg::CentrifugePressureValue);
 *       msg2 = new myTimer();               // see example below
 *          ...
 *       dispatch->dispatchLoop();           // run message loop
 *                                           // note: dispatchLoop doesn't
 *                                           // return.
 *    }
 */

#ifndef MSG_HPP                  // prevent multiple includes
#define MSG_HPP

#include "dispatch.hpp"

// class definitions

/*
 * SPECIFICATION:  FocusTimerMsg is used to get timer ticks, user must
 * implement the notify function in a derived class.   The time base is 50 ms.
 * This is a pure virtual class.   The timer interval is set in ms
 * at construction and can be changed with the interval() functions.
 *
 * ERROR HANDLING:   Will terminate program if error.
 *
 * EXAMPLE (100 ms, 10 Hertz timer):
 *
 *    class myTimer : public focusTimerMsg
 *    {
 *      public:
 *       myTimer() : focusTimerMsg( 100)
 *       {
 *       }
 *       void notify()
 *       {
 *          // your code here
 *       }
 *    };
 */

class focusTimerMsg
{
   public:
      focusTimerMsg( unsigned long interval);
      virtual ~focusTimerMsg();
      virtual void notify() = 0;
      void interval( unsigned long interval);

   private:
      focusTimerMsg();              // default constructor, not implemented
      focusTimerMsg( focusTimerMsg const &);
      focusTimerMsg& operator=( focusTimerMsg const &);

      timer_t timerID;              // timer number
      pid_t   proxy;                // proxy
};

/*
 * SPECIFICATION:    focusInt32Msg is used to get/set integer data values
 *
 * ERROR HANDLING:   will terminate program if error.
 */


class focusInt32Msg : public routeBuffer
{

   public:
// 
// NOTE:  INCLUDE intmsgs.h HERE SO THAT THE SCOPE IS RESOLVED CORRECTLY!!!!!
//
#include "intmsgs.h"
//
   focusInt32Msg( INT32_MESSAGES id);                    // constructor
   focusInt32Msg( INT32_MESSAGES id, bounce_t bounce);   // constructor
   virtual ~focusInt32Msg();                             // destructor
   long get() const;                                     // get current value
   void set( long newValue);                             // set and send message with new value
   void set();                                           // send message with old values

private:
   focusInt32Msg();                                      // default ctr disabled
   focusInt32Msg( focusInt32Msg const &);
   focusInt32Msg& operator=( focusInt32Msg const &);
      
      struct MSGDATA
      {
         long value;
      };
      
      struct MSG
      {
         MSGHEADER   header;
         MSGDATA     data;
      };

      MSG*  message;          // copy of pointer to message, data storage
                              // is in base class
};

#endif
