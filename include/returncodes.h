/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      returncodes.h
 *
 * ABSTRACT:   This class models enumerated type of return codes
 *
 */

#ifndef RETURNCODES_H                  
#define RETURNCODES_H

//
// Return codes used through out the procedure state
//  machine hierarchy
//
enum ReturnCodes { NOT_INITIALIZED=-1, 
                   PROCESSING_ERROR=0,
                   //
                   // 1-99 left for substate transition codes
                   //
                   TRANSITION_NOSTATE=100,
                   TRANSITION,
                   NO_TRANSITION,                   
                   NORMAL,
                   NO_SUBSTATES,
                   NO_MONITORS,
                   PROCESSING_FAILED             
};


#endif

