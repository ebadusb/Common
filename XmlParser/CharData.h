/*
* Copyright (C) 2007 Gambro BCT, Inc.  All rights reserved.
* $Header$ 
* 
*
* $Log: CharData.h $
* Initial revision 
*
*/

/*! \file CharData.h
*/
#ifndef CharData_Included
#define CharData_Included
#include "xmlmembertype.h"
#include <string>
#include <vector>
namespace BctXml
{
  /// \brief Data associated with an element
  ///
  /// A CharData object is an idividual piece of data. One or more of these objects
  /// are used to hold the value of an element. Multiple CharData objects are  
  /// used for a single element in the event that a comment or another element
  /// is embedded in the orignal task.  
  ///
  class CharData;

  typedef std::vector<CharData*> CharDataSet;
  class CharData :
    public XMLMemberType
  {
  public:
    CharData(void);
    CharData(const std::string& inValue);
    virtual ~CharData(void);
    /// Method for accessing the string data of a particular data type
    virtual std::string getDataString() const;
    /// Method for setting the string data for a data type
    int setDataString(const std::string& inValue);
    /// Puts the the element and all of its contents in valid XML format
    /// onto the supplied stream
    virtual void toStream(std::ostream &output) const;

    std::string getName()const;

    // These operators need to be added because VxWorks will not compile if I just use the operator
    // inherited from XMLMemberType
    virtual bool operator==(CharData& other) 
    {
      return((trim(other.getDataString())==trim(getDataString()))&&(trim(other.getName())==trim(getName())));
    }
    virtual bool operator!=(CharData &other) 
    {
      return !(*this==other);
    }
  private:
    /// Private Copy Constructor to avoid accidental usage.
    CharData(const CharData&);
    /// Private assignment operator to avoid accidental usage.
    CharData& operator = (const CharData&);
    std::string _sValue;
  };
}// End namespace BctXml
#endif //CharData_Included
