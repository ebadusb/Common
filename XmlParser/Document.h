/*
* Copyright (C) 2007 Gambro BCT, Inc.  All rights reserved.
* $Header$ 
* 
*/

/*! \file Document.h
*/

#ifndef Document_Included
#define Document_Included

#include <string>
#include <vector>

#include "Element.h"
namespace BctXml
{
/// \brief An XML document.

///
/// The Document class describes an XML document including its root element,
/// and a file name. The document class encapsulates reading and writing the
/// information to a file on the file system.
///
class Document
{
public:
   ///Default Constructor
   Document(const std::string& sFile);
   virtual ~Document();

   std::string getFileName() const;

   /// Reads the XML file with the given name
   /// and will parse it and populate our nodes.
   /// \return true if the read was successful, otherwise false if the read was unsuccesful
   bool read();

   /// \return true if the file is not present
   bool isFileMissing() const;

   /// \return true if the file is present but not reading properly
   bool isFileUnreadable() const;

   /// Write the document to the filename that we where
   /// created with.
   void write();

   /// Write the document to the passed in filename
   void write(const std::string& sFileName);
   
   // Force the document to appear dirty (see isDirty)
   void setDirty();

   // if an element or the entire document was not read from the main file
   // this method will return true
   bool isDirty();

   //send the document to a stream
   void toStream(std::ostream& output) const;

   /// print the documtent to a string so that it can be written
   /// to a file or serialized
   std::string print() const;

   // returns a reference to the root element of the document
   Element* getRootElement() const;

   /// if doc fails to read this will let an external user build a valid document
   void setRootElement(Element *inRoot);

   friend std::ostream& operator<<(std::ostream& str, const BctXml::Document& d);
   static const std::string _backupExtension;
private:
   /// delete the root element
   void clearDoc();

   /// Private Copy Constructor to avoid accidental usage.
   Document(const Document&);

   /// Private assignment operator to avoid accidental usage.
   Document& operator = (const Document&);

   /// copies the file to a file called_sFileName + .backup
   /// This should be used when a file is corrupted and the write
   /// is about to overwrite it.
   void writeBackupFile(const std::string& sFileName);

   Element *_pRootElement;
   std::string _fileName;
   bool _hasDocDecl;
   std::string _version;
   std::string _encoding;
   bool _isStandAlone;
   bool _fileMissing;
   bool _fileUnReadable;
};

} // End namespace BctXml
std::ostream& operator<<(std::ostream& s, BctXml::Document& d);
#endif //Document_Included
