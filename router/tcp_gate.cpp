/*
 * Copyright (c) 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: Z:/BCT_Development/Common/ROUTER/rcs/TCP_GATE.CPP 1.4 1999/08/10 00:09:03 TD10216 Exp MS10234 $
 * $Log: TCP_GATE.CPP $
 * Revision 1.3  1999/07/24 21:07:35  TD10216
 * IT4154
 * Revision 1.2  1999/06/30 21:40:21  TD10216
 * IT3908
 * Revision 1.1  1999/05/24 23:29:22  TD10216
 * Initial revision
 * Revision 1.7  1998/09/17 18:35:28  TM02109
 * Added a message counter in the taskPID.
 * Revision 1.6  1997/06/30 17:29:47  SM02805
 * Modified tcp gate header contents to put the unsigned long rep
 * of the FS IP into the taskNID field of the fs msg header. NID is always
 * '2' now for FS configuration, so no loss of data is caused by this change. 
 * The benefit is that now client apps can interrogate the header taskNID
 * to determine IP address ( identity ) of sender. 
 * Revision 1.5  1997/05/16 22:35:13  SM02805
 * Modified operation of tcp/ip gateway to timestamp the msgs 
 * prior to exportation. This provides the ability for an external computer 
 * to t-sync the FS msgs that originate on different processors 
 * ( control and safety ) .  
 * Revision 1.4  1996/12/30 20:16:50  SS03309
 * remove warning
 * Revision 1.3  1996/12/09 15:58:56  TM02109
 * Modified implementation of socket construction for TCP/IP gateway 
 * port : invoke tcp_gate with 5 params : parms 1 - 3 std router start up 
 * ( localQ,remote node,remoteQ) 
 * parm 4 : by an2 host ip in xxx.xxx.xxx.xxx format 
 * parm 5 : an2 host tcp/ip gate port.
 * Previous impl used argv[4] for an2 host name.	
 *  
 * 
 * Revision 1.2  1996/12/06 15:47:58  SS03309
 * tcp gateway and spoofer
 * Revision 1.1  1996/12/04 18:33:46  SS03309
 * Initial revision
 *
 * TITLE:		Focussed System message TCP/IP gateway.
 *
 * ABSTRACT:	The TCP/IP gateway sends (selected) messages from the
 * control computer's router to the outside world. By default, 
 * (Trima Releases 3.3a and above), tcp_gate forwards no messages and
 * only looks at the TcpgateRegistration message. That particular 
 * message provides a list of MsgIds which a client wishes to receive
 *
 * The following signals cause the gateway to
 * gracefully (closing resources) terminate:
 *	SIGHUP
 *	SIGINT
 *	SIGQUIT
 *	SIGTERM
 *	SIGPWR
 *
 * DOCUMENTS
 * Requirements:	I:\ieee1498\SSS2.DOC
 * Test:				I:\ieee1498\STD2.DOC
 *
 * PUBLIC FUNCTIONS:
 * main() - main entry point
 */

// #includes

#include <env.h>
#include <errno.h>
#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <sys/osinfo.h>
#include <sys/proxy.h>
#include <sys/sched.h>
#include <sys/types.h>
#include <sys/vc.h>
#include <time.h>
#include <unistd.h>

// tcp/ip stuff
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>	// added for inet_ functions

#include "intmsgs.h"
#include "buffmsg.hpp"
#include "crc.h"
#include "error.h"
#include "msghdr.h"
#include "sinver.h"
#include "an2msgs.h"
#include "reply.hpp"
#include "RegisterMsgs.hpp"

#define Q_LENGTH 80			// queue name length
#define MAX_MSGS 64			// max msgs in queue

#define QNX_ERROR (-1)

#define FS_SERVER_PORT	7030				// TCP/IP port number

static volatile unsigned char taskRunning=1;// task running
static mqd_t	gq;							// gateway queue
static char	gatewayQueueName[Q_LENGTH];		// gateway q name
static struct	sockaddr_in server;			// socket address
static int		sock = QNX_ERROR;			// socket
static mqd_t	routerQueue;				// router queue
static char		szAN2IP[32];				// External host IP address
static char		*szAN2Port;					// External host port #


