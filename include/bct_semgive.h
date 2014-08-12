/*
* Include for the BCT specific sempaphore give function.
* 
* This method performs a taskDelay(0) after the semGive
* on priority inverted tasks allowing tasks at the same
* priority to execute.
*/

#ifndef _BCT_SEMGIVE_INCLUDE
#define _BCT_SEMGIVE_INCLUDE

#include <vxWorks.h>
#include <semLib.h>

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

STATUS bctSemGive(SEM_ID semId);

#ifdef __cplusplus
}; // extern "C"
#endif /* ifdef __cplusplus */

#endif /* _BCT_SEMGIVE_INCLUDE */

