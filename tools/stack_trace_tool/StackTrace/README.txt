StackTraceUI v1.1 (GUI version of StackTrace Tool)


Description:

The StackTrace GUI wraps the command line-driven StackTrace Tool so that commonly used options need not be specified every
time, but can be saved and used at a later time.  This README file documents the behavior of each of these options, how
they should be supplied by the user, and how the user can save and reload specific configurations for later use. 


Command Line Usage: 

 [-location buildpath] [-version (2.02 | 2.20)] [-osfile filename] [-ospath pathnames] [-pathalias old:new] ...
 ... [-address values] dlogFileName

 The command-line usage is still available and can be entered in the data log file name textbox.  Note that any
 configuration parameters supplied under StackTrace configuration will override the parameters in the data log
 file name textbox.


Data Log File:

*** Data Log File - 

	Enter name (can optionally include the full path) of data log file or select file in File -> Open ... to browse for file.
	Note that a right-click setup (described below) will automatically fill in this textbox with the data log file name.
	Alternatively, a copy-to-clipboard can be used to automatically fill in this textbox if StackTraceUI is initiated with
	no input.


Configuration TextBoxes:

*** Location - 

	Enter an optional root build location (requires full path). This directory must contain the subdirectory "current_build".
	If a location is not supplied, the directory in the data log file is used. If the directory in the data log file is inaccessible,
	then "\\bctquad3\home" is substituted for the drive, and then that is used as the root build directory.  In the case of Trima,
	if the build path includes "Automated Builds", "current_build" is prepended with "I80486.".  

*** OSPath - 

	Paths to Safety, APC (if applicable), and Control OS images can be optionally included in "OSPaths" field. If used, 
	supply relative paths from "current_build" (assumed a subdirectory of the build path), include all nodes, delimit with
	commas (use quotes if there are spaces), and order as Control, APC (if applicable), and then Safety. The defaults for
	Trima, Optia, and CES are "/vxboot,/trima/safety/boot", 
	"/../base/vxworks_target/control,/../base/vxworks_target/apc,/../base/vxworks_target/safety",
	and "/vxboot,/ces", respectively.

*** Version - 

	Enter toolchain version.  In most cases, StackTrace can determine the device type and version, and a version need not
	be entered.  If not, select 2.02 to make use of "objdump386.exe" and 2.2 for "nmpentium.exe", respectively, to extract
	symbols from object files.

*** OSFile -

	Enter name of OS images.  Examples include "vxWorks", "vxWorks_ampro", and "vxWorks_versalogic".  If not supplied, a name
	is attempted via the data log file.  Note that if neither "vxWorks_ampro" or "vxWorks_versalogic" was deduced in this
	manner, then a default of "vxWorks" is utilized.

*** Alias -

	If your modules reside in a different location than the build location, or are named something different, then this option
	allows you to change all or portions of filename strings to something else prior to looking up their symbols.  Use the format
	"old:new" where "old" represents the old string and "new" represents the new string.

*** Address -

	To print out symbols in the StackTrace trace at specific machine addresses for all nodes, enter a comma-delimited list of
	addresses here.


Saving and Loading Configurations:

Once values have been placed in the configuration textboxes, they can be saved in a xml-formatted file (with extension
.config) by clicking the "Save Config" button.  Multiple configuration files can be saved wherever you like. Similarly, 
previously saved configuration files can be loaded by clicking the "Load Config" button.  Note that your previous configuration
prior to hitting "Stack Trace" the last time you used will already be loaded. 


Right-click Startup:
 
StackTraceUI can be launched from any data log file by making use of the right-click menu. To enable this, go into
"Tools" -> "Folder Options" under any Windows Explorer and click on the "File Types" tab and select "DLOG". Then under the 
"Advanced" button, add a New action using "<Path to StackTraceUI>\StackTraceUI.exe" "%L" as the "Application" and your choice 
of "Action" (e.g., "Open with StackTraceUI").  Alternatively, copy a shortcut icon to "StackTraceUI.exe" into your SendTo folder 
located in your "Documents and Settings". The latter enables you to right click a dlog and send it to a shortcut to "StackTraceUI.exe"
(which can be renamed to whatever you want).

  