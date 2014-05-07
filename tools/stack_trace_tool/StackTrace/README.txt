StackTraceUI v1.6 (GUI version of Stack Trace Tool)

$ Log: StackTraceUI $
Revision 1.6  2014/05/07  chowusb
- added support for multiple OS image names
Revision 1.5  2014/04/17  chowusb
- fixed problems with page fault information from Tornado 2.02 builds
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

The StackTraceUI wraps the command line-driven Stack Trace Tool so that commonly used parameters need not be rewritten every
time, but can be saved and used at a later time.  This README file documents the behavior of all configuration parameters, how
they should be supplied, and how they can be saved and reloaded for later use. 


Command Line Usage: 

 [-location buildpath] [-version (2.02 | 2.2)] [-osfile filename] [-ospath pathnames] [-pathalias old:new] ...
 ... [-address values] dlogFileName

The command-line version of the Stack Trace Tool is still available and can be entered in the data log file textbox. Note that 
any configuration parameters supplied under "StackTraceUI Configuration" will override their respective parameter in the data 
log textbox.


Data Log Textbox:

*** Data Log File Name *** 

Enter the file name of (optionally, the full path to) the data log, or use "File" -> "Open" menu to browse for data logs. Setting
up a right-click shortcut (described below), however, can be used to automatically fill in the data log textbox. Otherwise, a 
copy-to-clipboard can be used to pre-fill the textbox.


Configuration TextBoxes:

*** Location *** 

Enter an optional build location (must be full path). Usually, this is a path to a "current_build" or similarly named directory. 
Be sure to include "current_build" or the similarly named directory in the path. If a location is left unsupplied, the build 
directory in the data log file is used by appending "current_build" to it. If unsuccessful, "\\bctquad3\home" is substituted for
the drive as a last ditch effort.  

*** OSPath *** 

Paths to Safety, APC (if applicable), and Control OS images can be optionally included in "ospath" field. Supply relative paths
from the build directory, which is determined by the "location" field or what the StackTraceUI has determined automatically by 
default. In addition, delimit the paths with commas (include quotes if there are any spaces), and put in the order: Control, APC
(if applicable), and Safety. Defaults used for Trima, Optia, and CES are:

	Trima:   "/vxboot,/trima/safety/boot" 
	Optia:   "/../base/vxworks_target/control,/../base/vxworks_target/apc,/../base/vxworks_target/safety"
	CES:     "/vxboot,/ces" 

respectively. For other systems, you must explicitly specify these strings.

*** Version *** 

Enter toolchain version.  In most cases, StackTraceUI can determine the device type and version, and a version need not be 
entered. Otherwise, enter 2.02 to use "objdump386.exe" or 2.2 to use "nmpentium.exe", respectively, for symbol lookup. 

*** OSFile ***

Enter the OS image name. Examples include "vxWorks", "vxWorks_ampro", "vxWorks_versalogic", etc. If left unspecified, a name
is attempted from the data log. Note that if neither "vxWorks_ampro" nor "vxWorks_versalogic" can be deduced, "vxWorks" is used
as a default name. StackTraceUI can now handle multiple OS image names. To use this feature, pass in a comma-delimited list of OS
image names with the same ordering as is intended for the OS paths. One example may be "vxWorks_versalogic,vxWorks_versalogic_pxe",
which references an OS image named "vxWorks_versalogic" in /vxboot and "vxWorks_versalogic_pxe" in /trima/safety/boot in the case
of a Trima data log.

*** Pathalias ***

This option allows you to alias folder and/or file names. Aliasing is done before symbol lookup, so this could be used, e.g., to
redistribute module locations if your build tree hierarchy differs from the machine directory hierarchy. Use the format "old:new"
where "new" is an alias for "old". NOTE: This is a deprecated feature and much of what this configuration parameter provides
is already handled by the StackTraceUI.

*** Address ***

Enter a comma-delimited list of hex addresses for symbol lookup (done for each node).


Saving and Loading Configurations:

Once configuration values have been placed in their textboxes, they can be saved to a XML file (with extension .config) by 
clicking the "Save Config" button.  Multiple configuration files can be saved. Likewise, previously saved configuration files can
be loaded by clicking the "Load Config" button. Note that the StackTraceUI will remember the last-used configurations from your
last session upon boot up. 


Right-click Startup for Windows XP:
 
StackTraceUI can be launched directly from any data log file with a right-click shortcut. To enable this feature, go into "Tools"
-> "Folder Options" under any Windows Explorer and click on the "File Types" tab and select "DLOG". Then, under the "Advanced" 
button, add a New action with "<StackTrace installation directory>\StackTraceUI.exe" "%L" as the "Application" and your choice 
of "Action" (e.g., "Open with StackTraceUI"). Alternatively, copy a shortcut icon to this "StackTraceUI.exe" into your "SendTo"
folder located in your "Documents and Settings". This enables you to right click a data log file and send it to a shortcut to 
"StackTraceUI.exe" (which, again, can be named to something like "Open with StackTraceUI").

  