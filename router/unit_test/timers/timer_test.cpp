/* router_test.cpp - demonstrate the router functionality */

#include <vxWorks.h>
#include <sys/fcntlcom.h>
#include <taskLib.h>
#include <wvLib.h>
#include <stdio.h> 

#include <iostream.h>
#include <typeinfo>

#include "message.h"
#include "messagesystem.h"
#include "router.h"
#include "msgsystimer.h"
#include "timermessage.h"

void timertask1();
void timertask2();
void cbTimer20();
void cbTimer50();
void cbTimer100();
void timerInterrupt();

STATUS timerTest (void)
{
   if (    taskNameToId( "tRouter" ) == ERROR
        && taskSpawn ("tRouter", 60, 0, 20000,
                      (FUNCPTR) Router::Router_main , 0,0,0,0,0,0,0,0,0,0) == ERROR )
   {
      cout << "Could not spawn router task" << endl;
      return ERROR;
   }

   if (    taskNameToId( "tTimerTask" ) == ERROR
        && taskSpawn ("tTimerTask", 60, 0, 20000,
                      (FUNCPTR) MsgSysTimer::MsgSysTimer_main , 0,0,0,0,0,0,0,0,0,0) == ERROR )
   {
      cout << "Could not spawn timer task" << endl;
      return ERROR;
   }

   // if (    taskNameToId( "timerInterrupt" ) == ERROR 
        // && taskSpawn ("timerInterrupt", 4, 0, 20000,
                   // (FUNCPTR) ::timerInterrupt , 0,0,0,0,0,0,0,0,0,0) == ERROR )
   // {
      // cout << "Could not spawn timerInterrupt task" << endl;
      // return ERROR;
   // }

   if ( taskSpawn ("timertask1", 70, 0, 20000,
                   (FUNCPTR) ::timertask1 , 0,0,0,0,0,0,0,0,0,0) == ERROR )
   {
      cout << "Could not spawn task" << endl;
      return ERROR;
   }

   if ( taskSpawn ("timertask2", 70, 0, 20000,
                   (FUNCPTR) ::timertask2 , 0,0,0,0,0,0,0,0,0,0) == ERROR )
   {
      cout << "Could not spawn task" << endl;
      return ERROR;
   }

   return OK;

}

void timertask1()
{
   MessageSystem ms;
   ms.initBlocking();

   TimerMessage tm100;
   TimerMessage tm500;
   TimerMessage tm1000;
   tm100.init( 100, CallbackBase( &::cbTimer20 ) );
   tm500.init( 500, CallbackBase( &::cbTimer50 ) );
   tm1000.init( 1000, CallbackBase( &::cbTimer100 ) );

   ms.dispatchMessages();

}

void cbTimer20()
{
   cout << hex << taskIdSelf() << " - Received timer notification for 100 ms message" << endl;
}

void cbTimer50()
{
   cout << hex << taskIdSelf() << " - Received timer notification for 500 ms message" << endl;
}

void cbTimer100()
{
   cout << hex << taskIdSelf() << " - Received timer notification for 1000 ms message" << endl;
}

class timertask2Obj
{
public:
   timertask2Obj() : _tm() {}
   
   virtual ~timertask2Obj() {}

   void startup()
   {
      MessageSystem ms;
      ms.initBlocking();
   
      _tm.init( 200, Callback< timertask2Obj >( this, &timertask2Obj::cbTimerMsg ) );

      ms.dispatchMessages();
      
   }
   void cbTimerMsg( void *d)
   {
      cout << hex << taskIdSelf() << " - Recieved timer notification for 200 ms message" << endl;
   }

protected:
   TimerMessage _tm;
};

void timertask2()
{
   timertask2Obj t2O;
   t2O.startup();
}

void timerInterrupt()
{
   timespec tspec = { 5,0 };
   nanosleep( &tspec, 0 );

   timespec ts = { 0,50000 };

   //
   // open the queue
   mqd_t timerQueue;
   if ( ( timerQueue = mq_open( "timertask", O_WRONLY ) ) == (mqd_t) ERROR )
   {
      cout << "Unable to open the timertask queue" << endl;
      return;
   }

   MessagePacket tickPacket;
   tickPacket.msgData().osCode( MessageData::TIME_UPDATE );
   tickPacket.msgData().msgLength( sizeof( long long ) );
   tickPacket.msgData().nodeId( 0 );
   tickPacket.msgData().taskId( taskIdSelf() );
   tickPacket.msgData().totalNum( 1 );
   tickPacket.msgData().seqNum( 1 );

   long long tickCount = 1;
   while ( 1 )
   {
      tickPacket.msgData().msg( (unsigned char*)&tickCount, sizeof( long long ) );
      tickPacket.updateCRC();

      mq_send( timerQueue, &tickPacket, sizeof( MessagePacket ), 31 );

      tickCount += 10;
      nanosleep( &ts, 0 );
   }
}

/*

---- Tests for Router and Dispatcher classes ----

                  
TESTS ------------------------------------------------------------------
  
1. 
2. 
3. 
4.


*/


