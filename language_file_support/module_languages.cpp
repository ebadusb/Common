/*
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/language_file_support/rcs/module_languages.cpp 1.1 2007/01/18 21:44:03Z MS10234 Exp MS10234 $
 *
 * Utility for loading language information
 *
 * $Log: module_languages.cpp $
 *
 */

#include <vxWorks.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include "module_languages.h"
#include "datalog_levels.h"
#include "config_file_support/basic_config_file.h"



ModuleLanguages::ModuleLanguages(const char *englishFile,const char *dropInFilePath, const char *dropInFilePrefix)
 : _englishFile(englishFile)
 , _languagePath(dropInFilePath)
 , _filePrefix(dropInFilePrefix)
 , _languageInfoMap()
 , _languageInfoReadSuccess(false)
 , _languageFileRevision(0.0)
{
}

ModuleLanguages::~ModuleLanguages()
{
}

void ModuleLanguages::loadLanguageInfo()
{
	// Add English first ...
	//
	string english("english");
	ModuleLanguages::LanguageInfo englishLanguage;
	englishLanguage.languageId = "English";
	englishLanguage.filePath = _englishFile; 
	_languageInfoMap[english] = englishLanguage;

	// Get current string revision ...
	//
	FILE * fp = openLanguageFile(_englishFile.c_str());
	if ( fp )
	{
		string token("formatVersion");
		string tokenValue;
		if ( findToken(fp,token,tokenValue) )
		{
			ConfigFileTools tools;
			if ( !tools.getParamDouble(tokenValue.c_str(),&_languageFileRevision) )
			{
				DataLog(log_level_language_support_error) << "String revision invalid [" << tokenValue.c_str() 
															<< "]:"<< _languageFileRevision << endmsg;
			}
			else
			{
				DataLog(log_level_language_support_info) << "String revision [" << _languageFileRevision << "]" << endmsg;
			}
		}
		else
		{
			DataLog(log_level_language_support_error) << "Language file revision not found for file: \"" 
														<< _englishFile << "\"" << endmsg;
		}
		fclose(fp);
	}
	 
	// Add all other valid language files ...
	//
	_languageInfoReadSuccess = true;
	fileSort(_languagePath.c_str());

	DataLog(log_level_language_support_info) << "Loaded languages: ";
	LanguageInfoMap::const_iterator langIter;
	for ( langIter = _languageInfoMap.begin() ; langIter != _languageInfoMap.end() ; ++langIter )
	{
		DataLog(log_level_language_support_info) << "[" << (*langIter).first.c_str() 
											 << " Id: \"" << (*langIter).second.languageId 
											 << "\" FileName: \"" << (*langIter).second.filePath
											 << "\"] ";
	}
	DataLog(log_level_language_support_info) << endmsg;
}

const ModuleLanguages::LanguageInfoMap &ModuleLanguages::languages()
{
   return _languageInfoMap;
}

bool ModuleLanguages::languageExists(const char *lang)
{
	bool status = false;
	if ( lang )
	{
		string language(lang);
		LanguageInfoMap::const_iterator iter = _languageInfoMap.find(language);
		if ( iter != _languageInfoMap.end() )
		{
			status = true;
		}
	}
	return status;
}

const ModuleLanguages::LanguageInfo &ModuleLanguages::getLanguageInfo(const char *lang)
{
	if ( lang )
	{
		string language(lang);
		LanguageInfoMap::const_iterator iter = _languageInfoMap.find(language);
		if ( iter != _languageInfoMap.end() )
		{
			const ModuleLanguages::LanguageInfo &langRef = (*iter).second;
			return langRef;
		}
	}

	DataLog(log_level_language_support_error) << "Bad language request [" << lang << "]" << endmsg;
	static ModuleLanguages::LanguageInfo blankLanguage;
	return blankLanguage;
}

bool ModuleLanguages::verifyModuleLanguages( ModuleLanguages & standardModule )
{
	bool status = true;

	LanguageInfoMap::iterator standardLangIter;
	for ( standardLangIter = standardModule._languageInfoMap.begin() ; 
			standardLangIter != standardModule._languageInfoMap.end() ; 
			++standardLangIter )
	{
		LanguageInfoMap::iterator langIter;
		if ( _languageInfoMap.find( (*standardLangIter).first ) == _languageInfoMap.end() )
		{
			status = false;
			(*standardLangIter).second.verified = false;
		}
	}

	return status;
}

