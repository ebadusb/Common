/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains the unit test routines for the Firewire
 * driver project.
 *
 * $Log$
 *
 */

#include <vxWorks.h>
#include <stdio.h>
#include <string.h>
#include "fw_utility.h"
#include "fw_driver.h"
#include "fw_client.h"
#include "fw_link_layer.h"

/* Prototypes for unit test functions */
FWStatus fwReadAsyncQuadletUT(int adapter, int node, unsigned long csrAddress);
FWStatus fwReadAsyncBlockUT(int adapter, int node, unsigned long startAddr, unsigned long count);
FWStatus fwWriteAsyncQuadletUT(int adapter, int node, unsigned long csrAddress, UINT32 data);
FWStatus fwWriteAsyncBlockUT(int adapter, int node, unsigned long startAddr);
FWStatus fwCompareSwapUT(int adapter, int node, unsigned long csrAddress, UINT32 oldData, UINT32 newData);
FWStatus fwIsoReadUT(int adapter, int node);


FWStatus fwReadAsyncQuadletUT(int adapter, int node, unsigned long csrAddress)
{
	FWStatus retVal = FWInternalError;
	FWAsyncTransactionCmd transCmd;
	int clientHandle = 0;
	UINT32 buffer;
	unsigned short busNumber;
	unsigned char physNode;

	do
	{
		/* Register the client */
		retVal = fwRegisterClient( &clientHandle );

		if( retVal != FWSuccess )
		{
			printf("Could not register client.\n");
			break;
		}

		/* Fill in the transaction cmd structure */
		transCmd.clientHandle = clientHandle;
		transCmd.databuffer = &buffer;
		transCmd.dataBufferSize = 4;
		transCmd.dataLength = 4;
		transCmd.destinationID = node | 0xFFC0;
		transCmd.destinationOffset.highOffset = 0xFFFF;
		transCmd.destinationOffset.lowOffset = csrAddress;
		transCmd.speed = FWS400;

		retVal = fwAsyncRead( adapter, &transCmd, 10 );

		if( retVal == FWSuccess )
		{
			printf("Node %d returned 0x%08X for offset 0xFFFF %08X\n", node, buffer, csrAddress );
		}
		else
		{
			printf("Node %d returned error %d\n", node, retVal );
		}

	} while(0);

	retVal = fwUnregisterClient( clientHandle );

	return retVal;
}

FWStatus fwReadAsyncBlockUT(int adapter, int node, unsigned long startAddr, unsigned long count)
{
	FWStatus retVal = FWInternalError;
	FWAsyncTransactionCmd transCmd;
	int clientHandle = 0;
	UINT32 *buffer = NULL;
	unsigned long len = 0;
	unsigned long i;

	do
	{
		/* Register the client */
		retVal = fwRegisterClient( &clientHandle );

		if( retVal != FWSuccess )
		{
			printf("Could not register client.\n");
			break;
		}

		if( count > fwGetMaxPayload() )
		{
			printf("Count must be less than or equal to %d.\n", fwGetMaxPayload() );
			break;
		}

		len = (count + 3) & ~3;
		buffer = (UINT32*)fwMalloc( len );

		/* Fill in the transaction cmd structure */
		transCmd.clientHandle = clientHandle;
		transCmd.databuffer = buffer;
		transCmd.dataBufferSize = len;
		transCmd.dataLength = len;
		transCmd.destinationID = node | 0xFFC0;
		transCmd.destinationOffset.highOffset = 0xFFFF;
		transCmd.destinationOffset.lowOffset = startAddr;
		transCmd.speed = FWS400;

		retVal = fwAsyncRead( adapter, &transCmd, 10 );

		if( retVal == FWSuccess )
		{
			printf("Node %d returned %d bytes starting at offset 0xFFFF %08X\n", node, transCmd.dataLength, startAddr );
			for( i=0; i<transCmd.dataLength / 4; i++ )
			{
				printf("0x%08X\n", transCmd.databuffer[i] );
			}
		}
		else
		{
			printf("Node %d returned error %d\n", node, retVal );
		}

		fwFree( buffer );

	} while(0);

	retVal = fwUnregisterClient( clientHandle );

	return retVal;
}

