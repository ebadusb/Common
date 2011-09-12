About Datalog Downloader - A simple script to download and launch dlog files from the quad. 

You'll find all the source files necessary to create a distribution in this folder. To create a distribution, 
	Make sure you have the following installed - Python, py2exe and NSIS installer by nullsoft.
	From the command line prompt, type python setup.py py2exe. Now you'll have a 'dist' folder and a 'build' folder under your current directory. The 'dist' folder will contain the dlog.exe executable along with other dlls and files required for program execution. Make sure the right vcredist_x86.exe package is redistributed. See http://www.py2exe.org/index.cgi/Tutorial. Copy the appropriate vcredist_x86.exe to the dist folder.
	Now launch the NSIS executable and compile the setup.nsi script to create DatalogDownloadSetup.exe. You can then distribute this executable. 

Usage - 
	command line: dlog.exe dlog_name
	iexplore: highlight dlog, right click and select 'Dlog Download' from the popup menu. 
	Any other program: highlight the dlog, copy it. Then hit 'CTRL|ALT|D'.

Configuration - Use the config.ini file to control the software settings.
