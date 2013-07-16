A grep-like utility for Dlogs. You can build from scratch in Visual Studio,
or just copy the dlgrep.exe and its companion ICSharpCode.SharpZipLib.dll
to a place within your PATH.
 dlgrep [OPTION] -e PATTERN [-e PATTERN2 ...] [FILE_OR_DIR]
 
 Search for PATTERN in DLOG file(s). Only message records are searched.
 However, message records are prepended with timestamp, node name, task name, 
 and the source filename and line number; thus, they are searchable too.
 Returns with status: 0 if a pattern was matched, 1 if no matches, 2 on error.
 
 Options:
  -e  PATTERN : Regular expression to search (grep) for
  -ef PATTERN : Regular expression to match file names against. Default: \.dlog$
  -i          : Ignore case for regular expression(s) set by -e
  -c          : Print *only* the count of matching lines per file
  -l          : Print *only* names of files containing matches
  -H          : Print the filename for each match as a prefix
  -A   NUM    : Print NUM lines after a match
  -B   NUM    : Print NUM lines before a match
  -C   NUM    : Print NUM lines surounding a match
  -t   NUM    : Set the number of search threads. Default: 1
  -min NUM    : Minimum log file size, in kbytes. Default: 20
  -max NUM    : Maximum log file size, in kbytes. Default: 2048 (2MB)
  -h          : Display this help message and exit
  FILE_OR_DIR : Either a file or directory path; a directory will be
                recursively searched. Default: . (current directory)
 
 Examples:
 # Search for proc.cpp *or* 'Displayed Alarm' in current directory
   dlgrep -e proc.cpp -e 'Displayed Alarm'
   dlgrep -e 'proc.cpp|Displayed Alarm'
 # Count button pushes in all logs from 2012-2013
   dlgrep -c -e 'GUI button pushed' -ef '1T.*_201[23].*.dlog
 # Find logs in directory T5001 with BUILD versions 8.8 or 8.9
   dlgrep -l -e BUILD=8.[89] T5001
 
 dlgrep Version 1.5, Jul-2013 -- Copyright (C) 2013 Terumo BCT
