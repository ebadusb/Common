// sockinet.h -*- C++ -*- socket library
// Copyright (C) 1992,1993,1994 Gnanasekaran Swaminathan <gs4t@virginia.edu>
// 
// Permission is granted to use at your own risk and distribute this software
// in source and binary forms provided the above copyright
// notice and this paragraph are preserved on all copies.
// This software is provided "as is" with no express or implied warranty.
//
// Version: 17Oct95 1.10

#ifndef _SOCKINET_H
#define	_SOCKINET_H

#include "sockstream.h"

#include <sys/times.h>
#include <netinet/in.h>



class sockinetaddr: public sockAddr, public sockaddr_in
{
protected:
#if 0   // NOT SUPPORTED BY VXWORKS
    void		setport (const char* sn, const char* pn="tcp");
    void		setaddr (const char* hn);
#endif
public:
   enum byte_order {network, host};


   ~sockinetaddr () { }

   // defaults to INADDR_ANY and port of 0.
   sockinetaddr ();

   // gives INADDR_ANY and port number.
   sockinetaddr (unsigned short port_no);

   // pass address and port in host byte order
   sockinetaddr (unsigned long addr, unsigned short port_no = 0);

   // pass dot notation address and host byte order port number
   sockinetaddr (const char *addr, unsigned short port_no = 0);

   // pass in your own address in network byte order. 
   sockinetaddr (const sockinetaddr &sina);




#if 0 // NOT SUPPORTED BY VXWORKS
   sockinetaddr (const char* host_name,
                 const char* service_name,
                 const char* protocol_name="tcp");
   
   sockinetaddr (const char* host_name, int port_no=0);

   sockinetaddr (unsigned long addr,
                 const char* service_name,
                 const char* protocol_name="tcp");
#endif

   operator void * () const { return (sockaddr_in*)this; }

   friend bool operator == (const sockinetaddr &lhs, const sockinetaddr &rhs);

   friend DataLog_Stream & operator << (DataLog_Stream &os, const sockinetaddr &sa);

   int        size() const { return sizeof (sockaddr_in); }
   int        family() const { return sin_family; }
   sockaddr * addr() const { return (sockaddr*)((sockaddr_in*)this); }

   int          getport() const;
   const char * gethostname() const;

   // Sets the port number of the internet address
   void setport(unsigned short port) { sin_port = htons(port); };

   // Sets the address in standard IP dot notation (no host names)
   void setaddr(const char *inet_addr);
   void setaddr(unsigned long addr, byte_order order = host);
};



class sockinetbuf: public sockbuf 
{
protected:
   sockinetbuf & operator = (const sockbuf &si);     // needs to be fixed
   sockinetbuf & operator = (const sockinetbuf& si); // needs fixing

public:
   enum domain
   {
      af_inet  = AF_INET,    // For TCP, UDP
      af_unix  = AF_UNIX,    // Local to Host Pipe
      af_local = AF_LOCAL,   // Local to Host Pipe
      af_ccitt = AF_CCITT    // X.25
   };

   sockinetbuf (const sockbuf& si): sockbuf(si) { }
   sockinetbuf (const sockinetbuf& si): sockbuf (si) { }
   sockinetbuf (sockbuf::type ty, int proto=0);

   sockbuf * open (sockbuf::type, int proto=0);

   sockinetaddr localaddr() const;
   int          localport() const;
   const char * localhost() const;

   sockinetaddr peeraddr() const;
   int          peerport() const;
   const char * peerhost() const;

   int bind_until_success (int portno);

   virtual int bind (sockAddr& sa);
   int         bind ();
   int         bind (unsigned long addr, int port_no=0);


#if 0  // NOT SUPPORTED BY VXWORKS
int         bind (const char* host_name, int port_no=0);
int         bind (unsigned long addr,
                  const char* service_name,
                  const char* protocol_name="tcp");
int         bind (const char* host_name,
                  const char* service_name,
                  const char* protocol_name="tcp");
#endif

   virtual int connect (sockAddr& sa);
   int         connect (unsigned long addr, int port_no);
   int         connect (const char* host_name, int port_no);
   virtual int connectWithTimeout (sockAddr& sa, timeval *tv);
   int         connectWithTimeout (unsigned long addr, int port_no, timeval *tv);
   int         connectWithTimeout (const char* host_name, int port_no, timeval *tv);


#if 0  // NOT SUPPORTED BY VXWORKS
int            connect (unsigned long addr,
                        const char* service_name,
                        const char* protocol_name="tcp");
int            connect (const char* host_name,
                        const char* service_name,
                        const char* protocol_name="tcp");
#endif

};



class isockinet: public isockstream
{
public:
   isockinet (const sockbuf& sb);
   isockinet (sockbuf::type ty=sockbuf::sock_stream, int proto=0);
   ~isockinet ();

   sockinetbuf * rdbuf () { return(sockinetbuf*)ios::rdbuf (); }
   sockinetbuf * operator -> () { return rdbuf (); }
};



class osockinet: public osockstream
{
public:
   osockinet (const sockbuf& sb);
   osockinet (sockbuf::type ty=sockbuf::sock_stream, int proto=0);
   ~osockinet ();

   sockinetbuf * rdbuf () { return(sockinetbuf*)ios::rdbuf (); }
   sockinetbuf * operator -> () { return rdbuf (); }
};



class iosockinet: public iosockstream
{
public:
   iosockinet (const sockbuf& sb);
   iosockinet (sockbuf::type ty=sockbuf::sock_stream, int proto=0);
   ~iosockinet ();

   sockinetbuf * rdbuf () { return(sockinetbuf*)ios::rdbuf (); }
   sockinetbuf * operator -> () { return rdbuf (); }
};

#endif	// _SOCKINET_H
