// $Header: K:/BCT_Development/vxWorks/Common/tools/config_parse/unit_test/test1/rcs/test1_main.cpp 1.1 2005/05/11 15:15:03Z jl11312 Exp jl11312 $
//
// Main program for config_parse unit test 1
//
// $Log: test1_main.cpp $
// Revision 1.2  2005/01/24 16:01:26Z  jl11312
// - updated for new version of configuration parse utility
// Revision 1.1  2004/07/21 19:00:38Z  jl11312
// Initial revision
//

#include <vxWorks.h>
#include <unistd.h>
#include <sys/stat.h>
#include "test1.h"

static bool checkDouble(double d1, double d2)
{
	double low;
	double high;

	if ( d2 < 0 )
	{
		low = d2*1.00001;
		high = d2*0.99999;
	}
	else
	{
		low = d2*0.99999;
		high = d2*1.00001;
	}

	return ( d1 >= low && d1 <= high );
}

int test1_main(void)
{
	// Verify types of all generated config items.  Reference assignment will cause
	// a compiler error if any of the types are incorrect.
	//
	Test1::Test1_A_Data	data_a;
	Test1::Test1_B_Data	data_b;
	Test1::Test1_C_Data	data_c;

	Test1::TParam1 & ref1 = data_a.Section1.Param1;
	Test1::TParam2 & ref2 = data_a.Section1.Param2;
	bool & ref3 = data_a.Section2.Param3;
	bool & ref4 = data_a.Section2.Param4;
	const char *& ref5 = data_a.Section2.Param5;
	const char *& ref6 = data_a.Section2.Param6;
	long & ref7 = data_a.Section2.Param7;
	long & ref8 = data_a.Section2.Param8;
	long & ref9 = data_a.Section2.Param9;
	long & ref10 = data_a.Section2.Param10;
	long & ref11 = data_a.Section2.Param11;
	double & ref12 = data_a.Section2.Param12;
	double & ref13 = data_a.Section2.Param13;
	double & ref14 = data_a.Section2.Param14;
	double & ref15 = data_a.Section2.Param15;
	double & ref16 = data_a.Section2.Param16;
	Test1::TParam101 & ref17 = data_b.Section1.Param101;
	Test1::TParam201 & ref18 = data_c.Section1.Param201;

	// Check class names
	//
	fprintf(stderr, "Test 1.1 ...\n");
	if ( strcmp(Test1::Test1_A_Access.name(), "Test1_A") != 0 ||
		  strcmp(Test1::Test1_B_Access.name(), "Test1_B") != 0 ||
		  strcmp(Test1::Test1_C_Access.name(), "Test1_C") != 0 )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	// Check section names
	//
	fprintf(stderr, "Test 1.2 ...\n");
	if ( strcmp(Test1::Test1_A_Access.Section1.name(), "Section1") != 0 ||
		  strcmp(Test1::Test1_A_Access.Section2.name(), "Section2") != 0 ||
		  strcmp(Test1::Test1_B_Access.Section1.name(), "Section1") != 0 ||
		  strcmp(Test1::Test1_C_Access.Section1.name(), "Section1") != 0 )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	// Check parameter names
	//
	fprintf(stderr, "Test 1.3 ...\n");
	if ( strcmp(Test1::Test1_A_Access.Section1.Param1.name(), "Param1") != 0 ||
		  strcmp(Test1::Test1_A_Access.Section1.Param2.name(), "Param2") != 0 ||
		  strcmp(Test1::Test1_A_Access.Section2.Param3.name(), "Param3") != 0 ||
		  strcmp(Test1::Test1_A_Access.Section2.Param4.name(), "Param4") != 0 ||
		  strcmp(Test1::Test1_A_Access.Section2.Param5.name(), "Param5") != 0 ||
		  strcmp(Test1::Test1_A_Access.Section2.Param6.name(), "Param6") != 0 ||
		  strcmp(Test1::Test1_A_Access.Section2.Param7.name(), "Param7") != 0 ||
		  strcmp(Test1::Test1_A_Access.Section2.Param8.name(), "Param8") != 0 ||
		  strcmp(Test1::Test1_A_Access.Section2.Param9.name(), "Param9") != 0 ||
		  strcmp(Test1::Test1_A_Access.Section2.Param10.name(), "Param10") != 0 ||
		  strcmp(Test1::Test1_A_Access.Section2.Param11.name(), "Param11") != 0 ||
		  strcmp(Test1::Test1_A_Access.Section2.Param12.name(), "Param12") != 0 ||
		  strcmp(Test1::Test1_A_Access.Section2.Param13.name(), "Param13") != 0 ||
		  strcmp(Test1::Test1_A_Access.Section2.Param14.name(), "Param14") != 0 ||
		  strcmp(Test1::Test1_A_Access.Section2.Param15.name(), "Param15") != 0 ||
		  strcmp(Test1::Test1_A_Access.Section2.Param16.name(), "Param16") != 0 ||
		  strcmp(Test1::Test1_B_Access.Section1.Param101.name(), "Param101") != 0 ||
		  strcmp(Test1::Test1_C_Access.Section1.Param201.name(), "Param201") != 0 )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	// Check format versions
	//
	fprintf(stderr, "Test 1.4 ...\n");
	if ( strcmp(Test1::Test1_A_Access.formatVersion(), "1a") != 0 ||
		  strcmp(Test1::Test1_B_Access.formatVersion(), "1b") != 0 ||
		  strcmp(Test1::Test1_C_Access.formatVersion(), "1c") != 0 )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	// Check file read operation
	//
	fprintf(stderr, "Test 1.5 ...\n");
	if ( Test1::Test1_A_Access.readFile(&log_level_startup_info, &log_level_startup_error) != ConfigFile::ReadOK )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	fprintf(stderr, "Test 1.6 ...\n");
	if ( Test1::Test1_A().Section1.Param1 != Test1::enum1_id1 ||
		  Test1::Test1_A().Section1.Param2 != Test1::enum2_id1 )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	fprintf(stderr, "Test 1.7 ...\n");
	if ( Test1::Test1_A().Section2.Param3 != true ||
		  Test1::Test1_A().Section2.Param4 != false )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	fprintf(stderr, "Test 1.8 ...\n");
	if ( strcmp(Test1::Test1_A().Section2.Param5, "\tstring\n") != 0 ||
		  strcmp(Test1::Test1_A().Section2.Param6, "\"string \"with embedded quotes and comment # characters\"\"") != 0 )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	fprintf(stderr, "Test 1.9 ...\n");
	if ( Test1::Test1_A().Section2.Param7 != 1 ||
		  Test1::Test1_A().Section2.Param8 != -1 ||
		  Test1::Test1_A().Section2.Param9 != 0x33 ||
		  Test1::Test1_A().Section2.Param10 != 0x12345 ||
		  Test1::Test1_A().Section2.Param11 != -123456 )
	{
		fprintf(stderr, "test failed\n");
		return -1;	
	}
	fprintf(stderr, "test passed\n");

	fprintf(stderr, "Test 1.10 ...\n");
	if ( !checkDouble(Test1::Test1_A().Section2.Param12, 0.1) ||
		  !checkDouble(Test1::Test1_A().Section2.Param13, -0.1) ||
		  !checkDouble(Test1::Test1_A().Section2.Param14, 1e17) ||
		  !checkDouble(Test1::Test1_A().Section2.Param15, -1.0e2) ||
		  !checkDouble(Test1::Test1_A().Section2.Param16, 1.0e2) )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	// Test range check, validate check fails as expected
	//
	fprintf(stderr, "Test 1.11 ...\n");
	if ( Test1::Test1_D_Access.readFile(&log_level_startup_info, &log_level_startup_error) != ConfigFile::ReadFailed ||
		  Test1::Test1_E_Access.readFile(&log_level_startup_info, &log_level_startup_error) != ConfigFile::ReadFailed ||
		  Test1::Test1_F_Access.readFile(&log_level_startup_info, &log_level_startup_error) != ConfigFile::ReadFailed )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	fprintf(stderr, "Test 1.12 ...\n");
	if ( Test1::Test1_G_Access.readFile(&log_level_startup_info, &log_level_startup_error) != ConfigFile::ReadFailed ||
		  Test1::Test1_H_Access.readFile(&log_level_startup_info, &log_level_startup_error) != ConfigFile::ReadFailed ||
		  Test1::Test1_I_Access.readFile(&log_level_startup_info, &log_level_startup_error) != ConfigFile::ReadFailed )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	fprintf(stderr, "Test 1.13 ...\n");	
	if ( Test1::Test1_J_Access.readFile(&log_level_startup_info, &log_level_startup_error) != ConfigFile::ReadFailed )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	fprintf(stderr, "Test 1.14 ...\n");	
	if ( Test1::Test1_K_Access.readFile(&log_level_startup_info, &log_level_startup_error) != ConfigFile::ReadFailed )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	fprintf(stderr, "Test 1.15 ...\n");
	if ( Test1::Test1_L_Access.readFile(&log_level_startup_info, &log_level_startup_error) != ConfigFile::ReadFailed )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	// Cleanup files from previous test run (if any)
	//
	unlink(CONFIG_PATH "/file1_b.dat");
	unlink(CONFIG_PATH "/crc/file1_b.crc");
	unlink(CONFIG_PATH "/backup/file1_b.dat");
	unlink(CONFIG_PATH "/backup/crc/file1_b.crc");

	// Test reading file with CRC protection
	//
	FILE * fp = fopen(CONFIG_PATH "/file1_b.dat", "w");
	fprintf(fp, "[Version]\n");
	fprintf(fp, "FormatVersion=\"1b\"\n");
	fprintf(fp, "DataVersion=\"d1b\"\n");
	fprintf(fp, "[Section1]\n");
	fprintf(fp, "Param101=enum3_id3\n");
	fclose(fp);

	mkdir(CONFIG_PATH "/crc");
	fp = fopen(CONFIG_PATH "/crc/file1_b.crc", "w");
	fprintf(fp, "0x286e0d7f\n");
	fclose(fp);

	fprintf(stderr, "Test 1.16 ...\n");
	if ( Test1::Test1_B_Access.readFile(&log_level_startup_info, &log_level_startup_error) != ConfigFile::ReadOK )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	// Test CRC check
	//
	fp = fopen(CONFIG_PATH "/crc/file1_b.crc", "w");
	fprintf(fp, "0x286e0df7\n");	// incorrect CRC
	fclose(fp);

	fprintf(stderr, "Test 1.17 ...\n");
	if ( Test1::Test1_B_Access.readFile(&log_level_startup_info, &log_level_startup_error) != ConfigFile::ReadFailed )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	// Test use of backup file
	//
	mkdir(CONFIG_PATH "/backup");
	fp = fopen(CONFIG_PATH "/backup/file1_b.dat", "w");
	fprintf(fp, "[Version]\n");
	fprintf(fp, "FormatVersion=\"1b\"\n");
	fprintf(fp, "DataVersion=\"d1b\"\n");
	fprintf(fp, "[Section1]\n");
	fprintf(fp, "Param101=enum3_id1\n");
	fclose(fp);

	mkdir(CONFIG_PATH "/backup/crc");
	fp = fopen(CONFIG_PATH "/backup/crc/file1_b.crc", "w");
	fprintf(fp, "0x1a586ffd\n");
	fclose(fp);

	fprintf(stderr, "Test 1.18 ...\n");
	if ( Test1::Test1_B_Access.readFile(&log_level_startup_info, &log_level_startup_error) != ConfigFile::ReadBackupOK )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	// Test format version check
	//
	unlink(CONFIG_PATH "/backup/file1_b.dat");

	fp = fopen(CONFIG_PATH "/file1_b.dat", "w");
	fprintf(fp, "[Version]\n");
	fprintf(fp, "FormatVersion=\"1c\"\n");
	fprintf(fp, "DataVersion=\"d1b\"\n");
	fprintf(fp, "[Section1]\n");
	fprintf(fp, "Param101=enum3_id3\n");
	fclose(fp);

	fp = fopen(CONFIG_PATH "/crc/file1_b.crc", "w");
	fprintf(fp, "0xe250feec\n");
	fclose(fp);

	fprintf(stderr, "Test 1.19 ...\n");
	if ( Test1::Test1_B_Access.readFile(&log_level_startup_info, &log_level_startup_error) != ConfigFile::ReadFailed )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	// Test setting parameters for read/write files
	//
	fprintf(stderr, "Test 1.20 ...\n");
	if ( !Test1::Test1_C_Access.Version.FormatVersion.set(Test1::Test1_C_Access.formatVersion()) ||
		  !Test1::Test1_C_Access.Version.DataVersion.set("1") ||
		  !Test1::Test1_C_Access.Section1.Param201.set(Test1::enum4_id1) ||
		  !Test1::Test1_C_Access.Section2.Param1.set(1) ||
		  !Test1::Test1_C_Access.Section2.Param2.set(-1) ||
		  !Test1::Test1_C_Access.Section2.Param3.set(-1) ||
		  !Test1::Test1_C_Access.Section2.Param4.set(-1) ||
		  !Test1::Test1_C_Access.Section2.Param5.set(2) ||
		  !Test1::Test1_C_Access.Section2.Param6.set(2) )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	// Test file write
	//
	fprintf(stderr, "Test 1.21 ...\n");
	if ( Test1::Test1_C_Access.writeFile(&log_level_startup_info, &log_level_startup_error) != ConfigFile::WriteOK )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	// Test file read-back
	//
	Test1::Test1_C_Access.Version.FormatVersion.set("0");
	Test1::Test1_C_Access.Version.DataVersion.set("0");
	Test1::Test1_C_Access.Section1.Param201.set(Test1::enum4_id2);
	Test1::Test1_C_Access.Section2.Param1.set(2);
	Test1::Test1_C_Access.Section2.Param2.set(-2);
	Test1::Test1_C_Access.Section2.Param3.set(-2);
	Test1::Test1_C_Access.Section2.Param4.set(0);
	Test1::Test1_C_Access.Section2.Param5.set(1);
	Test1::Test1_C_Access.Section2.Param6.set(1);

	fprintf(stderr, "Test 1.22 ...\n");
	if ( Test1::Test1_C_Access.readFile(&log_level_startup_info, &log_level_startup_error) != ConfigFile::ReadOK ||
		  strcmp(Test1::Test1_C().Version.FormatVersion, "1c") != 0 ||
		  strcmp(Test1::Test1_C().Version.DataVersion, "1") != 0 ||
		  Test1::Test1_C().Section1.Param201 != Test1::enum4_id1 ||
		  Test1::Test1_C().Section2.Param1 != 1 ||
		  Test1::Test1_C().Section2.Param2 != -1 ||
		  Test1::Test1_C().Section2.Param3 != -1 ||
		  Test1::Test1_C().Section2.Param4 != -1 ||
		  Test1::Test1_C().Section2.Param5 != 2 ||
		  Test1::Test1_C().Section2.Param6 != 2 )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	// Test file backup
	//
	Test1::Test1_C_Access.Version.FormatVersion.set("0");
	Test1::Test1_C_Access.Version.DataVersion.set("0");
	Test1::Test1_C_Access.Section1.Param201.set(Test1::enum4_id2);
	Test1::Test1_C_Access.Section2.Param1.set(2);
	Test1::Test1_C_Access.Section2.Param2.set(-2);
	Test1::Test1_C_Access.Section2.Param3.set(-2);
	Test1::Test1_C_Access.Section2.Param4.set(0);
	Test1::Test1_C_Access.Section2.Param5.set(1);
	Test1::Test1_C_Access.Section2.Param6.set(1);

	fprintf(stderr, "Test 1.23 ...\n");
	if ( Test1::Test1_C_Access.writeFile(&log_level_startup_info, &log_level_startup_error) != ConfigFile::WriteOK )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	fprintf(stderr, "Test 1.24 ...\n");
	unlink(CONFIG_PATH "/file1_c.dat");
	if ( Test1::Test1_C_Access.readFile(&log_level_startup_info, &log_level_startup_error) != ConfigFile::ReadBackupOK ||
		  strcmp(Test1::Test1_C().Version.FormatVersion, "1c") != 0 ||
		  strcmp(Test1::Test1_C().Version.DataVersion, "1") != 0 ||
		  Test1::Test1_C().Section1.Param201 != Test1::enum4_id1 ||
		  Test1::Test1_C().Section2.Param1 != 1 ||
		  Test1::Test1_C().Section2.Param2 != -1 ||
		  Test1::Test1_C().Section2.Param3 != -1 ||
		  Test1::Test1_C().Section2.Param4 != -1 ||
		  Test1::Test1_C().Section2.Param5 != 2 ||
		  Test1::Test1_C().Section2.Param6 != 2 )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	// Test range functions
	//
	fprintf(stderr, "Test 1.25 ...\n");
	long	min, max;
	if ( !Test1::Test1_C_Access.Section2.Param1.getRange(min, max) ||
		  min != 1 ||
		  max != 2 ||
        !Test1::Test1_C_Access.Section2.Param1.set(1) ||
        !Test1::Test1_C_Access.Section2.Param1.set(2) ||
        Test1::Test1_C_Access.Section2.Param1.set(0) ||
        Test1::Test1_C_Access.Section2.Param1.set(3) ||
        Test1::Test1_C_Access.Section2.Param1.set(-1) )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	fprintf(stderr, "Test 1.26 ...\n");
	if ( !Test1::Test1_C_Access.Section2.Param2.getRange(min, max) ||
		  min != -2 ||
		  max != -1 ||
        !Test1::Test1_C_Access.Section2.Param2.set(-1) ||
        !Test1::Test1_C_Access.Section2.Param2.set(-2) ||
        Test1::Test1_C_Access.Section2.Param2.set(0) ||
        Test1::Test1_C_Access.Section2.Param2.set(1) ||
        Test1::Test1_C_Access.Section2.Param2.set(-3) )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	fprintf(stderr, "Test 1.27 ...\n");
	if ( Test1::Test1_C_Access.Section2.Param3.getRange(min, max) ||
        !Test1::Test1_C_Access.Section2.Param3.set(-1) ||
        !Test1::Test1_C_Access.Section2.Param3.set(-2) ||
        !Test1::Test1_C_Access.Section2.Param3.set(0) ||
        !Test1::Test1_C_Access.Section2.Param3.set(1) ||
        !Test1::Test1_C_Access.Section2.Param3.set(-3) )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	fprintf(stderr, "Test 1.28 ...\n");
	if ( !Test1::Test1_C_Access.Section2.Param4.getRange(min, max) ||
		  min != -1 ||
		  max != 1 ||
        !Test1::Test1_C_Access.Section2.Param4.set(-1) ||
        !Test1::Test1_C_Access.Section2.Param4.set(0) ||
        Test1::Test1_C_Access.Section2.Param4.set(1) ||
        Test1::Test1_C_Access.Section2.Param4.set(2) ||
        Test1::Test1_C_Access.Section2.Param4.set(-2) )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	fprintf(stderr, "Test 1.29 ...\n");
	if ( !Test1::Test1_C_Access.Section2.Param5.getRange(min, max) ||
		  min != -2 ||
		  max != 2 ||
        !Test1::Test1_C_Access.Section2.Param5.set(1) ||
        !Test1::Test1_C_Access.Section2.Param5.set(2) ||
        Test1::Test1_C_Access.Section2.Param5.set(3) ||
        Test1::Test1_C_Access.Section2.Param5.set(0) ||
        Test1::Test1_C_Access.Section2.Param5.set(-1) ||
        Test1::Test1_C_Access.Section2.Param5.set(-3) )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	fprintf(stderr, "Test 1.30 ...\n");
	if ( Test1::Test1_C_Access.Section2.Param6.getRange(min, max) ||
        !Test1::Test1_C_Access.Section2.Param6.set(1) ||
        !Test1::Test1_C_Access.Section2.Param6.set(2) ||
        !Test1::Test1_C_Access.Section2.Param6.set(3) ||
        Test1::Test1_C_Access.Section2.Param6.set(0) ||
        Test1::Test1_C_Access.Section2.Param6.set(-1) ||
        Test1::Test1_C_Access.Section2.Param6.set(-3) )
	{
		fprintf(stderr, "test failed\n");
		return -1;
	}
	fprintf(stderr, "test passed\n");

	fprintf(stderr, "All Tests passed\n");
	return 0;
}

