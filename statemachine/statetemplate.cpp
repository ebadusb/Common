/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      statetemplate.cpp
 *
 */

#include <statetemplate.h>

DEFINE_STATE( StateTemplate );

StateTemplate :: StateTemplate( ) :
StateAbs( )
{
}

StateTemplate :: StateTemplate( const StateTemplate &state ) :
StateAbs( state )
{
   copyOver( state );
}

StateTemplate :: ~StateTemplate()
{
   cleanup();
}

int StateTemplate :: transitionStatus()
{
   //
   // Place action here that will determine whether or not the
   //  state will transition and return the appropriate return code ...
   //
   return NO_TRANSITION;
}

int StateTemplate :: preProcess()
{
   //
   // Place action here that will occur before any substate processing ...
   //
   return NORMAL;
}

int StateTemplate :: postProcess()
{
   //
   // Place action here that will occur after any substate processing ...
   //
   return NORMAL;
}

int StateTemplate :: preEnter()
{
   //
   // Place action here that will occur the first time the state is entered ...
   //
   return NORMAL;
}

int StateTemplate :: postExit()
{
   //
   // Place action here that will occur after the state decides to transition ...
   //
   return NORMAL;
}

void StateTemplate :: reset()
{
   //
   // Put any code in here that is needed to reset
   //  the state so that it functions properly if it can be
   //  entered a second time ...
   //
}

void StateTemplate :: copyOver( const StateTemplate & )
{
}

void StateTemplate :: cleanup()
{
   //
   // Delete the dynamic memory ...
   //
}

