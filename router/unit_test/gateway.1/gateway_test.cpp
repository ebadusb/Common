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

void testtask2();

#ifdef __cplusplus 
extern "C" int gatewayTest();
#else 
int gatewayTest();
#endif

int gatewayTest()
{
   if (    taskNameToId( "tRouter" ) == ERROR
        && taskSpawn ("tRouter", 4, 0, 20000,
                      (FUNCPTR) Router::Router_main , 0,0,0,0,0,0,0,0,0,0) == ERROR )
   {
      cout << "Could not spawn router task" << endl;
      return ERROR;
   }

   if (    taskNameToId( "tTimerTask" ) == ERROR
        && taskSpawn ("tTimerTask", 4, 0, 20000,
                      (FUNCPTR) MsgSysTimer::MsgSysTimer_main , 0,0,0,0,0,0,0,0,0,0) == ERROR )
   {
      cout << "Could not spawn timer task" << endl;
      return ERROR;
   }

   if ( taskSpawn ("testtask2", 15, 0, 20000,
                   (FUNCPTR) ::testtask2 , 0,0,0,0,0,0,0,0,0,0) == ERROR )
   {
      cout << "Could not spawn task" << endl;
      return ERROR;
   }

   return OK;

}

class testtask2Obj
{
public:
   testtask2Obj() : _mi() {}
   
   virtual ~testtask2Obj() {}

   void startup()
   {
      MessageSystem ms;
      ms.initBlocking();
   
      timespec tspec = { 5,0 };
      nanosleep( &tspec, 0 );

      _mi.init( Callback<testtask2Obj>( this, &testtask2Obj::cbIntMsg ), MessageBase::SNDRCV_GLOBAL );
      _mi.send( 1 );

      tspec.tv_sec = 1;
      nanosleep( &tspec, 0 );

      _mi.send( 2 );

      nanosleep( &tspec, 0 );

      _mi.send( 3 );

      nanosleep( &tspec, 0 );

      _mi.send( 4 );

      nanosleep( &tspec, 0 );

      _mi.send( 5 );

      nanosleep( &tspec, 0 );

      // Router::globalRouter()->dump( cout );
      // MessageSystem::MsgSystem()->dispatcher().dump( cout );  

      _mi.init( Callback<testtask2Obj>( this, &testtask2Obj::cbIntMsg ), MessageBase::SNDRCV_LOCAL );

      // Router::globalRouter()->dump( cout );

      _mi.send( 6 );

      ms.dispatchMessages();
   }

   void cbIntMsg( void *d)
   {
      if ( d )
         cout << "Recieved int message : data " << ((Message<int>*)d)->getData() << endl;
      else
         cout << "Recieved int message : data " << _mi.getData() << endl;

      // _mi.deregisterMsg();

      // Router::globalRouter()->dump( cout );

      // MessageSystem::MsgSystem()->dispatcher().stopLoop();  
   }
   void cbTimerMsg( void *d)
   {
      cout << "Recieved timer message : 200 ms " << endl;

      // Router::globalRouter()->dump( cout );
   }
protected:
   Message<int> _mi;
   TimerMessage _tm;
};

void testtask2()
{
   testtask2Obj t2O;
   t2O.startup();
}

/*

---- Tests for Router and Dispatcher classes ----

                  
TESTS ------------------------------------------------------------------
  
1. 
2. 
3. 
4.


*/