FWStatus fwWriteAsyncQuadletUT(int adapter, int node, unsigned long csrAddress, UINT32 data)
{
	FWStatus retVal = FWInternalError;
	FWAsyncTransactionCmd transCmd;
	int clientHandle = 0;
	UINT32 buffer = data;

	do
	{
		/* Register the client */
		retVal = fwRegisterClient( &clientHandle );

		if( retVal != FWSuccess )
		{
			printf("Could not register client.\n");
			break;
		}

		/* Fill in the transaction cmd structure */
		transCmd.clientHandle = clientHandle;
		transCmd.databuffer = &buffer;
		transCmd.dataBufferSize = 4;
		transCmd.dataLength = 4;
		transCmd.destinationID = node | 0xFFC0;
		transCmd.destinationOffset.highOffset = 0xFFFF;
		transCmd.destinationOffset.lowOffset = csrAddress;
		transCmd.speed = FWS400;

		retVal = fwAsyncWrite( adapter, &transCmd, 10 );

		if( retVal == FWSuccess )
		{
			printf("Node %d wrote 0x%08X to offset 0xFFFF %08X\n", node, buffer, csrAddress );
		}
		else
		{
			printf("Node %d returned error %d\n", node, retVal );
		}

	} while(0);

	retVal = fwUnregisterClient( clientHandle );

	return retVal;
}

FWStatus fwWriteAsyncBlockUT(int adapter, int node, unsigned long startAddr)
{
	FWStatus retVal = FWInternalError;
	FWAsyncTransactionCmd transCmd;
	int clientHandle = 0;
	UINT32 buffer[3];
	unsigned long len = 12;
	unsigned long i;

	do
	{
		buffer[0] = 0x00000000;
		buffer[1] = 0xA0000000;
		buffer[2] = 0x80000000;

		/* Register the client */
		retVal = fwRegisterClient( &clientHandle );

		if( retVal != FWSuccess )
		{
			printf("Could not register client.\n");
			break;
		}

		/* Fill in the transaction cmd structure */
		transCmd.clientHandle = clientHandle;
		transCmd.databuffer = buffer;
		transCmd.dataBufferSize = len;
		transCmd.dataLength = len;
		transCmd.destinationID = node | 0xFFC0;
		transCmd.destinationOffset.highOffset = 0xFFFF;
		transCmd.destinationOffset.lowOffset = startAddr;
		transCmd.speed = FWS400;

		retVal = fwAsyncRead( adapter, &transCmd, 10 );

		if( retVal == FWSuccess )
		{
			printf("Node %d wrote %d bytes starting at offset 0xFFFF %08X\n", node, transCmd.dataLength, startAddr );
		}
		else
		{
			printf("Node %d returned error %d\n", node, retVal );
		}

	} while(0);

	retVal = fwUnregisterClient( clientHandle );

	return retVal;
}

FWStatus fwCompareSwapUT(int adapter, int node, unsigned long csrAddress, UINT32 oldData, UINT32 newData)
{
	FWStatus retVal = FWInternalError;
	FWAsyncTransactionCmd transCmd;
	int clientHandle = 0;
	UINT32 buffer[2];

	do
	{
		buffer[0] = oldData; /* if the current data is equal to this */
		buffer[1] = newData; /* then set the data equal to this new value */

		/* Register the client */
		retVal = fwRegisterClient( &clientHandle );

		if( retVal != FWSuccess )
		{
			printf("Could not register client.\n");
			break;
		}

		/* Fill in the transaction cmd structure */
		transCmd.clientHandle = clientHandle;
		transCmd.databuffer = buffer;
		transCmd.dataBufferSize = 8;
		transCmd.dataLength = 8;
		transCmd.destinationID = node | 0xFFC0;
		transCmd.destinationOffset.highOffset = 0xFFFF;
		transCmd.destinationOffset.lowOffset = csrAddress;
		transCmd.speed = FWS400;

		retVal = fwCompareSwap( adapter, &transCmd, 10 );

		if( retVal == FWSuccess )
		{
			printf("Compare swap Node %d data 0x%08X to offset 0xFFFF %08X\n", node, buffer[0], csrAddress );
		}
		else
		{
			printf("Node %d returned error %d\n", node, retVal );
		}

	} while(0);

	retVal = fwUnregisterClient( clientHandle );

	return retVal;
}

