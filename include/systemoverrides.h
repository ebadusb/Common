/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      systemoverrides.h, Functions which users must override.
 *
 * ABSTRACT:   This file contains functions which users must provide to
 *             use the messaging system.  
 */

#ifndef _SYSTEM_OVERRIDES_H_
#define _SYSTEM_OVERRIDES_H_

#include <map>

/*
 The functions in this header file must be provide to handle non-recoverable
  errors
*/
#include "error.h"

/*
 This function returns the local network address
*/
unsigned long getNetworkAddress( void );

/*
 This function takes in a user provided set.  The function will fill
  the map with all nodes for which the network must connect.  
  * The map will be keyed on the socket port for with the TCP 
    connection will be made.  The port must be distinct for each 
    node from all other connections on the system.
  * The data in the set must assigned the IP address of the networked
    processors.  
  * Add the local network address and port for which remote users can
    connect.
  * This function will get called upon Router task initialization.  
    Do not add any functionality which will not be usable when the 
    Router is started. i.e. Everything used in this function must be 
    available and initialized befor the Router and Gateway tasks 
    start.
*/
void getNetworkedNodes( map< short, unsigned long > &nodesAndPorts );

/*
 This function spawns the gateway process with the given name and 
  takes the portNo as an argument.
*/
long spawnGateway( char *gatewayName, FUNCPTR func, const short portNo );
                   
/*
 This function spawns the connector process with the given name and 
  takes a network address and a socket file descriptor as arguments.
*/
long spawnConnector( char *connectorName, FUNCPTR func, const unsigned long netAddress, const int sock );
                   
#endif
