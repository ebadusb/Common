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

   //
   // Make the message base object a spoofer message.  Use
   //  the function without a callback function to send spoof
   //  messages without receiving anything first.  Use the 
   //  function with a callback function to receive the message
   //  type first to trigger spoofing.
   void spoofMessage( MessageBase &mb );
   void spoofMessage( MessageBase &mb, const CallbackBase &cb );

   //
   // Change the message base object back to a normal, non-spoofed
   //  message.
   void despoofMessage( MessageBase &mb );


protected:

   //
   // Create the spoofer specific dispatcher
   virtual void createDispatcher();

private:
   SpooferMessageSystem( const SpooferMessageSystem & );            // not implemented
   SpooferMessageSystem &operator=( const SpooferMessageSystem & ); // not implemented

};

#endif
