/*
* Copyright (C) 2007 Gambro BCT, Inc.  All rights reserved.
* $Header$ 
* 
*
* $Log: Comment.h $
* Initial revision 
*
*/

/*! \file Comment.h
*/
#ifndef Comment_Included
#define Comment_Included

#include <string>
#include <vector>

#include "XMLMemberType.h"
namespace BctXml
{
  /// \brief An XML comment

  ///
  /// A comment is associated with an Xml element . 
  /// NOTE: Multi line comments are not supported.
  ///
  class Comment;

  typedef std::vector<Comment*> CommentSet;

  class Comment:
    public XMLMemberType
  {
  public:
    Comment(void);

    Comment(const std::string& inValue);
  public:
    virtual ~Comment(void);

    /// Method for accessing the string data of a particular data type
    virtual std::string getDataString() const;
    /// Method for setting the string data for a data type
    int setDataString(const std::string& inValue);
    /// Puts the the element and all of its contents in valid XML format
    /// onto the supplied stream
    virtual void  toStream(std::ostream &output) const;
    /// Puts the the element and all of its contents in valid XML format
    /// onto the supplied stream with leading tabs equal to 
    ///  input parameter inTabCounttab
    virtual void toStream(int inTabCount, std::ostream &output) const;
    std::string getName()const;

    // These operators need to be added because VxWorks will not compile if I just use the operator
    // inherited from XMLMemberType
    virtual bool operator==(Comment& other) 
    {
      return((trim(other.getDataString())==trim(getDataString()))&&(trim(other.getName())==trim(getName())));
    }
    virtual bool operator!=(Comment &other) 
    {
      return !(*this==other);
    }
  private: 
    /// Private Copy Constructor to avoid accidental usage.
    Comment(const Comment&);
    /// Private assignment operator to avoid accidental usage.
    Comment& operator = (const Comment&);	
    std::string _sValue;
  };

}// End namespace BctXml
#endif //Comment_Included

