
/******************************************************************************
 * Copyright (c) 1997 by Cobe BCT, Inc.
 *
 * FILENAME: softcrc.c
 * PURPOSE: main code for crc utilities
 * CHANGELOG:
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/softcrc/rcs/softcrc.cpp 1.10 2003/06/26 22:33:44Z jl11312 Exp MS10234 $
 * $Log: softcrc.cpp $
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

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
// #include <sys/uio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
// #include <sys/trace.h>

#include "softcrc.h"
#include "crcgen.h"
#include "optionparser.h"
#include "error.h"

#define TRACELOG(buf) fprintf(stderr, "%s: %s\n", ProgramName, buf);

// softcrc is for usage on Cobe QNX servers by non-root users
//#ifdef RESTRICT_TO_COBE
char* ProgramName = "softcrc";

// must redefine to avoid linking in trace calls whose linkage results in
// SIGSEGV when run by nonroot user.
#undef ASSERT
#undef LOG_ASSERT

#include "assert.h"
#define ASSERT(expr)  assert(expr)
#define LOG_ASSERT(expr, errmsg)  ((void)0)

//#endif // ifdef RESTRICT_TO_COBE


// machcrc is for use on Trima machines by root
#ifdef RESTRICT_TO_MACHINE
char* ProgramName = "machcrc";
#endif

int compare(const void*, const void*);


char* UsageText = "\
-filelist <filename> [other options] \n\
   -bufsize - size of read buffer in bytes, default=4096\n\
   -debug - turns on debug msgs\n\
   -filelist <filename> [-filelist <filename>] - list of files and directories to CRC\n\
   -chroot <path> [-chroot <path>] - specify prefix for absolute pathnames, default is no prefix\n\
        multiple -chroot OK and are associated with corresponding -filelist entries\n\
   -initcrc <value> - initial CRC value, default=0\n\
   -limit <value> - limits subdirectory nesting, default=10\n\
   -quiet - suppresses output to stderr and stdout, same as -verbose 0\n\
   -update <filename> - saves calculated CRC, default is no update\n\
   -symlink - follow symlinks, default is to ignore (NOT APPLICABLE in vxworks)\n\
   -travfs - follow symlinks across filesystems, default is not to\n\
   -verbose <value> - verbosity of CRC output\n\
                      0 - suppresses all output to stdout and stderr\n\
                      1 - outputs only the final CRC\n\
                      2 - outputs intermediate CRC's of things in filelist\n\
                      3 - outputs intermediate CRC's of all files and dirs\n\
   -verify <filename> - verify calculated CRC, default is no verify\n\
   \n\
   examples:\n\
   \n\
   softcrc -filelist filelists/trima.files -chroot /releases/build1.179/current_build\n\
           -filelist filelists/focgui.files -chroot /releases/build1.179/current_build\n\
           -filelist filelists/qnx.files -chroot /qnx_oem -update ../disk.crc\n\
   \n\
   machcrc -filelist filelists/trima.files -filelist filelists/focgui.files\n\
           -filelist filelists/qnx.files -verify /trima/disk.crc\n\
\n";

//
// Globals
//
static const char* Filelist[FILELIST_MAX+1];       // filelist to parse for files to compute CRC over
static const char* Rootdirlist[FILELIST_MAX+1];    // Rootdirlist to parse for chroots to compute CRC over
static const char* UpdateFile = 0;      // filename to update with new CRC
static const char* VerifyFile = 0;      // filename holding expected CRC
static unsigned long InitCRC = INITCRC_DEFAULT;  // initial CRC value
static int Verbosity = VERBOSE_DEFAULT;          // verbosity of output
static int Debug = 0;                             // flag to enable debug msgs
static int FollowSymlinks = 0;        // flag to enable following symlinks
static int TraverseFileSystems = 0;  // flag to enable following symlinks across file systems
static int SubdirLimit = LIMIT_DEFAULT;    // subdirectory recursion limit
static int ReadbufSize = READBUF_DEFAULT;  // size of read buffer
static unsigned char* Readbuf = 0;        // address of read buffer, initialized with malloc
static const char* Rootdir = 0;       // prefix to added to absolute pathnames in filelist

struct node {
   char* string;
   struct node* next;
};
static struct node* pList = 0;

//
// main entry point
//
int softcrc(const char* options)
{
   FILE* filelist;
   FILE* fp;
   char* path;
   unsigned long crc;
   char buf[BUF_SIZE];

   // Ensure that you have a NULL filelist and rootdirlist to start.
   Filelist[0] = NULL;
   Rootdirlist[0] = NULL;

   OptionParser cmdline( ProgramName, "crc file verification" );
   cmdline.init( options );
   parseCmdline( cmdline.getArgc(), cmdline.getArgv() );

   // get readbuffer
   if ((Readbuf = (unsigned char*)malloc(ReadbufSize)) == NULL) {
      logerrno("malloc failed\n");
      exit(-1);
   }

   for ( int i = 0; Filelist[i]; i++ )
   {

      if (Rootdirlist[i]) {
         // Assign the Root dir for this particular Filelist.
         Rootdir = Rootdirlist[i];
      }
      else {
         Rootdir = NULL;
      }

      if (Verbosity > 0) {
         printf("FILELIST: %s\n", Filelist[i]);
         if (Rootdir) {
            printf("ROOTDIR: %s\n", Rootdir);
         }
      }



      //
      // calculate CRC over filelist
      //
      if ((filelist = fopen(Filelist[i], "r")) == NULL) {
         sprintf(buf, "failed to open filelist %.256s\n", Filelist[i]);
         logerrno(buf);
         exit(-1);
      }
      while ((path = getnextitem(filelist)) != 0) {

         doitem(&InitCRC, path, 1);

         if (Verbosity == 2) {
            printf("ICRC: 0x%08lx   %s\n", InitCRC, path);
         }
      } // end while

      // Close the current filelist
      if ( fclose( filelist ) != 0 ) {
         sprintf(buf, "failed to close filelist %.256s\n", Filelist[i]);
         logerrno(buf);
         exit(-1);
      }

      // Ensure that the IGNORE list is freed between calls to different 
      // filelists.
      struct node* temp_node = 0;
      while ( pList )
      {
         temp_node = pList;         
         free( pList->string );
         pList = pList->next;
         free ( temp_node );
      }
   }

   //
   // output final CRC to stdout
   //
   if (Verbosity != 0)
      printf("CRC: 0x%08lx\n", InitCRC);

   //
   // CRC verification
   //
   if (VerifyFile) {
      if ((fp = fopen(VerifyFile, "r")) == NULL) {
         sprintf(buf, "fopen failed on %.256s, cannot verify\n", VerifyFile);
         logerrno(buf);
         exit(-1);
      }
      if (fgets(buf, sizeof(buf), fp) == NULL) {
         logerrno("fgets failed on -verify filename\n");
         exit(-1);
      }
      if (sscanf(buf, "%lx", &crc) != 1) {
         logerrno("sscanf could not read -verify CRC\n");
         exit(-1);
      }
      if (crc != InitCRC) {
         if (Verbosity > 0) {
            sprintf(buf, "%.256s verification failed, crc=0x%08lx  expected=0x%08lx\n",
                    VerifyFile, InitCRC, crc);
            logerror(buf);
         }
         exit(-1);
      }
      else {
         if (Verbosity > 0) {
            sprintf(buf, "%.256s verified OK crc=0x%08lx\n", VerifyFile, InitCRC);
            loginfo(buf);
         }
      }
   }

   //
   // CRC update
   //
   if (UpdateFile) {
      if ((fp = fopen(UpdateFile, "w")) == 0 ) {
         sprintf(buf, "fopen failed on %.256s, cannot update\n", UpdateFile);
         logerrno(buf);
         exit(-1);
      }
      if (fprintf(fp, "0x%08lx\n", InitCRC) < 0) {
         logerrno("fprintf failed to update -update filename\n");
         exit(-1);
      }

      if (fp != 0 )
      {
         if (fclose(fp) != 0 )
         {
            sprintf(buf, "fclose failed on %.256s\n", UpdateFile);
            logerrno(buf);
            exit(-1);
         }
#ifdef RESTRICT_TO_MACHINE
         else
         {
            if (chmod(UpdateFile, (S_IRUSR|S_IRGRP|S_IROTH)) != 0)
            {
               sprintf(buf, "chmod failed on %.256s, cannot change permission", UpdateFile);
               logerrno(buf);
               exit(-1);
            }
         }
#endif
      }

      if (Verbosity > 0) {
         sprintf(buf, "%.256s updated with new crc=0x%08lx\n", UpdateFile, InitCRC);
         loginfo(buf);
      }
   }

   return 0;

} // end main


// SPECIFICATION:  parseCmdline()
//   parses command line
//   validates arguments
//   implements -update restrictions
// ERROR HANDLING: gives usage message and exits -1 (255)
void parseCmdline(int argc, const char** argv)
{
   int n = argc;
   const char** parg = argv;
   char buf[BUF_SIZE];
   int i_filelists = 0;
   int i_rootdirlists = 0;

   ASSERT(argv != 0);
   ASSERT(argc > 0);

   parg++; // advance past command pathname
   n--;

   // first scan for -quiet to suppress usage msg
   for (parg = argv+1, n = argc-1; n > 0; n--, parg++) {
      if (strcmp("-quiet", *parg) == 0) {
         Verbosity = 0;
      }
   }

   for (parg = argv+1, n = argc-1; n > 0; n--, parg++) {

      if (strcmp("-quiet", *parg) == 0) {
         Verbosity = 0;
      }

      else if (strcmp("-debug", *parg) == 0) {
         Debug = 1;
      }

      else if (strcmp("-symlink", *parg) == 0) {
         FollowSymlinks = 1;
      }
      else if (strcmp("-travfs", *parg) == 0) {
         FollowSymlinks = 1;
         TraverseFileSystems = 1;
      }

      else if (strcmp("-filelist", *parg) == 0) {
         if (--n <= 0) {
            usage("needs -filelist value\n");
         }
         Filelist[i_filelists] = *(++parg);
         i_filelists++;
         Filelist[i_filelists] = NULL;
         if ( i_filelists >= FILELIST_MAX ) {
            usage("Maximum number of filelists exceeded\n");
         }
      }

      else if (strcmp("-verify", *parg) == 0) {
         if (--n <= 0) {
            usage("needs -verify value\n");
         }
         VerifyFile = *(++parg);
      }

      else if (strcmp("-chroot", *parg) == 0) {
         if (--n <= 0) {
            usage("needs -chroot value\n");
         }
         Rootdirlist[i_rootdirlists] = *(++parg);
         i_rootdirlists++;
         Rootdirlist[i_rootdirlists] = NULL;
         if ( i_rootdirlists >= FILELIST_MAX ) {
            usage("Maximum number of chroots exceeded\n");
         } 
      }

      else if (strcmp("-update", *parg) == 0) {
         if (--n <= 0) {
            usage("needs -update value\n");
         }
         UpdateFile = *(++parg);

         // softcrc can update any file
         // machcrc can update ONLY the machine.crc file or files in /tmp
        
#ifdef RESTRICT_TO_MACHINE
         if (strcmp(UpdateFile, MACH_CRCFILE) &&
             strncmp(UpdateFile, "/tmp/", 5)) {
             sprintf(buf, "-update restricted to %.256s or file in /tmp\n",
                     MACH_CRCFILE);
             usage(buf);
         }
#endif
      }

      else if (strcmp("-verbose", *parg) == 0) {
         if (--n <= 0) {
            usage("needs -verbose value\n");
         }
         if (sscanf(*(++parg), "%d", &Verbosity) != 1) {
            usage("invalid -verbose value\n");
         }
         if (Verbosity < 0 || Verbosity > VERBOSE_MAX) {
            usage("invalid -limit value\n");
         }
      }

      else if (strcmp("-limit", *parg) == 0) {
         if (--n <= 0) {
            usage("needs -limit value\n");
         }
         if (sscanf(*(++parg), "%d", &SubdirLimit) != 1) {
            usage("invalid -limit value\n");
         }
         if (SubdirLimit < 0 || SubdirLimit > LIMIT_MAX) {
            usage("invalid -limit value\n");
         }
      }

      else if (strcmp("-bufsize", *parg) == 0) {
         if (--n <= 0) {
            usage("needs -bufsize value\n");
         }
         if (sscanf(*(++parg), "%d", &ReadbufSize) != 1) {
            usage("invalid -bufsize value\n");
         }
         if (ReadbufSize <= 0 || ReadbufSize > READBUF_MAX) {
            usage("invalid -bufsize value\n");
         }
      }

      else if (strcmp("-initcrc", *parg) == 0) {
         if (--n <= 0) {
            usage("needs -initcrc value\n");
         }
         if (sscanf(*(++parg), "%lx", &InitCRC) != 1) {
            usage("invalid -initcrc value\n");
         }
      }

      else {
         sprintf(buf, "invalid argument: %.256s\n", *parg);
         usage(buf);
      }
   }

   if (Filelist[0] == 0)
      usage("-filelist value is required\n");

   if (VerifyFile && UpdateFile &&
      (strcmp(VerifyFile, UpdateFile) == 0))
      usage("verify and update on the same file is not allowed\n");
}


// SPECIFICATION:  usage()
//   display specific error msg
//   display usage text
//   exit -1
// ERROR HANDLING: none
void usage(char* msg)
{
   ASSERT(msg != 0);

   if (Verbosity > 0) {
      fprintf(stderr, "ERROR %s: %s", ProgramName, msg);
      fprintf(stderr, "USAGE: %s %s", ProgramName, UsageText);
   }
   exit(-1);
}


// SPECIFICATION: doitem()
//   handle a filelist or directory entry
//   special handling for symbolic links
// ERROR HANDLING: logs error and exits -1
void doitem(unsigned long* pcrc, char* path, int filelistItem)
// filelistItem - nonzero if from filelist, 0 if from directory traversal
//                used to restrict filesystem crossings
{
   struct stat statbuf;
   char buf[BUF_SIZE];
   static int Device = 0;

   ASSERT(pcrc != 0);
   ASSERT(path != 0);

   // check whether item should be ignored
   if (ignore(path)) {
      sprintf(buf, "doitem: ignoring %.256s\n", path);
      logdebug(buf);
      return;
   }


   if (filelistItem) {
      Device = statbuf.st_dev;
   }


   // check for mount point of different file system
   if (Device != statbuf.st_dev && TraverseFileSystems == 0) {
      sprintf(buf, "doitem: skipping mount point: %.256s\n", path);
      logdebug(buf);
      return;
   }

   if (Debug) {
      sprintf(buf, "doitem: %.256s device=0x%lx\n", path, statbuf.st_dev);
      logdebug(buf);
   }

   if (S_ISDIR(statbuf.st_mode)) { // handle directories
      dodir(pcrc, path);
   }
   else if (S_ISREG(statbuf.st_mode)) { // handle regular file
      dofile(pcrc, path);
   }
}


// SPECIFICATION: dodir()
//   handle directory
//   walks through directory entries
//   skips . and ..
//   implements recursion limit, warning message
//   generates a sorted table of directory entries to ensure
//     a deterministic crc calculation.
// ERROR HANDLING: exits -1
void dodir(unsigned long* pcrc, char* dirname)
{
   DIR* dp;
   struct dirent* pde;
   int nument = 0;
   char buf[BUF_SIZE];
   static int NestLevel = 0;
   char** pent;  // ptr to table entry
   char** table; // NULL terminated table of char*'s
   int i;

   ASSERT(pcrc != 0);
   ASSERT(dirname != 0);

   if ((dp = opendir(dirname)) == NULL) {
      sprintf(buf, "opendir failed on %.256s\n", dirname);
      logerrno(buf);
      exit(-1);
   }

   if (++NestLevel > SubdirLimit) {
      sprintf(buf, "recursion limit exceeded, skipping %.256s\n", dirname);
      logwarning(buf);
      NestLevel--;
      return;
   }


   //
   // sort all entries in the directory
   //
   //   first count the entries
   while (1) {
      if ((pde = readdir(dp)) != NULL) {
         if (strcmp(".", pde->d_name) == 0 ||
             strcmp("..", pde->d_name) == 0)
            continue;  // ignore . and .. entries

         nument++;
      }
      else {
         break;
      }
   }
   //   next, malloc the table, allow for NULL terminator
   table = (char**)malloc((nument+1) * sizeof(char*));
   if (table == 0) {
      logerrno("dodir: malloc of table failed\n");
      exit(-1);
   }
   //   next, malloc space for each entry and fill in table
   rewinddir(dp);
   pent = table, i = 0;
   while(1) {
      if ((pde = readdir(dp)) != NULL) {
         if (strcmp(".", pde->d_name) == 0 ||
             strcmp("..", pde->d_name) == 0)
            continue;  // ignore . and .. entries

         *pent = (char*)malloc(strlen(pde->d_name) + 1);
         if (*pent == 0) {
            logerrno("dodir: malloc of table entry failed\n");
            exit(-1);
         }
         strcpy(*pent, pde->d_name);  // copy name
      }
      else {
         break;
      }
      pent++, i++;
   }
   if (i != nument) {
      logerror("dodir: i==nument assertion failed\n");
      exit(-1);
   }
   *pent = 0;  // NULL terminate the table
   //
   //   finally, sort the table using qsort
   qsort(table, nument, sizeof(char*), compare);

   //
   // crc the sorted list
   //
   for (pent = table; *pent; pent++) {

      if (Debug) {
         sprintf(buf, "dodir: sorted entry = %.256s\n", *pent);
         logdebug(buf);
      }

      // avoid buffer overflow
      if ((strlen(dirname) + 1 + strlen(*pent)) >= sizeof(buf)) {
         logerror("dodir: pathname too big\n");
         exit(-1);
      }
      // append entry name to dirname
      if (strcmp("/", dirname))
         sprintf(buf, "%.256s/%.256s", dirname, *pent);
      else
         sprintf(buf, "/%.256s", *pent);  // special case for "/"

      doitem(pcrc, buf, 0);
   }

   //
   // free the table entries and the table
   //
   for (pent = table; *pent; pent++) {
      free(*pent);
   }
   free(table);

   if (Verbosity == 3)
      printf("ICRC: 0x%08lx   %s\n", *pcrc, dirname);

   if (Debug) {
      sprintf(buf, "dodir: dir=%.256s numentries=%d icrc=0x%08lx\n",
              dirname, nument, *pcrc);
      logdebug(buf);
   }

   if (closedir(dp) == -1) {
      sprintf(buf, "closedir failed on %.256s\n", dirname);
      logerrno(buf);
      exit(-1);
   }

   NestLevel--;
   ASSERT(NestLevel >= 0);
}


// SPECIFICATION: dofile()
//   handles a file - uses file contents to calculate a CRC
// ERROR HANDLING: exits -1
void dofile(unsigned long* pcrc, char* filename)
{
   char buf[BUF_SIZE];
   int fd;
   long length = 0;
   long n = 0;

   ASSERT(pcrc != 0);
   ASSERT(filename != 0);

   if ((fd = open(filename, O_RDONLY, 0444)) == -1) {
      sprintf(buf, "failed to open %.256s\n", filename);
      logerrno(buf);
      exit(-1);
   }

   // read the file and calculate new crc
   while(1) {
      if ((n = read(fd, (char*)Readbuf, ReadbufSize)) == -1) {
         sprintf(buf, "read failed on %.256s\n", filename);
         logerrno(buf);
         exit(-1);
      }
      if (n == 0)
         break;

      if (crcgen32(pcrc, Readbuf, n) == -1) {
         logerror("dofile: crcgen32 failed\n");
         exit(-1);
      }
      length += n;
   }

   if (Verbosity == 3)
      printf("ICRC: 0x%08lx   %.256s\n", *pcrc, filename);

   if (Debug) {
      sprintf(buf, "dofile: file=%.256s length=%ld icrc=0x%08lx\n",
           filename, length, *pcrc);
      logdebug(buf);
   }

   if (close(fd) == -1) {
      sprintf(buf, "close failed on %.256s\n", filename);
      logerrno(buf);
      exit(-1);
   }
}


// SPECIFICATION: getnextitem()
//   parses filelist for next pathname to CRC
//   skips comments and blank lines
//   skips leading white space
//   handle IGNORE keyword to skip specified files/dirs
//   returns pathname, prefixed with rootdir if required
// ERROR HANDLING: none
char* getnextitem(FILE* filelist)
{
   static char buf[BUF_SIZE]; // persistant storage for pathname
   char* ptr;
   char* pathname;
   char msg[BUF_SIZE];
   char tmpbuf[BUF_SIZE];

   ASSERT(filelist != 0);

   // look for non blank non comment
   while (1) {

      if (fgets(tmpbuf, sizeof(tmpbuf), filelist) == NULL)
         return 0;

      tmpbuf[sizeof(tmpbuf)-1] = 0;  // make sure buf is null terminated

      if (Debug) {
         sprintf(msg, "getnextitem: %.256s", tmpbuf);  // buf has \n already
         logdebug(msg);
      }

      // skip whitespace
      for(ptr=tmpbuf; *ptr && isspace(*ptr); ptr++);
      pathname = ptr;

      // handle IGNORE keyword
      if (strncmp(ptr, IGNORE_KEYWORD, strlen(IGNORE_KEYWORD)) == 0) {
         doignore(ptr);
         continue;
      }

      // terminate string on next whitespace
      while(*ptr && !isspace(*ptr))
         ptr++;
      *ptr = 0;

      // skip comments and blank lines
      if (*pathname == 0 || *pathname == '#')
         continue;

      // prefix -chroot value for absolute pathnames
      if (Rootdir && (*pathname == '/')) {
         strcpy(buf, Rootdir);

         // following check avoids problem of chroot usage with listitem "/" resulting in
         // pathnames containing "//" instead of "/" and so not matching IGNORE pathnames
         // containing a "/".
         if (*(pathname+1)) {
            strcat(buf, pathname);
         }
      }
      else {
         strcpy(buf, pathname);
      }

      if (Debug) {
         sprintf(msg, "getnextitem: returned pathname=%.256s\n", buf);
         logdebug(msg);
      }

      ASSERT(buf[0] != 0);
      return(buf);
   }
}



// SPECIFICATION:  loginfo()
//   sends msg string to stdout
//   prepends msg with ProgramName
//   checks verbosity setting to decide whether to send
//   logs message in tracelog
// ERROR HANDLING: none
void loginfo(char* msg)
{
   char buf[BUF_SIZE+40];

   ASSERT(msg != 0);

   sprintf(buf, "%.256s: %.256s", ProgramName, msg);

   if (Verbosity > 0)
      fprintf(stdout, "%.256s: %.256s", ProgramName, msg);

   TRACELOG(buf);
}


// SPECIFICATION: logerrno()
//   sends error msg to stderr
//   sends errno and errno string as well
//   prepends msg with ProgramName
//   checks verbosity setting to decide whether to send
//   logs message in tracelog
// ERROR HANDLING: none
void logerrno(char* msg)
{
   char buf[BUF_SIZE+120];

   ASSERT(msg != 0);

   sprintf(buf, "ERROR %.256s: %.256s\n    errno=%d %.256s\n",
           ProgramName, msg, errno, strerror(errno));

   if (Verbosity > 0)
      fprintf(stderr, "ERROR %.256s: %.256s\n    errno=%d %.256s\n",
           ProgramName, msg, errno, strerror(errno));
   
   TRACELOG(buf);
}


// SPECIFICATION: logerror()
//   sends error msg to stderr
//   prepends msg with ProgramName
//   checks verbosity setting to decide whether to send
//   logs message in tracelog
// ERROR HANDLING: none
void logerror(char* msg)
{
   char buf[BUF_SIZE+40];

   ASSERT(msg != 0);

   sprintf(buf, "ERROR %.256s: %.256s", ProgramName, msg);

   if (Verbosity > 0)
      fprintf(stderr, "ERROR %.256s: %.256s", ProgramName, msg);

   TRACELOG(buf);
}


// SPECIFICATION:
//   sends debug msg string to stderr
//   prepends msg with ProgramName
//   checks verbosity setting to decide whether to send
// ERROR HANDLING: none
void logdebug(char* msg)
{
   ASSERT(msg != 0);

   if (Debug)
      fprintf(stderr, "debug %s: %s", ProgramName, msg);
}


// SPECIFICATION:
//   sends warning msg string to stdout
//   prepends msg with ProgramName
//   checks verbosity setting to decide whether to send
// ERROR HANDLING: none
void logwarning(char* msg)
{
   char buf[BUF_SIZE+40];

   ASSERT(msg != 0);

   sprintf(buf, "Warning %.256s: %.256s", ProgramName, msg);

   if (Verbosity > 0)
      fprintf(stdout, "Warning %.256s: %.256s", ProgramName, msg);

   TRACELOG(buf);
}

// SPECIFICATION:
//   parse pathname from buf
//   add pathname to ignore list
// ERROR HANDLING: none
void doignore(char* pathbuf)
{
   struct node* pnode;
   char* ptr;
   char buf[BUF_SIZE];

   ASSERT(pathbuf != 0);

   // skip IGNORE
   pathbuf+=strlen(IGNORE_KEYWORD);

   // skip whitespace
   for(ptr=pathbuf; *ptr && isspace(*ptr); ptr++);
   pathbuf = ptr;

   // terminate string on next whitespace
   while(*ptr && !isspace(*ptr))
      ptr++;
   *ptr = 0;

   // if -chroot AND full pathname, prepend the -chroot string
   if (Rootdir && (*pathbuf == '/'))
      sprintf(buf, "%.256s%.256s", Rootdir, pathbuf);
   else
      strcpy(buf, pathbuf);

   // copy string to list node
   if ((pnode = (struct node*)malloc(sizeof(struct node))) == 0) {
      logerrno("doignore: node malloc failed\n");
      exit(-1);
   }
   if ((pnode->string = (char*)malloc(strlen(buf)+1)) == 0) {
      logerrno("doignore: string malloc failed\n");
      exit(-1);
   }
   strcpy(pnode->string, buf);

   // add node to front of list
   pnode->next = pList;
   pList = pnode;

   if (Debug) {
      sprintf(buf, "doignore: will ignore %.256s\n", pnode->string);
      logdebug(buf);
   }

   ASSERT(pList != 0);
}


// SPECIFICATION:
//   check ignore list for path, must be exact match
//   returns zero if no match, non-zero if match
// ERROR HANDLING: none
int ignore(char* pathname)
{
   struct node* pnode = pList;

   ASSERT(pathname != 0);

   while (pnode) {
      if (strcmp(pathname, pnode->string) == 0) {
         return 1;  // matched, so ignore
      }
      pnode = pnode->next;
   }
   return 0;  // no match
}


// SPECIFICATION:
//   qsort comparison function
//   uses strcmp() to compare ptrs to table entries
//     where table is array of char*'s.
// ERROR HANDLING: none
int compare(const void* pstring1, const void* pstring2)
{
   int rval;
   char buf[BUF_SIZE];

   ASSERT(pstring1 != 0);
   ASSERT(pstring2 != 0);
   ASSERT(*(char**)pstring1 != 0);
   ASSERT(*(char**)pstring2 != 0);

   rval =  (strcmp(*(char**)pstring1, *(char**)pstring2));

   if (Debug) {
      sprintf(buf, "compare: rval=%d %.256s %.256s\n",
              rval, *(char**)pstring1, *(char**)pstring2);
      logdebug(buf);
   }

   return(rval);
}