ModuleLanguages::FileProcessStatus ModuleLanguages::loadLanguageFile(const char * fullPathName)
{
	ModuleLanguages::FileProcessStatus status = ModuleLanguages::FILE_CONTINUE;
	struct stat	fileStat;
	ConfigFileTools fileTools(&log_level_language_support_info, &log_level_language_support_error);

	if ( stat((char *)fullPathName, &fileStat) == OK &&
		  (fileStat.st_mode & S_IFREG) != 0 &&
		  strcmp(&fullPathName[strlen(fullPathName)-5], ".info") == 0 )
	{
		FILE * fp = openLanguageFile(fullPathName);
		if ( fp )
		{
			char fileName[NAME_MAX+1]; memset(fileName,0,NAME_MAX+1);
			strcpy(fileName, strrchr(fullPathName,'/'));
	
			char crcFileName[NAME_MAX+1]; memset(crcFileName,0,NAME_MAX+1);
			strncpy(crcFileName, fullPathName, strlen(fullPathName)-strlen(fileName)+1);
			strcat(crcFileName, "crc");
			strcat(crcFileName, fileName);
			strcpy(&crcFileName[strlen(crcFileName)-5], ".crc");
	
			// Validate the CRC for the language file
			//
			if ( fileTools.checkCRC(fp,crcFileName) )
			// If valid CRC ...
			//
			{
				// Parse the file name and grab the English representation for
				//  the language ...
				char * nextToken;
				char workingString[NAME_MAX+1];
				int startPos = 1 /* '/' char */;
				if ( _filePrefix.length() > 0 ) startPos += _filePrefix.length() + 1 /* '_' char */; 
				strcpy(workingString,&fileName[startPos]);
				char * languageName = strtok_r(workingString,"_",&nextToken);
				if ( languageName )
				{
					string language( languageName );
		
					// Find the UTF8 representation for the language
					//
					string languageIdToken("languageId");
			 
	  				string languageIdValue;
	  				if ( findToken(fp,languageIdToken,languageIdValue) )
					{
						string formatToken("formatVersion");
						string formatTokenValue;
						if ( findToken(fp,formatToken,formatTokenValue) )
						{
							ConfigFileTools tools;
							double revision;
							if ( !tools.getParamDouble(formatTokenValue.c_str(),&revision) )
							{
								DataLog(log_level_language_support_error) << "String revision invalid [" << formatTokenValue.c_str() 
																			<< "]:"<< revision << endmsg;
								status = FILE_END;
							}
							else
							{
								if ( revision != _languageFileRevision ) 
								{
									DataLog(log_level_language_support_error) << "Invalid language revision [" 
																				<< formatTokenValue.c_str() 
																				<< "] for file: \"" 
																				<< fullPathName << "\"" << endmsg;
									status = FILE_END;
								}
								else
								{
									addLanguage(language.c_str(),languageIdValue.c_str(),fullPathName);
								}
							}
						}
						else
						{
							DataLog(log_level_language_support_error) << "Language file revision not found for file: \"" 
																		<< fullPathName << "\"" << endmsg;
							status = FILE_END;
						}
					}
					else
					{
						DataLog(log_level_language_support_error) << "Invalid language file: \"" 
																	<< fullPathName << "\"" << endmsg;
						status = FILE_END;
					}
				}
				else
				{
					DataLog(log_level_language_support_error) << "Invalid language filename: \"" 
																<< fullPathName << "\"" << endmsg;
					status = FILE_END;
				}
			}
			else
			{
				DataLog(log_level_language_support_error) << "CRC check failed for language file: \"" 
															<< fullPathName << "\"" << endmsg;
				status = FILE_END;
			}
			fclose(fp);
		}
		else
		{
			DataLog(log_level_language_support_error) << "Failed to open language file: \"" 
														<< fullPathName << "\"" << endmsg;
			status = FILE_END;
		}
	}

	if ( status == FILE_END )
	{
		_languageInfoReadSuccess = false;
	}

	return status;
}

FILE * ModuleLanguages::openLanguageFile(const char * fullPathName)
{
	FILE * fp = NULL;
	if ( fullPathName )
	{
		fp = fopen(fullPathName, "r");
	}
	return fp;
}

bool ModuleLanguages::findToken( FILE *fp, const string & token, string & tokenValue )
{
	bool foundToken = false;
	enum { MaxLineSize = 256 };
	char	line[MaxLineSize];
	while ( !foundToken && getLine( line, MaxLineSize, fp ) )
	{
		char * token1=NULL;
		char * token2=NULL;
	  
		parseLine( line, token1, token2 );
		if ( token1 && token == token1 )
		{
			foundToken = true;
			tokenValue = token2;
		}
	}
	// Set the file position back to the beginning ...
	//
	fseek(fp, 0, SEEK_SET);

	return foundToken;
}

bool ModuleLanguages::getLine(char * line, size_t lineSize, FILE * fp)
{
	bool moreToRead = ( fgets(line, lineSize, fp) != NULL );
	int newlinePos = strcspn(line, "\r\n");
	
	if ( line[newlinePos] != '\0' )
	{
		// Data fits in provided array, truncate at newline character
		//
		line[newlinePos] = '\0';
	}
	else
	{
		// Line longer than provided array.  Return as many characters as possible and
		// skip to end of line in file
		//
		int ch = fgetc(fp);
		while ( ch != EOF &&
				  ch != '\n' )
		{
			ch = fgetc(fp);
		}
		if ( ch == EOF ) 
			moreToRead = false;
	}

	return moreToRead;
}

