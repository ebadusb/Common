@echo off

rem $Header$
rem $Log: test1.bat $
rem Revision 1.1  2005/05/11 16:12:04Z  jl11312
rem Initial revision
rem Revision 1.2  2005/01/21 16:49:39Z  jl11312
rem - added new test cases
rem Revision 1.1  2004/07/20 19:33:42Z  jl11312
rem Initial revision

rm -rf tmp_data > nul
echo Parsing configuration files
..\..\release\config_parse -project taos -file_names .. -config_helper .. -output Test1 -export_output test1_enum.h -data_file_dir tmp_data test1_a test1_b test1_c test1_d test1_e test1_f test1_g test1_h test1_i test1_j test1_k test1_l test1_m test1_n test1_o test1_p
if errorlevel 1 goto fail

echo.
echo Check data file generation
echo The following files should have been generated:
echo - tmp_data\taos\file1_d.dat
echo - tmp_data\taos\file1_e.dat
echo - tmp_data\taos\file1_f.dat
echo - tmp_data\taos\file1_g.dat
echo - tmp_data\taos\file1_h.dat
echo - tmp_data\taos\file1_i.dat
echo - tmp_data\taos\file1_j.dat
echo - tmp_data\taos\file1_k.dat
echo - tmp_data\taos\file1_l.dat
echo - tmp_data\taos\file1_m.dat
echo - tmp_data\taos\file1_o.dat
echo - tmp_data\taos\subdir1a\subdir2\subdir3\file1_a.dat
echo - tmp_data\taos\subdir1b\subdir2\subdir3\file1_c.dat

echo.
echo Directory listing
dir /s/b/a-d tmp_data

echo.
echo.
echo The following should show only Param1 with a set function
grep "Test1::_C_Test1_M::_C_Section1::.*::set" test1.cpp

echo.
echo.
echo The following should show only the TParam0 enum defined
grep "enum.*;" test1_enum.h

echo.
echo.
mkdir c:\taos_sim > nul 2>&1
mkdir c:\taos_sim\taos > nul 2>&1
xcopy /S /Y tmp_data\taos\*.* c:\taos_sim\taos > nul 2>&1

echo Building generated code
make -f makefile.vx CPU=SIMNT
if errorlevel 1 goto fail

echo.
echo.
echo Unit Test 1 phase 1 complete - run test application to complete tests
goto done

:fail
echo Processing failed

:done

