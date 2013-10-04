//
// This file is meant to be included at the top of every implementation file
//

#ifndef OS_INC_INCLUDED
#define OS_INC_INCLUDED

#ifdef VXWORKS
#include <vxWorks.h>

//Put your specific win 32
#elif defined WIN32

#else error "Unsupported OS"

#endif
#endif

