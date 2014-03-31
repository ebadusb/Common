#ifndef ProtectedDocument_Included
#define ProtectedDocument_Included

#include <string>
#include <vector>

#include "Document.h"

namespace BctXml
{
	class ProtectedDocument : public Document
	{
	public:
		//Default constructor
		ProtectedDocument(const std::string& sFile);

		/// Reads in the configuration file with the given name
		/// and will parse and populate nodes.
		/// The CRC inside the file will be checked against a
		/// CRC generated 
		/// \return true if the read was successful
		/// \return false if the read was unsuccessful
		bool read();

		/// Write the document to the passed in file name
		/// Will generate the new CRC and then insert it into the file
		/// before writing the file to disk.
    /// (All write overloads from the base class should be covered.)
		void write( void );
    void write(const std::string& sFileName) ;

    bool isFileCorrupted(void) { return _isCorrupted; }

	private:
		bool _isCorrupted;

    void ProtectedDocument::writeHelper( void );
	};
}
#endif
