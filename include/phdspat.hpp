/*
 * Copyright (c) 1995, 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: Q:/home1/COMMON_PROJECT/Source/ROUTER/rcs/PHDSPAT.HPP 1.1 1999/05/24 23:29:51 TD10216 Exp $
 * $Log: PHDSPAT.HPP $
 * Revision 1.1  1999/05/24 23:29:51  TD10216
 * Initial revision
 * Revision 1.2  1996/07/24 19:49:58  SS03309
 * fix MKS 
 * Revision 1.2  1996/07/22 14:43:38  SS03309
 * Revision 1.7  1996/06/27 15:09:45  SS03309
 * Revised comments
 *
 * TITLE:      phdspat.hpp, Focussed System message router - photon message
 *             dispatcher.
 *
 * ABSTRACT:   These classes support message routing in the Focussed System.
 * This dispatcher is used with the Photon Graphics System and accepts
 * messages from the Focussed Router, vectors photon proxies if necessary,
 * copies the message data to the correct places, and calls the
 * various message notify functions.
 *
 * DOCUMENTS
 * Requirements:     I:\ieee1498\SSS2.DOC
 * Test:             I:\ieee1498\STD2.DOC
 *
 * for EXAMPLE see msg.hpp
 */

#ifndef PHDSPAT_HPP     // prevent multiple includes
#define PHDSPAT_HPP
#include <photon/PtT.h>

#include "dispatch.hpp"

// class definitions

/*
 * the dispatcher class is used to register and deregister messages
 * with the system router.   Only one dispatcher should be constructed
 * per task.   The class is also used to send messages.  The
 * dispatchLoop() loops forever (doesn't return) getting messages from
 * the router and sending them to the proper message class(es).
 *
 * This version interfaces with the Photon message loop.
 */

class photonDispatcher : public dispatcher
{
   public:
      photonDispatcher( int argc, char** argv, int maxMessages=150);
      ~photonDispatcher();

      void dispatchLoop();

   private:
      photonDispatcher();                                   // not implemented
      photonDispatcher( photonDispatcher const &);          // not implemented
      photonDispatcher& operator=(photonDispatcher const&); // not implemented
      PtInputId_t* id;
};

#endif

