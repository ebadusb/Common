
#ifndef anAgentFileInterface_h
#define anAgentFileInterface_h 

#include <rope.h>

#include "anagentinterface.h"

class anAgentFileInterface : public anAgentInterface
{
  public:
   //## Constructors (generated)
   anAgentFileInterface();

   anAgentFileInterface(const anAgentFileInterface &right);

   //## Destructor (generated)
   ~anAgentFileInterface();

   //## Assignment Operation (generated)
   const anAgentFileInterface & operator=(const anAgentFileInterface &right);

   //## Equality Operations (generated)
   int operator==(const anAgentFileInterface &right) const;

   int operator!=(const anAgentFileInterface &right) const;

   // This method connects the anAgentInterface to the manager
   // and sets up the filename and pathname for the instance of the
   // object.
   AGENT_INTERFACE_RETURN Initialize(const char *filename, const char *path);

   // Set information methods.
   AGENT_INTERFACE_RETURN setInformation(const char *section, 
                                         const char *variable, 
                                         const char *value);
   AGENT_INTERFACE_RETURN setInformation(const char *section,
                                         const char *variable, 
                                         const float &value);
   AGENT_INTERFACE_RETURN setInformation(const char *section,
                                         const char *variable, 
                                         const int &value);

   // This method updates the file located at the path from the Initialize method.
   AGENT_INTERFACE_RETURN updateFile();
   AGENT_INTERFACE_RETURN updateFileFromAgent();

   // This method update the agent memory space.
   AGENT_INTERFACE_RETURN updateAgent();

  private:

  protected:
   virtual AGENT_INTERFACE_RETURN retrieveInformation (const char* section,
                                                       const char* variable,
  														             aSectionfile_element& return_value);
  private:
   // Additional Private Declarations
   crope _filename;
   crope _path;
   aSectionfile *_sectionfile;

  private:  //## implementation
    // Additional Implementation Declarations

};

// Class anAgentFileInterface 

#endif
