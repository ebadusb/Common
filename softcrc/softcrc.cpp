/******************************************************************************
 * Copyright (c) 1997 by Cobe BCT, Inc.
 *
 * FILENAME: softcrc.c
 * PURPOSE: main code for crc utilities
 * CHANGELOG:
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/softcrc/rcs/softcrc.cpp 1.10 2003/06/26 22:33:44Z jl11312 Exp $
 * $Log: softcrc.cpp $
 * Revision 1.10  2003/06/26 22:33:44Z  jl11312
 * - moved base crc generation function into OS image
 * Revision 1.9  2003/06/23 19:40:01Z  jl11312
 * - added options for computing string and value list CRC's under Windows
 * Revision 1.8  2003/05/13 15:04:00Z  jl11312
 * - completed vxWorks port
 * - modified to allow compiling as both a vxWorks and a Win32 program
 * Revision 1.7  2002/11/20 17:15:00Z  rm70006
 * Change code to keep format string literal to keep compiler from whining.
 * Revision 1.6  2002/09/20 19:30:23Z  td07711
 * fix some constness mismatches
 * Revision 1.5  2002/09/19 22:34:44  td07711
 * port to vxworks
 * Revision 1.4  2000/12/08 01:54:04  ms10234
 * IT4896 - A maximum length was placed on all string operations to prevent
 * overflows from a non-terminated string.  The root path for ignore files was
 * set to 70 characters.  This was too short to accomodate long paths.  The
 * maximum for all strings was changed to 256 to prevent the problem.
 * Revision 1.3  1999/08/21 21:34:09  BS04481
 * Machcrc produces read-only crc files but softcrc produces 
 * read/write crc files
 * Revision 1.2  1999/08/20 23:58:18  BS04481
 * Chmod the resulting update file to readonly
 * Revision 1.1  1999/05/24 23:32:16  TD10216
 * Initial revision
 * Revision 1.1  1999/03/20 00:45:01  TD10216
 * Initial revision
 * Revision 1.13  1998/11/03 02:54:28  TM02109
 * Added ability to do multipule filelist's and multipule chroot's
 * Revision 1.12  1997/11/21 02:21:35  TD07711
 *   1. removed fieldcrc variant, not needed.
 *   2. added assertions for 
 *   3. updated sprintf %s format conversions to %.Ps to avoid potential
 *       buffer overflows, where P limits number of  characters copied.
 * Revision 1.11  1997/07/30 22:21:28  TD07711
 *   IT 2433 fix - usage of -chroot with filelist item of "/" resulted in IGNORE
 *   not working as expected since -chroot prepended pathname contained
 *   a "//" directory delimiter.   Fixed to generate "/" as expected.
 * Revision 1.10  1997/06/17 19:10:10  TD07711
 *    conditionalized the expensive debug code with "if (Debug)"
 * Revision 1.9  1997/06/14 04:54:42  TD07711
 *   1. modified to sort the directory entries before crc'ing them.  Some QNX
 *   filesystems maintain sorted entries and some don't.
 *   2. fixed a bug where -chroot string was not being applied to full path
 *    IGNORE strings.
 * Revision 1.8  1997/06/13 23:41:20  TD07711
 *   for softcrc version only (for use on COBE QNX servers), defined TRACELOG
 *   to be NULL since Trace3b() segfaults on server.
 * Revision 1.7  1997/06/03 19:50:14  TD07711
 *   allow simultaneous use of -verify AND -update.
 * Revision 1.6  1997/06/03 18:51:57  TD07711
 *   1. allow -update AND -verify as long as filenames are different.
 *   2. traverse file system mount points only if -travfs is used.
 * Revision 1.5  1997/05/23 21:30:21  TD07711
 *   moved CRC file paths
 *
 *  4/28/97 - dyes
 *  5/1/97 - dyes - restrict -update for fieldcrc and machcrc utilities
 *  5/2/97 - dyes - add -bufsize arg
 *  5/5/97 - dyes - add -travfs (follow symlinks to another file system)
 *  5/8/97 - dyes - add lines lost from bottom of file
 *                  add TRACELOG macro
 *  5/9/97 - dyes - add -chroot arg for use by project makefile
 *  5/16/97 - dyes - handle IGNORE keyword in filelist
 *  5/23/97 - dyes - moved CRC file paths
 ******************************************************************************/

