/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      spoofermessagesystem.h
 *
 * ABSTRACT:   This file initializes the message interface for the
 *              spoofer task
 */

#ifndef _SPOOFER_MESSAGE_SYSTEM_H_
#define _SPOOFER_MESSAGE_SYSTEM_H_

#include "messagesystem.h"

class SpooferMessageSystem : public MessageSystem
{
public:

   //
   // Default constructor
   SpooferMessageSystem();

   //
   // Default destructor
   virtual ~SpooferMessageSystem();

protected:

   //
   // Create the spoofer specific dispatcher
   virtual void createDispatcher();

private:
   SpooferMessageSystem( const SpooferMessageSystem & );            // not implemented
   SpooferMessageSystem &operator=( const SpooferMessageSystem & ); // not implemented

};

#endif