// 06/30/97 msm make 'global' so each msg forwarded by
// tcp gate will have FSIP as taskNID
static unsigned long ulFSIP;
// Trima IP address
static char *pFSIP;

// Message Registration 
static RegisterMsgs MsgRegistrar;
// forward ref
static void openGatewayQueue(  char* qname,
										mqd_t* mq,
										pid_t* qproxy,
										struct sigevent* qnotify);
static void openRouterQueue( char* qname, mqd_t* mqd);
static void processGatewayQueue( mqd_t gq, struct sigevent* qnotify);


// SPECIFICATION:   Read File to get pre-registered Everest messages
//                  Parameter: none
//
// ERROR HANDLING:  none
#include "PreRegisterMsgs.h"


// SPECIFICATION:	signal handler, causes program to stop, called by QNX
//						Parameters:
//						signum - signal number, see signal.h *unused)
//
// ERROR HANDLING:	none.
void signalHandler( int )
{
	taskRunning = 0;
}


// SPECIFICATION:	signal handler, causes program to stop, called by QNX
//					Parameter:
//					signum - signal number being processed, see signal.h
//
// ERROR HANDLING:	none
static void fatalError( int line, int code, char* err)
{
	mq_close( routerQueue);								// close remote router queue
	mq_close( gq);											// close gateway queue
	mq_unlink( gatewayQueueName);						// remove it
	close( sock);
	_FATAL_ERROR( __FILE__, line, TRACE_GATEWAY, code, err);
}


// SPECIFICATION:	main entry point
//						argc and argv are used to access command line parameters
//						parameter 1 = local queue name
//									2 = remote node number
//									3 = remote queue name
//						12/09/96  4 - an2 host ip in xxx.xxx.xxx.xxx format
//									5 - an2 host tcp/ip gateway port 
//
// ERROR HANDLING:	Calls fatalError().
void main(int argc, char** argv)
{
	pid_t			pid;						// msg received from pid
	char			msg[BSIZE];				// receive message buffer
	time_t			lastConnect=0;			// last connect time
	struct sigevent qnotify;					// q notify
	pid_t			qproxy;					// q proxy
	struct sched_param param;

	// set priority and scheduling method as round robin
	setprio( 0, 11);							// set priority
	sched_getparam( 0, &param);
	sched_setscheduler( 0, SCHED_RR, &param);

	// check parameter list
	if (argc < 4)
	{
		fatalError( __LINE__, argc, "Not enough parameters");
	}

	// signal handling
	signal( SIGHUP, signalHandler);
	signal( SIGINT, signalHandler);
	signal( SIGQUIT, signalHandler);					// used by procedure
	signal( SIGTERM, signalHandler);
	signal( SIGPWR, signalHandler);

	// open Queues
	sprintf( gatewayQueueName, "%s%d", basename(argv[0]), getpid());
	openGatewayQueue(gatewayQueueName, &gq, &qproxy, &qnotify);
	openRouterQueue( argv[1], &routerQueue);

	// initialize version detection
	sinVerInitialize();

	// Get Pre-Registered Messages, dont get Registration messages
	PreRegisterMsgs(0);

	// loop processing messages
	while(taskRunning)
	{
		// open TCP/IP link
		if ((sock == QNX_ERROR) &&						// not yet open
			((time(NULL) - lastConnect) > 10))		// and ten seconds
		{
			// struct hostent* hp;
			struct hostent* gethostbyname();

			lastConnect = time(NULL);
			sock = socket( AF_INET, SOCK_DGRAM, 0);

			server.sin_family = AF_INET;

			// 12/09/96 new style
			// pass host ip address in xxx.xxx.xxx.xxx notation as argv[4]
			// pass port as argv[5]
			//
			inet_aton( (char*)argv[4], (struct in_addr*)&(server.sin_addr.s_addr) ); 

			// save the host name for Registration response	
			strcpy(szAN2IP, (char *) argv[4]);	

			// get the host response port
			int iAN2Port = atoi( argv[5] );
			szAN2Port = argv[5];
#if 0
			// test
			char* pIP = inet_ntoa( server.sin_addr );
			printf("An2 Host IP : %s\n", pIP );
			printf("Port : %d\n", iAN2Port );
#endif
			server.sin_port = htons( (unsigned short)iAN2Port );
	
			// 06/30/97 msm
			// get ip in xxx.xxx.xxx.xxx converted to unsigned long
			// for passing in NID field of msg header
			pFSIP = getenv( "AN2IP" );
			ulFSIP = inet_addr( pFSIP );
		}

		// wait for messages
		pid = Receive( 0, msg, sizeof( msg));
		if (pid == -1)
		{
			if (errno == EINTR)							// signal
			{
				continue;
			}
			fatalError( __LINE__, 0, "Receive()");
		}

		// process messages
		if (!sinVerMessage( pid, (SINVERMSG*) msg))
		{
			// check for q messages
			if (pid != qproxy)
			{
				Reply( pid, 0, 0);							// prevent lock-ups
			}
			else
			{
				processGatewayQueue( gq, &qnotify);
			}
		}
	}


	// de-register this task as a gateway with the router
	MSGHEADER h;

	h.osCode = TASK_DEREGISTER;			// os message code
	h.length = sizeof( MSGHEADER);			// total message length, bytes
	h.taskPID = getpid();						// task PID number
	h.taskNID = getnid();						// task NID
	clock_gettime( CLOCK_REALTIME, &(h.sendTime));
	updateFocusMsgCRC( &h);					// update CRC

	// send message to local router to de-register
	int k = 0;										// counter
	while( (mq_send( routerQueue, &h, h.length, 0) == MQ_ERROR) )
	{
		if((errno==EINTR) && (k<RETRY_COUNT))  // signals
		{
			k++;
		}
		else											// all other errors
		{
			_LOG_ERROR( __FILE__, __LINE__, TRACE_DISPATCHER, errno, "Deregister:mq_send()");
			break;
		}
	}

	// graceful exit (via signal)
	mq_close( gq);											// close gateway queue
	mq_unlink( gatewayQueueName);						// remove it
	mq_close( routerQueue);
	close( sock);
	exit(1);
}