#ifdef VXWORKS
# include <vxWorks.h>
#endif /* ifdef VXWORKS */

#include <ctype.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "crcgen.h"

// Windows does not use opendir/readdir/closedir, so we define them here
#ifdef VXWORKS

#include <dirent.h>
#include <unistd.h>

static int stricmp(const char *s, const char *t)
{
	int status = 0;

	if ( s )
	{
		if ( t )
		{
			int i;
			for ( i=0 ; s[i] != 0 ; i++ )
			{
				if ( t[i] == 0 ) 
				{
					status = 1;
					break;
				}
				else 
				{
					int sI = tolower((int)s[i]);
					int tI = tolower((int)t[i]);
					if ( sI < tI ) 
					{
						status = -1;
						break;
					}
					else if ( sI > tI )
					{
						status = 1;
						break;
					}
				}
			}
			if ( s[i] == 0 && t[i] != 0 ) status = -1;
		}
		else
		{
			status = 1;	
		}
	}
	else if ( t )
	{
		status = -1;
	}

	return status;
}

#else /* ifdef VXWORKS */

#include <io.h>

struct dirent
{
   char  d_name[FILENAME_MAX];
};

struct DIR
{
   struct dirent  entry;
   struct _finddata_t   findData;
   long  findHandle;
};

DIR * opendir(const char * dirName)
{
   DIR * result = new DIR;
   char * wildcard = "/*";
   char * pattern = new char[strlen(dirName)+strlen(wildcard)+1];

   strcpy(pattern, dirName);
   strcat(pattern, wildcard);

   result->findHandle = _findfirst(pattern, &result->findData);
   if ( result->findHandle < 0 )
   {
      delete result;
      result = NULL;
   }

   delete[] pattern;
   return result;
}

struct dirent * readdir(DIR * dir)
{
   strcpy(dir->entry.d_name, dir->findData.name);
   if ( dir->findHandle >= 0 &&
        _findnext(dir->findHandle, &dir->findData) < 0 )
   {
      dir->findData.name[0] = '\0';
      _findclose(dir->findHandle);
      dir->findHandle = -1;
   }

   return ( dir->entry.d_name[0] ) ? &dir->entry : NULL;
}

int closedir(DIR * dir)
{
   if (dir->findHandle >= 9 )
   {
      _findclose(dir->findHandle);
   }

   delete dir;
   return 0;
}

char * strtok_r(char * str, const char * sep, char ** next)
{
   return strtok(str, sep);
}

bool S_ISDIR(int m)
{
   return ((m & _S_IFDIR) != 0 );
}

bool S_ISREG(int m)
{
   return ((m & _S_IFREG) != 0 );
}

#include "crc_table.h"
int crcgen32(unsigned long * pcrc, const unsigned char * pdata, long length)
{
   // check args
   if (length < 0)
      return -1;
   if (pdata == 0)
      return -1;
   if (pcrc == 0)
      return -1;

   // do crc calc
   while(length--)
      *pcrc = crctable[ (*pcrc ^ *pdata++) & 0xFFL] ^ (*pcrc >> 8);

	return 0;
}

#endif /* ifdef VXWORKS */

// Verbosity levels - these must match the integer values specified on the command line
enum OutputType
{
   OT_First = 0,
   OT_None = OT_First,
   OT_Normal,
   OT_CRCFileList,
   OT_CRCAll,
   OT_Debug,
   OT_Last
};

// Linked list of file names
struct FileNameNode
{
   char * _name;
   FileNameNode * _next;

   static void append(FileNameNode ** head, const char * name);
   static void append(FileNameNode ** head, const char * path, const char * name);
   static void clear(FileNameNode * head);
};

class SoftCRC;
class FileList
{
public:
   FileList(SoftCRC * crcObj);
   virtual ~FileList();

   bool init(const char * fileName, const char * rootDir);
   bool getNextItem(char * itemName, size_t nameSize);
   void processItem(const char * itemName, unsigned long * crc);

