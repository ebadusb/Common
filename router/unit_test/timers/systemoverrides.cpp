/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      systemoverrides.cpp
 *
 */

#include <arpa/inet.h>
#include "systemoverrides.h"

unsigned long getNetworkAddress( void )
{
   // return ( inet_addr( genenv( "LOCAL_INET_ADDR" ) ) );
   return inet_addr( "90.0.0.1" );
}

void getNetworkedNodes( map< short, unsigned long > &portsAndNodes )
{
   /*unsigned long no1 = inet_addr( "90.0.0.1" );
   unsigned long no2 = inet_addr( "90.0.0.2" );
   */
}
                   
long spawnGateway( char* gatewayName, FUNCPTR func, const short portNo )
{
   long tId = ERROR;
   if (    ( tId = taskNameToId( gatewayName ) ) != ERROR
        || (    taskNameToId( gatewayName ) == ERROR
             && ( tId = taskSpawn ( gatewayName , 4, 0, 20000, func, portNo,
                                    0,0,0,0,0,0,0,0,0) ) == ERROR 
           )
      )
   {
      return ERROR;
   }
   return tId;
}