// SPECIFICATION:	Open gateway queue.
//						Parameters:
//						*qname - queuue name string
//						*mqd - message queue descriptor
//						*qproxy - proxy used for this queue
//						*qnotify - message queue notify structure
//
// ERROR HANDLING:	Terminates program.
static void openGatewayQueue( char* qname,
							mqd_t* mqd,
							pid_t* qproxy,
							struct sigevent* qnotify)
{
	struct mq_attr attr;				// message queue attributes

	// open queue
	attr.mq_maxmsg = MAX_MSGS;		// max number of messages in queue
	attr.mq_msgsize = BSIZE;			// message size
	attr.mq_flags = MQ_NONBLOCK;	// set non-block communication
	*mqd = mq_open( qname, O_RDWR | O_CREAT, 0666, &attr);
	if (*mqd == -1)
	{
		fatalError( __LINE__, 0, "mq_open()");
	}

	// create proxy for message queue
	*qproxy = qnx_proxy_attach( 0, 0, 0, -1);
	if (*qproxy == -1)
	{
		fatalError( __LINE__, 0, "qnx_proxy_attach()");
	}

	// setup queue notify
	qnotify->sigev_signo = -(*qproxy);
	if (mq_notify( *mqd, qnotify) == -1)
	{
		fatalError( __LINE__, 0, "mq_notify()");
	}
	Trigger( *qproxy);					// avoid race condition by checking queue
}