   bool checkError(void) { return _error; }

private:
   bool checkIgnore(const char * fileName);
   void processDir(const char * dirName, unsigned long * crc);
   void processRegular(const char * fileName, unsigned long * crc);

private:
   SoftCRC *      _crcObj;
   char *         _fileName;
   char *         _rootDir;
   FILE *         _fp;
   FileNameNode * _ignoreList;
   int            _line;
   bool           _error;
   int            _subDirLevel;
};

// Program options from command line
class SoftCRC
{
public:
   SoftCRC(void);
   virtual ~SoftCRC();

   int process(char * cmdLine);

   void log_printf(OutputType logType, const char * format, ...);

   int recursionLimit(void) { return subDirLimit; }

private:
   int processFileList(const char * fileListName, const char * rootDir);
   bool setOptions(char * cmdLine);
   void usage(const char * message);

private:
   OutputType     outputType;       // output type (-verbose, -quiet)
   bool           debug;            // debug output enabled (-debug)
   FileNameNode * fileList;         // file lists (-filelist)
   FileNameNode * rootDirList;      // root directories (-chroot)
   char *         updateFileName;   // update file name (-update) 
   char *         verifyFileName;   // verify file name (-verify)
   unsigned long  initCRC;          // initial CRC value (-initcrc)
   int            subDirLimit;      // subdirectory recursion limit (-limit)

   unsigned long  currentCRC;
};

void FileNameNode::append(FileNameNode ** head, const char * name)
{
   append(head, "", name);
}

void FileNameNode::append(FileNameNode ** head, const char * path, const char * name)
{
   FileNameNode ** tail = head;
   while ( *tail )
   {
      tail = &((*tail)->_next);
   }

   *tail = new FileNameNode;
   (*tail)->_name = new char[strlen(path)+strlen(name)+1];
   strcpy((*tail)->_name, path);
   strcat((*tail)->_name, name);
   (*tail)->_next = NULL;
}

void FileNameNode::clear(FileNameNode * head)
{
   FileNameNode * current = head;

   while ( current )
   {
      FileNameNode * next = current->_next;
      delete[] current->_name;
      delete current;

      current = next;
   }
}

FileList::FileList(SoftCRC * crcObj)
      : _crcObj(crcObj), _fileName(NULL), _rootDir(NULL), _fp(NULL), _ignoreList(NULL), _line(0), _error(false), _subDirLevel(0)
{
}

FileList::~FileList()
{
   delete[] _fileName;
   delete[] _rootDir;

   if ( _fp )
   {
      fclose(_fp);
   }

   FileNameNode::clear(_ignoreList);
}

bool FileList::init(const char * fileName, const char * rootDir)
{
   // save the file name for use in error messages
   _fileName = new char[strlen(fileName)+1];
   strcpy(_fileName, fileName);

   // save the specified root directory (default is "/")
   const char * rootUse = (rootDir) ? rootDir : "/";
   _rootDir = new char[strlen(rootUse)+1];
   strcpy(_rootDir, rootUse);

   // remove final '/' in root directory if present
   if ( _rootDir[strlen(_rootDir)-1] == '/' )
   {
      _rootDir[strlen(_rootDir)-1] = '\0';
   }

   // open specified file list
   _fp = fopen(fileName, "r");
   if ( !_fp )
   {
      _crcObj->log_printf(OT_Normal, "%s: open failed (errno=%d)\n", fileName, errno);
      _error = true;
   }

   return (_fp != NULL);
}

