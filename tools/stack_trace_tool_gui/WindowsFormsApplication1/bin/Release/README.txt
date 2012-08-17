StackTraceUI v1.0 (GUI version of StackTrace Tool)

Description:

The StackTrace GUI wraps the command line-driven StackTrace Tool so that commonly used options need not be specified every time after
a config file is written, and enables the use of a right-click short cut. 


Author: 

Corey Howells
Terumo BCT
Aug 05, 2012


Inputs:

* Data Log File - 

	Enter name (including full path) of data log file or select file in File -> Open ... to browse for file. Alternatively, one can
	place the name of a data log file in the config file described below.

* Version - 

	Click version 2.02 for Trima and 2.20 for Optia. Note that StackTraceUI uses 2.20 as the default if neither is mentioned in the
	config file.

* Build Path - 

	An optional root build location can be supplied (include full path). This directory must contain the subdirectory "current_build".
	If not supplied, the directory in the data log file is used. If the directory in the data log file is inaccessible, then \\bctquad3\home
	is substituted for the drive, and then that is used as the root build directory.  

* Paths to OSs - 

	Paths to Safety, APC, and Control OS's can be optionally included in "OSPaths" field. If used, supply relative paths from "current_build"
	(assumed a subdirectory of the build path), include all nodes, delimit with commas (with no spaces), and order according to increasing
	node ID (currently, this is Safety, APC, then Control). The default is "/vwWorks" relative to the "current_build" subdirectory, which
	may not be what you want, so it is suggested to supply this argument in a config file as described below.

* Config file ("StackTrace.cfg") - 

	Default values for StackTraceUI fields, as well as values for -osfile, -alias, and -address options of the StackTrace Tool (refer to 
	StackTrace Tool	documentation) can be placed in an associated config file called "StackTrace.cfg", which is to be placed in the same
	directory as the executable "StackTraceUI.exe". This file should follow the same format as shown in "EXAMPLE.cfg", where options appear
	to the left of "=" and their values to the right.  E.g., to define version as 2.20, include "version=2.20" in the config file. Below,
	the following options are described (since they can only be set via the config file):

		* osfile  - "vxWorks" or some common name for the OS's.
		* alias   - used to replace a common sub-string in object files with an alias sub-string (use format "old:new" for the value
	of alias)
		* address - you can supply a list of comma-delimited target addresses (in hex), which, in the StackTrace results, will be a
	side-by-side list of addresses and symbols for all nodes


Other Behaviors & Issues:

* Right-click Startup -
 
	StackTrace GUI can be setup to launch for any data log file using the right-click menu. To enable, go into Tools -> Folder Options
	in Windows Explorer and click on the File Types tab and select DLOG. Then under	the Advanced button, add a New action using 
	"<Path to StackTraceUI>\StackTraceUI.exe" "%L" as the Application and your choice of Action (e.g., "Open with StackTraceUI"). 
	Alternatively, copy a shortcut icon to "StackTraceUI.exe" into your SendTo folder located in your Documents and Settings. The
	latter enables you to right click a dlog and send it to a shortcut to "StackTraceUI.exe".


* StackTrace couldn't find nmpentium or objdump -

	Unfortunately, the StackTrace Tool needs "nmpentium" or "objdump" (depending on whether you're Trima or Optia) on your system
	path. These must be the	Tornado versions, and are usually located in the "/host/x86-win32/bin" sub-directory of your Tornado 
	installation directory.


* StackTrace couldn't find dlog - 

	It may be that you need to include quotes around your arguments if they contain any spaces (e.g., a folder name that contains a 
	space).
  