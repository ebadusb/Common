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

   // 
   // Constructor for timer messages ...
   //  interval - time in milliseconds
   //           - a time of 0 disarms the timer
   //  cb       - function to call when the timer message arrives
   //  armTimer - the timer is created disarmed by default
   //           - to create the timer armed, set this flag
   //
   TimerMessage( unsigned long interval, CallbackBase &cb, bool armTimer=false );

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
   bool init( unsigned long interval, CallbackBase &cb, bool armTimer=false );

   //
   // Call this function to start the timer if the interval
   //  has already been given.  This function will also stop 
   //  the timer without changing the interval
   void armTimer( bool arm=true );

   //
   // Function to return the current 'armed' status of the timer
   bool timerArmed() { return _TimerArmed; }

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
   virtual const char *genMessageName();

private:
   TimerMessage( TimerMessage const &);               // Not implemented
   TimerMessage& operator=( TimerMessage const &);    // Not implemented

protected:

   //
   // Flag to signify whether or not the timer is active ...
   bool _TimerArmed;
};

#endif

