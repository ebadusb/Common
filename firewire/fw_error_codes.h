/*
 *  Copyright(c) 2005 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains the firewire OHCI interface level routines.
 *
 * $Log$
 *
 */

#ifndef _FW_ERROR_CODES_INCLUDE_
#define _FW_ERROR_CODES_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum FWStatusEnum
{
	FWSuccess					= 0,
	FWResponseError			= 1,
	FWResponseFormatError	= 2,
	FWRequestDataError		= 3,
	FWTypeError					= 4,
	FWAddressError				= 5,
	FWAcknowledgeMissing		= 6,
	FWUnsolicitedResponse	= 7,
	FWResponseRetryLimit		= 8,
	FWDataTruncated			= 9,
	FWTimeout					= 10,
	FWCanceled					= 11,
	FWInternalError			= 12,
	FWBoundsError				= 13,
	FWNotInitialized			= 14,
	FWMemAllocateError		= 15,
	FWSemAllocateError		= 16,
	FWShutdown					= 17,
	FWPhyReadFailed			= 18,
	FWPhyWriteFailed			= 19,
	FWHardwareFailure			= 20,
	FWInvalidArg				= 21,
	FWLockTransError			= 22,
	FWNotImplemented			= 23,
	FWQueuedPending			= 24,
	FWQueuedWaiting			= 25,
	FWBusGenerationError		= 26,
	FWResetOccured				= 27,
	FWLinkBusy					= 28,
	FWInvalidNodeAddr			= 29,
	FWInvalidTransactionType = 30,
	FWInvalidExtendedTransactionCode = 31,
	FWTransDataSizeError		= 32,
	FWResetUnderway			= 33,
	FWNotFound					= 34,
	FWDescriptorNotAvailable = 35,
	FWNoMoreData				= 36,
	FWContextError				= 37,
	FWNoTransactionData		= 38,
	FWInvalidAdapter			= 39,
	FWNoTransactions			= 40,
	FWSelfIdError				= 41,
	FWIsoChannelBusy			= 42,
	FWIsoChannelStopped		= 43,
	FWIsoBufferNotFound		= 44,
	FWIsoBufferSyncError		= 45

} FWStatus;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_ERROR_CODES_INCLUDE_ */

