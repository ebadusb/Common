/*
 * Copyright (c) 1995, 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: I:/trima_project/source/router/rcs/spoofdsp.hpp 1.2 1996/12/05 17:59:44 SS03309 Exp $
 * $Log: spoofdsp.hpp $
 * Revision 1.2  1996/12/05 17:59:44  SS03309
 * remove dispatchLoop
 *
 * TITLE:      Focussed System - spoofer message dispatcher.
 *
 * ABSTRACT:   These classes support message routing in the Focussed System.
 *             The spoofer dispatcher is used by the spoofer to intercept 
 *             message trafic from tasks before it gets distributed.  This 
 *             allows the spoofer to modify the data.
 *
 * DOCUMENTS
 * Requirements:     I:\ieee1498\SSS2.DOC
 * Test:             I:\ieee1498\STD2.DOC
 *
 * PUBLIC FUNCTIONS:
 * class dispatcher
 *    class systemError                // exception class
 *    dispatcher()                     // constructor
 *    ~dispatcher()                    // destructor
 *    void registerMessage()           // register message
 *    void deregisterMessage()         // deregister message
 *    void send()                      // send message
 *    void clearTimerEntry()           // clear timer entry
 *    void setTimerEntry()             // set timer entry
 *    virtual void dispatchLoop()      // dispatch loop
 */

#ifndef SPOOFDSP_HPP                   // prevent multiple includes
#define SPOOFDSP_HPP

#include <mqueue.h>
#include <string.h>
#include <sys/types.h>
#include "msghdr.h"
#include "dispatch.hpp"

// forward references

/*
 * the dispatcher class is used to register and deregister messages
 * with the system router(s).   Only one dispatcher should be constructed
 * per task.   The class is also used to send messages.  The
 * dispatchLoop() loops forever (doesn't return) getting messages from
 * the router and sending them to the proper message class(es).
 */

class spooferDispatcher : public dispatcher
{
   public:

// class members

      spooferDispatcher( int argc,                        // command line arg count
                  char** argv,                     // command line arguments
                  int maxMessages=DEFAULT_Q_SIZE); // max size of queue
      virtual ~spooferDispatcher();

      void registerMessage( routeBuffer*);         // used by msg constructor
      void deregisterMessage( routeBuffer*);       // used by msg destructor
      void send( routeBuffer*);                    // send message

    private:
      mqd_t routerQueue2;                          // other node's router queue id
      spooferDispatcher();                                // not implemented
      spooferDispatcher( spooferDispatcher const &);             // not implemented
      spooferDispatcher& operator=(spooferDispatcher const&);    // not implemented
};

#endif