bool FileList::getNextItem(char * itemName, size_t nameSize)
{
   enum { BufSize = 512 };
   const char * separators = " \t\r\n";

   char * buf = new char[BufSize];
   bool  result = false;

   while ( !result &&
           _fp &&
           fgets(buf, BufSize, _fp) )
   {
      _line += 1;
      if ( buf[strlen(buf)-1] != '\n' )
      {
         _crcObj->log_printf(OT_Normal, "%s(%d): line too long\n", _fileName, _line);
         _error = true;
         break;
      }

      int   startIdx = strspn(buf, separators);
      char * token = &buf[startIdx];
      char * nextToken = NULL;

      if ( strtok_r(token, separators, &nextToken) )
      {
         if ( strcmp(token, "IGNORE") == 0 )
         {
            token = strtok_r(NULL, separators, &nextToken);
            if ( !token || token[0] == '#' )
            {
               _crcObj->log_printf(OT_Normal, "%s(%d): expected file name after \"IGNORE\"\n", _fileName, _line);
               _error = true;
               break;
            }

            if ( token[0] == '/' )
            {
               FileNameNode::append(&_ignoreList, _rootDir, token);
            }
            else
            {
               FileNameNode::append(&_ignoreList, token);
            }
            
         }
         else if ( token[0] && token[0] != '#' )
         {
            if ( token[0] == '/' &&
                 strlen(token)+strlen(_rootDir) < nameSize )
            {
               strcpy(itemName, _rootDir);
               strcat(itemName, token);
               result = true;
            }
            else if ( token[0] != '/' &&
                      strlen(token) < nameSize )
            {
               strcpy(itemName, token);
               result = true;
            }
            else
            {
               _crcObj->log_printf(OT_Normal, "%s(%d): pathname too long (limited to %u characters)\n", _fileName, _line, nameSize-1);
               _error = true;
               break;
            }
         }
      }
   }

   if ( !result )
   {
      // found last entry, so just close the file
      fclose(_fp);
      _fp = NULL;
   }

   delete[] buf;
   return result;
}

void FileList::processItem(const char * itemName, unsigned long * crc)
{
   if ( !checkIgnore(itemName) )
   {
      struct stat statData;
      if ( stat((char *)itemName, &statData) != 0 )
      {
         _crcObj->log_printf(OT_Normal, "%s: stat failed (errno=%d)\n", itemName, errno);
         _error = true;
      }
      else if ( S_ISDIR(statData.st_mode) )
      {
         processDir(itemName, crc);
      }
      else if ( S_ISREG(statData.st_mode) )
      {
         processRegular(itemName, crc);
      }
   }
   else
   {
      _crcObj->log_printf(OT_Debug, "%s: ignoring %s\n", _fileName, itemName);
   }
}

bool FileList::checkIgnore(const char * fileName)
{
   bool result = false;
   FileNameNode * entry = _ignoreList;

   while ( !result && entry )
   {
      result = ( stricmp(entry->_name, fileName) == 0 );
      entry = entry->_next;
   }

   return result;
}

void FileList::processDir(const char * dirName, unsigned long * crc)
{
   DIR *    dir = NULL;
   size_t   nameListSize = 0;
   size_t   nameListCount = 0;
   char **  nameList = NULL;
   size_t   idx;

   // check for recursion limit reached
   _subDirLevel += 1;
   if ( _subDirLevel > _crcObj->recursionLimit() )
   {
      _crcObj->log_printf(OT_Normal, "%s: skipping directory - recursion limit reached\n", dirName);
      goto processDirDone;
   }
   
   // open the directory to be processed
   dir = opendir((char *)dirName);
   if ( !dir )
   {
      _crcObj->log_printf(OT_Normal, "%s: opendir failed (errno=%d)\n", dirName, errno);
      goto processDirDone;
   }

   // allocated storage area for sorted file list - this area is expanded as needed during processing
   enum { NameListBlockSize = 64 };
   nameListSize = NameListBlockSize;
   nameList = (char **)malloc(nameListSize*sizeof(char *));

   // since this isn't time critical, just do a slow but simple insertion sort
   struct dirent * dirEntry;
   while ( (dirEntry = readdir(dir)) != NULL &&
           !_error )
   {
      if ( strcmp(dirEntry->d_name, ".") != 0 &&
           strcmp(dirEntry->d_name, "..") != 0 )
      {
         nameListCount += 1;
         if ( nameListCount > nameListSize )
         {
            nameListSize += NameListBlockSize;
            nameList = (char **)realloc(nameList, nameListSize*sizeof(char *));
         }
 
         size_t nameListIdx = 0;
         while ( nameListIdx < nameListCount-1 &&
                stricmp(dirEntry->d_name,nameList[nameListIdx]) > 0 ) 
         {
            nameListIdx += 1;
         }

         memmove(&nameList[nameListIdx+1], &nameList[nameListIdx], (nameListCount-nameListIdx-1)*sizeof(char *));
         nameList[nameListIdx] = new char[strlen(dirEntry->d_name)+1];
         strcpy(nameList[nameListIdx], dirEntry->d_name);
      }
   }

   for ( idx=0; idx<nameListCount && !_error; idx++ )
   {
      char * itemName = new char[strlen(nameList[idx])+strlen(dirName)+2];
      strcpy(itemName, dirName);
      if ( itemName[strlen(itemName)-1] != '/' )
      {
         strcat(itemName, "/");
      }

      strcat(itemName, nameList[idx]);

      processItem(itemName, crc);
      _crcObj->log_printf(OT_CRCAll, "%s: ICRC 0x%08lx\n", itemName, *crc);
      delete[] itemName;
   }

processDirDone:
   _subDirLevel -= 1;

   if ( dir )
   {
      closedir(dir);
   }

   if ( nameListSize )
   {
      for ( idx=0; idx<nameListCount; idx++ )
      {
         delete[] nameList[idx];
      }

      free(nameList);
   }
}

