/*
 
	COPYRIGHT:
	Copyright (C) 1998 by Amida Software Services. Genesee, Colorado
	All rights reserved.  Any part of this work covered by the copyright
	hereon may be reproduced or used in any form or by any means without
	written permission of Amida Software Services.
 
	NAME: 
 
	DESCRIPTION:

 
********************************************************************/
#ifndef CSCLIB_H
#define CSCLIB_H

#ifndef __QNX__
static const char cscapph_sccsid[] = "@(#)%M% %I% Amida S/W %G%";
static const char cscapph_rcsid[] = "$Author: TD10216 $ $Id: CSCAPP.H 1.1 1999/06/30 20:28:42 TD10216 Exp TD10216 $";
#endif

#include <stdio.h>

#define HEADERSONLY 1
class cscapp
{
	public:
		cscapp(void);
		~cscapp();
		void Destroy(void);
		char *GetFileVersion(void);
		char *GetFixVersion(void);
		char *GetPlatform(void);
		int GetMaxPatternLen(void);
		int GetMaxLineNumChars(void);
		// wrapper routines
		int init(char *home);
		void build(void);
		char *Getrefsdir(void);
		FILE *Getrefsfound(void);
		int Isuptodate(void);
		void Setuptodate(int);
		int Osrcfiles(void);
		void opensymrefs(void);
		int Symrefs(void);
		void RewindSymrefs(void);
		void Unlinkrefs(void);
		void Unlinknewrefs(void);
		char *Getscriptfilename(void);
		char *Getrefsfilename(void);
		void Setrefsfilename(char *);
		void Setreffile(char *);
		void Setnamefile(char *);
		void Setcompress(int);
		void closerefsfound(void);
		int Newrefsfound(void);
		int Openrefsfound(void);
		int ReOpenrefsfound(void);
		void includedir(char *);
		void RemoveIncludedFiles(int osrcfiles);
		void sourcedir(char *);
		int Getnsrcfiles(void);
		char *basename(char *);
		void vpinitRecursively(char *);
		int FileListFromFile(char *fname);
		int FileListFromList(char *names[]);
		int FileListFromCurDir(int HeadersOnly=0);
		void findsymbol(char *p);
		void finddef(char *p);
		void calledby(char *p);
		void calling(char *p);
		void findobj(char *p);
		void finddecl(char *p);
		int Decltype(char *p);
		int findPrimitiveTypedefSource(char *p);
		void getstructdecl(char *StructName);
		void Inheriting(char *Class);
		void ListMethods(char *Class);
		void findstring(char *p);
		void findfile(char *p);
		void findinclude(char *p);

		int errorsfound(void);
		void Seterrorsfound(int);
};

// compile -DDebug & add dbgprintf(stderr,"") as needed
// NB: To use GetDebugFd requires the library be compiled with DEBUG, 
// too. This creates a serialized stream of app and lib output.
extern FILE *GetDbgFd(void);
#ifdef DEBUG
#define dbgprintf fprintf
#else
    inline void dbgprintf(FILE *, ...) {}
#endif

#endif
