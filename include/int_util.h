/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 *	vxWorks utilities for relating IRQ level and interrupt vector assignment.
 * The relationship betwteen the two is determined when the OS image is built,
 * through settings in config.h, and is stored in this file for use by
 * run-time applications.
 * 
 * $Header$
 * $Log$
 *
 */

#ifndef _INT_UTIL_INCLUDE
#define _INT_UTIL_INCLUDE

int getVectorFromIRQ(unsigned char irq);

#endif /* ifndef _INT_UTIL_INCLUDE */

