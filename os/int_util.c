/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 *	vxWorks utilities for relating IRQ level and interrupt vector assignment.
 * The relationship betwteen the two is determined when the OS image is built,
 * through settings in config.h, and is stored in this file for use by
 * run-time applications.
 * 
 * This file should be included as part of the build for an vxWorks OS image
 * and is not intended to be compiled separately.
 *
 * $Header$
 * $Log$
 *
 */

#include "config.h"	/* BSP configuration data */
#include "int_util.h"

/*
 *	PC-based hardware supports 16 hardware interrupts.  The NUMBER_OF_IRQS
 * value is used to insure that a compile-time error will be generated
 * if a different hardware configuration is used.
 */
static int irqVectorData[NUMBER_OF_IRQS] =
{
 	INT_VEC_GET(0),
 	INT_VEC_GET(1),
 	INT_VEC_GET(2),
 	INT_VEC_GET(3),
 	INT_VEC_GET(4),
 	INT_VEC_GET(5),
 	INT_VEC_GET(6),
 	INT_VEC_GET(7),
 	INT_VEC_GET(8),
 	INT_VEC_GET(9),
 	INT_VEC_GET(10),
 	INT_VEC_GET(11),
 	INT_VEC_GET(12),
 	INT_VEC_GET(13),
 	INT_VEC_GET(14),
 	INT_VEC_GET(15)
};

int getVectorFromIRQ(unsigned char irq)
{
	if ( irq >= sizeof(irqVectorData)/sizeof(irqVectorData[0]) )
		return -1;
	else
		return irqVectorData[irq];
}
