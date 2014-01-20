StackTraceUI v1.4 (GUI version of StackTrace Tool)

$ Log: StackTraceUI $
Revision 1.4  2014/01/20  chowusb
- fixed bug to use build path in Optia dlog as default search path
Revision 1.3  2013/12/09  chowusb
- added support for recursive object file search in subdirectories of build path
- fixed bug for when revision number is missing (seen in Optia local builds) 
Revision 1.2  2012/12/03  chowusb
- pre- and postfix quotes to dlog name whenever user opens dlog via menubar
Revision 1.1  2012/10/11  chowusb
- modified the way build path is located (user-specified path must now include path to "current_build")
- added objdump386 capability for older toolchain
- removed necessity for user to manually set their path
Revision 1.0  2012/09/26  chowusb
- initial revision

Description:

The StackTraceUI wraps the command line-driven StackTrace Tool so that commonly used options need not be specified every
time, but can be saved and used at a later time.  This README file documents the behavior of each of these options, how
they should be supplied by the user, and how the user can save and reload specific configurations for later use. 


Command Line Usage: 

 [-location buildpath] [-version (2.02 | 2.2)] [-osfile filename] [-ospath pathnames] [-pathalias old:new] ...
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

	Enter an optional build location (requires full path). Usually, this is a path to a "current_build" or similarly named 
	directory. Be sure to include the name of this directory in the path. If a location is not supplied, the directory in the
	data log file is used upon appending "current_build". In this case, if such a directory is inaccessible,
	then as a last attempt, "\\bctquad3\home" is substituted for the drive and that is used for the build directory.  

*** OSPath - 

	Paths to Safety, APC (if applicable), and Control OS images can be optionally included in "OSPaths" field. If used, 
	supply relative paths from the build path as supplied by the user or determined automatically from StackTrace.  Be
	sure to include all nodes, delimit with	commas (use quotes if there are any spaces), and order as Control, APC (if applicable),
	and then Safety. The defaults for Trima, Optia, and CES are "/vxboot,/trima/safety/boot", 
	"/../base/vxworks_target/control,/../base/vxworks_target/apc,/../base/vxworks_target/safety",
	and "/vxboot,/ces", respectively. For other systems, you must explicitly specify these strings.

*** Version - 

	Enter toolchain version.  In most cases, StackTrace can determine the device type and version, and a version need not
	be entered.  If not, select 2.02 to make use of "nm386.exe" and 2.2 for "nmpentium.exe", respectively, to extract
	symbols from object files. NOTE: You must have the location of these executables set in your system path (they are
	in the StackTrace installation directory).

*** OSFile -

	Enter name of OS images.  Examples include "vxWorks", "vxWorks_ampro", and "vxWorks_versalogic".  If not supplied, a name
	is attempted via the data log file.  Note that if neither "vxWorks_ampro" or "vxWorks_versalogic" was deduced in this
	manner, a default of "vxWorks" will be utilized. Currently, StackTrace cannot handle mixtures of these names.

*** Alias -

	This option allows you to change all or portions of folders or filename strings to something else. This is done prior to
	looking up any symbols, so this could be used, e.g., if your module names change or portions of your build tree hierarchy
	are located at separate locale.  Use the format "old:new" where "old" represents the old string and "new" represents
	the new string. NOTE: Be careful since string replacement occurs wherever it can.

*** Address -

	To print out symbols in the StackTrace trace at specific machine addresses for all nodes, enter a comma-delimited list of
	addresses here.


Saving and Loading Configurations:

Once values have been placed in the configuration textboxes, they can be saved in a xml-formatted file (with extension
.config) by clicking the "Save Config" button.  Multiple configuration files can be saved wherever you like. Similarly, 
previously saved configuration files can be loaded by clicking the "Load Config" button.  Note that your previous configuration
prior to hitting "Stack Trace" the last time you used will already be loaded. 


Right-click Startup:
 
StackTraceUI can be launched from any data log file by making use of the right-click menu. To enable this, go into "Tools" -> "Folder Options"
under any Windows Explorer and click on the "File Types" tab and select "DLOG". Then under the "Advanced" button, add a New action
using "<StackTrace installation directory>\StackTraceUI.exe" "%L" as the "Application" and your choice of "Action" (e.g., "Open
with StackTraceUI").  Alternatively, copy a shortcut icon to this "StackTraceUI.exe" into your "SendTo" folder located in your
"Documents and Settings". The latter enables you to right click a data log file and send it to a shortcut to "StackTraceUI.exe" 
(which can be renamed as "Open with StackTraceUI" or whatever you want).

  