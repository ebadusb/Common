/*
 * Copyright (c) 2001 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:     hal_notify.h
 *
 * Handle notification of HAL (hardware anomaly locator) of a pending reboot.
 * This is called during fatal error handling, or during a normal system
 * shutdown.
 *
 * $Header$
 * $Log$
 *
 */

#ifndef _HAL_NOTIFY_INCLUDE
#define _HAL_NOTIFY_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif

/*
 * notify hal of a normal system shutdown
 */
void hal_notify_shutdown(void);

/*
 * notify hal of a fatal error shutdown
 */
void hal_notify_fatal(int line, int code, const char * file);

#ifdef __cplusplus
};
#endif

#endif /* ifndef _HAL_NOTIFY_INCLUDE */

