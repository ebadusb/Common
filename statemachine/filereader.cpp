/*                                    -
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      filereader.cpp
 *
 */

#include "datalog_levels.h"
#include "filereader.h"
#include "returncodes.h"
#include "transcomplex.h"
#include "transconditional.h"
#include "transunconditional.h"
#include "transtimer.h"
#include "transmessage.h"
#include "logicaloperationtype.h"

#include <ctype.h>

//
// Delimiter tokens which can separate entries in the list
//
const char *DELIMITERS=" ,:\t\n\f\r\v";
const char *COMMENT_CHAR="#";
static const int FILEREADER_BUFFER_SIZE = 256;


FileReader :: FileReader() :
_FileName( ),
_FilePtr( 0 ),
_FilePos( 0 ),
_MainState( 0 ),
_StateList(0),
_AllStatesList(0),
_CurrentState(0),
_LineCount( 0 )
{
}

FileReader :: ~FileReader()
{
   cleanup();
}

int FileReader :: init( const char *file, StateAbs *state )
{
   //
   // Save the file name for logging ...
   //
   _FileName = file;

   //
   // Save the ptr to our main state and main state list ...
   //
   _MainState = state;
   _StateList = &( state->substates() );

   //
   // Open the file ...
   //
   //  ( readonly access )
   //
   _FilePtr = fopen( file , "r" );

   if ( _FilePtr != NULL)
   {
      _LineCount = 0;
      return NORMAL;
   }
   fLog( "Cannot open file", file );
   return PROCESSING_ERROR;
}

int FileReader :: readFile()
{

   int status = NORMAL;
   char constBuffer[FILEREADER_BUFFER_SIZE]="";
   char changableBuffer[FILEREADER_BUFFER_SIZE]="";

   if ( _FilePtr == NULL )
   {
      // File Pointer is not initialized
      return PROCESSING_ERROR;
   }

   //
   // Read the monitors
   //
   if ( readMonitors( _MainState ) == PROCESSING_ERROR )
   {
      return PROCESSING_ERROR;
   }

   //
   // read the whole file
   //
   while ( getLine( constBuffer ) != PROCESSING_ERROR )
   {
      //
      // Duplicate the buffer ...
      //
      strcpy (changableBuffer , constBuffer );

      //
      // Read a new file if found ...
      //
      if ( readNewFile( changableBuffer ) == PROCESSING_ERROR )
      {
         //
         // Error processing new file ...
         //
         status =  PROCESSING_ERROR;
         break;
      }

      //
      // Duplicate the buffer ...
      //
      strcpy (changableBuffer , constBuffer );
      //
      // Read a new state (including transitions) if found ...
      //
      if ( readState( changableBuffer ) == PROCESSING_ERROR )
      {
         //
         // Error processing new state ...
         //
         status = PROCESSING_ERROR;
         break;
      }
   }
   //
   // Close the file ...
   //
   fclose( _FilePtr );
   _FilePtr = 0;

   return status;
}

int FileReader :: readNewFile( char *buffer )
{
   //
   // Check buffer for 'include' keyword ...
   //
   char *savePtr=0;
   char *p = strtok_r( buffer, DELIMITERS, &savePtr );
   if ( strncmp( p, "[include]", 9 ) == 0 ) 
   {
      //
      // Parse the file name ...
      //
      p = strtok_r( NULL, DELIMITERS, &savePtr );
      if ( !p || strlen( p ) == 0 )
      {
         // 
         // Error, no filename given ...
         //
         fLog( "No filename given", buffer );
         return PROCESSING_ERROR;
      }

      StateAbs *curState;
      //
      // Add to the current state's list ...
      //
      if ( _CurrentState != 0 )
      {
         curState = _CurrentState;
      }
      //
      // If we are working on the main state list ...
      //
      else
      {
         curState = _MainState;
      }

      //
      // Create a new file reader ...
      //
      FileReader newReader;
      if ( !newReader.init( p, curState ) ) 
      {
         //
         // Error initializing the new file ...
         //
         fLog( "Cannot initialize the new file", buffer );
         return PROCESSING_ERROR;
      }
      if ( !newReader.readFile() )
      {
         //
         // Error reading in the new file ...
         //
         return PROCESSING_ERROR;
      }

   }
   return NORMAL;
}

