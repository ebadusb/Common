/*
* Copyright (c) 2007 by Gambro BCT, Inc.  All rights reserved.
* $Log: FileParser.cpp $
* Revision 1.3  2007/12/13 17:35:34Z  estausb
* Revision 1.2  2007/12/07 18:31:07Z  estausb
* Modifications for Prototype 1
* Revision 1.1  2007/11/15 20:42:43Z  estausb
* Initial revision
*
*/

/*! \file FileParser.cpp
*/

#include "FileParser.h"

#include <fstream>
#include <iostream>

#include "Element.h"

using namespace BctXml;
using namespace std;

FileParser::FileParser():  _currentElement(NULL),_documentHasDocDecl(false),_rootElement(NULL),_documentIsStandAlone(true),_documentVersion(""),_documentEncoding("")
{
  create();
}

FileParser::~FileParser()
{
  enableElementHandler(false);
  enableCharacterDataHandler(false);
  destroyAllComments();
  destroy();
}

Element* FileParser::parseFile(const std::string& sFileName)
{
  destroyAllComments();
  bool parserCreatedProperly = create();

  _currentElement = NULL;
  _documentHasDocDecl=false;
  _rootElement = NULL;

  enableElementHandler();
  enableCharacterDataHandler();
  enableXmlDeclHandler();
  enableCommentHandler();

  ifstream stream;
  stream.open(sFileName.c_str());

  if(parserCreatedProperly)
  {
    int lineNumber=1;
    char ch;
    while(!stream.eof() && stream.is_open())
    { 
      // Parse one line at a time making sure that we don't pass
      // eof to the parser
      std::string str;
      do
      {
        ch = stream.get();
        if(!stream.eof())
          str += ch;
      }
      while(ch != '\n' && !stream.eof());

      if(!parse(str.c_str(),static_cast<int>(str.size()), stream.eof()))
        std::cout << "Error Parsing Line:"<<str.c_str()<<" at Line #: "<<lineNumber<< std::endl;
      lineNumber++;
    }
  }
  return _rootElement;
}

void FileParser::onStartElement(const XML_Char* name, const XML_Char** attrs)
{
  Element* newElement = new Element(name, _currentElement);

  if(_currentElement != NULL && newElement != NULL)
  {
    _currentElement->addChildElement(newElement);
  }
  else
  {
    // This is the root Element, so hang on to the reference
    _rootElement = newElement;
  }
  // Populate the atrributes of this Element
  for(int i = 0; attrs[i]; i += 2)
  {
    newElement->addAttribute(attrs[i], attrs[i+1]);
  }
  _currentElement = newElement;

}

void FileParser::onEndElement(const XML_Char* name)
{
  if(_currentElement != NULL)
  {
    _currentElement = _currentElement->getParent();


  }

}
bool FileParser::documentHasDocDecl()const
{
  return _documentHasDocDecl;
}
bool FileParser::documentIsStandAlone()const
{
  return _documentIsStandAlone;
}
std::string FileParser::documentVersion()const
{
  return _documentVersion;
}
std::string FileParser::documentEncoding()const
{
  return _documentEncoding;
}
void FileParser::onXmlDecl(const XML_Char* version, const XML_Char* encoding, bool isStandalone)
{
  _documentHasDocDecl=true;
  _documentVersion=version;
  _documentEncoding=encoding;
  _documentIsStandAlone=isStandalone;

}

void FileParser::onComment(const XML_Char* data)
{
  std::string sData;
  sData=data;
  Comment*  theData= new Comment(data);
  if(_currentElement)
  {
    attachAllCommentsToElement(_currentElement);
    _currentElement->addComment(theData);
  }
  else
  {
    _tempCommentStorage.push_back(theData);
  }


}

void FileParser::destroyAllComments()
{
  std::vector<Comment*>::iterator it =_tempCommentStorage.begin();
  Comment* theData=NULL;
  while(it !=_tempCommentStorage.end())
  {
    // get next data element
    theData = *it;

    // delete for vector
    _tempCommentStorage.erase(it);
    if(theData)
      delete theData;
    // go to new begining
    it =_tempCommentStorage.begin();

  }
}
void FileParser::attachAllCommentsToElement( Element* inElement)
{
  std::vector<Comment*>::iterator it =_tempCommentStorage.begin();
  Comment* theData;
  while(it !=_tempCommentStorage.end())
  {
    // get next data element
    theData = *it;
    // add to desired element
    inElement->addComment(theData);
    // delete for vector
    _tempCommentStorage.erase(it);
    // go to new begining
    it =_tempCommentStorage.begin();

  }
}
void FileParser::onCharacterData(const XML_Char* data, int nLen)
{
  std::string sToAdd;
  for(int i = 0; i < nLen; i++)
    sToAdd += data[i];

  if(_currentElement)
    _currentElement->addData(sToAdd);

}
