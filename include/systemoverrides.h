/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      systemoverrides.h, Functions which users must override.
 *
 * ABSTRACT:   This file contains functions which users must provide to
 *             use the messaging system.  
 */

#ifndef _SYSTEM_OVERRIDES_H_
#define _SYSTEM_OVERRIDES_H_

#include <set>

/*
 The functions in this header file must be provide to handle non-recoverable
  errors
*/
#include "error.h"

/*
 This function returns the local network address
*/
unsigned long getNetworkAddress( void );

/*
 This function takes in a user provided set.  The function will fill
  the set with all nodes for which the network must connect.  
  * The data in the set must assigned the IP address of the networked
    processors.  
  * Do not add the local node's network address to this set.  Only 
    add remote addreses to the set.  
  * This function will get called upon Router task initialization.  
    Do not add any functionality which will not be usable when the 
    Router is started. i.e. Everything used in this function must be 
    available and initialized befor the Router and Gateway tasks 
    start.
*/
void getNetworkedNodes( set< unsigned long > &nodes );
                   
#endif
