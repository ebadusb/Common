/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      stateabs.cpp
 *
 */

#include "datalog_levels.h"
#include "stateabs.h"
#include "manager.h"

StateAbs :: StateAbs( StateAbs *parentState ) :
_StateManager( 0 ),
_SubstateList(),
_StateName(),
_Transition(),
_TransitionStatus( NO_TRANSITION ),
_ParentState( parentState ),
_MaintainHistoryFlag( 0 ),
_ProcessingDone( NO_TRANSITION ),
_MonitorsList() 
{
}

StateAbs :: StateAbs( const StateAbs &state ) :
_StateManager( 0 ),
_SubstateList(),
_Transition( state._Transition ),
_TransitionStatus( state._TransitionStatus ),
_ParentState( state._ParentState ),
_MaintainHistoryFlag( state._MaintainHistoryFlag ),
_ProcessingDone( state._ProcessingDone ),
_MonitorsList()
{
   copyOver( state );
}

StateAbs :: ~StateAbs()
{
   cleanup();
}

int StateAbs :: init( const TransComplex &t )
{
   _Transition = t;
   return init();
}

int StateAbs :: init()
{

   //
   // Initialize the substates ...
   //
   list< StateAbs* >::iterator state;
   for ( state = _SubstateList.begin() ;
         state != _SubstateList.end()  ;
         ++state )
   {
      if ( (*state)->init() == PROCESSING_ERROR )
      {
         DataLog( log_level_state_machine_error ) << "Error initializing -> " << (*state)->_StateName << endmsg;
         return PROCESSING_ERROR;
      }
   }

   //
   // Create the state manager ...
   //
   if ( _StateManager )
   {
      delete _StateManager;
      _StateManager = 0;
   }
   _StateManager = new Manager;

   if ( !_StateManager )
   {
      //
      // Creation failed (probably won't happen)
      return PROCESSING_ERROR;
   }

   //
   // Initialize the state manager ...
   //
   DataLog( log_level_state_machine_debug ) << "Initializing state machine for -> " << _StateName << endmsg;
   int status = _StateManager->init( _SubstateList );
   if ( status == PROCESSING_ERROR ) 
   {
      DataLog( log_level_state_machine_error ) << "Error initializing state machine for -> " << _StateName << endmsg;
   }
   else if ( status == NO_SUBSTATES )
   {
      _ProcessingDone = NO_SUBSTATES;
   }

   //
   // Initialize the monitors ...
   //
   DataLog( log_level_state_machine_debug ) << "Initializing monitors for -> " << _StateName << endmsg;
   if ( initMonitors() == PROCESSING_ERROR )
   {
      DataLog( log_level_state_machine_error ) << "Error initializing monitors" << endmsg;
      return PROCESSING_ERROR;
   }

   DataLog( log_level_state_machine_debug ) << "Done initializing state machine for -> " << _StateName << endmsg;
   
   return status;
}

const char *StateAbs :: currentState() const
{
   const char *t = _StateManager->currentState();
   if ( strcmp( t , "__null__" ) == 0 )
   {
      return _StateName.c_str();
   }
   return t;
}

int StateAbs :: enter()  
{
   DataLog( log_level_state_machine_info ) << "Enter: " << _StateName << endmsg;

   //
   // One time call upon entering the state
   //
   if ( preEnter() != NORMAL )
   {
      //
      // Don't activate the transitions if we failed
      //  the enter code
      //
      return PROCESSING_ERROR;
   }

   //
   // Activate the transitions
   //
   _Transition.activate();
   //
   // Enable the monitors
   //
   enableMonitors();

   return NORMAL;
}

