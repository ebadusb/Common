/*
* Copyright (C) 2007 Gambro BCT, Inc.  All rights reserved.
* $Header$ 
* 
*
* $Log: FileParser.h $
* Revision 1.3  2007/12/13 20:44:23Z  estausb
* Revision 1.1  2007/11/15 20:42:43Z  estausb
* Initial revision
*
*/

/*! \file FileParser.h
*/


#ifndef FileParser_Included
#define FileParser_Included

#include "XMLParser.h"
#include "Comment.h"
#include <string>
#include <vector>

namespace BctXml
{

  class Element;
  /// \brief

  ///
  /// Detailed Description Here
  ///
  class FileParser : public XmlParser
  {
  public:
    ///Default Constructor
    FileParser();
    virtual ~FileParser();

    /// Parses the document passed in and returns the root element
    /// of that document.  If the root element is NULL then the 
    /// document was not well formed.
    /// NOTE: The element passed back (and all of its children) are now
    /// the responsibility of the caller to clean up.
    Element* parseFile(const std::string& sFileName);

    bool documentHasDocDecl()const;
    bool documentIsStandAlone()const;
    std::string documentVersion()const;
    std::string documentEncoding()const;

  protected:
    virtual void onStartElement(const XML_Char* name, const XML_Char** attrs);
    virtual void onEndElement(const XML_Char* name);
    virtual void onXmlDecl(const XML_Char* version, const XML_Char* encoding, bool isStandalone);
    virtual void onCharacterData(const XML_Char* data, int len);
    virtual void onComment(const XML_Char* data);

  private:

    /// Private Copy Constructor to avoid accidental usage.
    FileParser(const FileParser&);
    /// Private assignment operator to avoid accidental usage.
    FileParser& operator = (const FileParser&);
    CommentSet _tempCommentStorage;
    void attachAllCommentsToElement( Element* inElement);
    void destroyAllComments( );
    Element* _currentElement; ///< The current node that we are parsing
    Element* _rootElement;

    bool _documentHasDocDecl;
    std::string _documentVersion;
    std::string _documentEncoding;
    bool _documentIsStandAlone;
  };
} // End namespace CES
#endif //FileParser_Included