int FileReader :: readState( char *buffer )
{
   //
   // Check buffer for 'state' keyword ...
   //
   char *savePtr=0;
   char *p = strtok_r( buffer, DELIMITERS, &savePtr );
   if ( strncmp( p, "[state]", 7 ) == 0 )
   {
      //
      // Parse the state name ...
      //
      p = strtok_r( NULL, DELIMITERS, &savePtr );
      if ( !p || strlen( p ) == 0 ) 
      {
         //
         // Error no state name given ...
         //
         fLog("No state name given", p );
         return PROCESSING_ERROR;
      }

      //
      // Create the new state ...
      //
      StateAbs *newState = (StateAbs*)StateDictionary::create( p );
      if ( !newState ) 
      {
         //
         // Cannot find state in StateDictionary ...
         //
         fLog("Cannot find state name in dictionary", p );
         return PROCESSING_ERROR;
      }
      _CurrentState = newState;
      _CurrentState->stateName( p );

      //
      // Parse the parent state name ...
      //
      p = strtok_r( NULL, DELIMITERS, &savePtr );
      if ( !p || strlen( p ) == 0 ) 
      {
         _StateList->push_back( newState );
         _AllStatesList.push_front( newState );
      }
      else
      {
         //
         // Add this state to the parent state's list ...
         //
         StateAbs *pPtr = findParentState( p );
         if ( !pPtr ) 
         {
            //
            // Did not find parent pointer ...
            //
            fLog("Did not find parent in state list", p);
            return PROCESSING_ERROR;
         }
         pPtr->addSubstate( newState );
         _AllStatesList.push_front( newState );
      }

      //
      // Read the monitors ...
      //
      if ( readMonitors( newState ) == PROCESSING_ERROR ) 
      {
         //
         // Error reading the monitors ...
         //
         fLog("Reading the monitors for the state", newState->stateName() );
         return PROCESSING_ERROR;
      }

      //
      // Read the transitions ...
      //
      if ( readTransitions( newState->transition() ) == PROCESSING_ERROR ) 
      {
         //
         // Error reading the transitions ...
         //
         fLog("Reading the transitions for the state", newState->stateName() );
         return PROCESSING_ERROR;
      }
   }

   return NORMAL;
}

int FileReader :: readMonitors( StateAbs *state )
{
   //
   // Read the monitors ...
   //
   char constBuffer[FILEREADER_BUFFER_SIZE]="";
   char changableBuffer[FILEREADER_BUFFER_SIZE]="";
   while ( getLine( constBuffer ) != PROCESSING_ERROR )
   {
      //
      // Read a comment line ...
      //
      if ( strlen( constBuffer ) == 0 ) 
      {
         continue;
      }

      //
      // Duplicate the buffer ...
      //
      strcpy(changableBuffer, constBuffer );
      //
      // Read in the monitor ...
      //
      int status=NORMAL;
      if ( (status=readMonitor( changableBuffer, state )) == NO_MONITORS )
      {
         //
         // Read too far, back up and return ...
         //
         return restorePosition();
      }
      if ( status == PROCESSING_ERROR ) 
      {
         //
         // Error reading monitors ...
         //
         fLog("Reading monitors", constBuffer );
         return PROCESSING_ERROR;
      }
   }
   return NORMAL;
}

int FileReader :: readMonitor( char *buffer, StateAbs *state )
{
   if ( !state ) 
   {
      fLog("Invalid state pointer", 0 );
      return PROCESSING_ERROR;
   }

   //
   // Check buffer for 'alarm', 'algorithm', or 'monitor' keywords ...
   //
   char *savePtr=0;
   char *p = strtok_r( buffer, DELIMITERS, &savePtr );
   if (    strncmp( p, "[alarm]", 7 ) == 0 
        || strncmp( p, "[algorithm]", 11 ) == 0 
        || strncmp( p, "[monitor]", 9 ) == 0 ) 
   {
      //
      // Parse the monitor name ...
      //
      p = strtok_r( NULL, DELIMITERS, &savePtr );
      if ( !p || strlen( p ) == 0 ) 
      {
         //
         // Error no monitor name given ...
         //
         fLog("No monitor name given", p );
         return PROCESSING_ERROR;
      }
      //
      // Create the new monitor ...
      //
      MonitorBase* newMonitor=(MonitorBase*)ObjDictionary::create( p );
      if ( !newMonitor ) 
      {
         //
         // Cannot find monitor in ObjDictionary ...
         //
         fLog("Cannot find monitor name in dictionary", p );
         return PROCESSING_ERROR;
      }
      state->addMonitor( newMonitor );
   }
   else
   {
      return NO_MONITORS;
   }
   return NORMAL;
}

