// $Header: K:/BCT_Development/vxWorks/Common/tools/config_parse/rcs/section.h 1.1 2005/05/11 15:15:08Z jl11312 Exp jl11312 $
//
// Configuration file section class
//
// $Log: section.h $
// Revision 1.1  2005/01/13 20:25:54Z  jl11312
// Initial revision
//

#ifndef _SECTION_INCLUDE
#define _SECTION_INCLUDE

#include <string>
using namespace std;

class Section
{
public:
	Section(const string & name, int arraySize)
		: _name(name), _arraySize(arraySize) { }

	const string & name(void) { return _name; }
	int arraySize(void) { return _arraySize; }

private:
	string _name;
	int _arraySize;
};	

#endif /* ifndef _SECTION_INCLUDE */

