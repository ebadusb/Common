/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      tcpconnect.h, TCP connection routines
 *
 * ABSTRACT:   This file defines the interface for functions which connect
 *              both sides of a TCP socket
 */

#ifndef _TCP_CONNECT_H_
#define _TCP_CONNECT_H_

#include "sockinet.h"

sockinetbuf *tcpConnect ( const unsigned long address, 
                          const int port, 
                          const unsigned long millisecs );

sockinetbuf *tcpAcceptConnect( unsigned long address, int port );


#endif
