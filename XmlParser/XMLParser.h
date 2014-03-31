/*
* Copyright (C) 2007 Gambro BCT, Inc.  All rights reserved.
* $Header$ 
* 
*
* $Log: XMLParser.h $
* Revision 1.3  2007/12/13 20:44:23Z  estausb
* Revision 1.1  2007/11/15 20:42:44Z  estausb
* Initial revision
*
*/

/*! \file XmlParser.h
*/


#ifndef XmlParser_Included
#define XmlParser_Included

#include "Expat.h"

/// \brief A C++ wrapper for the Expat XML parser.

///
/// This is the base class to wrap the functionallity of the Expat 
/// xml stream based parser.  Overriding the virtual functions will 
/// give full control to the derived class over how elements contained within
/// the document is parsed.
///  
/// See http://expat.sourceforge.net/ for a detailed description of
/// the functionallity provided by the underlying library.
///
class XmlParser
{
public:
  ///Default Constructor
  XmlParser();
  virtual ~XmlParser();

  enum XML_Error getErrorCode () ;
  static const XML_LChar* getErrorString(enum XML_Error err);

protected:
  const XML_LChar* getErrorString();
  long getCurrentByteIndex(void);
  int  getCurrentLineNumber(void);
  int  getCurrentColumnNumber(void);
  int  getCurrentByteCount(void);
  const char* getInputContext(int* pOffset, int* pSize);

  static int getExpatMajorVersion(void);
  static int getExpatMinorVersion(void);
  static int getExpatMicroVersion(void);

  static const XML_LChar* getExpatVersion(void);
protected:
  bool create (const XML_Char *pszEncoding = NULL, 
    const XML_Char *pszSep = NULL);
  void destroy();


  bool parse (const char *pszBuffer, int nLength = -1, bool fIsFinal = true);
  bool parseBuffer (int nLength, bool fIsFinal = true);
  void* getBuffer (int nLength);

  // Override these virtual functions in order to provide the particular parsing functionallity 
  // you are interested in.

  virtual void onStartElement(const XML_Char* name, const XML_Char** attrs){}
  virtual void onEndElement(const XML_Char* name){}
  virtual void onCharacterData(const XML_Char* data, int len){}
  virtual void onProcessingInstruction(const XML_Char* target, const XML_Char* data){}
  virtual void onComment(const XML_Char* data){}
  virtual void onStartCdataSection(void){}
  virtual void onEndCdataSection(void){}
  virtual void onDefault(const XML_Char* data, int len){}
  virtual bool onExternalRef(const XML_Char *pszContext,
    const XML_Char *pszBase,
    const XML_Char *pszSystemID,
    const XML_Char *pszPublicID){return false;} 
  virtual void onUnknownEncoding(const XML_Char* name, XML_Encoding* pInfo){}
  virtual void onStartNamespaceDecl(const XML_Char* prefix, const XML_Char* uri){}
  virtual void onEndNamespaceDecl(const XML_Char* prefix){}
  virtual void onXmlDecl(const XML_Char* version, const XML_Char* encoding, bool isStandalone){}
  virtual void onStartDoctypeDecl(const XML_Char* doctype, const XML_Char* sysId, const XML_Char* pubId, bool hasInternalSubset){}
  virtual void onEndDoctypeDecl(void){}


  // Enable handlers section
  void enableAllHandlers();
  void enableStartElementHandler(bool bEnable = true);
  void enableEndElementHandler (bool bEnable = true);
  void enableElementHandler (bool bEnable = true);
  void enableCharacterDataHandler (bool bEnable = true);
  void enableProcessingInstructionHandler (bool bEnable = true);
  void enableCommentHandler (bool bEnable = true);
  void enableStartCDataSectionHandler(bool bEnable = true);
  void enableEndCDataSectionHandler(bool bEnable = true);
  void enableCDataSectionHandler(bool bEnable = true);
  void enableDefaultHandler(bool bEnable = true);
  void enableExternalEntityRefHandler (bool bEnable = true);
  void enableStartNamespaceDeclHandler (bool bEnable = true);
  void enableEndNamespaceDeclHandler (bool bEnable = true);
  void enableNamespaceDeclHandler (bool bEnable = true);
  void enableXmlDeclHandler (bool bEnable = true);
  void enableStartDoctypeDeclHandler (bool bEnable = true);
  void enableEndDoctypeDeclHandler (bool bEnable = true);
  void enableDoctypeDeclHandler (bool bEnable = true);


protected:
  static void XMLCALL startElementHandler (void *pUserData,
    const XML_Char *pszName, 
    const XML_Char **papszAttrs);

  static void XMLCALL endElementHandler (void *pUserData,
    const XML_Char *pszName);

  static void XMLCALL characterDataHandler (void *pUserData,
    const XML_Char *pszData, 
    int nLength);
  static void XMLCALL processingInstructionHandler (void *pUserData,
    const XML_Char *pszTarget,
    const XML_Char *pszData);

  static void XMLCALL commentHandler (void *pUserData,
    const XML_Char *pszData);

  static void XMLCALL startCdataSectionHandler (void *pUserData);

  static void XMLCALL endCdataSectionHandler (void *pUserData);

  static void XMLCALL defaultHandler (void *pUserData, 
    const XML_Char *pszData, 
    int nLength);
  static int XMLCALL externalEntityRefHandler (XML_Parser parser, 
    const XML_Char *pszContext, 
    const XML_Char *pszBase, 
    const XML_Char *pszSystemID,
    const XML_Char *pszPublicID);

  static void XMLCALL unknownEncodingHandler (void *pUserData, 
    const XML_Char *pszName, 
    XML_Encoding *pInfo);

  static void XMLCALL startNamespaceDeclHandler (void *pUserData, 
    const XML_Char *pszPrefix, 
    const XML_Char *pszURI);

  static void XMLCALL endNamespaceDeclHandler (void *pUserData, 
    const XML_Char *pszPrefix);


  static void XMLCALL xmlDeclHandler (void *pUserData,
    const XML_Char *pszVersion, 
    const XML_Char *pszEncoding,
    int nStandalone);

  static void XMLCALL startDoctypeDeclHandler (void *pUserData,
    const XML_Char *pszDoctypeName,
    const XML_Char *pszSysID, 
    const XML_Char *pszPubID, 
    int nHasInternalSubset);

  static void XMLCALL endDoctypeDeclHandler (void *pUserData);

  // @access Protected members
private:
  /// Private Copy Constructor to avoid accidental usage.
  XmlParser(const XmlParser&);
  /// Private assignment operator to avoid accidental usage.
  XmlParser& operator = (const XmlParser&);

  XML_Parser _parser; ///< The expat XML paser.
};
#endif //XmlParser_Included

