/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      tcpconnect.cpp
 *
 */

#include <errnoLib.h>
#include "tcpconnect.h"

sockinetbuf *tcpConnect ( const unsigned long address, 
                          const int port, 
                          const unsigned long millisecs )
{
   sockinetbuf *si = new sockinetbuf( sockbuf::sock_stream );
   if ( !si )
   {
      cout << "Create socket buffer failed" << endl;
      return (sockinetbuf*)ERROR;
   }

   struct timeval tv;
   tv.tv_sec = 0;
   tv.tv_usec = millisecs * 1000;
   int status = si->connectWithTimeout( address, port, &tv );
   if ( status != OK )
   {
      cout << "Connect failed -- errno " << errnoGet() << endl;
      return (sockinetbuf*)0;
   }

   return si;
}


sockinetbuf *tcpAcceptConnect( unsigned long address, int port )
{
   sockinetbuf *si = new sockinetbuf( sockbuf::sock_stream );
   if ( !si )
   {
      cout << "Create socket buffer failed" << endl;
      return (sockinetbuf*)0;
   }

   si->bind( address, port );
   si->listen();

   sockinetaddr clientAddress;
   sockbuf clientSocket = si->accept( clientAddress );

   return si;
}


