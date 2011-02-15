@echo off
rem
rem create_common.bat - Rebuild Common under vxWorks
rem
rem $Header$
rem
rem $Log: create_common.bat $
rem Revision 1.5  2003/02/04 16:00:30Z  ms10234
rem readded the latest_common_build_dir.mk file creation step
rem Revision 1.4  2003/01/31 21:45:52Z  ms10234
rem temporarily removed latest_common_build_dir file
rem Revision 1.3  2003/01/31 19:17:18Z  ms10234
rem added support for latest_common_build_dir.mk
rem Revision 1.2  2002/08/15 21:00:52  jl11312
rem - added date/time stamp
rem Revision 1.1  2002/04/30 20:07:22  rm70006
rem Initial revision
rem

if not exist .\makefile.vxc goto not_common_sandbox
if not exist .\create_common.bat goto not_common_sandbox
if not exist .\convert goto not_common_sandbox
if not exist .\router goto not_common_sandbox
if not exist .\socket++ goto not_common_sandbox
if not exist .\softcrc goto not_common_sandbox

if not defined WIND_HOST_TYPE goto find_tornado
if not defined WIND_BASE goto find_tornado
goto tornado_ok

:tornado_ok
grep -q "_VERSION *\"5.4" %WIND_BASE%\target\h\version.h
if ERRORLEVEL 1 goto check_55

echo vxWorks version 5.4.x

if exist makefile.flags (
    attrib -r makefile.flags
    del makefile.flags
)

copy makefile.flags_54 makefile.flags
attrib +r makefile.flags
goto start_build

:check_55
grep -q "_VERSION *\"5.5" %WIND_BASE%\target\h\version.h
if ERRORLEVEL 1 goto unknown_version

echo vxWorks version 5.5.x

if exist makefile.flags (
    attrib -r makefile.flags
    del makefile.flags
)

copy makefile.flags_55 makefile.flags
attrib +r makefile.flags
goto start_build

:unknown_version
echo Unknown vxWorks version
goto end_of_script

:start_build
echo Start of common build
date/t
time/t
make -f makefile.vx %*
if ERRORLEVEL 1 goto end_of_script
         
make -f makefile.vx %* latest_common_build_dir.mk
date/t
time/t
echo Common build completed successfully
goto end_of_script

:find_tornado
if exist c:\tornado\host\x86-win32\bin\torVars.bat goto tornado_on_c
if exist d:\tornado\host\x86-win32\bin\torVars.bat goto tornado_on_d
goto no_tornado

:tornado_on_c
call c:\tornado\host\x86-win32\bin\torVars.bat
goto tornado_ok

:tornado_on_d
call d:\tornado\host\x86-win32\bin\torVars.bat
goto tornado_ok

:not_common_sandbox
echo create_common must be run from the base common sandbox directory (duh!)
goto end_of_script

:no_tornado
echo Unable to locate the Tornado installation directory.
echo The file torVars.bat from the Tornado host tools must be run to
echo properly set the build environment
goto end_of_script

:end_of_script
