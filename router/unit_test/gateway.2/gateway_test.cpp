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

   return OK;

}

/*

---- Tests for Router and Dispatcher classes ----

                  
TESTS ------------------------------------------------------------------
  
1. 
2. 
3. 
4.


*/


