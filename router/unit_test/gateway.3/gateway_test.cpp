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

#ifdef __cplusplus 
extern "C" int gatewayTest();
#else 
int gatewayTest();
#endif

void testtask();
void cbIntMsg( void *ptr );

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

   if ( taskSpawn ("testtask", 15, 0, 20000,
                   (FUNCPTR) ::testtask , 0,0,0,0,0,0,0,0,0,0) == ERROR )
   {
      cout << "Could not spawn task" << endl;
      return ERROR;
   }

   return OK;

}

void testtask()
{
   MessageSystem ms;
   ms.initBlocking();

   Message< int > intMsg;
   intMsg.init( CallbackBase( &cbIntMsg, ((void*)&intMsg) ), MessageBase::SNDRCV_GLOBAL );

   ms.dispatchMessages();

}

void cbIntMsg( void *ptr )
{
   Message< int > *msg = (Message< int >*)ptr;
   msg->dump( cout );
}

/*

---- Tests for Router and Dispatcher classes ----

                  
TESTS ------------------------------------------------------------------
  
1. 
2. 
3. 
4.


*/


