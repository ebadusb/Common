/*
 * Copyright (c) 1995, 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: Z:/BCT_Development/Common/INCLUDE/rcs/BUFFMSG.HPP 1.2 1999/05/31 20:34:42 BS04481 Exp MS10234 $
 * $Log: BUFFMSG.HPP $
 * Revision 1.1  1999/05/24 23:26:14  TD10216
 * Initial revision
 * Revision 1.25  1999/04/03 14:53:10  TD10216
 * Revision 1.24  1998/10/23 19:38:42  TM02109
 * Modified to allow for inclusion of the buffmsgs.h and intmsgs.h files
 * without getting the entire array of class definitions.
 * Revision 1.23  1998/10/23 19:31:56  TM02109
 * Revision 1.22  1998/10/22 22:28:20  TM02109
 * Modifications to allow for inclusion of just the messages and 
 * header information for objects that do not need the entire
 * messaging system.
 * Revision 1.21  1998/09/22 18:29:11  bs04481
 * Reserve 10 more buffer message IDs for Spectra
 * Revision 1.20  1998/02/04 21:27:24  bs04481
 * Implementation of fast response to APS violations in the control 
 * driver.
 * Revision 1.19  1998/01/30 21:05:44  BF02858
 * - re-rev'd files after Mike had to back rev them because of build problem
 *   Problem was that router built before gui and the info for "how to make"
 *   cfg_file was not in the router.mak file.  Fixed problem by creating a new 
 *   copy of cfg_file in /router renamed as meter_io.cpp/hpp.
 * Revision 1.17  1998/01/29 17:24:32  BF02858
 * - code for software-based hour meter.  The meter object is created by
 *   the ctl_drv.  It accumulates total time on, centrifuge on time, and number
 *   of runs.  The data is displayed on the service screen.
 * Revision 1.16  1997/11/21 14:57:15  SB07663
 * Added messaging support for power fail recovery
 * Revision 1.15  1997/05/29 17:46:23  SM02805
 * Changed An2Test4 msg ID to AN2ManualServiceControlCommand
 * msg ID.  In manual service mode, an2 port will modify incoming
 * msgs of ControlHardwareCommands to  AN2ManualServiceControlCommand
 * b4 router jam.
 * Revision 1.14  1997/05/29 16:54:51  SM02805
 * Changed AN2Test5 msg ID name to AN2AutoServiceControlCommand.
 * New msg id used for auto service mode in an2 port.
 * Revision 1.13  1997/05/12 17:15:28  SB07663
 * Added "miscellaneous logging" buffer message
 * Revision 1.12  1997/05/10 00:24:26  tw09453
 * Add message ID's for GUI commutator's stack swap command
 * messages
 * Revision 1.11  1997/03/24 21:00:55  SS03309
 * Changes message numbering tables to allow for fixed
 * AN2 message numbers and for easier updates
 * Revision 1.10  1997/01/10 22:01:49  SS03309
 * Added three spectra messages
 * Revision 1.9  1996/12/30 18:24:16  SS03309
 * Added SpectraPmMsg
 * Revision 1.8  1996/12/02 17:32:12  SM02805
 * Added a block of buffer messages for AN2.
 * Revision 1.7  1996/11/25 20:17:17  SS03309
 * Spectra Mods
 * Revision 1.6  1996/10/19 04:04:21  MC03015
 * Modification to GUI -> Procedure product request messge
 * Revision 1.5  1996/10/07 11:29:59  MC03015
 * Addition of PREDICT Screen messaging
 * Revision 1.4  1996/09/28 22:34:14  Barry_Fulkerson
 * New message for Configuration rev 2.3
 * Revision 1.3  1996/09/12 04:49:04  MC03015
 * Addition of the KeypadReturnMsg, GUI -> GUI
 * Revision 1.2  1996/07/24 19:49:03  SS03309
 * fix MKS
 * Revision 1.2  1996/07/22 14:41:31  SS03309
 * Revision 1.25  1996/07/18 22:41:24  SS03309
 * lint
 * Revision 1.24  1996/06/28 17:01:08  SS03309
 * add comments to function headers
 * Revision 1.23  1996/06/27 23:42:06  tw09453
 * Added two GUI screen SWAP messages.
 * Revision 1.22  1996/06/27 15:16:53  SS03309
 * Added Header
 *
 * TITLE:      buffmsg.hpp, Focussed System message router -
 *             buffer message classes.
 *
 * ABSTRACT:   These classes support buffer messages in the Focussed System.
 *             This message is a template which allows typeing of
 *             the get and set functions.
 *
 * DOCUMENTS
 * Requirements:     I:\ieee1498\SSS2.DOC
 * Test:             I:\ieee1498\STD2.DOC
 *
 * EXAMPLE:
 *    see msg.hpp
 *
 *    to create a instance of this class
 *    first create a structure to send:
 *
 *    typedef struct
 *    {
 *       ....
 *    } myStruct;
 *
 *    then create your message class:
 *    class myClass : public focusBufferMsg<myStruct>
 *    {
 *       public:
 *          myClass() : focusBufferMsg<myStruct>(messageID)    // constructor
 *          {
 *          };
 *          ~myClass()                       // destructor
 *          {
 *          };
 *          void notify()                    // notify function
 *          {
 *             // your code here
 *          };
 *       private:
 *    };
 *
 *    to create a message object:
 *    myClass* msg = new myClass;
 *
 *    to send a buffer
 *    msg->set( myStruct* data);
 *
 *    to get the data
 *    msg->get( myStruct* data);
 */

