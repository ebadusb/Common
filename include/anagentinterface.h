
#ifndef anAgentInterface_h
#define anAgentInterface_h 1

#include <Rope.h>
#include <Set.h>
#include <sys/stat.h>

#include "ap2agent.h"
#include "aSectionfile_element.hpp"
#include "aSectionfile.hpp"
#include "agent_sem.h"

#ifndef RUN_WINDOWS
#include "buffmsg.hpp"
#else
class AgentSemaphore;
#endif

class aSection_and_CRC;

class aSection_and_CRC
{
   public:
      aSection_and_CRC() 
      {
         sectionfile = NULL;
      };

      aSection_and_CRC(const aSection_and_CRC &orig)
      {
         name = orig.name;
         CRC = orig.CRC;
         sectionfile = orig.sectionfile;
      };

      aSection_and_CRC &operator=(const aSection_and_CRC &orig)
      {
         name = orig.name;
         CRC = orig.CRC;
         sectionfile = orig.sectionfile;
         return *this;
      };

      int operator==(const aSection_and_CRC &orig) const
      {
         // Only check the name for comparison.
         // This is done because the name is the defining characteristic.
         return (name == orig.name);
      };      

	   int operator<(const aSection_and_CRC &orig) const
      {
         return (name < orig.name);
      };

      crope name;
      crope CRC;
      aSectionfile *sectionfile;
};

class aSection_and_CRC_compare
{  
	public:

	int operator()(aSection_and_CRC s1, aSection_and_CRC s2) const
	{
      return (s1.name < s2.name);
   }
};

//## Class: anAgentInterface%3592C72B0301; Abstract
//	This is the general interface functions for an Agent.
//	It is VIRTUAL in that dataChanged() is a virtual
//	function and should be implemented in an inherited class
//	in order to get notification that data in a section has
//	changed.
//## Category: <Top Level>
//## Subsystem: <Top Level>
//## Persistence: Transient
//## Cardinality/Multiplicity: n
#ifndef RUN_WINDOWS
class anAgentInterface : public focusBufferMsg<AP2_Section_Name_Struct>
#else
class anAgentInterface
#endif
{
  public:
    //## Constructors (generated)
#ifndef RUN_WINDOWS
	  anAgentInterface() : focusBufferMsg<AP2_Section_Name_Struct>(AN2Test3),
						   agent_semaphore(0) {};
#else
	  anAgentInterface() : agent_semaphore(0) {};
#endif

      anAgentInterface(const anAgentInterface &right);

    //## Destructor (generated)
      ~anAgentInterface();

    //## Assignment Operation (generated)
      const anAgentInterface & operator=(const anAgentInterface &right);

    //## Equality Operations (generated)
      int operator==(const anAgentInterface &right) const;

      int operator!=(const anAgentInterface &right) const;

    //## Other Operations (specified)
      //## Operation: Initialize%901051816
      //	This method connects the instance to the Agent interface
      //	manager.  It also connects the instance properly to the
      //	common information storage area so that data may be
      //	retrieved from the Agent.  Upon proper completion of
      //	this task AGENT_OK is returned.
      AGENT_INTERFACE_RETURN Initialize (const int semaphore_already_locked = 0);

	  AGENT_INTERFACE_RETURN getInformation (const char* section, 
		                                      const char* variable, 
											           aSectionfile_element& return_value);
      
	  //## Operation: getInformation%901051813
      //	This method should be called by the user to get the
      //	requested information described uniquely by section and
      //	name.  The returned character string assumes that a
      //	buffer of the size described has been allocated for the
      //	method call.
      AGENT_INTERFACE_RETURN getInformation (const char* section,
		                                       const char* variable, 
											            char* return_value, 
											            const int return_value_buffer_length);

      //## Operation: getInformation%901051814
      //	This method should be called by the user to get the
      //	requested information described uniquely by section and
      //	name.  The values is returned in the reference passed as
      //	value.
      AGENT_INTERFACE_RETURN getInformation (const char* section,
		                                       const char* variable,
											            float& return_value_float,
											            const float min = 0, 
                                             const float max = 0);

      //## Operation: getInformation%901051815
      //	This method should be called by the user to get the
      //	requested information described uniquely by section and
      //	name.  The values is returned in the reference passed as
      //	value.
      AGENT_INTERFACE_RETURN getInformation (const char* section,
		                                       const char* variable,
											            int& return_value,
                                             const int min = 0,
                                             const int max = 0);

      //## Operation: dataChanged%901051817
      //	This method is called by the sectionUpdate method when
      //	information in a section which is currently being used
      //	is updated. The method will be called anytime data in a
      //	section you have used in a sectionExists or get
      //	Information call has changed.
      virtual void dataChanged (const char* section);

      // This is called when the current update of all sections is completed.
      virtual void dataChangedComplete();

      //## Operation: sectionExists%901051812
      //	This method should be called by the user to determine if
      //	a section currently has data/exists in the AP2 Agent.
      int sectionExists (const char* section = NULL);

    // Additional Public Declarations
      //## begin anAgentInterface%3592C72B0301.public preserve=yes
      
      //## Operation: compareSectionCRC works like string compare.
      int compareSectionCRC(const char *section, const char *section_crc);

      //## end anAgentInterface%3592C72B0301.public


  protected:

    //## Other Operations (specified)
      //## Operation: notify%901926112
      //	This method is implemented to overload the focusBuffer
      //	Msg notify.  This will allow all agents to be notifyed
      //	when data is changed.  The method will retrieve using
      //	the get method from the focusBufferMsg class the section
      //	name which has changed.  It will then call section
      //	Updated with this information.
      void notify ();

      //## Operation: sectionUpdated%901051821
      //	This method is called in response to receiving a notify
      //	from the Agent interface manager that information in the
      //	AP2 Agent has changed.  The method will then process all
      //	requests made by this instance and decide if it should
      //	call the dataChanged method to inform the user of the
      //	instance that new information is available.
      void sectionUpdated (const char* section);

      //## Operation: retrieveInformation%901051828
      //	This method is used by the getInformation calls to
      //	retreive the requested information from the AP2 Agent
      //	and returns the value in the container provided.  The
      //	method is pure virtual and resolves based on object
      //	calling type.
      virtual AGENT_INTERFACE_RETURN retrieveInformation (const char* section,
		                                                    const char* variable,
														                aSectionfile_element& return_value) = 0;

      //## Operation: retrieveInformation%901051819
      //	This method is used by other retreiveInformation methods
      //	to actually retreive the information from the AP2
      //	agent.
      AGENT_INTERFACE_RETURN retrieveInformation (const char* filename,
		                                            const char *path,
												              const char* section,
												              const char* variable,
												              aSectionfile_element& return_value);

    // Additional Protected Declarations
    AgentSemaphore *agent_semaphore;

    // Additional Private Declarations
	  __STD::set<aSection_and_CRC, aSection_and_CRC_compare> _current_sections;

  private:  //## implementation
    // Additional Implementation Declarations

};

// Class anAgentInterface 

#endif
