/*
 * $Header$
 * Defines constants used for the DataLogReserved message type
 * RESERVED_HEADER is the text tag name for reserved messages
 * DEFAULT_NAME is used when no name is passed into a DataLogReserved call
 *
 * $Log: datalog_reserved_stream.cpp $
 * Revision 1.2  2006/10/23 20:23:27Z  jmedusb
 * Added standard header.
 */


#include "vxworks.h"
#include "datalog_reserved_stream.h"

const char * DataLogRes::RESERVED_HEADER = "DATALOG_RESERVED";
const char * DataLogRes::DEFAULT_NAME    = "DEFAULT_NAME";
