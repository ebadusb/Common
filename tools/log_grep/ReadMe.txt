A grep-like utility for Dlogs. You can build from scratch in Visual Studio,
or just copy the Log_Grep.exe and its companion ICSharpCode.SharpZipLib.dll
to a place within your PATH.

 Log_Grep Version 1.4 -- Terumo BCT 
 Options :
  -t         : Set the number of search threads. Default is 1
  -h         : Displays this help message
  -e  [RegEx]: A regular expression to search (grep) for
  -ef [RegEx]: A regular expression to match file names against
  -i         : Ignores case for the regular expressions
  -l         : Supresses normal output and just lists file names
  -H         : Prefixes the output with the file name
  -A  [NUM]  : Prints NUM lines after a match
  -B  [NUM]  : Prints NUM lines before a match
  -C  [NUM]  : Prints NUM lines surounding the match
      [FILE] : Either a path or a file. For path, it will recurse down the path
