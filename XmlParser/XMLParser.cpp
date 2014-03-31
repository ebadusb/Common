/*
* Copyright (c) 2007 by Gambro BCT, Inc.  All rights reserved.
* $Log: XMLParser.cpp $
* Revision 1.1  2007/11/15 20:42:44Z  estausb
* Initial revision
*
*/

/*! \file XmlParser.cpp
*/

#include "XmlParser.h"
#include <iostream>
#include <cstring>

XmlParser::XmlParser():_parser(NULL)
{

}

XmlParser::~XmlParser()
{
  destroy();
}

bool XmlParser::create(const XML_Char *pszEncoding,const XML_Char *pszSep)
{  
  destroy ();
  //
  // If the encoding or seperator are empty, then NULL
  //
  if (pszEncoding != NULL && pszEncoding [0] == 0)
    pszEncoding = NULL;
  if (pszSep != NULL && pszSep [0] == 0)
    pszSep = NULL;

  _parser = XML_ParserCreate_MM (pszEncoding, NULL, pszSep);
  if (_parser == NULL)
    return false;
  //
  // Set the user data used in callbacks
  //
  XML_SetUserData(_parser,(void*) this);
  return true;
}

void XmlParser::destroy ()
{
  if (_parser != NULL)
    XML_ParserFree(_parser);
  _parser = NULL;
}

bool XmlParser::parse(const char *pszBuffer, int nLength, bool fIsFinal)
{
  XML_Status stat = XML_STATUS_ERROR;
  // Get the length if not specified
  // NOTE: This assumes a null terminated string
  if (nLength <= 0)
    nLength = static_cast<int>(strlen(pszBuffer));

  if(_parser != NULL)
  {
    stat =  XML_Parse(_parser, pszBuffer, nLength, fIsFinal);

    if(stat == XML_STATUS_ERROR)
    {
      XML_Error err = XML_GetErrorCode(_parser);  
      std::cout << "Parsing Error: " << getErrorString() << std::endl;
    }
  }
  else
  {
    stat = XML_STATUS_ERROR;
    std::cout << "Parser is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;

  }
  return (XML_STATUS_OK == stat);
}

bool XmlParser::parseBuffer (int nLength, bool fIsFinal)
{
  if(_parser != NULL)
    return XML_ParseBuffer(_parser, nLength, fIsFinal) != 0;
  else return false;
}

void* XmlParser::getBuffer(int nLength)
{
  if(_parser!= NULL)
    return XML_GetBuffer(_parser, nLength);
  else
  {
    std::cout << "Parser is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
    return NULL;
  }
}