void ModuleLanguages::parseLine( char* line, char* &token1, char* &token2 )
{
	token1 = line;
	char * startToken2 = strchr(line,'"');
	token2 = &startToken2[1];

	size_t token1Size = strcspn(line," \t");
	char * endToken2 = strrchr(line,'"');

	if ( token1Size > 0 ) token1[token1Size] = 0;
	if ( endToken2 ) endToken2[0] = 0;
}

void ModuleLanguages::addLanguage( const char * languageName, const char * languageId, const char * filePath )
{
	if ( languageName && languageId && filePath )
	{
		LanguageInfo newLanguage;
		newLanguage.languageId = languageId;
		newLanguage.filePath = filePath;
		newLanguage.verified = true;
		_languageInfoMap[languageName] = newLanguage;
	}
}

static int fileCmpByDateAscending(const ModuleLanguages::FileSortEntry * e1, const ModuleLanguages::FileSortEntry * e2)
{
	int	retVal;

	if ( e1->lastModifyTime > e2->lastModifyTime ) retVal = 1;
	else if ( e1->lastModifyTime < e2->lastModifyTime ) retVal = -1;
	else retVal = 0;

	return retVal;
}

STATUS ModuleLanguages::fileSort(const char * dirName)
{
	STATUS	retVal = ERROR;
	DIR * sortDir = NULL;

	typedef int FileSortCmp(const void *e1, const void *e2);
	FileSortCmp	* fileSortCmp = (FileSortCmp *)fileCmpByDateAscending;
	sortDir = opendir((char *)dirName);

	if ( sortDir )
	{
		struct dirent * dirEntry;
		struct stat	  	 fileStat;
		char 	fullPathName[NAME_MAX+1];

		size_t	fileCount = 0;
		size_t	fileListSize = 512;
		ModuleLanguages::FileSortEntry * fileList = (ModuleLanguages::FileSortEntry*)malloc(fileListSize*sizeof(ModuleLanguages::FileSortEntry));

		size_t	fileNameBufferSize = fileListSize*32;
		size_t	fileNameBufferPos = 0;
		char * fileNameBuffer = (char*)malloc(fileNameBufferSize*sizeof(char));

		FileProcessStatus	callBackStatus = FILE_CONTINUE;

		/*
		 *	Scan directory
		 */
		while ( (dirEntry = readdir(sortDir)) != NULL )
		{
			/*
			 *	Skip . and .. directory entries
			 */
			if ( strcmp(dirEntry->d_name, ".") != 0 &&
				  strcmp(dirEntry->d_name, "..") != 0 )
			{
				int	fileNameLen = strlen(dirEntry->d_name);

				if ( fileCount >= fileListSize )
				{
					/*
					 *	Double file list size or increase by 4096 entries, whichever
					 * is smaller.
					 */
					fileListSize += (fileListSize < 4096) ? fileListSize : 4096;
					fileList = (ModuleLanguages::FileSortEntry*)realloc(fileList, fileListSize*sizeof(ModuleLanguages::FileSortEntry));
				}

				if ( fileNameBufferPos+fileNameLen+1 >= fileNameBufferSize )
				{
					/*
					 *	Look at average file name size so far and increase buffer
					 * size according to current file list size.
					 */
					size_t	avgFileNameLen = fileNameBufferPos/fileCount + 1;

					fileNameBufferSize = avgFileNameLen*fileListSize + fileNameLen + 1;
					fileNameBuffer = (char*)realloc(fileNameBuffer, fileNameBufferSize*sizeof(char));
				}

				/*
				 *	Get file information and add it to the file list
				 */
				stat(dirEntry->d_name,&fileStat);
				strcpy(&fileNameBuffer[fileNameBufferPos], dirEntry->d_name);
				fileList[fileCount].fileNameIndex = fileNameBufferPos;
				fileList[fileCount].lastModifyTime = fileStat.st_mtime;

				fileNameBufferPos += fileNameLen+1;
				fileCount += 1;
			}
		}

		/*
		 *	Send sorted list of files to call back function
		 */
		if ( fileCount > 0 )
		{
			int	file = 0;

			/*
			 *	Sort file entries
			 */
			qsort(fileList, fileCount, sizeof(ModuleLanguages::FileSortEntry), fileSortCmp);
			while ( file < fileCount &&
					  callBackStatus == FILE_CONTINUE )
			{
				buildFullPathName(fullPathName, dirName, &fileNameBuffer[fileList[file].fileNameIndex]);
				callBackStatus = loadLanguageFile(fullPathName);
				file += 1;
			}
		}

		/*
		 *	Clean up
		 */
		closedir(sortDir);
		free(fileNameBuffer);
		free(fileList);
		retVal = OK;
	}

	return retVal;
}

void ModuleLanguages::buildFullPathName(char * result, const char * dirName, const char * fileName)
{
	strcpy(result, dirName);
	if ( dirName[strlen(dirName)-1] != '/' )
	{
		strcat(result, "/");
	}

	strcat(result, fileName);
}


