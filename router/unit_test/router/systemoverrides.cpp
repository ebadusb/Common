/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      systemoverrides.cpp
 *
 */

#include "systemoverrides.h"

unsigned long getNetworkAddress( void )
{
   return ( inet_addr( genenv( "LOCAL_INET_ADDR" ) ) );
}

void getNetworkedNodes( set< unsigned long > &nodes )
{
   unsigned long no1 = inet_addr( "90.0.0.1" );
   unsigned long no2 = inet_addr( "90.0.0.2" );

   if ( inet_addr( genenv( "LOCAL_INET_ADDR" ) ) != no1 )
      nodes.insert( inet_addr( "90.0.0.1" ) );
   else if ( inet_addr( genenv( "LOCAL_INET_ADDR" ) ) != no2 )
      nodes.insert( inet_addr( "90.0.0.2" ) );
}
                   
