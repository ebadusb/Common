/* mpckttest.cpp - demonstrate the typeinfo function */

#include "vxWorks.h"
#include "sys/fcntlcom.h"
#include "taskLib.h"
#include "wvLib.h"
#include "stdio.h" 

#include <iostream.h>
#include <typeinfo>

#include "messagesystem.h"
#include "trimamsgs.h"

class temp
{
public:
   long _Array[50];
};

class temporaryClass
{
public:
   char _Array[600];
};

void callback( void *msg ) {  cout << "Recieved message" << endl; };
void testMsgSystem();

STATUS msgPcktTest (void)
{
   mqd_t mqRouter;
   struct mq_attr attr;             /* // message queue attributes */

   // open queue
   attr.mq_maxmsg =  4;                       /* // set max number of messages */
   attr.mq_msgsize = sizeof( MessagePacket );  /* // set message size */
   attr.mq_flags =   O_NONBLOCK;               /* // set non-block */

   mqRouter = mq_open( "router", O_RDWR | O_CREAT | O_NONBLOCK , 0666, &attr);

   MessageSystem msgSystem;
   msgSystem.initBlocking();

   cout << "--------------- Dispatch ---------------------------------" << endl;
   MessageSystem::MsgSystem()->dispatcher().dump( cout );

   testMsgSystem();

   cout << "--------------- Dispatch ---------------------------------" << endl;
   MessageSystem::MsgSystem()->dispatcher().dump( cout );

   char buffer[ attr.mq_msgsize ];
   char ch[2];
   int size=0;
   cout << "Size the message should be: " << attr.mq_msgsize << endl;
   while ( ( size = mq_receive( mqRouter, &buffer, attr.mq_msgsize, 0 ) ) != ERROR )
   {
      cout << dec << "Size: " << size << " ";
      for ( unsigned int i=0 ; i<attr.mq_msgsize ; i++ )
      {
         sprintf( ch,"%2x",(short)buffer[i] );
         cout << ch;
      }
      cout << endl;
      MessagePacket mp;
      memmove( &mp, &buffer, attr.mq_msgsize );
      mp.dump( cout );
      cout << endl;
   }

   mq_close( mqRouter );

   return OK;
}

void testMsgSystem()
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

   ACConnected accon( ACConnected::SEND_LOCAL );
   cout << typeid(accon).name() << endl;
   accon.dump( cout );
   AckRunComplete ackrc( AckRunComplete::SEND_LOCAL );
   cout << typeid(ackrc).name() << endl;
   ackrc.dump( cout );

   Message<temp> mb( MessageBase::SEND_LOCAL );
   mb.dump( cout );
   Message<temporaryClass> mb1( MessageBase::SEND_LOCAL );
   mb1.dump( cout );
   temporaryClass tc;
   strcpy( tc._Array, "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789" );

   mb1.setData( tc );
   mb1.dump( cout );

   cout << "Data :" << mb1.getData()._Array << endl;

   cout << "--------------- Dispatch ---------------------------------" << endl;
   MessageSystem::MsgSystem()->dispatcher().dump( cout );
   cout << "--------------- Create MB2 --------------------------------------" << endl;
   Message<temp> mb2;
   mb2.init( CallbackBase( & ::callback, &mb2 ), MessageBase::SNDRCV_RECEIVE_ONLY );
   cout << "--------------- MB2 --------------------------------------" << endl;
   mb2.dump( cout );
   cout << "--------------- Dispatch ---------------------------------" << endl;
   MessageSystem::MsgSystem()->dispatcher().dump( cout );
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


