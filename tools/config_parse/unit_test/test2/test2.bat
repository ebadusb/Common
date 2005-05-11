@echo off

rem $Header: K:/BCT_Development/Taos/tools/config_parse/unit_test/test2/rcs/test2.bat 1.1 2005/01/25 17:53:14Z jl11312 Exp jl11312 $
rem $Log: test2.bat $
rem Revision 1.1  2005/01/25 17:53:14Z  jl11312
rem Initial revision

rm -rf tmp_data > nul
echo Parsing configuration files
..\..\release\config_parse -project taos -output Test2 -data_file_dir tmp_data test2_a test2_b test2_c test2_d test2_e test2_f test2_g test2_h test2_i test2_j test2_k test2_l test2_m
if errorlevel 1 goto fail

echo.
echo Check data file generation
echo The following files should have been generated:
echo - tmp_data\taos\file2_d.dat
echo - tmp_data\taos\file2_e.dat
echo - tmp_data\taos\file2_f.dat
echo - tmp_data\taos\file2_g.dat
echo - tmp_data\taos\file2_h.dat
echo - tmp_data\taos\file2_i.dat
echo - tmp_data\taos\file2_j.dat
echo - tmp_data\taos\file2_k.dat
echo - tmp_data\taos\file2_l.dat
echo - tmp_data\taos\file2_m.dat
echo - tmp_data\taos\subdir1a\subdir2\subdir3\file2_a.dat
echo - tmp_data\taos\subdir1b\subdir2\subdir3\file2_c.dat

echo.
echo Directory listing
dir /s/b/a-d tmp_data

echo.
echo.
echo The following should show only Param1 with a set function
grep "Test2::_C_Test2_M::_C_Section1::.*::set" test2.cpp

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
echo Unit Test 2 phase 1 complete - run test application to complete tests
goto done

:fail
echo Processing failed

:done

