@echo off
rem
rem create_common.bat - Rebuild Common under vxWorks
rem
rem $Header: //bctquad3/home/BCT_Development/vxWorks/Common/rcs/create_common.bat 1.4 2003/01/31 21:45:52Z ms10234 Exp ms10234 $
rem
rem $Log: create_common.bat $
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