// SPECIFICATION:	Open router queue, and register this task with router
//						Parameters:
//						*qname - queuue name string
//						*mqd - message queue descriptor
//
// ERROR HANDLING:	Terminates program.
static void openRouterQueue( char* qname, mqd_t* mqd)
{
	*mqd = mq_open( qname, O_WRONLY, 0, 0);
	if (*mqd == MQ_ERROR)						// open fail
	{
		_FATAL_ERROR( __FILE__, __LINE__, TRACE_DISPATCHER, 0, "mq_open()");
	}

	// register this task as a gateway with the router
	MSGHEADER h;

	h.osCode = GATEWAY_REGISTER;			// os message code
//	h.osCode = TASK_REGISTER;			// os message code
	h.length = sizeof( MSGHEADER);			// total message length, bytes
	h.taskPID = getpid();					// task PID number
	h.taskNID = getnid();					// task NID
	clock_gettime( CLOCK_REALTIME, &(h.sendTime));
	updateFocusMsgCRC( &h);					// update CRC

	// send message to local router to register
	int k = 0;										// counter
	while((mq_send( *mqd, &h, h.length, 0) == MQ_ERROR) &&
			(taskRunning))
	{
		if((errno==EINTR) && (k<RETRY_COUNT))  // signals
		{
			k++;
		}
		else											// all other errors
		{
			_FATAL_ERROR( __FILE__, __LINE__, TRACE_DISPATCHER, 0, "mq_send()");
		}
	}

#if 0
	// register for Register message
	h.length = sizeof( MSGHEADER);			// total message length, bytes
	h.osCode = MESSAGE_REGISTER;
	h.msgID = (short)TcpgateRegistration;
	clock_gettime( CLOCK_REALTIME, &(h.sendTime));
	updateFocusMsgCRC( &h);					// update CRC

	// send message to local router to register
	k = 0;										// counter
	while((mq_send( *mqd, &h, h.length, 0) == MQ_ERROR) &&
			(taskRunning))
	{
		if((errno==EINTR) && (k<RETRY_COUNT))  // signals
		{
			k++;
		}
		else											// all other errors
		{
			_LOG_ERROR( __FILE__, __LINE__, TRACE_DISPATCHER, errno, "Deregister:mq_send()");
			break;
		}
	}
#endif
}


// SPECIFICATION:	Process gateway queue.
//						Parameters:
//						gqd - gateway queue
//						*qnotify - queue notify structure
//
// ERROR HANDLING:	Terminates program.
static void processGatewayQueue( mqd_t gqd, struct sigevent* qnotify)
{
	char			msg[BSIZE];				// receive message buffer
	unsigned long  number_of_messages = 0; // Number of messages sent out.

	// re-arm Q
	if (mq_notify( gqd, qnotify) == -1)
	{
		if (errno!=EBUSY)
		{
			fatalError( __LINE__, 0, "mq_notify()");
		}
	}

	// loop getting messages
	while ((mq_receive( gqd, msg, BSIZE, 0) != -1) && (taskRunning))
	{
		// external message registration request
		if((((MSGHEADER*)msg)->msgID) == TcpgateRegistration)
		{
			MsgRegistrar.RegisterMessages( (TCPGate_Reg_ReqMsg *) msg,
				RegisterMsgs::DoReply);
			continue;
		}
		switch(MsgRegistrar.MessageRegistered(((MSGHEADER*)msg)->msgID))
		{
			// message is not registered for
			case 0:	
			break;
			// known & registered for - relay it
			case 1:
			// dont change anything as this alters the CRC
				// time stamp message b4 send out to 
				// allow for t-sync on external computer for
				// msgs originating on differnet procesors
#if 0
				clock_gettime( CLOCK_REALTIME,
					&((MSGHEADER*)msg)->sendTime );
				
				// 06/30/97 msm
				// rpl NID field with unsigned long rep of FS IP
				((MSGHEADER*)msg)->taskNID = (long)ulFSIP;
				((MSGHEADER*)msg)->taskPID = number_of_messages;
#endif		
				if( sendto( sock, msg,
										((MSGHEADER*)msg)->length,
										0,
										(struct sockaddr*) &server,
										sizeof( server) ) < 0 )
				{
					fatalError( __LINE__, 0, "sending datagram message");
				}
				// Increment the number of messages sent out.
				number_of_messages++;
			break;
            case -1:        // Unknown Message Id received
                fatalError( __LINE__, ((MSGHEADER*)msg)->msgID , 
                    "Invalid Registration Msg Id Received ");
            break;
            default:        // Unknown return code received
                fatalError( __LINE__, ((MSGHEADER*)msg)->msgID , 
                    "Invalid Return Code Received from MessageRegistered");
            break;
		}
	}
}