int FileReader :: readTransitions( TransComplex *trans, int numTrans )
{
   int numRead=0;
   //
   // Read the transitions ...
   //
   char constBuffer[FILEREADER_BUFFER_SIZE]="";
   char changableBuffer[FILEREADER_BUFFER_SIZE]="";
   while ( getLine( constBuffer ) != PROCESSING_ERROR )
   {
      //
      // Read a comment line ...
      //
      if ( !constBuffer || strlen( constBuffer ) == 0 ) 
      {
         continue;
      }

      //
      // Duplicate the buffer ...
      //
      strcpy(changableBuffer,constBuffer );
      //
      // Read in the transitions ...
      //
      int status=NORMAL;
      if ( ( ( status=readComplexTrans( changableBuffer, trans )) == NO_TRANSITION ) &&
           ( ( status=readCondTrans( changableBuffer, trans )) == NO_TRANSITION ) &&
           ( ( status=readUncondTrans( changableBuffer, trans )) == NO_TRANSITION ) &&
           ( ( status=readTimerTrans( changableBuffer, trans )) == NO_TRANSITION ) &&
           ( ( status=readMessageTrans( changableBuffer, trans )) == NO_TRANSITION ) )
      {
         //
         // Read too far, back up and return ...
         //
         return restorePosition();
      }
      if ( status == PROCESSING_ERROR ) 
      {
         //
         // Error reading transitions ...
         //
         fLog("Reading transitions", constBuffer );
         return PROCESSING_ERROR;
      }
      numRead++;
      //
      // Check for too many transitions read ...
      //
      if ( numTrans > 0 && numRead >= numTrans ) 
      {
         break;
      }
   }
   return NORMAL;
}

int FileReader :: readComplexTrans( char *constBuffer, TransComplex *trans )
{
   //
   // Check buffer for 'complex' keyword ...
   //
   char *savePtr=0;
   char buffer[FILEREADER_BUFFER_SIZE];
   strcpy( buffer, constBuffer );
   char *p = strtok_r( buffer, DELIMITERS, &savePtr );
   if ( strncmp( p, "[complex]", 9 ) == 0 ) 
   {
      //
      // Create the complex transition ...
      //
      TransComplex *newTrans = new TransComplex;
      trans->addTransition( newTrans );

      //
      // Parse the number of transitions under this one
      //
      p = strtok_r( NULL, DELIMITERS, &savePtr );
      if ( !p || strlen( p ) == 0 ) 
      {
         //
         // Error no transition count given ...
         //
         fLog("No transition count given", p );
         return PROCESSING_ERROR;
      }
      int transitionCount = (int)strtol( p, NULL, 10 );

      //
      // Parse the condition type ...
      //
      p = strtok_r( NULL, DELIMITERS, &savePtr );
      if ( !p || strlen( p ) == 0 ) 
      {
         //
         // Error no condition type given ...
         //
         fLog("No condition type given", p );
         return PROCESSING_ERROR;
      }
      LogicalOperationType::Type t = LogicalOperationType::type( p );
      if ( t == LogicalOperationType::nullOperation )
      {
         //
         // Error, invalid condition type ...
         //
         fLog("Invalid condition type", p );
         return PROCESSING_ERROR;
      }
      newTrans->conditionType( LogicalOperationType::type( p ) ); 

      //
      // Parse the transition state name ...
      //
      p = strtok_r( NULL, DELIMITERS, &savePtr );
      if ( p && strlen( p ) > 0 ) 
      {
         //
         // Assign the state name ...
         //
         if ( !StateDictionary::valid( p ) ) 
         {
            //
            // Error, invalid transition state name ...
            // 
            fLog("Invalid transition state name", p );
            return PROCESSING_ERROR;
         }
         newTrans->transitionState( p );
      }
      //
      // Read the transitions ...
      //
      if ( readTransitions( newTrans ) == PROCESSING_ERROR ) 
      {
         //
         // Error reading the transitions ...
         //
         fLog("Reading the transitions",buffer);
         return PROCESSING_ERROR;
      }
   }
   else
   {
      return NO_TRANSITION;
   }
   return NORMAL;
}

