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

void routertask1();
void routertask2();

STATUS routerTest (void)
{
   if (    taskNameToId( "tRouter" ) == ERROR
        && taskSpawn ("tRouter", 4, 0, 20000,
                      (FUNCPTR) Router::Router_main , 0,0,0,0,0,0,0,0,0,0) == ERROR )
   {
      cout << "Could not spawn task" << endl;
      return ERROR;
   }

   if ( taskSpawn ("routertask1", 15, 0, 20000,
                   (FUNCPTR) ::routertask1 , 0,0,0,0,0,0,0,0,0,0) == ERROR )
   {
      cout << "Could not spawn task" << endl;
      return ERROR;
   }

   if ( taskSpawn ("routertask2", 10, 0, 20000,
                   (FUNCPTR) ::routertask2 , 0,0,0,0,0,0,0,0,0,0) == ERROR )
   {
      cout << "Could not spawn task" << endl;
      return ERROR;
   }

   return OK;

}

void routertask1()
{
   MessageSystem ms;
   ms.initBlocking();

   timespec tspec = { 5,0 };
   nanosleep( &tspec, 0 );

   cout << "Send int message" << endl;
   Message<int> mi( MessageBase::SEND_LOCAL );

   mi.send( 35 );

   /*
   nanosleep( &tspec, 0 );
   
   nanosleep( &tspec, 0 );

   nanosleep( &tspec, 0 );

   nanosleep( &tspec, 0 );
   
   nanosleep( &tspec, 0 );
   */
}

class routertask2Obj
{
public:
   routertask2Obj() : _mi() {}
   
   virtual ~routertask2Obj() {}

   void startup()
   {
      MessageSystem ms;
      ms.initBlocking();
   
      _mi.init( Callback< routertask2Obj >( this, &routertask2Obj::cbIntMsg, &_mi ) , MessageBase::SNDRCV_RECEIVE_ONLY );

      Router::globalRouter()->dump( cout );
      MessageSystem::MsgSystem()->dispatcher().dump( cout );  
      ms.dispatchMessages();
      
      cout << "OUT of Dispatch Loop" << endl;

      Router::globalRouter()->dump( cout );

      _mi.init( Callback< routertask2Obj >( this, &routertask2Obj::cbIntMsg ) , MessageBase::SNDRCV_LOCAL );
      Router::globalRouter()->dump( cout );
      _mi.send(36);

      ms.dispatchMessages();
   }

   void cbIntMsg( void *d)
   {
      if ( d )
         cout << "Recieved int message : data " << ((Message<int>*)d)->getData() << endl;
      else
         cout << "Recieved int message : data " << _mi.getData() << endl;

      _mi.deregisterMsg();

      Router::globalRouter()->dump( cout );

      MessageSystem::MsgSystem()->dispatcher().stopLoop();  
   }
protected:
   Message<int> _mi;
};

void routertask2()
{
   routertask2Obj t2O;
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


