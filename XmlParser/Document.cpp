/*
* Copyright (c) 2007 by Gambro BCT, Inc.  All rights reserved.
* $Log: Document.cpp $
* Revision 1.4  2007/12/13 20:44:20Z  estausb
* Revision 1.2  2007/12/07 18:31:07Z  estausb
* Modifications for Prototype 1
* Revision 1.1  2007/11/15 20:42:43Z  estausb
* Initial revision
*
*/

/*! \file Document.cpp
*/

#include "Document.h"
#include "FileParser.h"
#include "Element.h"
#include <fstream>
#include <iostream>

using namespace BctXml;
const std::string Document::_backupExtension=".backup";
Document::Document(const std::string& sFile) : _fileName(sFile),
_isStandAlone(false),
_fileMissing(false),
_fileUnReadable(false),
_hasDocDecl(false),
_version(""),
_encoding(""),
_pRootElement(NULL)
{
}

Document::~Document()
{
  // Cleanup the docuemnt pointers.
  clearDoc();
}

void Document::clearDoc()
{
  if(_pRootElement)
  {
    delete _pRootElement;
    _pRootElement = NULL;
  }
}

std::string Document::getFileName() const
{
  return _fileName;
}
void Document::setRootElement(Element* inRoot)
{
  // destroy any old doc that this document may contain
  clearDoc();
  _pRootElement=inRoot;
  _pRootElement->setDirty();

}

void Document::writeBackupFile(const std::string& sFileName)
{
  std::ifstream in(sFileName.c_str());  // Use binary mode so we can
  std::ofstream out((sFileName+_backupExtension).c_str()); // content.
  // Make sure the streams opened okay...

  char buf[2048];
  if(!in.eof())
  {   
    do 
    {
      if(!in.eof())
      {
        in.read(&buf[0], 2048);      // Read at most n bytes into
        out.write(&buf[0], in.gcount( )); // buf, then write the buf to
      }
    } while (in.gcount( ) > 0 && !in.eof());          // the output.
  }
  // Check streams for problems...
  in.close( );
  out.close( );
}
bool Document::read()
{
  FileParser parse;
  // NOTE We now own the root node and must cleanup the memory.
  _pRootElement = parse.parseFile(_fileName);
  _hasDocDecl=parse.documentHasDocDecl();
  if(_hasDocDecl){
    _version=parse.documentVersion();
    _encoding=parse.documentEncoding();
    _isStandAlone=parse.documentIsStandAlone();
  }
  XML_Error err = parse.getErrorCode();
  if(err!=0)
  {
    // if error then the file is just corrupt
    _fileUnReadable=true;
    std::cout << "Error Parsing:" << parse.getErrorString(err) << std::endl;

  }
  else if(_pRootElement== NULL)
  {
    // if no error but no root then the file is missing
    _fileUnReadable=true;
    _fileMissing=true;
    
  }
  bool bReadSuccess=false;
  if(!err &&_pRootElement!= NULL)
    bReadSuccess = true;
  else 
    bReadSuccess = false;
  return bReadSuccess;
}
bool Document::isFileMissing()const
{
  return _fileMissing;
}
bool Document::isFileUnreadable()const
{
  return _fileUnReadable;
}
Element* Document::getRootElement() const
{

  return _pRootElement;
}

void Document::write()
{
  if(_pRootElement->isDirty())
    write(_fileName);
}

void Document::write(const std::string& sFileName) 
{	

  // if the original file is un readable then back it up before you
  // overwrite it.  
  if(_fileUnReadable&&(!_fileMissing))
  {
    writeBackupFile(sFileName);
    _fileUnReadable=false;
  }

  std::ofstream file_op(sFileName.c_str(),std::ios::out);
  file_op<<print();
  file_op.close();
  _pRootElement->setClean();

}
void Document::setDirty()
{
  _pRootElement->setDirty();
}
bool Document::isDirty()
{
  return _pRootElement->isDirty();

}
std::string Document::print()const
{
  std::ostringstream oStr;
  toStream(oStr);
  return oStr.str();

}
void Document::toStream(std::ostream &output)const
{
  if(_hasDocDecl){
    if(_isStandAlone)
      output<<"<?xml version=\""<<_version<<"\" encoding=\""<<_encoding<<"\" ?>";
    else
      output<<"<?xml version=\""<<_version<<"\" encoding=\""<<_encoding<<"\" standalone=\"no\" ?>";
  }
  if(!(_pRootElement==NULL))
    getRootElement()->toStream(output);


}

std::ostream& operator<<(std::ostream&s,BctXml::Document&d)
{
  d.toStream(s);
  return s;
}
