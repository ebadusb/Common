/*
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/language_file_support/rcs/module_languages.h 1.1 2007/01/18 21:44:03Z MS10234 Exp $
 *
 * Utility for loading language information
 *
 * $Log: module_languages.h $
 * Revision 1.1  2007/01/18 21:44:03Z  MS10234
 * Initial revision
 *
 */

#ifndef _LOAD_LANGUAGE_INFO_INCLUDE
#define _LOAD_LANGUAGE_INFO_INCLUDE

#include <map>
#include <string>
#include <sys/stat.h>
                                     

class ModuleLanguages
{
public:

	struct LanguageInfo
	{
		string languageId;
		string filePath;
		bool verified;
	};

	typedef struct
	{ 
		size_t	fileNameIndex;
		TIME		lastModifyTime;
	} FileSortEntry;

	typedef map< string, ModuleLanguages::LanguageInfo > LanguageInfoMap;

public:

	ModuleLanguages(const char * defaultCodeString,
						 const char * defaultId,
						 const char * defaultFile, 
						 const char * languageIdTag,
						 const char * formatVersionTag,
						 const char * dropInFilePath, 
						 const char * dropInFilePrefix );
	virtual ~ModuleLanguages();

	//
	// Find and verify all languages for this module
	void loadLanguageInfo();
	bool languageInfoReadSuccessfully() { return _languageInfoReadSuccess; }

	//
	// Returns a referece to a map containing the
	//  English name of the language and its UTF8
	//  name in the specified language, and file path
	//
	const LanguageInfoMap &languages();
	bool languageExists(const char *lang);
	const LanguageInfo &getLanguageInfo(const char *lang);

	//
	// Function to verify all languages exist between two
	//  software modules
	//
	bool verifyModuleLanguages( ModuleLanguages & langRef );

protected:

	string _defaultLangCode;
	string _defaultId;
	string _defaultFile;
	string _languageIdTag;
	string _formatVersionTag;
	string _languagePath;
	string _filePrefix;

private:

	enum FileProcessStatus
	{
		FILE_CONTINUE,		/* fileSort() should continue to next file */
		FILE_END				/* fileSort() should stop with no further callbacks */
	};
	
private:

	FileProcessStatus loadLanguageFile(const char * fullPathName);
	FILE * openLanguageFile(const char * fullPathName);
	bool findToken(FILE *fp, const string &token, string &tokenValue);
	bool getLine(char * line, size_t lineSize, FILE * fp);
	void parseLine( char* line, char* &token1, char* &token2 );
	void addLanguage(const char *languageName, const char *languageId, const char *filePath);

	STATUS fileSort(const char * dirName);
	void buildFullPathName(char * result, const char * dirName, const char * fileName);

private:

	LanguageInfoMap _languageInfoMap;
	bool _languageInfoReadSuccess;
	double _languageFileRevision;

};

#endif /* _LOAD_LANGUAGE_INFO_INCLUDE */

