/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/include/rcs/logstream.h 1.2 2002/03/20 16:37:53 jl11312 Exp jl11312 $
 * $Log: logstream.h $
 * Revision 1.1  2002/03/19 16:11:34  jl11312
 * Initial revision
 *
 *	LogStream interface - a stream-based interface for logging functions.
 *
 */

#ifndef _LOGSTREAM_INCLUDE
#define _LOGSTREAM_INCLUDE

#include <iostream.h>

class logstreambuf : public streambuf
{
public:
	logstreambuf(void);
	virtual ~logstreambuf();

protected:
	virtual int overflow(int ch=EOF);
	virtual int sync(void);

private:
	enum { BufferSize = 512 };
	char * _buffer;
};

class logstreambase : virtual public ios
{
public:
	logstreambuf* rdbuf() { return &_sb; }

protected:
	logstreambuf _sb;
   logstreambase() { init(&_sb); }
};

class ologstream : public logstreambase, public ostream
{
public:
	ologstream(void);
};

inline ologstream& endmsg(ologstream & os)
{
	return os;
}

#endif // ifndef _LOGSTREAM_INCLUDE

