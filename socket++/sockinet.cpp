// sockinet.C  -*- C++ -*- socket library
// Copyright (C) 1992,1993,1994 Gnanasekaran Swaminathan <gs4t@virginia.edu>
// 
// Permission is granted to use at your own risk and distribute this software
// in source and binary forms provided the above copyright
// notice and this paragraph are preserved on all copies.
// This software is provided "as is" with no express or implied warranty.
//
// Version: 17Oct95 1.10

#include <vxWorks.h>
//#include <config.h>
#include "sockinet.h"

//EXTERN_C_BEGIN
#include <netdb.h>
#include <time.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

// VxWorks needed
#include <arpa/inet.h>
#include <hostLib.h>
#include <sockLib.h>
#include <inetLib.h>   // Needed for inet_aton
//EXTERN_C_END

#include "datalog.h"


//
sockinetaddr::sockinetaddr () 
{
   sin_len         = sizeof (struct in_addr);    // Correct value
   sin_family      = sockinetbuf::af_inet;       // Correct value
   sin_addr.s_addr = htonl(INADDR_ANY);          // Default Values.  Need to be set to correct value.
   sin_port        = 0;                          // Default Values.  Need to be set to correct value.
}



sockinetaddr::sockinetaddr (unsigned short port_no) 
{
   sin_len         = sizeof (struct in_addr);
   sin_family      = sockinetbuf::af_inet;
   sin_addr.s_addr = htonl(INADDR_ANY);
   sin_port        = htons(port_no);
}



sockinetaddr::sockinetaddr(unsigned long addr, unsigned short port_no)
// addr and port_no are in host byte order
{
   sin_len         = sizeof (struct in_addr);
   sin_family      = sockinetbuf::af_inet;
   sin_addr.s_addr = htonl(addr);
   sin_port        = htons(port_no);
}



sockinetaddr::sockinetaddr(const char *addr, unsigned short port_no)
// addr and port_no are in host byte order
{
   sin_len    = sizeof (struct in_addr);
   sin_family = sockinetbuf::af_inet;
   sin_port   = htons(port_no);

   if (inet_aton ((char *)addr, &sin_addr) != OK)   // Stupid VxWorks doesn't have const in prototype
   {
      sin_addr.s_addr = INADDR_NONE;

      DataLog_Critical critical;
      DataLog(critical) << "Could not translate socket address " << addr << endmsg;
   }
}



void sockinetaddr::setaddr(const char *inet_addr)
{
   struct in_addr sockAddr;

   if (inet_aton ((char *)inet_addr, &sin_addr) != OK)   // Stupid VxWorks doesn't have const in prototype
   {
      sin_addr.s_addr = INADDR_NONE;

      DataLog_Critical critical;
      DataLog(critical) << "Could not translate socket address " << inet_addr << endmsg;
   }
}



void sockinetaddr::setaddr(unsigned long addr, byte_order order)
{
   if (order == host)
      sin_addr.s_addr = htonl(addr);
   else
      sin_addr.s_addr = addr;
}




#if 0  // NOT SUPPORTED BY VXWORKS
sockinetaddr::sockinetaddr(unsigned long addr, const char* sn, const char* pn)
// addr is in host byte order
{
   sin_family      = sockinetbuf::af_inet;
   sin_addr.s_addr = htonl (addr); // Added by cgay@cs.uoregon.edu May 29, 1993
   setport(sn, pn);
}


sockinetaddr::sockinetaddr (const char* host_name, int port_no)
// port_no is in host byte order
{
   setaddr(host_name);
   sin_port = htons(port_no);
}


sockinetaddr::sockinetaddr(const char* hn, const char* sn, const char* pn)
{
   setaddr(hn);
   setport(sn, pn);
}
#endif


sockinetaddr::sockinetaddr (const sockinetaddr& sina)
{
   sin_family      = sockinetbuf::af_inet;
   sin_addr.s_addr = sina.sin_addr.s_addr;
   sin_port        = sina.sin_port;
}   



bool operator == (const sockinetaddr &lhs, const sockinetaddr &rhs)
{
   if ( (lhs.sin_len         == rhs.sin_len)    &&
        (lhs.sin_family      == rhs.sin_family) &&
        (lhs.sin_port        == rhs.sin_port)   &&
        (lhs.sin_addr.s_addr == rhs.sin_addr.s_addr)
      )
   {
      return true;
   }
   else
   {
      return false;
   }
}



