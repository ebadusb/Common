/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      connector.h, Message System inter-processor communication task
 *
 */

#ifndef _CONNECTOR_H_
#define _CONNECTOR_H_

#include <vxWorks.h>

/*
 * C/C++ functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

//
// This function will act as the main entry point for the connector task.  This
//  function will connect with the remote socket.  When the connection is established
//  it will send a message to the Router to inform the router of the status.
int Connector_main( unsigned long netAddress, int sock );

//
// This function will return the TCP port to which all requests to remote
//  nodes will be attempted.
short getGatewayPort();

#ifdef __cplusplus
}; // extern "C"
#endif /* ifdef __cplusplus */

#endif
