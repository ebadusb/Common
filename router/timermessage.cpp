/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      timermessage.cpp
 *
 */

#include "timermessage.h"
#include "messagesystem.h"

TimerMessage :: TimerMessage()
:  Message< unsigned long >(), 
   _TimerArmed( false )
{
}

TimerMessage :: TimerMessage( unsigned long interval, const CallbackBase &cb, bool armTimer )
:  Message< unsigned long >(), 
   _TimerArmed( false )
{
   init( interval, cb, armTimer );
}

TimerMessage :: ~TimerMessage()
{
   deregisterTimer();
}

bool TimerMessage :: init( unsigned long intrvl, const CallbackBase &cb, bool armTimer )
{
   bool status = MessageBase::init( cb, MessageBase::SNDRCV_RECEIVE_ONLY );
   MessageSystem::MsgSystem()->dispatcher().registerMessage( MessageBase::msgId(), *this );

   if ( armTimer == true )
      interval( intrvl );
   else
      Message::set( intrvl );

   return status;
}

void TimerMessage :: armTimer( bool arm )
{
   unsigned long intrvl = Message::get();

   _TimerArmed=arm;
   if ( intrvl == 0 )
      _TimerArmed=false;
   if ( arm == false )
      intrvl = 0;

   //
   // Notify the timer task ...
   //  ( send the timer the new interval )
}


void TimerMessage :: interval( unsigned long interval)
{
   Message::set( interval );
   armTimer();
}


void TimerMessage :: deregisterTimer()
{
   //
   // Deregister this timer from the dispatcher ...
   MessageSystem::MsgSystem()->dispatcher().deregisterMessage( MessageBase::msgId(), *this );
}

#include <iostream.h>
const char *TimerMessage::genMessageName() 
{
   char buffer[13];
   sprintf( buffer, "Timer%x", (unsigned int)this );
   buffer[12] = '\0';
   cout << "MessageName: " << buffer << " " << strlen( buffer ) <<  endl;
   _MessageName = buffer;

   return (const char *)_MessageName.data();
}
