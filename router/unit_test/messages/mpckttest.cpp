/* mpckttest.cpp - demonstrate the typeinfo function */

#include <vxWorks.h>
#include <taskLib.h>
#include <wvLib.h>
#include <stdio.h> 

#include <iostream.h>
#include <typeinfo>

#include "messagepacket.h"

STATUS msgPcktTest (void)
{

   MessagePacket pkt1, pkt2, pkt3;
   cout << "Message Size: " << sizeof( MessagePacket ) << endl;

   struct timespec ts;
   clock_gettime( TIMER_ABSTIME, &ts );

   pkt1.msgData().osCode( MessageData::MESSAGE_REGISTER );
   pkt1.msgData().msgId( 0xffffffff );
   pkt1.msgData().msgLength( sizeof( MessagePacket ) );
   pkt1.msgData().nodeId( 1 );
   pkt1.msgData().taskId( taskIdSelf() );
   pkt1.msgData().seqNum( 1 );
   pkt1.msgData().totalNum( 1 );
   pkt1.msgData().msg( (const unsigned char *)"This is my test", strlen( "This is my test") );
   pkt1.msgData().packetLength( strlen( "This is my test" ) );
   pkt1.msgData().sendTime( ts );

   pkt2 = pkt1;

   cout << dec << pkt1.msgData().osCode() << endl;
   cout << dec << pkt2.msgData().osCode() << endl;
   cout << hex << pkt1.msgData().msgId() << endl;
   cout << hex << pkt2.msgData().msgId() << endl;
   cout << hex << pkt1.msgData().nodeId() << endl;
   cout << hex << pkt2.msgData().nodeId() << endl;
   cout << hex << pkt1.msgData().taskId() << endl;
   cout << hex << pkt2.msgData().taskId() << endl;
   cout << dec << pkt1.msgData().totalNum() << endl;
   cout << dec << pkt2.msgData().totalNum() << endl;
   cout << dec << pkt1.msgData().msgLength() << endl;
   cout << dec << pkt2.msgData().msgLength() << endl;
   cout << dec << pkt1.msgData().packetLength() << endl;
   cout << dec << pkt2.msgData().packetLength() << endl;
   cout << dec << pkt1.msgData().sendTime().tv_sec << " ";
   cout << dec << pkt1.msgData().sendTime().tv_nsec << endl;
   cout << dec << pkt2.msgData().sendTime().tv_sec << " ";
   cout << dec << pkt2.msgData().sendTime().tv_nsec << endl;
   cout << dec << pkt1.msgData().seqNum() << endl;
   cout << dec << pkt2.msgData().seqNum() << endl;
   cout << pkt1.msgData().msg() << endl;
   cout << pkt2.msgData().msg() << endl;

   if ( pkt1 == pkt2 )
      cout << "Pkt1 equals Pkt2" << endl;
   else
      cout << "Pkt1 is not equal Pkt2" << endl;

   pkt2.msgData().msgId( 0xfffffffe );
   if ( pkt1 == pkt2 )
      cout << "Pkt1 equals Pkt2" << endl;
   else
      cout << "Pkt1 is not equal Pkt2" << endl;

   return OK;
}

/*

---- Tests for MessagePacket, MessageData, and MessageBase classes ----

Functional capabilities of the classes:

MessageData   ->  encapsulates the packetized message data
                  and allows for easy copying of MessageData class

MessagePacket ->  provides for access to MessageData class
                  provides CRC generation and validation of MessageData class
                  allows for easy copying of MessagePacket class
                  holds the unopened flag
                 
MessageBase   ->  allows for reinitialization of messages
                  packetizes the messages
                  registers and deregisters the messages
                  provides for application notification after all packets have been received
                  transfers data to/from MessagePacket's
                  generates message id's
                  sets message sent time
                  
TESTS ------------------------------------------------------------------
  
1. Tests MessageData accessor's
2. Tests MessageData operator=
3. Tests MessageData operator==
4.


*/