int FileReader :: readCondTrans( char *constBuffer, TransComplex *trans )
{              
   //
   // Check buffer for 'conditional' keyword ...
   //
   char *savePtr=0;
   char buffer[FILEREADER_BUFFER_SIZE];
   strcpy( buffer, constBuffer );
   char *p = strtok_r( buffer, DELIMITERS, &savePtr );
   if ( strncmp( buffer, "[conditional]", 13 ) == 0 )
   {
      //
      // Create the conditional transition ...
      //
      TransConditional *newTrans = new TransConditional;
      trans->addTransition( newTrans );

      //
      // Parse the return code
      //
      p = strtok_r( NULL, DELIMITERS, &savePtr );
      if ( !p || strlen( p ) == 0 ) 
      {
         //
         // Error no return code given...
         //
         fLog("No return code given", p );
         return PROCESSING_ERROR;
      }
      newTrans->statusFlag( (int)strtol( p, NULL, 10 ) );

      //
      // Parse the transition state name ...
      //
      p = strtok_r( NULL, DELIMITERS, &savePtr );
      if ( p && strlen( p ) > 0 ) 
      {
         //
         // Assign the state name ...
         //
         if ( !StateDictionary::valid( p ) ) 
         {
            //
            // Error, invalid transition state name ...
            // 
            fLog("Invalid transition state name", p );
            return PROCESSING_ERROR;
         }
         newTrans->transitionState( p );
      }

   }
   else
   {
      return NO_TRANSITION;
   }
   return NORMAL;
}

int FileReader :: readUncondTrans( char *constBuffer, TransComplex *trans )
{              
   //
   // Check buffer for 'unconditional' keyword ...
   //
   char *savePtr=0;
   char buffer[FILEREADER_BUFFER_SIZE];
   strcpy( buffer, constBuffer );
   char *p = strtok_r( buffer, DELIMITERS, &savePtr );
   if ( strncmp( buffer, "[unconditional]", 13 ) == 0 )
   {
      //
      // Create the conditional transition ...
      //
      TransUnconditional *newTrans = new TransUnconditional;
      trans->addTransition( newTrans );

      //
      // Parse the transition state name ...
      //
      p = strtok_r( NULL, DELIMITERS, &savePtr );
      if ( p && strlen( p ) > 0 ) 
      {
         //
         // Assign the state name ...
         //
         if ( !StateDictionary::valid( p ) ) 
         {
            //
            // Error, invalid transition state name ...
            // 
            fLog("Invalid transition state name", p );
            return PROCESSING_ERROR;
         }
         newTrans->transitionState( p );
      }

   }
   else
   {
      return NO_TRANSITION;
   }
   return NORMAL;
}

int FileReader :: readTimerTrans( char *constBuffer, TransComplex *trans )
{
   //
   // Check buffer for 'timer' keyword ...
   //
   char *savePtr=0;
   char buffer[FILEREADER_BUFFER_SIZE];
   strcpy( buffer, constBuffer );
   char *p = strtok_r( buffer, DELIMITERS, &savePtr );
   if ( strncmp( buffer, "[timer]", 7 ) == 0 ) 
   {
      //
      // Create the timer transition ...
      //
      TransTimer *newTrans = new TransTimer;
      trans->addTransition( newTrans );

      //
      // Parse the timeout time
      //
      p = strtok_r( NULL, DELIMITERS, &savePtr );
      if ( !p || strlen( p ) == 0 ) 
      {
         //
         // Error no timeout time given ...
         //
         fLog("No timeout time given", p );
         return PROCESSING_ERROR;
      }
      int time = (int)strtol( p, NULL, 10 );
      newTrans->init( time );

      //
      // Parse the transition state name ...
      //
      p = strtok_r( NULL, DELIMITERS, &savePtr );
      if ( p && strlen( p ) > 0 ) 
      {
         //
         // Assign the state name ...
         //
         if ( !StateDictionary::valid( p ) ) 
         {
            //
            // Error, invalid transition state name ...
            // 
            fLog("Invalid transition state name", p );
            return PROCESSING_ERROR;
         }
         newTrans->transitionState( p );
      }

   }
   else
   {
      return NO_TRANSITION;
   }
   return NORMAL;
}

