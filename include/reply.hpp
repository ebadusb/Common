/*
 * Copyright (c) 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * TITLE:      reply.hpp
 *
 * AUTHOR:     Tate Moore
 *
 * ABSTRACT:   This file implements the header for the reply object. 
 *
 * DOCUMENTS
 * Requirements:
 * Test:
 *
 */

#ifndef REPLY_HPP  //  Prevents multiple compilations.
#define REPLY_HPP

#include "msghdr.h"
#include "crc.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>     // added for inet_ functions
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>

#define IP_LENGTH 16

template <class the_struct>
class UDP_Reply
{
   private:
      int                iAN2Socket;  // FS_AN2_SERVICE_PORT socket descriptor  
      int                iAN2Port;
      unsigned short     msgLength;
      unsigned short     _msgID;
      MSG*               pfsan2_msg;
      void*              reply_msg_buff;
      MSGHEADER*         hdr;
      struct sockaddr_in client;
      char               szIP[IP_LENGTH]; // IP format ip addreess : xxx.xxx.xxx
      char log_string[256];
   public:
      UDP_Reply() {};
      // Constructors.
      UDP_Reply( char* szHostIP, char* szHostPort, unsigned short msgID ) :
      _msgID(msgID) 
      {
		 UDP_Init(szHostIP, szHostPort, sizeof(the_struct));
      };

      UDP_Reply( char* szHostIP, char* szHostPort, unsigned short msgID,
		int MsgLen ) : _msgID(msgID) 
      {
		 UDP_Init(szHostIP, szHostPort, MsgLen);
      };

      UDP_Init( char* szHostIP, char* szHostPort, int len)
		{
         /* Create socket as an Internet Datagram */
         iAN2Socket = socket(AF_INET, SOCK_DGRAM, 0);
   
         if (iAN2Socket < 0)
         {
            _FATAL_ERROR( __FILE__, __LINE__, TRACE_PROC, (int)iAN2Socket,
                          "Datagram socket could not be created" );
         }

         // Assign the IP of the reply to address.
         client.sin_family = AF_INET;
         // Assign the Port of the reply to address.
         iAN2Port = atoi( szHostPort );
         client.sin_port = htons( (unsigned short)iAN2Port );

         // If the Host is NULL then this is a broadcast message.
         if ( szHostIP == NULL )
         {
            int on = 1;
            setsockopt(iAN2Socket, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
#if 0	// wont compile, trying to find out why (rtd)
			// get the broadcast host name
			if(ioctl(iAN2Socket,SIOCGIFBRDADDR,szIP) == -1)
			{
				sprintf(log_string,"ioctl of iAN2Socket failed errno=%d\n",errno);					
         		_LOG_ERROR ( __FILE__, __LINE__, TRACE_PROC, 0, log_string );
			}
			else
			{
				sprintf(log_string,"Broadcast address is %s\n",szIP);
			}
#endif
			// get the broadcast host name from globvars
	         char *pTrimaIP        = getenv( "AN2BIP" );
			 if(pTrimaIP)
			 {
				 strcpy(szIP,pTrimaIP);
		         inet_aton( (char*)szIP, 
	   		         (struct in_addr*)&(client.sin_addr.s_addr) ); 
		         sprintf( log_string, "broadcast IP is %s", szIP);
       			 _LOG_ERROR ( __FILE__, __LINE__, TRACE_PROC, 0, log_string );
			 }
			 else
			 {
         		_LOG_ERROR ( __FILE__, __LINE__, TRACE_PROC, 0, 
					"Broadcast:Cant getenv AN2BIP,using NULL broadcast address" );
				szIP[0] = '\0';
			 }
         }
         else
         {
            strncpy( szIP, szHostIP, IP_LENGTH );
	         inet_aton( (char*)szHostIP, 
   	         (struct in_addr*)&(client.sin_addr.s_addr) ); 
         }
      

         // Create a buffer for the reply message.
         msgLength = (unsigned short) (len + sizeof( MSGHEADER ));
         reply_msg_buff      = new char[msgLength];
         pfsan2_msg       = (MSG*)reply_msg_buff;

         // Stuff the basic message header.
         hdr                   = (MSGHEADER*)&(pfsan2_msg->header);
         hdr->osCode           = MSG_MULTICAST;     // router code
         hdr->length           = msgLength;         // total length, bytes
         hdr->msgID            = _msgID;            // msg id
         // Use the taskPID as a message counter.
         hdr->taskPID          = 0;              
         // The task ID is really the IP address of the machine as a long
         // for all replies...
         char *pTrimaIP        = getenv( "AN2IP" );
         hdr->taskNID          = (long)inet_addr( pTrimaIP );
		};

      // Destructor.
      ~UDP_Reply() 
      {
         // Close the opened socket.
         close(iAN2Socket);

         // Delete the reply message buffer.
         if (reply_msg_buff != NULL)
         {
            delete [] reply_msg_buff;
         }
      };

      // Methods.
      send_reply( the_struct* msg_struct )
      {
         // Copy the new data to the focus message.
         memcpy( (void *)&pfsan2_msg->data, (void const *)msg_struct, sizeof( the_struct ) );

         // time stamp message b4 send out to 
         // allow for t-sync on external computer for
         // msgs originating on differnet procesors
         clock_gettime( CLOCK_REALTIME, &hdr->sendTime );

         // Update the CRC of the message being sent.
         updateFocusMsgCRC( (void *)pfsan2_msg );

#ifdef DEBUG
         // Log the sending of the reply.
         sprintf( log_string, "Sending reply message #%d to ip #%s on port #%d", 
                  _msgID, szIP, iAN2Port );
         _LOG_ERROR ( __FILE__, __LINE__, TRACE_PROC, 0, log_string );
#endif

         // Send the reply message.
         sendto( iAN2Socket, reply_msg_buff,
                 ((MSGHEADER*)pfsan2_msg)->length,
                 0,
                 (struct sockaddr*) &client,
                 sizeof( client) );
         
         // Update the message counter.         
         (hdr->taskPID)++;
      };

		send_string_message( char *msg )
		{
			_send_message((void *) msg, strlen(msg));
		}		

      send_message( the_struct* msg_struct )
		{
			_send_message((void *) msg_struct, sizeof(the_struct));
		}

      _send_message( void *msg, int len)
      {
         // Copy the new data to the focus message.
         memcpy( (void *)&pfsan2_msg->data, (void const *)msg, len );

         // time stamp message b4 send out to 
         // allow for t-sync on external computer for
         // msgs originating on differnet procesors
         clock_gettime( CLOCK_REALTIME, &hdr->sendTime );

         // Update the CRC of the message being sent.
         updateFocusMsgCRC( (void *)pfsan2_msg );

#ifdef DEBUG
         // Log the sending of the reply.
         sprintf( log_string, "send_message: #%d to ip #%s on port #%d", 
                  _msgID, szIP, iAN2Port );
         _LOG_ERROR ( __FILE__, __LINE__, TRACE_PROC, 0, log_string );
#endif
         // Send the reply message.
         sendto( iAN2Socket, reply_msg_buff,
                 ((MSGHEADER*)pfsan2_msg)->length,
                 0,
                 (struct sockaddr*) &client,
                 sizeof( client) );
         
         // Update the message counter.
         (hdr->taskPID)++;
      };

};

#endif