int StateAbs :: process()
{
   DataLog( log_level_state_machine_debug ) << "Enter Process: " << _StateName << endmsg;

   int processStatus;
   //
   // Do the pre-substate processing for this state
   //
   if ( preProcess() == PROCESSING_ERROR )
   {
      DataLog( log_level_state_machine_error ) << _StateName << " Pre-process error" << endmsg;
      return PROCESSING_ERROR;
   }

   //
   // Determine current processing status ...
   //
   processStatus = transitionStatus();

   if ( processStatus == NO_TRANSITION )
   {
      int smRetCode = NO_TRANSITION;

      if ( _ProcessingDone == NO_TRANSITION)
      {
         DataLog( log_level_state_machine_debug ) << "State Manager Process: " << _StateName << endmsg;

         //
         // Call the process of the state manager ...
         //
         smRetCode = _StateManager->process();

         DataLog( log_level_state_machine_debug ) << "State Manager Processing Done: " << _StateName << " (code = " << smRetCode << ")" << endmsg;
      }
      
      if ( smRetCode == NOT_INITIALIZED )
      {
         //
         // The state manager isn't initialized ...
         //
         DataLog( log_level_state_machine_error ) << _StateName << " State manager isn't initialized" << endmsg;
         return PROCESSING_ERROR;
      }
      else if ( smRetCode == PROCESSING_FAILED )
      {
         //
         // The substate processing failed
         //
         DataLog( log_level_state_machine_error ) << _StateName << " Substate processing failed" << endmsg;
         return PROCESSING_ERROR;
      } 
      else if ( smRetCode == TRANSITION_NOSTATE )
      {
         //
         // No more substates found for this state
         //
         _ProcessingDone = TRANSITION_NOSTATE;
      }
      
      //
      // Do the post-substate processing for this state
      //
      if ( postProcess() == PROCESSING_ERROR )
      {
         DataLog( log_level_state_machine_error ) << _StateName << " Post-process error" << endmsg;
         return PROCESSING_ERROR;
      }

      //
      // Determine current processing status ...
      //
      processStatus = transitionStatus();
   }
   
   //
   // This state is through processing, transition
   //  whether or not the substates are finished ...
   //
   if ( processStatus != NO_TRANSITION ) 
   {
      _ProcessingDone = TRANSITION;
      DataLog( log_level_state_machine_debug ) << _StateName << " Processing complete status=" << processStatus << endmsg;
   }
   //
   // This state has substates, and they are finished
   //  processing ...
   //
   else if ( _SubstateList.size() != 0 &&
             _ProcessingDone == TRANSITION_NOSTATE ) 
   {
      processStatus = TRANSITION_NOSTATE;
      DataLog( log_level_state_machine_debug ) << _StateName << " Processing complete status=" << processStatus << endmsg;
   }

   //
   // Check state specific monitors after all processing is complete
   //  and we are not transitioning out of this state ...
   //
   if ( processStatus == NO_TRANSITION )
   {
      checkMonitors();
   }

   DataLog( log_level_state_machine_debug ) << "Exit Process: " << _StateName << " status=" << processStatus << endmsg;

   return processStatus;

}

int StateAbs :: exit()
{
   DataLog( log_level_state_machine_info ) << "Exit: " << _StateName << endmsg;

   int status = NORMAL;

   //
   // Deactivate the transitions
   //
   _Transition.deactivate();
   //
   // Disable the monitors
   //
   disableMonitors();
                                                    
                                                    
   if ( postExit() != NORMAL )
   {
      status = PROCESSING_ERROR;
   }

   //
   // Reset the state if the history flag is not set ...
   //
   if ( !_MaintainHistoryFlag )
   {
      reset();
      DataLog( log_level_state_machine_debug ) << "Reset state -> " << _StateName << endmsg;

      int status = _StateManager->reset();

      //
      // Reset the processing done flag ...
      //
      if ( status == NO_SUBSTATES ) 
      {
         _ProcessingDone = NO_SUBSTATES;
      }
      else 
      {
         _ProcessingDone = NO_TRANSITION;
      }

      DataLog( log_level_state_machine_debug ) << "Reset state manager -> " << _StateName << endmsg;
   }

   return status;
}


bool StateAbs :: checkForTransition( const int status )
{  
   //
   // Get the next state we should transition into ...
   string newState = nextState( status );

   //
   // If no transitions out of this state were specified and
   //  the state is not considered done with processing ...
   //
   if (    ( newState.empty() || newState == "__null__" ) 
        && ( _ProcessingDone == NO_SUBSTATES || _ProcessingDone == NO_TRANSITION ) )
   {
      //
      // No transition is allowed ...
      //
      return false;
   }
   if ( !newState.empty() && newState != "__null__" ) 
      DataLog( log_level_state_machine_info ) << "Transition " << _StateName << "->" << newState << endmsg;
   return true;
}

const string &StateAbs::nextState( const int status )
{
   if ( _StateManager &&
        !_StateManager->nextState().empty() )
   {
      DataLog( log_level_state_machine_info ) << " NextState " << _StateManager->nextState() << endmsg;
      return _StateManager->nextState();
   }
   else
   {
      _Transition.canTransition( status );
      return _Transition.transitionState();
   }
}

