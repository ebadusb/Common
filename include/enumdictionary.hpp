/*
 
    COPYRIGHT:
    Copyright (C) 1999 by Cobe BCT, Inc.  All rights reserved.

 
    DESCRIPTION:

 
********************************************************************/

#include "cscapp.h"
#include <map.h>

#define DefaultEverestMap "/trima/tcpip/EverestMap"

class EnumDictionary : public cscapp
{
	public:
		// comparison structure for maptype defined below
		struct ltstr
		{  
			bool operator()(const char* s1, const char* s2) const  
			{ return strcmp(s1, s2) < 0;  }
		};
		typedef map< const char *, int, ltstr > maptype;

	private:
		// Put unique Enumerated Type instance names & their values
		// into an STL map
		maptype EnumMap;
		// File containing enumerations to be catalogued
		char EnumFileName[PATH_MAX+1];
		FILE *EnumFile;
		// File containing definition of current section enum
		char *EnumDefFileName;
		FILE *EnumDefFile;
		// Enumerated type name, used for uniquification
		char CurEnumName[80];

		EnumDictionary(void);
		int InitLib(void);
		int GetSectionName(char *s);
	public:
		EnumDictionary(char *InputFileName); 
		int BuildEnumDictionary(void);
		maptype *GetEnumMap(void) { return (maptype *) &EnumMap; }
		void Dump(void);
		~EnumDictionary();
};
