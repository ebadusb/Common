/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      timermessage.cpp
 *
 */

#include <vxWorks.h>

#include "error.h"
#include "timermessage.h"
#include "messagepacket.h"
#include "messagesystem.h"

TimerMessage :: TimerMessage()
:  Message< unsigned long >(), 
   _TimerArmed( DISARMED )
{
}

TimerMessage :: TimerMessage( unsigned long interval, const CallbackBase &cb, TimerState armTimer )
:  Message< unsigned long >(), 
   _TimerArmed( DISARMED )
{
   init( interval, cb, armTimer );
}

TimerMessage :: ~TimerMessage()
{
   deregisterTimer();
}

bool TimerMessage :: init( unsigned long intrvl, const CallbackBase &cb, TimerState armTimer )
{
   bool status = MessageBase::init( cb, MessageBase::SNDRCV_LOCAL );

   if ( armTimer == ARMED )
      interval( intrvl );
   else
      Message::setData( intrvl );

   return status;
}

void TimerMessage :: armTimer( TimerState arm )
{
   unsigned long intrvl = Message::getData();

   _TimerArmed=arm;
   if ( intrvl == 0 )
      _TimerArmed=DISARMED;
   if ( arm == DISARMED )
      intrvl = 0;

   //
   // Notify the timer task ...
   //  ( send the timer the new interval )
   if ( _PacketList.front() == 0 || _PacketList.empty() )
   {
      _FATAL_ERROR( __FILE__, __LINE__, "Initialization failed : packet list is empty" );
      return;
   }

   MessagePacket mp( *( _PacketList.front() ) );
   mp.msgData().msg( (const unsigned char*) &intrvl, sizeof( unsigned long ) );
   clock_gettime( CLOCK_REALTIME, &_SentTime );
   mp.msgData().sendTime( _SentTime );
   mp.updateCRC();

   MessageSystem::MsgSystem()->dispatcher().sendTimerMessage( mp );
}


void TimerMessage :: interval( unsigned long interval)
{
   Message::setData( interval );
   armTimer();
}


void TimerMessage :: deregisterTimer()
{
   //
   // Deregister this timer from the dispatcher ...
   MessageSystem::MsgSystem()->dispatcher().deregisterMessage( MessageBase::msgId(), *this );
}

const char *TimerMessage::genMsgName() 
{
   char buffer[13];
   sprintf( buffer, "Timer%x", (unsigned int)this );
   buffer[12] = '\0';
   _MessageName = buffer;

   return (const char *)_MessageName.data();
}
