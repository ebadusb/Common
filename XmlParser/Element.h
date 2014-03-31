/*
* Copyright (C) 2007 Gambro BCT, Inc.  All rights reserved.
* $Header$ 
* 
*
* $Log: Element.h $
* Revision 1.4  2007/12/13 20:44:23Z  estausb
* Revision 1.2  2007/12/07 18:31:07Z  estausb
* Modifications for Prototype 1
* Revision 1.1  2007/11/15 20:42:44Z  estausb
* Initial revision
*
*/

/*! \file Element.h
*/


#ifndef Element_Included
#define Element_Included
#include "XMLMemberType.h"
#include "CharData.h"
#include <string>
#include <vector>
#include <sstream>
#include "Attribute.h"
#include "Mutex.h"

#include "Comment.h"

namespace BctXml
{

  class Element;

  typedef std::vector<Element*> ElementSet;

  /// \brief An element within an XML document.

  ///
  /// This represents an element within an XML document.  An Element
  /// can currently have attributes, child elements, comments, and data.  The data
  /// is represented as a collection of CharData objects.   If a document is created
  /// in this format, the element will have string data that is the concatanation of all character data
  /// found in the document, and the children elements added to the list.
  /// 
  /// NOTE: Objects of Element type OWN their children and must free memory for them.
  ///
  class Element:
    public XMLMemberType
  {
  public:
    Element(void);
    Element(const std::string& sName, Element* pParent = NULL);
    virtual ~Element();
    /// Adds a comment to this element
    void addComment( Comment* inComment);
    /// Get All comments
    CommentSet getComments();
    /// Adds an attribute to our set with the given name and value.  If the attribute already exists
    /// we will update its value.
    void addAttribute(const std::string& sAttName, const std::string& sAttVal);

    template<class TYPE>
    void addValueChildElement(const std::string& name, TYPE data)
    {
      Element* child = new Element(name);
      child->setValue(data);
      this->addChildElement(child);
    }

    /// Adds the given Element pointer to our children. We assume responsibility for
    /// managing our childrens memory.
    void addChildElement(Element* child);
    /// Adds character data to this element.  If there is alreardy data it will append
    /// the data to the end of our value.
    void addData(const std::string& sValue);

    /// Removes all children from our list, and deletes their memory.
    void deleteChildren();
    /// Removes all children with the given name and Attribute. This will only
    /// search our immidiate children.
    void deleteChildren(const std::string& sName,const Attribute &inAttribute);
    /// Removes all children with the given name and Attribute Set. This will only
    /// search our immidiate children.
    void deleteChildren(const std::string& sName,const AttributeSet &inAttributeSet);
    /// Removes all children with the given element name. This will only
    /// search our immidiate children.
    void deleteChildren(const std::string& sName);
    /// Finds the first child with the given name. NOTE: Do not delete the child.
    Element* findFirstChild(const std::string& sName);

    /// Finds the first child with the given name and Attribute Set.
    Element* findFirstChild(const std::string& sName, const AttributeSet &inAttributeSet);

    /// Finds the first child with the given name and Attribute. 
    Element* findFirstChild(const std::string& sName, const Attribute &inAttribute);
    /// Removes the first child with the given name and Attribute. This will only
    /// search our immidiate children.
    void deleteFirstChild(const std::string& sName,const Attribute &inAttribute);
    /// Removes the first child with the given name and Attribute Set. This will only
    /// search our immidiate children.
    void deleteFirstChild(const std::string& sName,const AttributeSet &inAttributeSet);
    /// Removes the first child with the given element name. This will only
    /// search our immidiate children.
    void deleteFirstChild(const std::string& sName);
    /// Returns true if this element has children
    bool hasChildren()const;


    ///
    ///  getImmediateChildren: returns all the child elements directly associated with an element as an element set
    ///  getAllChildren: returns all of the children, grandchildren, etc. in an element tree as an element set
    ///  findChildren: returns all of the immediate children of an element that have the supplied characteristics as an element set
    ///

    ///Returns all childern and sub children for an element
    ElementSet getAllChildren();
    ///Returns all Immediate children for an element
    ElementSet getImmediateChildren();
    /// Finds all children with the given name and Attribute. This will only
    /// search our immidiate children.
    ElementSet findChildren(const std::string& sName,const Attribute &inAttribute);
    /// Finds all children with the given name and Attribute Set. This will only
    /// search our immidiate children.
    ElementSet findChildren(const std::string& sName,const AttributeSet &inAttributeSet);
    /// Finds all children with the given element name. This will only
    /// search our immidiate children.
    ElementSet findChildren(const std::string& sName);

    /// Returns the value of the attribute requested.
    /// If the passed in attribute string doesn't exist within the
    /// element an empty string will be returned.
    std::string getAttributeValue(const std::string& sAttName) ;
    /// Returns the name of the element.
    std::string getName() const;
    /// returns a refernce to all the attributes
    AttributeSet getAllAttributes() const;
    /// Returns the character data contained within the element.  
    /// The data is represented as a concatanation of all character data.
    virtual std::string getDataString() const;

    /// Puts the the element and all of its contents in valid XML format
    /// onto the supplied stream
    virtual void toStream(std::ostream &output) const;
    /// Will return the parent element.  This may be null which indicates 
    /// the element is a root element.
    Element* getParent() const;

    /// Will return the root element of this element
    /// If this root element is the root elemenet will return null
    Element* getRootElement();
    /// Converts the data string to the passed in type (TYPE).  Passing in 
    /// the correct format flag is neccassary (std::ios_base::dec, hex etc)
    ///
    /// \return true If the data was successfully converted from the string.
    /// \return false If there was no conversion possible.
    bool getValue(std::string& t)
    {
      t = getDataString();
      return true;
    }
    template<class TYPE>
    bool getValue(TYPE& t, const std::ios::fmtflags& flags)
    {

      std::istringstream iss(getDataString());
      iss.setf(flags);
      return !(iss >> t).fail();
    }

    template<class TYPE>
    bool getValue(TYPE& t)
    {

      return getValue(t,std::ios::dec);
    }
    /// Looks for an attribute with the given name.
    /// \return true If an attribute with the given name exists.
    /// \return false If an attribute with the given name DOES NOT exist.
    bool hasAttribute(const std::string& sAttName) ;
    /// Looks for an attribute with same name AND same value
    bool hasAttribute(const Attribute &inAttribute);

    /// If we have the given attribute update its value, otherwise return false.
    bool updateAttribute(const std::string& sAttName, const std::string& sAttVal);
    /// Converts the data string to the passed in type (TYPE).  Passing in 
    /// the correct format flag is neccassary (std::ios_base::dec, hex etc)
    ///
    /// \return true If the data was successfully converted to the string.
    /// \return false If there was no conversion possible.
    template<class TYPE>
    bool setValue(const TYPE& t)
    {
      bool output=true;
      std::ostringstream oss;
      oss.setf(std::ios::dec);
      output=(!( oss<<t).fail());
      setDataString(oss.str());
      setDirty();
      return output;
    }
    //template<class TYPE>
    //  void operator <<(TYPE& inValue)
    // {
    //   setValue(inValue);
    // }

    void setDirty();
    bool isDirty();
    void setClean();

    /// Merges this element with a default element.
    /// This will add data wherever the default has data but this does not.
    void merge(Element* inElement, bool ignoreAttrInNameCompare = false);
    /// Merges only the attributes for a given XML element
    /// Does not iterate through XML element's children like merge() does
    void mergeAttributes(Element *inElement);

    /// removes all attributes from the current element
    void deleteAllAttributes();
    /// removes only the attribute named from the current element
    void deleteAttribute(const std::string &sAttributeName);

    /// creates a deep recursive copy of an element
    Element* createCopy(Element* inParent=NULL);

    /// Allows setting and clearing the "replace" attribute to override factory defaults etc
    void setOverrideAttributeFlag(Element *inElement);
    void clearOverrideAttributeFlag(Element *inElement);
    void deleteOverrideAttributeFlag(Element *inElement);

    // Two elements are equivalent if they have the same name, the same
    // data string, all of the same attributes with each attribute having the 
    // same value and it has all the same children elements;
    // note: comments do not effect the equivalency of elements
    bool operator==(Element &other); 

    bool operator!=(Element &other) 
    {
      return !(*this==other);
    }
  private:
    ///Returns all children and sub children for a given elementset
    static ElementSet Element::getAllChildren(ElementSet *input);

    /// Determines if we already have ownership of the element passed in
    bool isChildElement(const Element* child) const;
    /// private assignment operator  to avoid accidental usage.
    Element& operator = (const Element&);

    enum ProcessingType { FindFirst, FindAll, DeleteFirst, DeleteAll };
    /// search our immediate or all children. Destroy if desired
    ElementSet processMatchingChildren(const std::string& sName,const AttributeSet &inAttributeSet,ProcessingType type);
    /// Private Copy Constructor to avoid accidental usage.
    Element(const Element&);
    /// Method for setting the string data for a data type
    int setDataString(const std::string& inValue);
    mutable Bct::Mutex _mutexMember;
    mutable Bct::Mutex _mutexAttribute;

    /// Returns True if current inElement has attribute: "replace" with value "true"
    bool checkForReplaceAttribute(Element* inElement, bool nameCheckOnly);
    /// Adds children to an element from another element (used during a replace merge)
    void addChildren(Element* curElement, Element* inElement);

    Element* _pParent;
    std::string _sName;
    bool _dirty;
    bool _attributeMisMatch;
    bool isNameMatch(const Element* inElement, bool ignoreAttrInNameCompare);

    /// Data, children elements, and comments all go in this set in order of appearance.
    /// This is used to display all the various members in the proper order.
    XMLMemberTypeSet _vMembers;

    AttributeSet _attributes;

  };
} // End namespace CES


#endif //Element_Included
