@echo off
rem
rem create_common.bat - Rebuild Common under vxWorks
rem
rem $Header: //bctquad3/home/BCT_Development/vxWorks/Common/rcs/create_common.bat 1.2 2002/08/15 21:00:52Z jl11312 Exp ms10234 $
rem
rem $Log: create_common.bat $
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
make -f makefile.vx %*
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