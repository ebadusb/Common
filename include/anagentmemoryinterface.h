#ifndef anAgentMemoryInterface_h
#define anAgentMemoryInterface_h 1

// anAgentInterface
#include "anAgentInterface.h"

//## Class: anAgentMemoryInterface%3589669D0355
//	This class is ABSTRACT (it does not implement data
//	Changed()).  It allows users to request information from
//	AP2 Agent.  It connects to the AP2 Agent and will verify
//	all information to be consistent with other information
//	in that catagory.  That is, information within a
//	catagory is guaranteed to be consistent with other
//	information from that catagory if it is obtained from
//	the same instance of this object.
//## Category: <Top Level>
//## Subsystem: <Top Level>
//## Persistence: Transient
//## Cardinality/Multiplicity: n

class anAgentMemoryInterface : public anAgentInterface 
{

  public:
    //## Constructors (generated)
      anAgentMemoryInterface() {};

      // anAgentMemoryInterface(const anAgentMemoryInterface &right);

    //## Destructor (generated)
      ~anAgentMemoryInterface() {};

    //## Assignment Operation (generated)
      // const anAgentMemoryInterface & operator=(const anAgentMemoryInterface &right);

    //## Equality Operations (generated)
      // int operator==(const anAgentMemoryInterface &right) const;

      // int operator!=(const anAgentMemoryInterface &right) const;


    //## Other Operations (specified)
      //## Operation: Initialize%901051824
      //	This method calls AddRef and connects the instance to
      //	the Agent interface manager.  It also connects the
      //	instance properly to the common information storage area
      //	so that either AP2 or config data may be retreived from
      //	the Agent.
      AGENT_INTERFACE_RETURN Initialize (const int semaphore_alread_locked = 0);

      //## Operation: Commit%901051822
      //	This method is used to Commit a catagory/name pair.
      //	When the user  of this class decides that this
      //	information can not change from now on, it should commit
      //	this information so that others know that this data may
      //	not change.  Once ANY catagory/name field in a unique
      //	catagory is commited no other value in that catagory may
      //	change.  Any rejection of this catagory after a commit
      //	has occured will cause a non-recoverable software error.
      AGENT_INTERFACE_RETURN Commit (const char* section);

      //## Operation: Reject%901051823
      //	This method rejects any catagory/name pair.  It will
      //	cause the AP2 Agent to request new information for this
      //	catagory.  When new data is available the dataChanged
      //	method will be called.
      AGENT_INTERFACE_RETURN Reject (const char* section);

    // Additional Public Declarations

  protected:

    //## Other Operations (specified)
      //## Operation: retrieveInformation%901051826
      //	This method is used by the getInformation calls to
      //	retreive the requested information from the AP2 Agent
      //	and returns the value in the container provided. This
      //	version of retreive information validates that the
      //	information is consistent with other information in the
      //	AP2 Agent.  It retreives information from the
      //	configuration file only if it is not available in the
      //	downloaded AP2 data.
      AGENT_INTERFACE_RETURN retrieveInformation (const char* section,
                                                  const char* variable,
                                                  aSectionfile_element& return_value);

    // Additional Protected Declarations

  private:
    // Additional Private Declarations

  private:  //## implementation
    // Additional Implementation Declarations

};

// Class anAgentMemoryInterface 

#endif