int FileReader :: readMessageTrans( char *constBuffer, TransComplex *trans )
{
   //
   // Check buffer for 'message' keyword ...
   //
   char *savePtr=0;
   char buffer[FILEREADER_BUFFER_SIZE];
   strcpy( buffer, constBuffer );
   char *p = strtok_r( buffer, DELIMITERS, &savePtr );
   if ( strncmp( buffer, "[message]", 9 ) == 0 ) 
   {
      //
      // Create the message transition ...
      //
      TransMessage *newTrans = new TransMessage;
      trans->addTransition( newTrans );

      //
      // Parse the message enumeration
      //
      p = strtok_r( NULL, DELIMITERS, &savePtr );
      if ( !p || strlen( p ) == 0 ) 
      {
         //
         // Error no message given ...
         //
         fLog("No message id given", p );
         return PROCESSING_ERROR;
      }
      else 
      {
         if ( !newTrans->init( p ) )
         {
            //
            // Error invalid message given ...
            //
            fLog("Invalid message name given", p );
            return PROCESSING_ERROR;
         }
      }

      //
      // Parse the transition state name ...
      //
      p = strtok_r( NULL, DELIMITERS, &savePtr );
      if ( p && strlen( p ) > 0 ) 
      {
         //
         // Assign the state name ...
         //
         if ( !StateDictionary::valid( p ) ) 
         {
            //
            // Error, invalid transition state name ...
            // 
            fLog("Invalid transition state name", p );
            return PROCESSING_ERROR;
         }
         newTrans->transitionState( p );
      }
   }
   else
   {
      return NO_TRANSITION;
   }
   return NORMAL;
}

int FileReader :: getLine( char *buffer )
{
   //
   // Read in 1 line from the File ...
   //
   fgetpos( _FilePtr, &_FilePos );

   taskDelay( 1 );
   if ( fgets( buffer, FILEREADER_BUFFER_SIZE , _FilePtr ) != NULL )
   {
	   taskDelay( 1 );
      _LineCount++;

      //
      //
      // Strip out the comments and whitespace characters ...
      //
      if ( removeComments( buffer ) == PROCESSING_ERROR ||
           removeWhitespace( buffer) == PROCESSING_ERROR )
      {
         //
         // Error while removing whitespace ...
         //
         return PROCESSING_ERROR;
      }

      //
      // Line read in OK ...
      //
      return NORMAL;
   }

   //
   // Line read failed
   //
   return PROCESSING_ERROR;
}

int FileReader :: restorePosition()
{
   //
   // Put the file back to its position before the
   //  previous read ...
   //
   if ( fsetpos( _FilePtr, &_FilePos ) )
   {
      //
      // Error setting the file position
      //
      fLog ("Setting the file position","");
      return PROCESSING_ERROR;
   }
   _LineCount--;
   return NORMAL;
}

int FileReader :: removeComments( char *buffer )
{
   // 
   // Find the first occurence of the Comment character
   //
   char *p = strchr( buffer, COMMENT_CHAR[0] );

   //
   // If the comment character was found ...
   if ( p != NULL ) 
   {
      //
      // If it is the first character ...
      //
      if ( p == buffer )
      {
         //
         // Set the first character to be NULL
         //  thereby terminating the string
         //
         buffer[0]='\0';
      }
      else 
      {
         //
         // Grab everything up to the comment ...
         //  ( this will change buffer removing the comment )
         //
         char *savePtr=0;
         p = strtok_r( buffer, COMMENT_CHAR, &savePtr );
      }
   }
   return NORMAL;
}

int FileReader :: removeWhitespace( char *constBuffer )
{
   char *token = constBuffer;
   char buffer[FILEREADER_BUFFER_SIZE];

   int i=0;
   int alreadyAddedOne=1;
   while ( *token ) 
   {
      if ( !isspace( *token ) )
      {
         alreadyAddedOne = 0;
         buffer[i] = *token;
         i++;
      }
      else if ( !alreadyAddedOne )
      {
         alreadyAddedOne = 1;
         buffer[i] = ' ';
         i++;
      }
      token++;
   }
   buffer[i] = '\0';

   //
   // Reset the buffer for return ...
   //
   strcpy( constBuffer, buffer );

   return NORMAL;
}

StateAbs *FileReader :: findParentState( const char *parent )
{
   StateAbs *pPtr=0;
   // 
   // Find the parent state ...
   //
   list< StateAbs* >::iterator state;
   for ( state = _AllStatesList.begin() ;
         state != _AllStatesList.end()  ;
         state++ )
   {
      //
      // This state is the specified state ...
      //
      if ( *(*state) == parent )
      {
         pPtr = (*state);
         break;
      }
   }
   return pPtr;
}

void FileReader :: fLog( const char *errorMsg, const char *line )
{
   if ( errorMsg && line )
   DataLog( log_level_state_machine_error ) << "Error: " << errorMsg
                << "       File: " << _FileName << " Line#: " << _LineCount
                << " | " << line << endmsg;
}

void FileReader :: cleanup()
{
}
