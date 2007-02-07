/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * This file contains the firewire configuration ROM routines.
 *
 * $Log$
 *
 */

#ifndef _FW_CONFIG_ROM_INCLUDE_
#define _FW_CONFIG_ROM_INCLUDE_

#include <firewire/fw_types.h>
#include <firewire/fw_error_codes.h>
#include <firewire/fw_driver_data.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Create the config ROM data structures and
 * allocate resources.
 */
FWStatus fwCreateConfigROM(FWDriverData *pDriver);

/* Free allocated resources.
 */
FWStatus fwDestroyConfigROM(FWDriverData *pDriver);

/* Initialize the config ROM. This function must be called
 * to setup the config ROM data area.
 */
FWStatus fwInitializeConfigROM(FWDriverData *pDriver);

/* Update the config ROM. A bus reset is issued once the
 * new config ROM image is built.
 */
FWStatus fwUpdateConfigROM(FWDriverData *pDriver);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_CONFIG_ROM_INCLUDE_ */

