/*
* Copyright (C) 2007 Gambro BCT, Inc.  All rights reserved.
* $Header$ 
* 
*
* $Log: XMLMemeberType.h $
* Initial revision 
*
*/

/*! \file XMLMemeberType.h
*/
#ifndef XMLMemberType_Included
#define XMLMemberType_Included
#define TAB  "    "






#include <sstream>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
namespace BctXml
{
  class XMLMemberType;

  typedef std::vector<XMLMemberType*> XMLMemberTypeSet;
  typedef enum {ELEMENT, DATA, COMMENT, ERR } Type;


  /// \brief Base class for elements, character data and comments
  ///
  /// A XMLMemberType contains the common features of elements characters and
  /// comments. It is essential for polymorphically printing the outputs in 
  // the proper order.
  ///
  class XMLMemberType
  {
  public:
    XMLMemberType(void);
    virtual ~XMLMemberType(void);
    /// method for getting name only meaningful for Element
    virtual std::string getName()const=0;
    /// Method for accessing the string data of a particular data type
    virtual std::string getDataString()const =0;
    /// Method for setting the string data for a data type
    virtual int setDataString(const std::string& inValue)=0;
    /// purely virutal function that all derived classes must override
    /// so that they will print their information the proper way.
    virtual std::string print() const;
    /// Print function that a derived class may override if it has special
    /// print processing that depends on the the number of tabs.
    /// The default functionality is to just run the regular print command.
    virtual std::string print(int inTabCount) const;
    /// Puts the the element and all of its contents in valid XML format
    /// onto the supplied stream
    virtual void toStream(std::ostream &output) const =0 ;
    /// Puts the the element and all of its contents in valid XML format
    /// onto the supplied stream Tabs are processed if necessary
    virtual void toStream( int inTabCount, std::ostream &output) const;

    // This function trims leading and trailing whitespace from an input value
    // This is useful when determining if two values are equivalent.
    static std::string trim(const std::string& inString);
    bool isData(void);
    bool isElement(void);
    bool isComment(void);		

    //Equivalence for most member types is set if the name, Data string and type are the same
    // a noteable exception is Element. 
    virtual bool operator==(XMLMemberType& other) 
    {
      return((_dataType==other._dataType)&&(trim(other.getDataString())==trim(getDataString()))&&(trim(other.getName())==trim(getName())));
    }
    virtual bool operator!=(XMLMemberType &other) 
    {
      return !(*this==other);
    }
  protected:


    /// The type of member 
    Type _dataType;


  private:
    /// Private Copy Constructor to avoid accidental usage.
    XMLMemberType(const XMLMemberType&);
    /// Private assignment operator to avoid accidental usage.
    XMLMemberType& operator = (const XMLMemberType&);	
  };
}// End namespace BctXml
std::ostream& operator<<(std::ostream&s,const BctXml::XMLMemberType&e);
#endif //XMLMemberType_Included
