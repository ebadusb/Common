/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/datalog/rcs/datalog_port.cpp 1.2 2002/08/15 20:53:57Z jl11312 Exp rm70006 $
 * $Log: datalog_port.cpp $
 *
 */

#include "datalog.h"
#include "datalog_internal.h"

#ifdef VXWORKS
# include "datalog_port_vxworks.cpp"
#else /* ifdef VXWORKS */
# error "Unknown platform"
#endif /* ifdef VXWORKS */

