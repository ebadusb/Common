/*
 *	Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: L:/vxWorks/TaosGUIDemo/gui/rcs/bitmap_info.h 1.4 2003/07/24 16:59:50Z jl11312 Exp $
 * $Log: bitmap_info.h $
 * Revision 1.4  2003/07/24 16:59:50Z  jl11312
 * initial demo version
 * Revision 1.3  2003/06/06 20:12:39Z  rm70006
 * IT 6096.
 * Share files with Vista
 * Revision 1.2  2003/02/26 17:10:36Z  rm10919
 * IT 5863, 5891 - lowered rows for CFG_RBC screen and deleted x,y coordinates for second placard on disconnect screen.
 * Revision 1.1  2003/01/06 18:35:19Z  jl11312
 * Initial revision
 *
 */

#ifndef _BITMAP_INFO_INCLUDE
#define _BITMAP_INFO_INCLUDE

#include "bitmap_info_id.h"

struct BITMAP_DATA_ENTRY
{
	bool absolutePath;
	int  width;
	int  height;
	unsigned char * data;
	unsigned long dataSize;
};

extern BITMAP_DATA_ENTRY bitmap_data_table[BITMAP_ID_COUNT];

#endif /* ifndef _BITMAP_INFO_INCLUDE */