void FileList::processRegular(const char * fileName, unsigned long * crc)
{
	// Windows requires opening files in binary mode to avoid translating line endings
#ifdef VXWORKS
   int   fd = open(fileName, O_RDONLY, 0666);
#else /* ifdef VXWORKS */
   int   fd = open(fileName, O_RDONLY | O_BINARY, 0666);
#endif /* ifdef VXWORKS */

   if ( fd < 0 )
   {
      _crcObj->log_printf(OT_Normal, "%s: open failed (errno = %d)\n", fileName, errno);
      _error = true;
   }
   else
   {
      enum { ReadBuffSize = 4096 };
      unsigned char  * buffer = new unsigned char[ReadBuffSize];

      int   readSize; 
      while ( (readSize = read(fd, (char *)buffer, ReadBuffSize)) > 0 )
      {
         crcgen32(crc, buffer, readSize);
      }
      
      delete[] buffer;
      close(fd);
   }
}

SoftCRC::SoftCRC(void)
      : outputType(OT_Normal),
        debug(false),
        fileList(NULL),
        rootDirList(NULL),
        updateFileName(NULL),
        verifyFileName(NULL),
        initCRC(INITCRC_DEFAULT),
        subDirLimit(10),
        currentCRC(0)
{
}

SoftCRC::~SoftCRC()
{
   FileNameNode::clear(fileList);
   FileNameNode::clear(rootDirList);

   delete[] updateFileName;
   delete[] verifyFileName;
}

int SoftCRC::process(char * cmdLine)
{
   if ( !setOptions(cmdLine) )
   {
      return -1;
   }

   FileNameNode * currentFileList = fileList;
   FileNameNode * currentRootDir = rootDirList;
   currentCRC = initCRC;

   while ( currentFileList )
   {
      log_printf(OT_Normal, "FILELIST: %s\n", currentFileList->_name);
      if ( currentRootDir )
      {
         log_printf(OT_Normal, "ROOTDIR: %s\n", currentRootDir->_name);
      }

      int result = processFileList(currentFileList->_name, (currentRootDir) ? currentRootDir->_name : NULL);
      if ( result < 0 )
      {
         return result;
      }

      currentFileList = currentFileList->_next;
      currentRootDir = (currentRootDir) ? currentRootDir->_next : NULL;
   }

   log_printf(OT_Normal, "CRC: 0x%08lx\n", currentCRC);
   if ( verifyFileName )
   {
      FILE * fp = fopen(verifyFileName, "r");
      if ( !fp )
      {
         log_printf(OT_Normal, "%s: open failed (errno=%d)\n", verifyFileName, errno);
         return -1;
      }

      unsigned long  compareCRC;
      bool  compareReadOK = (fscanf(fp, "%lx", &compareCRC) == 1);
      fclose(fp);

      if ( !compareReadOK )
      {
         log_printf(OT_Normal, "%s: CRC read failed\n", verifyFileName);
         return -1;
      }

      if ( compareCRC != currentCRC )
      {
         log_printf(OT_Normal, "%s: verification failed (crc=0x%08lx expected=0x%08lx)\n", verifyFileName, compareCRC, currentCRC);
         return -1;
      }
      else
      {
         log_printf(OT_Normal, "%s: verified OK (crc=0x%08lx)\n", verifyFileName, currentCRC);
      }
   }

   if ( updateFileName )
   {
      FILE * fp = fopen(updateFileName, "w");
      if ( !fp )
      {
         log_printf(OT_Normal, "%s: open failed (errno=%d)\n", updateFileName, errno);
         return -1;
      }

      fprintf(fp, "0x%08lx\n", currentCRC);
      fclose(fp);

      log_printf(OT_Normal, "%s: updated (crc=0x%08lx)\n", updateFileName, currentCRC);
   }

   return 0;
}

