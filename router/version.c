/*
 * Copyright (C) 2001 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log$
 *
 * version.c - provide stub function for application specific version
 *             information to the version message handler
 *
 */

#include "sinver.h"
#include <stdio.h>

// SPECIFICATION:    getAppRevisionData can be provided to override
//                   the common project version information.  The
//                   default version simply returns NULL, so that
//                   the common build revision data will be used.
//
// ERROR HANDLING:   none

const char * getAppRevisionData( )
{
   return NULL;
}


