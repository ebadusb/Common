/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      timermessage.h
 *
 * ABSTRACT:   This file defines the interface for the base class for timer messages
 */

#ifndef _TIMER_MESSAGE_H_
#define _TIMER_MESSAGE_H_

#include <stdio.h>
#include <time.h>

#include "message.h"

class TimerMessage : public Message< unsigned long >
{
public:
   enum TimerState
   {
      ARMED = 1,
      DISARMED = 0
   };

public:

   //
   // Default constructor
   TimerMessage();

   // 
   // Constructor for timer messages ...
   //  interval - time in milliseconds
   //           - a time of 0 disarms the timer
   //  cb       - function to call when the timer message arrives
   //  armTimer - the timer is created disarmed by default
   //           - to create the timer armed, set this flag
   //
   TimerMessage( unsigned long interval, const CallbackBase &cb, TimerState armTimer=ARMED );

   //
   // Destructor
   virtual ~TimerMessage();

   //
   // Function called to initialize or re-initialize the timer object
   //  interval - time in milliseconds
   //           - a time of 0 disarms the timer
   //  cb       - function to call when the timer message arrives
   //  armTimer - the timer is created disarmed by default
   //           - to create the timer armed, set this flag
   //
   bool init( unsigned long interval, const CallbackBase &cb, TimerState armTimer=ARMED );

   //
   // Call this function to start the timer if the interval
   //  has already been given.  This function will also stop 
   //  the timer without changing the interval
   void armTimer( TimerState arm=ARMED );

   //
   // Function to return the current 'armed' status of the timer
   TimerState timerArmed() { return _TimerArmed; }

   //
   // Function to set the timer interval.  This function will
   //  automatically 'arm' the timer if given a value greater
   //  than 0.  Conversely, the timer will be stopped if given
   //  a 0 as an argument.
   void interval( unsigned long interval);

protected:

   //
   // Deregister the timer from the dispatcher ( called upon timer destruction ) ...
   void deregisterTimer();

   //
   // Get the message name ...
   //  (used for creating message Ids )
   virtual const char *genMsgName();

private:

   TimerMessage( TimerMessage const &);                           // Not implemented
   TimerMessage& operator=( TimerMessage const &);                // Not implemented

   void send( const unsigned long &buffer);                       // Not implemented
   const unsigned long &get( ) const;                             // Not implemented
   void set( const unsigned long &buffer);                        // Not implemented

   bool init( const SendType dt );                                // Not implemented
   bool init( const CallbackBase &cb, const SendReceiveType dt ); // Not implemented
   void registerMsg( const CallbackBase &cb );                    // Not implemented
   void deregisterMsg();                                          // Not implemented
   void send( );                                                  // Not implemented
   unsigned long msgId( ) const;                                  // Not implemented
   unsigned long originNode( ) const;                             // Not implemented
   unsigned long originTask( ) const;                             // Not implemented

protected:

   //
   // Flag to signify whether or not the timer is active ...
   TimerState _TimerArmed;

   //
   // Callback to disable the callback if the timer is disarmed ...
   CallbackBase _DisarmedCallback;

};

#endif