bool SoftCRC::setOptions(char * cmdLine)
{
   const char * separators = " \t\n";

   bool result = true;
   int   startIdx = strspn(cmdLine, separators);
   char * token = &cmdLine[startIdx];
   char * nextToken = NULL;

   strtok_r(token, separators, &nextToken);
   while ( result && token && token[0] )
   {
      if ( strcmp(token, "-chroot") == 0 )
      {
         token = strtok_r(NULL, separators, &nextToken);
         if ( !token )
         {
            usage("expected path name after -chroot");
            result = false;
         }
         else
         {
            FileNameNode::append(&rootDirList, token);
         }
      }
      else if ( strcmp(token, "-debug") == 0 )
      {
         debug = true;
      }
      else if ( strcmp(token, "-filelist") == 0 )
      {
         token = strtok_r(NULL, separators, &nextToken);
         if ( !token )
         {
            usage("expected file name after -filelist");
            result = false;
         }
         else
         {
            FileNameNode::append(&fileList, token);
         }
      }
      else if ( strcmp(token, "-initcrc") == 0 )
      {
         token = strtok_r(NULL, separators, &nextToken);
         if ( !token || sscanf(token, "%lu", &initCRC) != 1 )
         {
            usage("expected value after -initcrc");
            result = false;
         }
      }
      else if ( strcmp(token, "-limit") == 0 )
      {
         token = strtok_r(NULL, separators, &nextToken);
         if ( !token || sscanf(token, "%d", &subDirLimit) != 1 )
         {
            usage("expected value after -limit");
            result = false;
         }
      }
      else if ( strcmp(token, "-update") == 0 )
      {
         token = strtok_r(NULL, separators, &nextToken);
         if ( !token )
         {
            usage("expected file name after -update");
            result = false;
         }
         else
         {
            updateFileName = new char[strlen(token)+1];
            strcpy(updateFileName, token);
         }
      }
      else if ( strcmp(token, "-verbose") == 0 )
      {
         int   level;
         token = strtok_r(NULL, separators, &nextToken);
         if ( !token || sscanf(token, "%d", &level) != 1 )
         {
            usage("expected value after -verbose");
            result = false;
         }
         else if ( level < OT_First || level >= OT_Last )
         {
            usage("value of out range for -verbose");
            result = false;
         }
         else
         {
            outputType = (OutputType)level;
         }
      }
      else if ( strcmp(token, "-verify") == 0 )
      {
         token = strtok_r(NULL, separators, &nextToken);
         if ( !token )
         {
            usage("expected file name after -verify");
            result = false;
         }
         else
         {
            verifyFileName = new char[strlen(token)+1];
            strcpy(verifyFileName, token);
         }
      }

      if ( result )
      {
         token = strtok_r(NULL, separators, &nextToken);
      }
   }

   if ( result )
   {
      if ( !fileList )
      {
         usage("-filelist option required");
         result = false;
      }

      if ( verifyFileName && updateFileName && strcmp(verifyFileName, updateFileName) == 0 )
      {
         usage("verify and update on the same file is not allowed");
         result = false;
      }
   }

   return result;
}

int SoftCRC::processFileList(const char * fileListName, const char * rootDir)
{
   FileList fileList(this);
   if ( !fileList.init(fileListName, rootDir) )
   {
      return -1;
   }

   enum { ItemSize = 512 };
   char item[ItemSize];
   while ( !fileList.checkError() &&
           fileList.getNextItem(item, ItemSize) )
   {
      fileList.processItem(item, &currentCRC);
      log_printf(OT_CRCFileList, "%s: ICRC 0x%08lx\n", item, currentCRC);
   }

   return ( fileList.checkError() ) ? -1 : 0;
}

