
#include <iostream.h>
#include "error.h"

void
_FATAL_ERROR(char* file, int line, char* eString) 
{ 
   cout << file << " " << line << " " << eString << endl; 
}


