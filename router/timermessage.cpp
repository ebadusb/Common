/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      timermessage.cpp
 *
 */

#include <vxWorks.h>

#include "datalog.h"
#include "error.h"
#include "messagepacket.h"
#include "messagesystem.h"
#include "systemoverrides.h"
#include "timermessage.h"

TimerMessage :: TimerMessage()
:  Message< unsigned long >(), 
   _TimerArmed( DISARMED ),
   _DisarmedCallback()
{
}

TimerMessage :: TimerMessage( unsigned long interval, const CallbackBase &cb, TimerState armTimer )
:  Message< unsigned long >(), 
   _TimerArmed( DISARMED ),
   _DisarmedCallback()
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
   _DisarmedCallback = cb;

   if ( armTimer == ARMED )
      interval( intrvl );
   else
      Message< unsigned long >::setData( intrvl );

   return status;
}

void TimerMessage :: armTimer( TimerState arm )
{
   if ( _DistributionType == SNDRCV_RECEIVE_ONLY )
   {
      postConstructInit();
      DataLog_Critical criticalLog;
      DataLog(criticalLog) << "Message Id " << hex << _MsgId << " ( " << _MessageName.c_str() << " ) not initialized for sending" << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "Message usage error" );
      return;
   }

   unsigned long intrvl = Message< unsigned long >::getData();

   TimerState newState=arm;
   if ( intrvl == 0 )
      newState=DISARMED;
   if ( arm == DISARMED )
      intrvl = 0;

   if ( newState == DISARMED )
      _VirtualNotify = CallbackBase();
   else
      _VirtualNotify = _DisarmedCallback;
   
   //
   // Only send the notification if we are resetting the timer or
   //  disarming an armed timer. ( If the timer was already disarmed
   //  and we are still wanting it in a disarmed state then don't send
   //  any message packets to the Timer task ).
   if (    newState != DISARMED 
        || newState != _TimerArmed )
   {
      _TimerArmed = newState;

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
   
      if ( MessageSystem::MsgSystem() )
         MessageSystem::MsgSystem()->dispatcher().sendTimerMessage( mp );
      else
         _FATAL_ERROR( __FILE__, __LINE__, "Message system not initialized" );
   }
}


void TimerMessage :: interval( unsigned long interval)
{
   Message< unsigned long >::setData( interval );
   armTimer();
}


void TimerMessage :: deregisterTimer()
{
   if ( MessageSystem::MsgSystem() )
      //
      // Deregister this timer from the dispatcher ...
      MessageSystem::MsgSystem()->dispatcher().deregisterMessage( MessageBase::msgId(), *this );
   else
      _FATAL_ERROR( __FILE__, __LINE__, "Message system not initialized" );
   
}

const char *TimerMessage::genMsgName() 
{
   char buffer[21];
   sprintf( buffer, "Timer%lx%lx", (unsigned long)this, getNetworkAddress() );
   buffer[20] = '\0';
   _MessageName = buffer;

   return (const char *)_MessageName.c_str();
}
