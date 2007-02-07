/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains the client routines that
 * update or manage client resources.
 *
 * $Log$
 *
 */

#ifndef _FW_CLIENT_INCLUDE_
#define _FW_CLIENT_INCLUDE_

#include <vxWorks.h>
#include <semLib.h>
#include <firewire/fw_types.h>
#include <firewire/fw_error_codes.h>
#include <firewire/fw_client_data.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Creates and initializes a client structure. The notify
 * semaphore is a binary semaphore used to signal the client
 * when a IEEE-1394 communication event occurs. Events are
 * ususally either a transaction has completed, a CSR address
 * is updated, or a isochronous message is ready. If notifySem
 * is NULL, a semaphore will be created automatically.
 */
FWStatus fwCreateClient(int *clientHandle, SEM_ID notifySem);

/* Destroys a client structure.
 */
FWStatus fwDestroyClient(int clientHandle);

/* Return the registered client's resource structure.
 */
FWStatus fwGetClientResource(int clientHandle, FWClientResource **clientResource );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_CLIENT_INCLUDE_ */