void XmlParser::enableStartElementHandler(bool bEnable)
{
  if(_parser)
    XML_SetStartElementHandler(_parser, bEnable ? startElementHandler : NULL);
  else
    std::cout << "Parser is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}

void XmlParser::enableEndElementHandler (bool bEnable)
{
  if(_parser)
    XML_SetEndElementHandler(_parser, bEnable ? endElementHandler : NULL);
  else
    std::cout << "Parser is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}
void XmlParser::enableElementHandler (bool bEnable)
{
  enableStartElementHandler(bEnable);
  enableEndElementHandler(bEnable);
}
void XmlParser::enableCharacterDataHandler (bool bEnable)
{
  if(_parser)
    XML_SetCharacterDataHandler(_parser, bEnable ? characterDataHandler : NULL);
  else
    std::cout << "Parser is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}
void XmlParser::enableProcessingInstructionHandler (bool bEnable)
{
  if(_parser)
    XML_SetProcessingInstructionHandler(_parser, bEnable ? processingInstructionHandler : NULL);
  else
    std::cout << "Parser is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}
void XmlParser::enableCommentHandler (bool bEnable)
{
  if(_parser)
    XML_SetCommentHandler(_parser, bEnable ? commentHandler : NULL);
  else
    std::cout << "Parser is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}
void XmlParser::enableStartCDataSectionHandler(bool bEnable)
{
  if(_parser)
    XML_SetStartCdataSectionHandler(_parser, bEnable ? startCdataSectionHandler : NULL);
  else
    std::cout << "Parser is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}
void XmlParser::enableEndCDataSectionHandler(bool bEnable)
{
  if(_parser)
    XML_SetEndCdataSectionHandler(_parser, bEnable ? endCdataSectionHandler : NULL);
  else
    std::cout << "Parser is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}
void XmlParser::enableCDataSectionHandler(bool bEnable)
{
  enableStartCDataSectionHandler(bEnable);
  enableEndCDataSectionHandler(bEnable);
}
void XmlParser::enableDefaultHandler(bool bEnable)
{
  if(_parser)
    XML_SetDefaultHandler(_parser, bEnable ? defaultHandler : NULL);
  else
     std::cout << "Parser is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}

void XmlParser::enableExternalEntityRefHandler (bool bEnable)
{
  if(_parser)
    XML_SetExternalEntityRefHandler(_parser, bEnable ? externalEntityRefHandler : NULL);
  else
    std::cout << "Parser is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}


void XmlParser::enableStartNamespaceDeclHandler(bool bEnable)
{
  if(_parser)
    XML_SetStartNamespaceDeclHandler(_parser, bEnable ? startNamespaceDeclHandler : NULL);
  else
    std::cout << "Parser is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}
void XmlParser::enableEndNamespaceDeclHandler (bool bEnable)
{
  if(_parser)
    XML_SetEndNamespaceDeclHandler(_parser, bEnable ? endNamespaceDeclHandler : NULL);
  else
    std::cout << "Parser is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}
void XmlParser::enableNamespaceDeclHandler (bool bEnable)
{
  enableStartNamespaceDeclHandler(bEnable);
  enableEndNamespaceDeclHandler(bEnable);
}
void XmlParser::enableXmlDeclHandler (bool bEnable)
{
  if(_parser)
    XML_SetXmlDeclHandler(_parser, bEnable ? xmlDeclHandler : NULL);
  else
    std::cout << "Parser is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}
void XmlParser::enableStartDoctypeDeclHandler (bool bEnable)
{
  if(_parser)
    XML_SetStartDoctypeDeclHandler(_parser, bEnable ? startDoctypeDeclHandler : NULL);
  else
    std::cout << "Parser is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}
void XmlParser::enableEndDoctypeDeclHandler (bool bEnable)
{
  if(_parser)
    XML_SetEndDoctypeDeclHandler(_parser, bEnable ? endDoctypeDeclHandler : NULL);
  else
    std::cout << "Parser is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}
void XmlParser::enableDoctypeDeclHandler (bool bEnable)
{
  enableStartDoctypeDeclHandler(bEnable);
  enableEndDoctypeDeclHandler(bEnable);
}

enum XML_Error XmlParser::getErrorCode () 
{
  return XML_GetErrorCode(_parser);
}

const XML_LChar* XmlParser::getErrorString()
{
  return XML_ErrorString(XML_GetErrorCode(_parser));
}

long XmlParser::getCurrentByteIndex(void)
{
  return XML_GetCurrentByteCount(_parser);
}
int  XmlParser::getCurrentLineNumber(void)
{
  return XML_GetCurrentLineNumber(_parser);
}
int  XmlParser::getCurrentColumnNumber(void)
{
  return XML_GetCurrentColumnNumber(_parser);
}
int  XmlParser::getCurrentByteCount(void)
{
  return 0;
}
const char* XmlParser::getInputContext(int* pOffset, int* pSize)
{
  return NULL;
}

/*************************************************************
Static Functions
*************************************************************/
int XmlParser::getExpatMajorVersion(void)
{
  XML_Expat_Version ver = XML_ExpatVersionInfo();
  return ver.major;
}
int XmlParser::getExpatMinorVersion(void)
{
  XML_Expat_Version ver = XML_ExpatVersionInfo();
  return ver.minor;
}
int XmlParser::getExpatMicroVersion(void)
{
  XML_Expat_Version ver = XML_ExpatVersionInfo();
  return ver.micro;
}

const XML_LChar *XmlParser::getExpatVersion(void)
{
  return XML_ExpatVersion();
}
const XML_LChar* XmlParser::getErrorString(enum XML_Error err)
{
  return XML_ErrorString(err);
}

void XMLCALL XmlParser::startElementHandler (void *pData,
                                             const XML_Char *pszName, 
                                             const XML_Char **papszAttrs)
{
  if(pData != NULL)
    ((XmlParser*)pData)->onStartElement(pszName, papszAttrs);
  else
    std::cout << "Data is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}


void XMLCALL XmlParser::endElementHandler (void *pData,
                                           const XML_Char *pszName)
{
  if(pData != NULL)
    ((XmlParser*)pData)->onEndElement(pszName);
  else
    std::cout << "Data is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}

void XMLCALL XmlParser::characterDataHandler (void *pData,
                                              const XML_Char *pszData, 
                                              int nLength)
{
  if(pData != NULL)
  {
    int nTrim = 0;
    // Trim leading spaces
    for( ; nTrim < nLength; nTrim ++ )
    {
      if( ' ' == *pszData )
      {
        pszData ++;
      }
      else
      {
        break;
      }
    }
    nLength  -= nTrim;
    // Special case for all white space
    if( 0 == nLength )
    {
      // do nothing
    }
    // Special case to avoid making a token out of a line feed.
    else if( nLength == 1 && *pszData == 0x0A )
    {
      // Do nothing
    }
    else
    {
      // Normal case
      ((XmlParser*)pData)->onCharacterData(pszData, nLength);
    }
  }
  else
    std::cout << "Data is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}
void XMLCALL XmlParser::processingInstructionHandler (void *pData,
                                                      const XML_Char *pszTarget,
                                                      const XML_Char *pszData)
{
  if(pData != NULL)
    ((XmlParser*)pData)->onProcessingInstruction(pszTarget, pszData);
  else
    std::cout << "Data is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}

void XMLCALL XmlParser::commentHandler (void *pData,
                                        const XML_Char *pszData)
{
  if(pData != NULL)
    ((XmlParser*)pData)->onComment(pszData);
  else
    std::cout << "Data is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}

void XMLCALL XmlParser::startCdataSectionHandler (void *pData)
{
  if(pData != NULL)
    ((XmlParser*)pData)->onStartCdataSection();
  else
    std::cout << "Data is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}


void XMLCALL XmlParser::endCdataSectionHandler (void *pData)
{
  if(pData != NULL)
    ((XmlParser*)pData)->onEndCdataSection();
  else
    std::cout << "Data is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}

void XMLCALL XmlParser::defaultHandler (void *pData, 
                                        const XML_Char *pszData, 
                                        int nLength)
{
  if(pData != NULL)
    ((XmlParser*)pData)->onDefault(pszData, nLength);
  else
    std::cout << "Data is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}

int XMLCALL XmlParser::externalEntityRefHandler (XML_Parser parser,
                                                 const XML_Char *pszContext, 
                                                 const XML_Char *pszBase, 
                                                 const XML_Char *pszSystemID,
                                                 const XML_Char *pszPublicID)
{
  // External Entity references will not be handled
  return 0;
}


void XMLCALL XmlParser::unknownEncodingHandler (void *pData, 
                                                const XML_Char *pszName, 
                                                XML_Encoding *pInfo)
{
  if(pData != NULL)
    ((XmlParser*)pData)->onUnknownEncoding(pszName, pInfo);
  else
    std::cout << "Data is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}


void XMLCALL XmlParser::startNamespaceDeclHandler (void *pData, 
                                                   const XML_Char *pszPrefix, 
                                                   const XML_Char *pszURI)
{
  if(pData != NULL)
    ((XmlParser*)pData)->onStartNamespaceDecl(pszPrefix, pszURI);
  else
    std::cout << "Data is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}


void XMLCALL XmlParser::endNamespaceDeclHandler (void *pData, 
                                                 const XML_Char *pszPrefix)
{
  if(pData != NULL)
    ((XmlParser*)pData)->onEndNamespaceDecl(pszPrefix);
  else
    std::cout << "Data is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}



void XMLCALL XmlParser::xmlDeclHandler (void *pData,
                                        const XML_Char *pszVersion, 
                                        const XML_Char *pszEncoding,
                                        int nStandalone)
{
  if(pData != NULL)
    ((XmlParser*)pData)->onXmlDecl(pszVersion, pszEncoding, (nStandalone == 0)? false : true);
  else
    std::cout << "Data is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}


void XMLCALL XmlParser::startDoctypeDeclHandler (void *pData,
                                                 const XML_Char *pszDoctypeName,
                                                 const XML_Char *pszSysID, 
                                                 const XML_Char *pszPubID, 
                                                 int nHasInternalSubset)
{
  if(pData != NULL)
    ((XmlParser*)pData)->onStartDoctypeDecl(pszDoctypeName, pszSysID, pszPubID, (nHasInternalSubset == 0) ? false : true);
  else
    std::cout << "Data is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}


void XMLCALL XmlParser::endDoctypeDeclHandler (void *pData)
{
  if(pData != NULL)
    ((XmlParser*)pData)->onEndDoctypeDecl();
  else
    std::cout << "Data is NULL. File :"<<__FILE__<<" Line :"<<__LINE__<< std::endl;
}