FWStatus fwIsoReadUT(int adapter, int node)
{
	FWStatus retVal = FWInternalError;
	FWAsyncTransactionCmd transCmd;
	FWIsochronousRecvCmd isoRecvCmd;
	STATUS status = OK;
	int clientHandle = 0;
	UINT32 buffer;
	UINT32 xferStatus;
	unsigned char *myBuffer;
	unsigned long bufSize;
	int i;
	int frames = 0;

	do
	{
		isoRecvCmd.clientSem = NULL;

		bufSize = 1024 * 768;
		myBuffer = (unsigned char*)fwMalloc( bufSize );
		if( myBuffer == NULL )
		{
			printf("Could not allocate buffer.\n");
			break;
		}

		/* Register the client */
		retVal = fwRegisterClient( &clientHandle );

		if( retVal != FWSuccess )
		{
			printf("Could not register client.\n");
			break;
		}

		/* Fill in the transaction cmd structure */
		transCmd.clientHandle = clientHandle;
		transCmd.databuffer = &buffer;
		transCmd.dataBufferSize = 4;
		transCmd.dataLength = 4;
		transCmd.destinationID = node | 0xFFC0;
		transCmd.destinationOffset.highOffset = 0xFFFF;
		transCmd.destinationOffset.lowOffset = 0xF0F00600;
		transCmd.speed = FWS400;

		buffer = 0x80000000;
		transCmd.destinationOffset.lowOffset = 0xF0F00600;
		retVal = fwAsyncWrite( adapter, &transCmd, 5 );
		if( retVal != FWSuccess )
		{
			printf("Node %d returned error %d\n", node, retVal );
			break;
		}

		buffer = 0xA0000000;
		transCmd.destinationOffset.lowOffset = 0xF0F00604;
		retVal = fwAsyncWrite( adapter, &transCmd, 5 );
		if( retVal != FWSuccess )
		{
			printf("Node %d returned error %d\n", node, retVal );
			break;
		}

		buffer = 0x20000000;
		transCmd.destinationOffset.lowOffset = 0xF0F00608;
		retVal = fwAsyncWrite( adapter, &transCmd, 5 );
		if( retVal != FWSuccess )
		{
			printf("Node %d returned error %d\n", node, retVal );
			break;
		}

		buffer = 0x02000000;
		transCmd.destinationOffset.lowOffset = 0xF0F0060C;
		retVal = fwAsyncWrite( adapter, &transCmd, 5 );
		if( retVal != FWSuccess )
		{
			printf("Node %d returned error %d\n", node, retVal );
			break;
		}

		/* Open channel 0 */
		retVal = fwIsoOpenChannel( adapter, 0, 4915 );
		if( retVal != FWSuccess )
		{
			printf("Could not open channel. Error %d\n", retVal );
			break;
		}

		isoRecvCmd.channel = 0;
		isoRecvCmd.clientSem = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
		isoRecvCmd.firstPacketSize = 3072;
		isoRecvCmd.numBuffers = 3;
		isoRecvCmd.sync = 1;
		isoRecvCmd.totalSize = 1024 * 768;

		retVal = fwIsoInitReceive( adapter, &isoRecvCmd );
		if( retVal != FWSuccess )
		{
			printf("Could not initialize channel. Error %d\n", retVal );
			break;
		}

		retVal = fwIsoStartChannel(adapter, isoRecvCmd.channel );
		if( retVal != FWSuccess )
		{
			printf("Could not start channel. Error %d\n", retVal );
			break;
		}

		/* Perform a 9 shot capture */
		buffer = 0x40000009;
		transCmd.destinationOffset.lowOffset = 0xF0F0061C;
		retVal = fwAsyncWrite( adapter, &transCmd, 5 );
		if( retVal != FWSuccess )
		{
			printf("Node %d returned error %d\n", node, retVal );
			break;
		}

		while( frames < 9 && status == OK )
		{
			status = semTake( isoRecvCmd.clientSem, 240 );

			if( status == OK )
			{
				retVal = fwIsoReadRecvData( adapter, isoRecvCmd.channel, myBuffer, bufSize, &xferStatus );
				if( retVal != FWSuccess )
				{
					printf("Could not read buffer. Error:%d\n", retVal);
				}
				else
				{
					printf("Image read from camera. XferStatus:0x%08X\n", xferStatus );
					for(i=0; i<128; i++)
					{
						printf("%02X ", myBuffer[i]);
					}
				}
			}
			else
			{
				printf("Iso recv timed out.\n");
				break;
			}

			frames++;
		}

		retVal = fwIsoStopChannel( adapter, isoRecvCmd.channel );
		if( retVal != FWSuccess )
		{
			printf("Could not stop channel. Error %d\n", retVal );
			break;
		}

	} while(0);

	fwIsoCloseChannel( adapter, 0 );

	retVal = fwUnregisterClient( clientHandle );

	if( myBuffer ) fwFree( myBuffer );

	if( isoRecvCmd.clientSem )
		semDelete( isoRecvCmd.clientSem );

	return retVal;
}