DataLog_Stream & operator << (DataLog_Stream &os, const sockinetaddr &sa)
{
   char buffer[INET_ADDR_LEN];

   inet_ntoa_b (sa.sin_addr, buffer);

   os << "(" << buffer << ", " << ntohs(sa.sin_port) << ")";

   return os;
}




#if 0 // NOT SUPPORTED BY VXWORKS
void sockinetaddr::setport(const char* sn, const char* pn)
{
   :rror ("sockinetaddr: setport not supported.");
   exit(1);

  servent* sp = getservbyname(sn, pn);
  if (sp == 0) {
    perror(sn);
    error ("sockinetaddr: invalid service name");
    exit(1);
  }
  sin_port = sp->s_port;
}
#endif



int sockinetaddr::getport () const
{
   return ntohs (sin_port);
}


#if 0  // NOT SUPPORTED BY VXWORKS
void sockinetaddr::setaddr(const char* host_name)
{
   error ("sockinetaddr: setaddr not supported.");
   exit(1);
  if ( (sin_addr.s_addr = inet_addr(host_name)) == (unsigned long)-1) {
    hostent* hp = gethostbyname(host_name);
    if (hp == 0) {
      herror("sockinetaddr::sockinetaddr");
      exit(1);
    }
    memcpy(&sin_addr, hp->h_addr, hp->h_length);
    sin_family = hp->h_addrtype;
  }else
    sin_family = sockinetbuf::af_inet;
}
#endif



const char* sockinetaddr::gethostname () const
{
   return inet_ntoa(sin_addr);

/*
   if (sin_addr.s_addr == htonl(INADDR_ANY))
   {
      static char hostname[64];
      
      if (::gethostname(hostname, 63) == -1)
      {
         perror("in sockinetaddr::gethostname");
         return "";
      }
      
      return hostname;    
   }

   perror("in sockinetaddr::gethostname, host not found.");
   return "";

  hostent* hp = gethostbyaddr((const char*) &sin_addr,
               sizeof(sin_addr),
               family());
  if (hp == 0) {
    herror("sockinetaddr::gethostname");
    return "";
  }
  if (hp->h_name) return hp->h_name;
  return "";
*/
}



//
// SOCKINETBUF
//

sockinetbuf::sockinetbuf(sockbuf::type ty, int proto)
: sockbuf(af_inet, ty, proto)
{
}

sockinetbuf& sockinetbuf::operator = (const sockbuf& si)
{
   this->sockbuf::operator = (si);
   return *this;
}

sockinetbuf& sockinetbuf::operator = (const sockinetbuf& si)
{
   this->sockbuf::operator = (si);
   return *this;
}

sockbuf* sockinetbuf::open(sockbuf::type st, int proto)
{
   *this = sockinetbuf(st, proto);
   return this;
}

sockinetaddr sockinetbuf::localaddr() const
{
   sockinetaddr sin;
   int len = sin.size();

   if (::getsockname(rep->sock, sin.addr (), &len) == -1)
      perror("sockinetbuf::localaddr()");

   return sin;
}

int sockinetbuf::localport() const
{
   sockinetaddr sin = localaddr();
   if (sin.family() != af_inet) return -1;
   return sin.getport();
}

const char* sockinetbuf::localhost() const
{
   sockinetaddr sin = localaddr();

   if (sin.family() != af_inet)
      return "";

   return sin.gethostname();
}


sockinetaddr sockinetbuf::peeraddr() const
{
   sockinetaddr sin;
   int len = sin.size();
   if (::getpeername(rep->sock, sin.addr (), &len) == -1)
      perror("sockinetbuf::peeraddr()");
   return sin;
}

int sockinetbuf::peerport() const
{
   sockinetaddr sin = peeraddr();
   if (sin.family() != af_inet)
      return -1;

   return sin.getport();
}

const char* sockinetbuf::peerhost() const
{
   sockinetaddr sin = peeraddr();
   if (sin.family() != af_inet)
      return "";

   return sin.gethostname();
}