int StateAbs :: addMonitor( MonitorBase *monitor )
{
   if ( monitor )
   {
      _MonitorsList.push_back( monitor );
      return NORMAL;
   }
   return PROCESSING_ERROR;
}

int StateAbs :: initMonitors()
{
   //
   // Loop through the monitor list ...
   //
   list< MonitorBase* >::iterator monitor;
   for ( monitor = _MonitorsList.begin() ;
         monitor != _MonitorsList.end()  ;
         ++monitor )
   {
      //
      // Initialize the monitor ...
      //
      (*monitor)->Initialize(); 
   }
   return NORMAL;
}

void StateAbs :: enableMonitors()
{
   //
   // Loop through the monitors ...
   //
   list< MonitorBase* >::iterator monitor;
   for ( monitor = _MonitorsList.begin() ;
         monitor != _MonitorsList.end()  ;
         ++monitor )
   {
      //
      // Enable the monitor ...
      //
      (*monitor)->enable(); 
   }
}

void StateAbs :: disableMonitors()
{
   //
   // Loop through the monitors list ...
   //
   list< MonitorBase* >::iterator monitor;
   for ( monitor = _MonitorsList.begin() ;
         monitor != _MonitorsList.end()  ;
         ++monitor )
   {
      //
      // Disable the monitor ...
      //
      (*monitor)->disable(); 
   }
}

void StateAbs :: checkMonitors()
{
   //
   // Loop through the monitor list ...
   //
   list< MonitorBase* >::iterator monitor;
   for ( monitor = _MonitorsList.begin() ;
         monitor != _MonitorsList.end()  ;
         ++monitor )
   {
      //
      // Ask each monitor to check itself with the current status
      //  information ...
      //
      (*monitor)->Monitor( ); 
   }
}

StateAbs &StateAbs :: operator=( const StateAbs &state )
{
   if ( &state != this ) 
   {
      _Transition = state._Transition;
      _TransitionStatus = state._TransitionStatus;
      _ParentState = state._ParentState;
      _MaintainHistoryFlag = state._MaintainHistoryFlag;
      _ProcessingDone = state._ProcessingDone;
      copyOver( state );
   }
   return *this;
}

int StateAbs :: operator==(const StateAbs &state) const
{
   return operator==( state._StateName );
}

int StateAbs :: operator==(const string &state) const
{
   return ( compare( state ) == 0 ? 1 : 0 );
}

int StateAbs :: operator==(const char *state) const
{
   return ( compare( state ) == 0 ? 1 : 0 );
}

int StateAbs :: operator<(const StateAbs &state) const
{
   return operator<( state._StateName );
}


int StateAbs :: operator<(const string &state) const
{
   return ( compare( state ) <0 ? 1 : 0 );
}

int StateAbs :: operator<(const char *state) const
{
   return ( compare( state ) <0 ? 1 : 0 );
}

int StateAbs :: compare( const StateAbs &state ) const
{
   return compare( state._StateName );
}

int StateAbs :: compare( const string &state ) const
{
   return _StateName.compare( state );
}

int StateAbs :: compare( const char *state ) const
{
   return _StateName.compare( state );
}


void StateAbs :: addSubstate( StateAbs *state )
{
   _SubstateList.push_back( state );
}



void StateAbs :: copyOver( const StateAbs &s )
{
   //
   // Copy the state name
   //
   _StateName = s._StateName;

   //
   // Loop through the substate list and clone each state
   //  for my own list
   //
   list< StateAbs* >::iterator state;
   for ( state = ( (StateAbs&)s )._SubstateList.begin() ;
         state != ( (StateAbs&)s )._SubstateList.end()  ;
         ++state )
   {
      _SubstateList.push_back( (*state)->clone() );
   }

}

void StateAbs :: cleanup()
{
   //
   // Release the state name string
   //
   _StateName.erase();

   // 
   // Cleanup memory owned by the lists, but
   //  we don't own the memory in these lists
   //  we just have pointers.
   //
   list< StateAbs* >::iterator state;
   for ( state = _SubstateList.begin() ;
         state != _SubstateList.end()  ;
         ++state )
   {
      delete (*state);
   }
   _SubstateList.clear( );
   list< MonitorBase* >::iterator monitor;
   for ( monitor = _MonitorsList.begin() ;
         monitor != _MonitorsList.end()  ;
         ++monitor )
   {
      delete (*monitor);
   }
   _MonitorsList.clear( );
}

