/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: I:/BCT_Development/vxWorks/Common/datalog/rcs/datalog_port.cpp 1.2 2002/08/15 20:53:57Z jl11312 Exp $
 * $Log: datalog_port.cpp $
 * Revision 1.2  2002/08/15 20:53:57Z  jl11312
 * - added support for periodic logging
 * Revision 1.1  2002/07/18 21:20:59  jl11312
 * Initial revision
 *
 */

#ifdef VXWORKS
# include "datalog_port_vxworks.cpp"
#elif defined(WIN32)
# include "datalog_port_windows.cpp"
#else /* ifdef VXWORKS */
# error "Unknown platform"
#endif /* ifdef VXWORKS */

