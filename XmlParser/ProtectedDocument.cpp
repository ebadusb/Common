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

/*! \file ProtectedDocument.cpp
*/


#include "ProtectedDocument.h"
#include "Document.h"
#include "FileParser.h"
#include "Element.h"
#include "crcgen.h"   // TODO: Check with Dan or Steve as to how they want this referenced.
#include <fstream>
#include <iostream>
#include <stdio.h>

using namespace BctXml;

static void MyHexCvt( unsigned long u, char * pszChars, int maxLen );

const std::string CRCAttribute = "CRC";

ProtectedDocument::ProtectedDocument(const std::string& sFile) : Document(sFile)
{
}

/*
  Reads an XMl document.

  If the read document has a CRC, the integrity of the document's contents
  are checked.  If the documents are OK, true is returned.  Else false.

  if the read document has _no_ crc, a CRC attribute is added to the root,
  the contents of this attribute are set to a zero length string, and 
  true is returned.
  */
bool ProtectedDocument::read()
{
  bool success = false;

	//calls the parent 'read' method to open the file
	success = Document::read();
  
  if (success)
  {
    BctXml::Element* root = getRootElement();
    if (NULL != root)
    {
	    //if the document doesn't have the CRC attribute, add it in
	    if(!root->hasAttribute(CRCAttribute))
	    {
		    getRootElement()->addAttribute(CRCAttribute, "");
        // Return OK even though no CRC was present to check.
		    // success will be returned and it is true
	    }
	    else // if the document already has the CRC attribute
	    {
		    //pull out the old CRC value so we can calculate the CRC
		    std::string CRCValue = getRootElement()->getAttributeValue(CRCAttribute);
		    getRootElement()->updateAttribute(CRCAttribute, "");

		    // variable to send to CRC method and calculate CRC of the file
		    unsigned long newCRCValue = 0; 

		    //calculates the CRC
		    //if it returns -1, the CRC failed to compute
        std::string docPrint = Document::print();

        if ( crcgen32( &newCRCValue, (unsigned char*)docPrint.c_str(), docPrint.length()) == -1)  
		    {
          std::cerr << "ProtDoc: CRC failed to calculate " << std::endl; 
			    _isCorrupted = true;
			    success = false;
		    }
        else
        {
		      //CRCs do not match, someone has corrupted or tampered with the file?
		      unsigned long oldCRCValue = 0;
          sscanf(CRCValue.c_str(), "%lu", &oldCRCValue); 
		      if( oldCRCValue != newCRCValue )
		      {
			      _isCorrupted = true;
            std::cerr << "ProtDoc: CRC mismatch - file:  " << oldCRCValue << " calculated: " << newCRCValue << std::endl; 
			      success = false;
		      }
		      //else, the file is good and return true
		      else
		      {
			      _isCorrupted = false;
			      success = true;
		      }
        }
	    }
    } // root != NULL
    else
      success = false;
  }

  return success;
}

void ProtectedDocument::write(const std::string& sFileName)
{
  writeHelper();
  Document::write( sFileName );
}

void ProtectedDocument::write()
{
  writeHelper();
  Document::write();
}

// Private function to do the bulk of the write() work.
// Shared by all the public write() overloads.
void ProtectedDocument::writeHelper()
{
	//check if document has been read
	if(getRootElement() == NULL)
  {
    return;
  }

	//if the document doesn't have the CRC attribute, add it in
	if(!getRootElement()->hasAttribute(CRCAttribute))
	{
		getRootElement()->addAttribute(CRCAttribute, "");
	}
	//else, updates the CRC element to be empty so we can generate an accurate CRC on the file
	else
	{
		getRootElement()->updateAttribute(CRCAttribute, "");
	}

	// variable to be fed into the CRC gen
	unsigned long newCRCValue = 0;

	//if the crcgen32 returns -1 there is something wrong with the content
	//return false and note that the ProtectedDocument is corrupted
  std::string writeDoc = Document::print();
	if ( crcgen32( & newCRCValue, (unsigned char* )writeDoc.c_str(), writeDoc.length()) == -1) 
	{
		_isCorrupted = true;
		return;
	}

	//puts the new CRC into the document and returns the successful write
	//getRootElement()->updateAttribute(CRCAttribute, std::to_string( (_ULonglong)newCRCValue ));
	char crcString[20];
	sprintf( crcString, "%lu", newCRCValue);

	getRootElement()->updateAttribute(CRCAttribute, crcString );
	_isCorrupted = false;

	setDirty();

	return;
}

/*
static void MyHexCvt( unsigned long u, char * pszChars, int maxLen )
{
	static const char NibbleMask = 0x0f;
	static const  char charDecode[] = { '0', '1', '2',  '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	if( maxLen < sizeof( u ) * 2 + 1 + 2 )
	{
		return;
	}

	*pszChars ++ = '0';
	*pszChars ++ = 'x';

	for( int i = 0; i < 8; i ++ )
	{
		char nibble = (char)( u >> 4 * ( 7-i));
		* pszChars ++ = charDecode[ NibbleMask & nibble ];
	}
	* pszChars = (char) '\0';
}
*/