#ifndef BUFFMSG_HPP                 // prevent multiple includes
#define BUFFMSG_HPP

#include "dispatch.hpp"
#include "error.h"
#include "msg.hpp"
#include "buffmsgs.h"

extern dispatcher* dispatch;        // external ref for dispatcher

// class definition

/*
 * SPECIFICATION:  focusBufferMsg is used to distribute buffers
 * and/or structures.  It is a templated, in-line class.
 *
 * ERROR HANDLING:   will terminate program if error.
 */

template<class Type>
class focusBufferMsg : public routeBuffer
{
   public:
      // constructor
      // id - message id from above enum
      inline focusBufferMsg( BUFFER_MESSAGES id) :
         routeBuffer( (void**) &message,
                      sizeof( Type) + sizeof( MSGHEADER),
                      (short)id)
      {
      };
      inline focusBufferMsg( BUFFER_MESSAGES id, bounce_t bounce) :
         routeBuffer( (void**) &message,
                      sizeof( Type) + sizeof( MSGHEADER),
                      (short)id, 
                      (bounce_t)bounce)
      {
      };


      // destructor
      inline ~focusBufferMsg()
      {
      };

      // get current value (buffer)
      // Type* buffer is pointer to user data
      inline void get( Type *buffer) const
      {
         memcpy( buffer, message->data.buffer, sizeof(Type));
         if (memcmp( message->data.buffer, buffer, sizeof( Type)) != 0)
         {
            _FATAL_ERROR( __FILE__, __LINE__, TRACE_DISPATCHER, 0, "focusBufferMsg copy");
         }
      };

      // send current value (buffer)
      // Type* buffer is pointer to user data
      inline void set( Type* buffer)
      {
         memcpy( message->data.buffer, buffer, sizeof(Type));
         if (memcmp( message->data.buffer, buffer, sizeof( Type)) != 0)
         {
            _FATAL_ERROR( __FILE__, __LINE__, TRACE_DISPATCHER, 0, "focusBufferMsg copy");
         }
         send();
      }

   private:
      // these functions are not implemented and
      // are made private so that default types by the
      // compilier are not used.
      focusBufferMsg();             // default constructor, not implemented
      focusBufferMsg( focusBufferMsg const &);
      focusBufferMsg& operator=( focusBufferMsg const &);

      struct MSGDATA
      {
         unsigned char buffer[1];   // length allocated in constructor
      };

      struct MSG
      {
         MSGHEADER   header;
         MSGDATA     data;
      };
      struct MSG* message;          // copy of pointer to message, data storage
                                    // is in base class
};

#endif