void SoftCRC::log_printf(OutputType logType, const char * format, ...)
{
   if ( logType <= outputType )
   {
      va_list  argList;
      va_start(argList, format);

      vfprintf(stderr, format, argList);
   }
}

void SoftCRC::usage(const char * message)
{
   const char * usageText =
      "softcrc -filelist <filename> [other options]\n"
      "   -filelist <filename> [-filelist <filename>] - list of files and directories to CRC\n"
      "   -chroot <path> [-chroot <path>] - specify prefix for absolute pathnames, default is no prefix\n"
      "        multiple -chroot OK and are associated with corresponding -filelist entries\n"
      "   -initcrc <value> - initial CRC value, default=0xffffffff\n"
      "   -limit <value> - limits subdirectory nesting, default=10\n"
      "   -update <filename> - saves calculated CRC, default is no update\n"
      "   -verbose <value> - verbosity of CRC output\n"
      "                      0 - suppresses all output to stdout and stderr\n"
      "                      1 - outputs only the final CRC\n"
      "                      2 - outputs intermediate CRC's of things in filelist\n"
      "                      3 - outputs intermediate CRC's of all files and dirs\n"
      "                      4 - full debug information\n"
      "   -verify <filename> - verify calculated CRC, default is no verify\n"
      "   \n"
      "   examples:\n"
      "   \n"
      "   softcrc -filelist filelists/trima.files -chroot /releases/build1.179/current_build\n"
      "           -filelist filelists/focgui.files -chroot /releases/build1.179/current_build\n"
      "           -update ../disk.crc\n"
      "   \n"
      "   machcrc -filelist filelists/trima.files -filelist filelists/focgui.files\n"
      "           -filelist filelists/qnx.files -verify /trima/disk.crc\n";

   if ( outputType >= OT_Normal )
   {
      fprintf(stderr, "%s\n", message);
      fprintf(stderr, "\nUsage:\n%s\n", usageText);
   }
}

//
// main entry point
//
#ifdef VXWORKS

extern "C"
int softcrc(const char* options)
{
   // make a writable copy of the command line options
   char * cmdLine = new char[strlen(options)+1];
   strcpy(cmdLine, options);

   // perform the actual CRC calculations
   SoftCRC  softCRCObj;
   int result = softCRCObj.process(cmdLine);

   delete[] cmdLine;
   return result;
}

#else /* ifdef VXWORKS */

int main(int argc, char ** argv)
{
	if ( argc == 3 &&
		  strcmp(argv[1], "-string") == 0 )
	{
		// Special case - just CRC the specified string
		unsigned long	crc = 0;
		crcgen32(&crc, (unsigned char *)argv[2], strlen(argv[2]));

		printf("String CRC=%08lx\n", crc);
		return 0;
   }
	else if ( argc >= 3 &&
				 strcmp(argv[1], "-values") == 0 )
	{
		// Special case - CRC a list of unsigned long values
		unsigned long	crc = 0;
		unsigned long	value;

		for ( int i=2; i<argc; i++ )
		{
			sscanf(argv[i], "%lx", &value);
			crcgen32(&crc, (unsigned char *)&value, sizeof(value));
		}

		printf("Value list CRC=%08lx\n", crc);
		return 0;
	}

   // concatenate the command line options into a single string
   // to allow use of the same interface as vxWorks
   size_t   optLength = 1;
   int      arg;

   for ( arg=1; arg<argc; arg++ )
   {
      optLength += strlen(argv[arg])+1;
   }

   char * options = new char[optLength];
   options[0] = '\0';

   for ( arg=1; arg<argc; arg++ )
   {
      strcat(options, argv[arg]);
      strcat(options, " ");
   }

   // perform the actual CRC calculations
   SoftCRC  softCRCObj;
   int result = softCRCObj.process(options);

   delete[] options;
   return result;
}

#endif /* ifdef VXWORKS */