int sockinetbuf::bind_until_success (int portno)
// a. bind to (INADDR_ANY, portno)
// b. if success return 0
// c. if failure and errno is EADDRINUSE, portno++ and go to step a.
// d. return errno.
{
   for (;;)
   {
      sockinetaddr sa ((unsigned long) INADDR_ANY, portno++);
      int eno = bind (sa);

      if (eno == 0)
         return 0;

      if (eno != EADDRINUSE)
         return eno;
   }
}

int sockinetbuf::bind (sockAddr& sa)
{
   return sockbuf::bind (sa);
}

int sockinetbuf::bind ()
{
   sockinetaddr sa;
   return bind (sa);
}

int sockinetbuf::bind (unsigned long addr, int port_no)
// address and portno are in host byte order
{
   sockinetaddr sa (addr, port_no);
   return bind (sa);
}

#if 0   // NOT SUPPORTED BY VXWORKS
int sockinetbuf::bind (const char* host_name, int port_no)
{
  sockinetaddr sa (host_name, port_no);
  return bind (sa);
}

int sockinetbuf::bind (unsigned long addr,
             const char* service_name,
             const char* protocol_name)
{
  sockinetaddr sa (addr, service_name, protocol_name);
  return bind (sa);
}


int sockinetbuf::bind (const char* host_name,
             const char* service_name,
             const char* protocol_name)
{
  sockinetaddr sa (host_name, service_name, protocol_name);
  return bind (sa);
}
#endif


int sockinetbuf::connect (sockAddr& sa)
{
   return sockbuf::connect (sa);
}

int sockinetbuf::connect (unsigned long addr, int port_no)
// address and portno are in host byte order
{
   sockinetaddr sa (addr, port_no);
   return connect (sa);
}


int sockinetbuf::connect (const char* host_name, int port_no)
{
   in_addr sock_addr;
   sock_addr.s_addr = inet_addr((char *)host_name);
   sockinetaddr sa (ntohl(inet_addr((char *)host_name)), port_no);

   return connect (sa);
}

int sockinetbuf::connectWithTimeout (sockAddr& sa, timeval *tv)
{
   return sockbuf::connectWithTimeout (sa, tv);
}

int sockinetbuf::connectWithTimeout (unsigned long addr, int port_no, timeval *tv)
// address and portno are in host byte order
{
   sockinetaddr sa (addr, port_no);
   return connectWithTimeout (sa, tv);
}


int sockinetbuf::connectWithTimeout (const char* host_name, int port_no, timeval *tv)
{
   in_addr sock_addr;
   sock_addr.s_addr = inet_addr((char *)host_name);
   sockinetaddr sa (ntohl(inet_addr((char *)host_name)), port_no);

   return connectWithTimeout (sa, tv);
}


#if 0  // NOT SUPPORTED BY VXWORKS
int sockinetbuf::connect (unsigned long addr,
           const char* service_name,
           const char* protocol_name)
{
  sockinetaddr sa (addr, service_name, protocol_name);
  return connect (sa);
}


int sockinetbuf::connect (const char* host_name,
           const char* service_name,
           const char* protocol_name)
{
  sockinetaddr sa (host_name, service_name, protocol_name);
  return connect (sa);
}
#endif


isockinet::isockinet (sockbuf::type ty, int proto)
: ios (new sockinetbuf (ty, proto))
{
}

isockinet::isockinet (const sockbuf& sb)
: ios (new sockinetbuf (sb))
{
}

isockinet::~isockinet ()
{
   delete ios::rdbuf ();
   init (0);
}

osockinet::osockinet (sockbuf::type ty, int proto)
: ios (new sockinetbuf (ty, proto))
{
}

osockinet::osockinet (const sockbuf& sb)
: ios (new sockinetbuf (sb))
{
}

osockinet::~osockinet ()
{
   delete ios::rdbuf ();
   init (0);
}

iosockinet::iosockinet (sockbuf::type ty, int proto)
: ios (new sockinetbuf (ty, proto))
{
}

iosockinet::iosockinet (const sockbuf& sb)
: ios (new sockinetbuf (sb))
{
}

iosockinet::~iosockinet ()
{
   delete ios::rdbuf ();
   init (0);
}
