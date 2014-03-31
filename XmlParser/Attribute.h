/*
* Copyright (C) 2007 Gambro BCT, Inc.  All rights reserved.
* $Header$ 
* 
*
* $Log: Attribute.h $
* Revision 1.2  2007/12/07 18:31:07Z  estausb
* Modifications for Prototype 1
* Revision 1.1  2007/11/15 20:42:43Z  estausb
* Initial revision
*
*/

/*! \file Attribute.h
*/


#ifndef Attribute_Included
#define Attribute_Included

#include <string>
#include <vector>

namespace BctXml
{
  /// \brief An XML attribute

  ///
  /// An attribute is part of an Xml element that appears within the start tag.  They
  /// have names, and values. A user of the Attribute class can change an attributes value, 
  /// however once an attribute is created, the name cannot be changed.
  ///
  class Attribute
  {
  public:
    /// Constructor
    Attribute(const std::string& name, 
      const std::string& sVal);
    /// Constructor that will set the value to a null string with the given name.
    /// This can be useful in the case where there needs to be a two step construction.
    Attribute(const std::string& name);
    virtual ~Attribute();

    void changeValue(const std::string& newValue);
    std::string getName() const;
    std::string getValue() const;
    bool operator==(const Attribute &other) const
    {
      return((other.getValue()==getValue())&&(other.getName()==getName()));
    }
    bool operator!=(const Attribute &other) const
    {
      return !(*this==other);
    }
  private:
    std::string _name;
    std::string _val;
  };
  typedef std::vector<Attribute> AttributeSet;
} // End namespace BctXml
#endif //Attribute_Included
