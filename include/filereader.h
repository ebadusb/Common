
/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      filereader.h
 *
 * ABSTRACT:   This class declares operations used to read in 
 *              files which define the state machine for proc 
 *
 */

#ifndef FILEREADER_H
#define FILEREADER_H

#include <vxworks.h>
#include <stdio.h>
#include <list>
#include <string>

#include "stateabs.h"

class FileReader
{
public:

   // Default constructor
   FileReader();

   // Default destructor
   virtual ~FileReader();

   //
   // Function which opens the file for reading and accepts the
   //  list to hold the states which are read 
   //
   //   returns NULL  -- file cannot be opened for reading
   //   returns NORMAL - file opened OK
   //
   //int init( const char *file, list< StateAbs* > &slist );
   int init( const char *file, StateAbs *state );

   //
   // This function reads the given file to completion. 
   //
   //   returns 1 -- the states in the file belong to only one state
   //   returns 0 -- the more that one substate list is specified in
   //                 the file.
   //
   int readFile();

protected:

   //
   // These functions are the main section reading functions
   //
   int readNewFile( char *buffer );
   int readState( char *buffer );
   int readMonitors( StateAbs *state );

   //
   // These functions read the minor transition sections
   //
   int readMonitor( char *buffer, StateAbs *state );
   int readTransitions( TransComplex *trans, int numTrans=-1 );
   int readComplexTrans( char *buffer, TransComplex *trans );
   int readCondTrans( char *buffer, TransComplex *trans );
   int readUncondTrans( char *buffer, TransComplex *trans );
   int readTimerTrans( char *buffer, TransComplex *trans );
   int readMessageTrans( char *buffer, TransComplex *trans );

   //
   // These functions are for generic maintainence
   //
   int getLine( char *buffer );
   int restorePosition();
   int removeComments( char *buffer );
   int removeWhitespace( char *buffer );
   StateAbs *findParentState( const char *parent );
   void fLog( const char *errorMsg, const char *buffer);

   //
   // Delete any dynamically created memory
   //
   void cleanup();

protected:

   //
   // File name
   //
   string _FileName;

   //
   // File ptr of the opened file
   //
   FILE *_FilePtr;

   //
   // File position before last read
   //
   fpos_t _FilePos;

   //
   // Main control state ...
   //
   StateAbs *_MainState;
   //
   // List of read in states
   //
   list< StateAbs* > *_StateList;
   list< StateAbs* > _AllStatesList;

   //
   // Current state which we are reading from this file
   //
   StateAbs *_CurrentState;

   //
   // Line count of the file ...
   //
   int _LineCount;
};

#endif
