A grep-like utility for Dlogs. You can build from scratch in Visual Studio,
or just copy the dlgrep.exe and its companion ICSharpCode.SharpZipLib.dll
to a place within your PATH.

 dlgrep [OPTION]... PATTERN [PATHNAME]...
 
 Search for PATTERN in DLOG file(s). Only message records are searched.
 However, message records are prepended with timestamp, node name, task name, 
 and the source filename and line number; thus, they are searchable too.
 Returns with status: 0 if a pattern was matched, 1 if no matches, 2 on error.
 
 Options:
  -h          : Display this help message and exit
  -e  PATTERN : Regular expression to search (grep) for; supports multiple -e args
  -ef PATTERN : Regular expression to match file names against. Default: \.dlog$
  -i          : Ignore case for regular expression(s) set by -e
  -c          : Print *only* the count of matching lines per file
  -cn NUM     : Print *only* the count of matching lines if >= NUM
  -l          : Print *only* names of files containing matches
  -r          : Recursively search directories
  -H          : Prefix each match with the embedded DLOG filename.
  -HL         : Print filename head-line on 1st match. Default: on if PATHNAME is a list
  -A   NUM    : Print NUM lines after a match
  -B   NUM    : Print NUM lines before a match
  -C   NUM    : Print NUM lines surounding a match
  -t   NUM    : Set the number of search threads. Default: 1
  -min NUM    : Minimum log file size, in kbytes. Default: 20
  -max NUM    : Maximum log file size, in kbytes. Default: 2048 (2MB)
  PATHNAME    : A list of files and/or directories to search. Default: .
 
 Examples:
 # Search for Trima's build label in current directory's *.dlog files:
   dlgrep 'BUILD=8.[89]
   dlgrep ' BUILD=' *.dlog
   dlgrep -e 'BUILD=8.8|BUILD=12.0' .
 # Recursively search for proc.cpp *or* 'Displayed Alarm' in current directory and below:
   dlgrep -r -e proc.cpp -e 'Displayed Alarm'
   dlgrep -r -e 'proc.cpp|Displayed Alarm' .
 # Count button pushes in *all* logs from 2012-2013:
   dlgrep -r -c -e 'GUI button pushed' -ef '1T.*_201[23].*.dlog
 # For machines T5001 and T5002, show logs from Oct 2013 that have 9 or more alarms:
   dlgrep -r -cn 9 -ef '1T.*_201310.*.dlog 'Displayed Alarm' T5001 T5002
 # Find logs in directory T5001 with BUILD versions 8.8 or 8.9:
   dlgrep -l -e TRIMA_BUILD=8.[89] T5001
 
 dlgrep Version 1.6, Oct-2013 -- Copyright (C) 2013 Terumo BCT
