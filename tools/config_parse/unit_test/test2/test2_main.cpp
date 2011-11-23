// $Header$
//
// Main program for config_parse unit test 2
//
// $Log: test2_main.cpp $
// Revision 1.1  2005/01/25 20:54:38Z  jl11312
// Initial revision
//

#include <vxWorks.h>
#include <unistd.h>
#include <sys/stat.h>
#include "test2.h"

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

int test2_main(void)
{
   // Verify types of all generated config items.  Reference assignment will cause
   // a compiler error if any of the types are incorrect.
   //
   Test2::Test2_A_Data	data_a;
   Test2::Test2_B_Data	data_b;
   Test2::Test2_C_Data	data_c;

   Test2::TParam1 & ref1 = data_a.Section1.Param1[0];
   Test2::TParam2 & ref2 = data_a.Section1.Param2[0];
   bool & ref3 = data_a.Section2.Param3[0];
   const char *& ref4 = data_a.Section2.Param4[0];
   long & ref5 = data_a.Section2.Param5[0];
   long & ref6 = data_a.Section2.Param6[0];
   long & ref7 = data_a.Section2.Param7[0];
   long & ref8 = data_a.Section2.Param8[0];
   double & ref9 = data_a.Section2.Param9[0];
   double & ref10 = data_a.Section2.Param10[0];
   double & ref11 = data_a.Section2.Param11[0];
   double & ref12 = data_a.Section2.Param12[0];
   double & ref13 = data_a.Section2.Param13[0];
   Test2::TParam101 & ref14 = data_b.Section1.Param101[0];
   Test2::TParam201 & ref15 = data_c.Section1.Param201[0];

   // Check class names
   //
   fprintf(stderr, "Test 2.1 ...\n");
   if ( strcmp(Test2::Test2_A_Access.name(), "Test2_A") != 0 ||
         strcmp(Test2::Test2_B_Access.name(), "Test2_B") != 0 ||
         strcmp(Test2::Test2_C_Access.name(), "Test2_C") != 0 )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   // Check section names
   //
   fprintf(stderr, "Test 2.2 ...\n");
   if ( strcmp(Test2::Test2_A_Access.Section1.name(), "Section1") != 0 ||
         strcmp(Test2::Test2_A_Access.Section2.name(), "Section2") != 0 ||
         strcmp(Test2::Test2_B_Access.Section1.name(), "Section1") != 0 ||
         strcmp(Test2::Test2_C_Access.Section1.name(), "Section1") != 0 )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   // Check parameter names
   //
   fprintf(stderr, "Test 2.3 ...\n");
   if ( strcmp(Test2::Test2_A_Access.Section1.Param1[0].name(), "Param1[0]") != 0 ||
         strcmp(Test2::Test2_A_Access.Section1.Param1[1].name(), "Param1[1]") != 0 ||
         strcmp(Test2::Test2_A_Access.Section1.Param2[0].name(), "Param2[0]") != 0 ||
         strcmp(Test2::Test2_A_Access.Section1.Param2[1].name(), "Param2[1]") != 0 ||
         strcmp(Test2::Test2_A_Access.Section2.Param3[0].name(), "Param3[0]") != 0 ||
         strcmp(Test2::Test2_A_Access.Section2.Param4[0].name(), "Param4[0]") != 0 ||
         strcmp(Test2::Test2_A_Access.Section2.Param5[0].name(), "Param5[0]") != 0 ||
         strcmp(Test2::Test2_A_Access.Section2.Param6[0].name(), "Param6[0]") != 0 ||
         strcmp(Test2::Test2_A_Access.Section2.Param7[0].name(), "Param7[0]") != 0 ||
         strcmp(Test2::Test2_A_Access.Section2.Param8[0].name(), "Param8[0]") != 0 ||
         strcmp(Test2::Test2_A_Access.Section2.Param9[0].name(), "Param9[0]") != 0 ||
         strcmp(Test2::Test2_A_Access.Section2.Param10[0].name(), "Param10[0]") != 0 ||
         strcmp(Test2::Test2_A_Access.Section2.Param11[0].name(), "Param11[0]") != 0 ||
         strcmp(Test2::Test2_A_Access.Section2.Param12[0].name(), "Param12[0]") != 0 ||
         strcmp(Test2::Test2_A_Access.Section2.Param13[0].name(), "Param13[0]") != 0 ||
         strcmp(Test2::Test2_B_Access.Section1.Param101[0].name(), "Param101[0]") != 0 ||
         strcmp(Test2::Test2_B_Access.Section1.Param101[0].name(), "Param101[0]") != 0 ||
         strcmp(Test2::Test2_B_Access.Section2.Param[0].name(), "Param[0]") != 0 ||
         strcmp(Test2::Test2_B_Access.Section2.Param[15].name(), "Param[15]") != 0 ||
         strcmp(Test2::Test2_B_Access.Section3.Param.name(), "Param") != 0 ||
         strcmp(Test2::Test2_C_Access.Section1.Param201[0].name(), "Param201[0]") != 0 )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   // Check format versions
   //
   fprintf(stderr, "Test 2.4 ...\n");
   if ( strcmp(Test2::Test2_A_Access.formatVersion(), "2a") != 0 ||
         strcmp(Test2::Test2_B_Access.formatVersion(), "2b") != 0 ||
         strcmp(Test2::Test2_C_Access.formatVersion(), "2c") != 0 )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   // Check file read operation
   //
   fprintf(stderr, "Test 2.5 ...\n");
   if ( Test2::Test2_A_Access.readFile(&log_level_config_data_info, &log_level_config_data_error) != ConfigFile::ReadOK )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   fprintf(stderr, "Test 2.6 ...\n");
   if ( Test2::Test2_A().Section1.Param1[0] != Test2::enum1_id1 ||
         Test2::Test2_A().Section1.Param1[1] != Test2::enum1_id2 ||
         Test2::Test2_A().Section1.Param2[0] != Test2::enum2_id2 ||
         Test2::Test2_A().Section1.Param2[1] != Test2::enum2_id1 )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   fprintf(stderr, "Test 2.7 ...\n");
   if ( Test2::Test2_A().Section2.Param3[0] != true ||
         Test2::Test2_A().Section2.Param3[1] != false )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   fprintf(stderr, "Test 2.8 ...\n");
   if ( strcmp(Test2::Test2_A().Section2.Param4[0], "\tstring\n") != 0 ||
         strcmp(Test2::Test2_A().Section2.Param4[1], "\"string \"with embedded quotes and comment # characters\"\"") != 0 )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   fprintf(stderr, "Test 2.9 ...\n");
   if ( Test2::Test2_A().Section2.Param5[0] != 1 ||
         Test2::Test2_A().Section2.Param5[1] != -1 ||
         Test2::Test2_A().Section2.Param6[0] != 0x33 ||
         Test2::Test2_A().Section2.Param6[1] != 0 ||
         Test2::Test2_A().Section2.Param7[0] != 0x12345 ||
         Test2::Test2_A().Section2.Param7[1] != 0 ||
         Test2::Test2_A().Section2.Param8[0] != -123456 ||
         Test2::Test2_A().Section2.Param8[1] != -1 )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   fprintf(stderr, "Test 2.10 ...\n");
   if ( !checkDouble(Test2::Test2_A().Section2.Param10[0], -0.1) ||
         !checkDouble(Test2::Test2_A().Section2.Param10[1], 0.1) ||
         !checkDouble(Test2::Test2_A().Section2.Param11[0], 1e17) ||
         !checkDouble(Test2::Test2_A().Section2.Param11[1], -1e25) ||
         !checkDouble(Test2::Test2_A().Section2.Param12[0], -1.0e2) ||
         !checkDouble(Test2::Test2_A().Section2.Param12[1], -1.0e-5) ||
         !checkDouble(Test2::Test2_A().Section2.Param13[0], 1.0e2) ||
         !checkDouble(Test2::Test2_A().Section2.Param13[1], 1.0e-2) )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   // Test range check, validate check fails as expected
   //
   fprintf(stderr, "Test 2.11 ...\n");
   if ( Test2::Test2_D_Access.readFile(&log_level_config_data_info, &log_level_config_data_error) != ConfigFile::ReadFailed ||
         Test2::Test2_E_Access.readFile(&log_level_config_data_info, &log_level_config_data_error) != ConfigFile::ReadFailed ||
         Test2::Test2_F_Access.readFile(&log_level_config_data_info, &log_level_config_data_error) != ConfigFile::ReadFailed )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   fprintf(stderr, "Test 2.12 ...\n");
   if ( Test2::Test2_G_Access.readFile(&log_level_config_data_info, &log_level_config_data_error) != ConfigFile::ReadFailed ||
         Test2::Test2_H_Access.readFile(&log_level_config_data_info, &log_level_config_data_error) != ConfigFile::ReadFailed ||
         Test2::Test2_I_Access.readFile(&log_level_config_data_info, &log_level_config_data_error) != ConfigFile::ReadFailed )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   fprintf(stderr, "Test 2.13 ...\n");
   if ( Test2::Test2_J_Access.readFile(&log_level_config_data_info, &log_level_config_data_error) != ConfigFile::ReadFailed )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   fprintf(stderr, "Test 2.14 ...\n");
   if ( Test2::Test2_K_Access.readFile(&log_level_config_data_info, &log_level_config_data_error) != ConfigFile::ReadFailed )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   fprintf(stderr, "Test 2.15 ...\n");
   if ( Test2::Test2_L_Access.readFile(&log_level_config_data_info, &log_level_config_data_error) != ConfigFile::ReadFailed )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   // Cleanup files from previous test run (if any)
   //
   unlink(CONFIG_PATH "/file2_b.dat");
   unlink(CONFIG_PATH "/crc/file2_b.crc");
   unlink(CONFIG_PATH "/backup/file2_b.dat");
   unlink(CONFIG_PATH "/backup/crc/file2_b.crc");

   // Test reading file with CRC protection
   //
   FILE * fp = fopen(CONFIG_PATH "/file2_b.dat", "w");
   fprintf(fp, "[Version]\n");
   fprintf(fp, "FormatVersion=\"2b\"\n");
   fprintf(fp, "DataVersion=\"d2b\"\n");
   fprintf(fp, "[FileInfo]\n");
   fprintf(fp, "ReadOnly=true\n");
   fprintf(fp, "FileName=\"/config/file2_b.dat\"\n");
   fprintf(fp, "[Section1]\n");
   fprintf(fp, "Param101[0]=enum3_id3\n");
   fprintf(fp, "[Section2]\n");
   for ( int i=0; i<16; i++ ) fprintf(fp, "Param[%d]=%d\n", i^0x5, i^0x9);
   fprintf(fp, "[Section3]\n");
   fprintf(fp, "Param=2\n");
   fclose(fp);

   mkdir(CONFIG_PATH "/crc");
   fp = fopen(CONFIG_PATH "/crc/file2_b.crc", "w");
   fprintf(fp, "0x2096f1e4\n");
   fclose(fp);

   fprintf(stderr, "Test 2.16 ...\n");

   if ( Test2::Test2_B_Access.readFile(&log_level_config_data_info, &log_level_config_data_error) != ConfigFile::ReadOK )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   else
   {
      bool contentsOK = ( Test2::Test2_B().Section1.Param101[0] == Test2::enum3_id3 );

      for ( int i=0; i<16; i++ ) contentsOK = contentsOK && ( Test2::Test2_B().Section2.Param[i] == (i^5)^9 );

      contentsOK = contentsOK && ( Test2::Test2_B().Section3.Param == 2 );

      if ( !contentsOK )
      {
         fprintf(stderr, "test failed\n");
         return -1;
      }
   }
   fprintf(stderr, "test passed\n");

   // Test CRC check
   //
   fp = fopen(CONFIG_PATH "/crc/file2_b.crc", "w");
   fprintf(fp, "0x1f43d96e\n");	// incorrect CRC
   fclose(fp);

   fprintf(stderr, "Test 2.17 ...\n");
   if ( Test2::Test2_B_Access.readFile(&log_level_config_data_info, &log_level_config_data_error) != ConfigFile::ReadFailed )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   // Test use of backup file
   //
   mkdir(CONFIG_PATH "/backup");
   fp = fopen(CONFIG_PATH "/backup/file2_b.dat", "w");
   fprintf(fp, "[Version]\n");
   fprintf(fp, "FormatVersion=\"2b\"\n");
   fprintf(fp, "DataVersion=\"d2b\"\n");
   fprintf(fp, "[FileInfo]\n");
   fprintf(fp, "ReadOnly=true\n");
   fprintf(fp, "FileName=\"/backup/file2_b.dat\"\n");
   fprintf(fp, "[Section1]\n");
   fprintf(fp, "Param101[0]=enum3_id3\n");
   fprintf(fp, "[Section2]\n");
   for ( int i=0; i<16; i++ ) fprintf(fp, "Param[%d]=%d\n", i, i^0x9);
   fprintf(fp, "[Section3]\n");
   fprintf(fp, "Param=3\n");
   fclose(fp);

   mkdir(CONFIG_PATH "/backup/crc");
   fp = fopen(CONFIG_PATH "/backup/crc/file2_b.crc", "w");
   fprintf(fp, "0x943b393f\n");
   fclose(fp);

   fprintf(stderr, "Test 2.18 ...\n");
   if ( Test2::Test2_B_Access.readFile(&log_level_config_data_info, &log_level_config_data_error) != ConfigFile::ReadBackupOK )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   // Test format version check
   //
   unlink(CONFIG_PATH "/backup/file2_b.dat");

   fp = fopen(CONFIG_PATH "/file2_b.dat", "w");
   fprintf(fp, "[Version]\n");
   fprintf(fp, "FormatVersion=\"2c\"\n");
   fprintf(fp, "DataVersion=\"d2b\"\n");
   fprintf(fp, "[FileInfo]\n");
   fprintf(fp, "ReadOnly=true\n");
   fprintf(fp, "FileName=\"/file2_b.dat\"\n");
   fprintf(fp, "[Section1]\n");
   fprintf(fp, "Param101[0]=enum3_id3\n");
   fprintf(fp, "[Section2]\n");
   for ( int i=0; i<16; i++ ) fprintf(fp, "Param[%d]=%d\n", i, i^0x9);
   fprintf(fp, "[Section3]\n");
   fprintf(fp, "Param=3\n");
   fclose(fp);

   fp = fopen(CONFIG_PATH "/crc/file2_b.crc", "w");
   fprintf(fp, "0x888f507d\n");
   fclose(fp);

   fprintf(stderr, "Test 2.19 ...\n");
   if ( Test2::Test2_B_Access.readFile(&log_level_config_data_info, &log_level_config_data_error) != ConfigFile::ReadFailed )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");


   // Test setting parameters for read/write files
   //
   fprintf(stderr, "Test 2.20 ...\n");
   if ( !Test2::Test2_C_Access.Version.FormatVersion.set(Test2::Test2_C_Access.formatVersion()) ||
        !Test2::Test2_C_Access.Version.DataVersion.set("1") ||
        !Test2::Test2_C_Access.FileInfo.FileName.set(Test2::Test2_C_Access.fileName()) ||
        !Test2::Test2_C_Access.Section1.Param201[0].set(Test2::enum4_id1) ||
        !Test2::Test2_C_Access.Section2.Param1[0].set(1) ||
        !Test2::Test2_C_Access.Section2.Param2[0].set(-1) ||
        !Test2::Test2_C_Access.Section2.Param3[0].set(-1) ||
        !Test2::Test2_C_Access.Section2.Param4[0].set(-1) ||
        !Test2::Test2_C_Access.Section2.Param5[0].set(2) ||
        !Test2::Test2_C_Access.Section2.Param6[0].set(2) )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   // Test file write
   //
   fprintf(stderr, "Test 2.21 ...\n");
   if ( Test2::Test2_C_Access.writeFile(&log_level_config_data_info, &log_level_config_data_error) != ConfigFile::WriteOK )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   // Test file read-back
   //
   Test2::Test2_C_Access.Version.FormatVersion.set("0");
   Test2::Test2_C_Access.Version.DataVersion.set("0");
   Test2::Test2_C_Access.Section1.Param201[0].set(Test2::enum4_id2);
   Test2::Test2_C_Access.Section2.Param1[0].set(2);
   Test2::Test2_C_Access.Section2.Param2[0].set(-2);
   Test2::Test2_C_Access.Section2.Param3[0].set(-2);
   Test2::Test2_C_Access.Section2.Param4[0].set(0);
   Test2::Test2_C_Access.Section2.Param5[0].set(1);
   Test2::Test2_C_Access.Section2.Param6[0].set(1);

   fprintf(stderr, "Test 2.22 ...\n");
   if ( Test2::Test2_C_Access.readFile(&log_level_config_data_info, &log_level_config_data_error) != ConfigFile::ReadOK ||
         strcmp(Test2::Test2_C().Version.FormatVersion, "2c") != 0 ||
         strcmp(Test2::Test2_C().Version.DataVersion, "1") != 0 ||
         Test2::Test2_C().Section1.Param201[0] != Test2::enum4_id1 ||
         Test2::Test2_C().Section2.Param1[0] != 1 ||
         Test2::Test2_C().Section2.Param2[0] != -1 ||
         Test2::Test2_C().Section2.Param3[0] != -1 ||
         Test2::Test2_C().Section2.Param4[0] != -1 ||
         Test2::Test2_C().Section2.Param5[0] != 2 ||
         Test2::Test2_C().Section2.Param6[0] != 2 )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   // Test file backup
   //
   Test2::Test2_C_Access.Version.FormatVersion.set("0");
   Test2::Test2_C_Access.Version.DataVersion.set("0");
   Test2::Test2_C_Access.Section1.Param201[0].set(Test2::enum4_id2);
   Test2::Test2_C_Access.Section2.Param1[0].set(2);
   Test2::Test2_C_Access.Section2.Param2[0].set(-2);
   Test2::Test2_C_Access.Section2.Param3[0].set(-2);
   Test2::Test2_C_Access.Section2.Param4[0].set(0);
   Test2::Test2_C_Access.Section2.Param5[0].set(1);
   Test2::Test2_C_Access.Section2.Param6[0].set(1);

   fprintf(stderr, "Test 2.23 ...\n");
   if ( Test2::Test2_C_Access.writeFile(&log_level_config_data_info, &log_level_config_data_error) != ConfigFile::WriteOK )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   fprintf(stderr, "Test 2.24 ...\n");
   unlink(CONFIG_PATH "/file2_c.dat");
   if ( Test2::Test2_C_Access.readFile(&log_level_config_data_info, &log_level_config_data_error) != ConfigFile::ReadBackupOK ||
         strcmp(Test2::Test2_C().Version.FormatVersion, "2c") != 0 ||
         strcmp(Test2::Test2_C().Version.DataVersion, "1") != 0 ||
         Test2::Test2_C().Section1.Param201[0] != Test2::enum4_id1 ||
         Test2::Test2_C().Section2.Param1[0] != 1 ||
         Test2::Test2_C().Section2.Param2[0] != -1 ||
         Test2::Test2_C().Section2.Param3[0] != -1 ||
         Test2::Test2_C().Section2.Param4[0] != -1 ||
         Test2::Test2_C().Section2.Param5[0] != 2 ||
         Test2::Test2_C().Section2.Param6[0] != 2 )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   // Test range functions
   //
   fprintf(stderr, "Test 2.25 ...\n");
   long	min, max;
   if ( !Test2::Test2_C_Access.Section2.Param1[0].getRange(min, max) ||
         min != 1 ||
         max != 2 ||
         !Test2::Test2_C_Access.Section2.Param1[0].set(1) ||
         !Test2::Test2_C_Access.Section2.Param1[0].set(2) ||
         Test2::Test2_C_Access.Section2.Param1[0].set(0) ||
         Test2::Test2_C_Access.Section2.Param1[0].set(3) ||
         Test2::Test2_C_Access.Section2.Param1[0].set(-1) )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   fprintf(stderr, "Test 2.26 ...\n");
   if ( !Test2::Test2_C_Access.Section2.Param2[0].getRange(min, max) ||
         min != -2 ||
         max != -1 ||
         !Test2::Test2_C_Access.Section2.Param2[0].set(-1) ||
         !Test2::Test2_C_Access.Section2.Param2[0].set(-2) ||
         Test2::Test2_C_Access.Section2.Param2[0].set(0) ||
         Test2::Test2_C_Access.Section2.Param2[0].set(1) ||
         Test2::Test2_C_Access.Section2.Param2[0].set(-3) )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   fprintf(stderr, "Test 2.27 ...\n");
   if ( Test2::Test2_C_Access.Section2.Param3[0].getRange(min, max) ||
         !Test2::Test2_C_Access.Section2.Param3[0].set(-1) ||
         !Test2::Test2_C_Access.Section2.Param3[0].set(-2) ||
         !Test2::Test2_C_Access.Section2.Param3[0].set(0) ||
         !Test2::Test2_C_Access.Section2.Param3[0].set(1) ||
         !Test2::Test2_C_Access.Section2.Param3[0].set(-3) )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   fprintf(stderr, "Test 2.28 ...\n");
   if ( !Test2::Test2_C_Access.Section2.Param4[0].getRange(min, max) ||
         min != -1 ||
         max != 1 ||
         !Test2::Test2_C_Access.Section2.Param4[0].set(-1) ||
         !Test2::Test2_C_Access.Section2.Param4[0].set(0) ||
         Test2::Test2_C_Access.Section2.Param4[0].set(1) ||
         Test2::Test2_C_Access.Section2.Param4[0].set(2) ||
         Test2::Test2_C_Access.Section2.Param4[0].set(-2) )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   fprintf(stderr, "Test 2.29 ...\n");
   if ( !Test2::Test2_C_Access.Section2.Param5[0].getRange(min, max) ||
         min != -2 ||
         max != 2 ||
         !Test2::Test2_C_Access.Section2.Param5[0].set(1) ||
         !Test2::Test2_C_Access.Section2.Param5[0].set(2) ||
         Test2::Test2_C_Access.Section2.Param5[0].set(3) ||
         Test2::Test2_C_Access.Section2.Param5[0].set(0) ||
         Test2::Test2_C_Access.Section2.Param5[0].set(-1) ||
         Test2::Test2_C_Access.Section2.Param5[0].set(-3) )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   fprintf(stderr, "Test 2.30 ...\n");
   if ( Test2::Test2_C_Access.Section2.Param6[0].getRange(min, max) ||
         !Test2::Test2_C_Access.Section2.Param6[0].set(1) ||
         !Test2::Test2_C_Access.Section2.Param6[0].set(2) ||
         !Test2::Test2_C_Access.Section2.Param6[0].set(3) ||
         Test2::Test2_C_Access.Section2.Param6[0].set(0) ||
         Test2::Test2_C_Access.Section2.Param6[0].set(-1) ||
         Test2::Test2_C_Access.Section2.Param6[0].set(-3) )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   //	Test array sizes
   //
   fprintf(stderr, "Test 2.31 ...\n");
   if ( sizeof(Test2::Test2_A().Section1.Param1)/sizeof(Test2::Test2_A().Section1.Param1[0]) != 2 ||
         sizeof(Test2::Test2_A().Section2.Param13)/sizeof(Test2::Test2_A().Section2.Param13[0]) != 2 ||
         sizeof(Test2::Test2_B().Section1.Param101)/sizeof(Test2::Test2_B().Section1.Param101[0]) != 1 ||
         sizeof(Test2::Test2_B().Section2.Param)/sizeof(Test2::Test2_B().Section2.Param[0]) != 16 )
   {
      fprintf(stderr, "test failed\n");
      return -1;
   }
   fprintf(stderr, "test passed\n");

   fprintf(stderr, "All Tests passed\n");
   return 0;
}

