/*
 * Auto-generated file, do not edit
 */

#include <vxWorks.h>
#include "test2.h"

Test2::Test2_A_Data Test2::_Test2_A_Data;
Test2::_C_Test2_A Test2::Test2_A_Access(_Test2_A_Data);

Test2::Test2_B_Data Test2::_Test2_B_Data;
Test2::_C_Test2_B Test2::Test2_B_Access(_Test2_B_Data);

Test2::Test2_C_Data Test2::_Test2_C_Data;
Test2::_C_Test2_C Test2::Test2_C_Access(_Test2_C_Data);

Test2::Test2_D_Data Test2::_Test2_D_Data;
Test2::_C_Test2_D Test2::Test2_D_Access(_Test2_D_Data);

Test2::Test2_E_Data Test2::_Test2_E_Data;
Test2::_C_Test2_E Test2::Test2_E_Access(_Test2_E_Data);

Test2::Test2_F_Data Test2::_Test2_F_Data;
Test2::_C_Test2_F Test2::Test2_F_Access(_Test2_F_Data);

Test2::Test2_G_Data Test2::_Test2_G_Data;
Test2::_C_Test2_G Test2::Test2_G_Access(_Test2_G_Data);

Test2::Test2_H_Data Test2::_Test2_H_Data;
Test2::_C_Test2_H Test2::Test2_H_Access(_Test2_H_Data);

Test2::Test2_I_Data Test2::_Test2_I_Data;
Test2::_C_Test2_I Test2::Test2_I_Access(_Test2_I_Data);

Test2::Test2_J_Data Test2::_Test2_J_Data;
Test2::_C_Test2_J Test2::Test2_J_Access(_Test2_J_Data);

Test2::Test2_K_Data Test2::_Test2_K_Data;
Test2::_C_Test2_K Test2::Test2_K_Access(_Test2_K_Data);

Test2::Test2_L_Data Test2::_Test2_L_Data;
Test2::_C_Test2_L Test2::Test2_L_Access(_Test2_L_Data);

Test2::Test2_M_Data Test2::_Test2_M_Data;
Test2::_C_Test2_M Test2::Test2_M_Access(_Test2_M_Data);

ConfigFile * Test2::getConfigFileObj(int index)
{
  switch (index)
  {
  case 0: return (ConfigFile *)&Test2_A_Access;
  case 1: return (ConfigFile *)&Test2_B_Access;
  case 2: return (ConfigFile *)&Test2_C_Access;
  case 3: return (ConfigFile *)&Test2_D_Access;
  case 4: return (ConfigFile *)&Test2_E_Access;
  case 5: return (ConfigFile *)&Test2_F_Access;
  case 6: return (ConfigFile *)&Test2_G_Access;
  case 7: return (ConfigFile *)&Test2_H_Access;
  case 8: return (ConfigFile *)&Test2_I_Access;
  case 9: return (ConfigFile *)&Test2_J_Access;
  case 10: return (ConfigFile *)&Test2_K_Access;
  case 11: return (ConfigFile *)&Test2_L_Access;
  case 12: return (ConfigFile *)&Test2_M_Access;
  default: return NULL;
  }
}

const ConfigData::EnumMap Test2::_enumMap[15] =
{
  { "enum1_id1", 0xd83faecf },
  { "enum1_id2", 0xc136ff75 },
  { "enum1_id3", 0xb631cfe3 },
  { "enum2_id1", 0x9f9fd41f },
  { "enum2_id2", 0x869685a5 },
  { "enum3_id1", 0xa2fffdaf },
  { "enum3_id2", 0xbbf6ac15 },
  { "enum3_id3", 0xccf19c83 },
  { "enum4_id1", 0x90df21bf },
  { "enum4_id2", 0x89d67005 },
  { "enum4_id3", 0xfed14093 },
  { "enum_k_id1", 0xa6d60de6 },
  { "enum_k_id2", 0xbfdf5c5c },
  { "enum_k_id3", 0xc8d86cca },
  { NULL, 0 }
};

const char * Test2::_C_Test2_A::_name[30] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param1[0]",
  "Param2[0]",
  "Param1[1]",
  "Param2[1]",
  "Param3[0]",
  "Param3[1]",
  "Param4[0]",
  "Param4[1]",
  "Param5[0]",
  "Param5[1]",
  "Param6[0]",
  "Param6[1]",
  "Param7[0]",
  "Param7[1]",
  "Param8[0]",
  "Param8[1]",
  "Param9[0]",
  "Param9[1]",
  "Param10[0]",
  "Param10[1]",
  "Param11[0]",
  "Param11[1]",
  "Param12[0]",
  "Param12[1]",
  "Param13[0]",
  "Param13[1]"
};

const char * Test2::_C_Test2_A::_sectionName[4] =
{
  "Version",
  "FileInfo",
  "Section1",
  "Section2"
};

Test2::_C_Test2_A::_C_Test2_A(Test2_A_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TEnum, (void *)&data.Section1.Param1[0], _enumMap, NULL, &Test2::_C_Test2_A::_C_Section1::_C_Param1::validate);
  _dataMap[5].setMap(_sectionName[2], _name[5], TEnum, (void *)&data.Section1.Param2[0], _enumMap, NULL, &Test2::_C_Test2_A::_C_Section1::_C_Param2::validate);
  _dataMap[6].setMap(_sectionName[2], _name[6], TEnum, (void *)&data.Section1.Param1[1], _enumMap, NULL, &Test2::_C_Test2_A::_C_Section1::_C_Param1::validate);
  _dataMap[7].setMap(_sectionName[2], _name[7], TEnum, (void *)&data.Section1.Param2[1], _enumMap, NULL, &Test2::_C_Test2_A::_C_Section1::_C_Param2::validate);
  _dataMap[8].setMap(_sectionName[3], _name[8], TBool, (void *)&data.Section2.Param3[0], NULL, NULL, NULL);
  _dataMap[9].setMap(_sectionName[3], _name[9], TBool, (void *)&data.Section2.Param3[1], NULL, NULL, NULL);
  _dataMap[10].setMap(_sectionName[3], _name[10], TString, (void *)&data.Section2.Param4[0], NULL, NULL, NULL);
  _dataMap[11].setMap(_sectionName[3], _name[11], TString, (void *)&data.Section2.Param4[1], NULL, NULL, NULL);
  _dataMap[12].setMap(_sectionName[3], _name[12], TLong, (void *)&data.Section2.Param5[0], NULL, (RangeFunc *)&Test2::_C_Test2_A::_C_Section2::_C_Param5::getRange, NULL);
  _dataMap[13].setMap(_sectionName[3], _name[13], TLong, (void *)&data.Section2.Param5[1], NULL, NULL, NULL);
  _dataMap[14].setMap(_sectionName[3], _name[14], TLong, (void *)&data.Section2.Param6[0], NULL, (RangeFunc *)&Test2::_C_Test2_A::_C_Section2::_C_Param6::getRange, NULL);
  _dataMap[15].setMap(_sectionName[3], _name[15], TLong, (void *)&data.Section2.Param6[1], NULL, NULL, NULL);
  _dataMap[16].setMap(_sectionName[3], _name[16], TLong, (void *)&data.Section2.Param7[0], NULL, (RangeFunc *)&Test2::_C_Test2_A::_C_Section2::_C_Param7::getRange, NULL);
  _dataMap[17].setMap(_sectionName[3], _name[17], TLong, (void *)&data.Section2.Param7[1], NULL, NULL, NULL);
  _dataMap[18].setMap(_sectionName[3], _name[18], TLong, (void *)&data.Section2.Param8[0], NULL, NULL, &Test2::_C_Test2_A::_C_Section2::_C_Param8::validate);
  _dataMap[19].setMap(_sectionName[3], _name[19], TLong, (void *)&data.Section2.Param8[1], NULL, NULL, NULL);
  _dataMap[20].setMap(_sectionName[3], _name[20], TDouble, (void *)&data.Section2.Param9[0], NULL, (RangeFunc *)&Test2::_C_Test2_A::_C_Section2::_C_Param9::getRange, NULL);
  _dataMap[21].setMap(_sectionName[3], _name[21], TDouble, (void *)&data.Section2.Param9[1], NULL, NULL, NULL);
  _dataMap[22].setMap(_sectionName[3], _name[22], TDouble, (void *)&data.Section2.Param10[0], NULL, (RangeFunc *)&Test2::_C_Test2_A::_C_Section2::_C_Param10::getRange, NULL);
  _dataMap[23].setMap(_sectionName[3], _name[23], TDouble, (void *)&data.Section2.Param10[1], NULL, NULL, NULL);
  _dataMap[24].setMap(_sectionName[3], _name[24], TDouble, (void *)&data.Section2.Param11[0], NULL, NULL, &Test2::_C_Test2_A::_C_Section2::_C_Param11::validate);
  _dataMap[25].setMap(_sectionName[3], _name[25], TDouble, (void *)&data.Section2.Param11[1], NULL, NULL, NULL);
  _dataMap[26].setMap(_sectionName[3], _name[26], TDouble, (void *)&data.Section2.Param12[0], NULL, (RangeFunc *)&Test2::_C_Test2_A::_C_Section2::_C_Param12::getRange, &Test2::_C_Test2_A::_C_Section2::_C_Param12::validate);
  _dataMap[27].setMap(_sectionName[3], _name[27], TDouble, (void *)&data.Section2.Param12[1], NULL, NULL, NULL);
  _dataMap[28].setMap(_sectionName[3], _name[28], TDouble, (void *)&data.Section2.Param13[0], NULL, (RangeFunc *)&Test2::_C_Test2_A::_C_Section2::_C_Param13::getRange, &Test2::_C_Test2_A::_C_Section2::_C_Param13::validate);
  _dataMap[29].setMap(_sectionName[3], _name[29], TDouble, (void *)&data.Section2.Param13[1], NULL, NULL, NULL);
}

Test2::_C_Test2_A::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test2::_C_Test2_A::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test2::_C_Test2_A::_C_Section1::_C_Section1(void)
{
  Param1[0].initialize(4);
  Param2[0].initialize(5);
  Param1[1].initialize(6);
  Param2[1].initialize(7);
}

Test2::_C_Test2_A::_C_Section2::_C_Section2(void)
{
  Param3[0].initialize(8);
  Param3[1].initialize(9);
  Param4[0].initialize(10);
  Param4[1].initialize(11);
  Param5[0].initialize(12);
  Param5[1].initialize(13);
  Param6[0].initialize(14);
  Param6[1].initialize(15);
  Param7[0].initialize(16);
  Param7[1].initialize(17);
  Param8[0].initialize(18);
  Param8[1].initialize(19);
  Param9[0].initialize(20);
  Param9[1].initialize(21);
  Param10[0].initialize(22);
  Param10[1].initialize(23);
  Param11[0].initialize(24);
  Param11[1].initialize(25);
  Param12[0].initialize(26);
  Param12[1].initialize(27);
  Param13[0].initialize(28);
  Param13[1].initialize(29);
}

void Test2::_C_Test2_A::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 30, readStatus);
}

void Test2::_C_Test2_A::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 30, writeStatus);
}

bool Test2::_C_Test2_A::_C_Section1::_C_Param1::validate(void * valuePtr)
{
  if ( !Test2::validateParam1(valuePtr) ) return false;
  return true;

}

bool Test2::_C_Test2_A::_C_Section1::_C_Param2::validate(void * valuePtr)
{
  if ( !Test2::validateParam2(valuePtr) ) return false;
  return true;

}

bool Test2::_C_Test2_A::_C_Section2::_C_Param8::validate(void * valuePtr)
{
  long value = *(long *)valuePtr;
return ( value < 0 );

}

bool Test2::_C_Test2_A::_C_Section2::_C_Param11::validate(void * valuePtr)
{
  double value = *(double *)valuePtr;
return ( value < 1e20 );

}

bool Test2::_C_Test2_A::_C_Section2::_C_Param12::validate(void * valuePtr)
{
  double value = *(double *)valuePtr;
return true;

}

bool Test2::_C_Test2_A::_C_Section2::_C_Param13::validate(void * valuePtr)
{
  double value = *(double *)valuePtr;
return true;

}

bool Test2::_C_Test2_A::_C_Section2::_C_Param7::getRange(long & min, long & max)
{
min=0; max=0x22222; return true;

}

bool Test2::_C_Test2_A::_C_Section2::_C_Param10::getRange(double & min, double & max)
{
min=0; max=0; return false;

}

const char * Test2::_C_Test2_B::_name[22] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param101[0]",
  "Param[0]",
  "Param[1]",
  "Param[2]",
  "Param[3]",
  "Param[4]",
  "Param[5]",
  "Param[6]",
  "Param[7]",
  "Param[8]",
  "Param[9]",
  "Param[10]",
  "Param[11]",
  "Param[12]",
  "Param[13]",
  "Param[14]",
  "Param[15]",
  "Param"
};

const char * Test2::_C_Test2_B::_sectionName[5] =
{
  "Version",
  "FileInfo",
  "Section1",
  "Section2",
  "Section3"
};

Test2::_C_Test2_B::_C_Test2_B(Test2_B_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TEnum, (void *)&data.Section1.Param101[0], _enumMap, NULL, &Test2::_C_Test2_B::_C_Section1::_C_Param101::validate);
  _dataMap[5].setMap(_sectionName[3], _name[5], TLong, (void *)&data.Section2.Param[0], NULL, NULL, NULL);
  _dataMap[6].setMap(_sectionName[3], _name[6], TLong, (void *)&data.Section2.Param[1], NULL, NULL, NULL);
  _dataMap[7].setMap(_sectionName[3], _name[7], TLong, (void *)&data.Section2.Param[2], NULL, NULL, NULL);
  _dataMap[8].setMap(_sectionName[3], _name[8], TLong, (void *)&data.Section2.Param[3], NULL, NULL, NULL);
  _dataMap[9].setMap(_sectionName[3], _name[9], TLong, (void *)&data.Section2.Param[4], NULL, NULL, NULL);
  _dataMap[10].setMap(_sectionName[3], _name[10], TLong, (void *)&data.Section2.Param[5], NULL, NULL, NULL);
  _dataMap[11].setMap(_sectionName[3], _name[11], TLong, (void *)&data.Section2.Param[6], NULL, NULL, NULL);
  _dataMap[12].setMap(_sectionName[3], _name[12], TLong, (void *)&data.Section2.Param[7], NULL, NULL, NULL);
  _dataMap[13].setMap(_sectionName[3], _name[13], TLong, (void *)&data.Section2.Param[8], NULL, NULL, NULL);
  _dataMap[14].setMap(_sectionName[3], _name[14], TLong, (void *)&data.Section2.Param[9], NULL, NULL, NULL);
  _dataMap[15].setMap(_sectionName[3], _name[15], TLong, (void *)&data.Section2.Param[10], NULL, NULL, NULL);
  _dataMap[16].setMap(_sectionName[3], _name[16], TLong, (void *)&data.Section2.Param[11], NULL, NULL, NULL);
  _dataMap[17].setMap(_sectionName[3], _name[17], TLong, (void *)&data.Section2.Param[12], NULL, NULL, NULL);
  _dataMap[18].setMap(_sectionName[3], _name[18], TLong, (void *)&data.Section2.Param[13], NULL, NULL, NULL);
  _dataMap[19].setMap(_sectionName[3], _name[19], TLong, (void *)&data.Section2.Param[14], NULL, NULL, NULL);
  _dataMap[20].setMap(_sectionName[3], _name[20], TLong, (void *)&data.Section2.Param[15], NULL, NULL, NULL);
  _dataMap[21].setMap(_sectionName[4], _name[21], TLong, (void *)&data.Section3.Param, NULL, NULL, NULL);
}

Test2::_C_Test2_B::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test2::_C_Test2_B::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test2::_C_Test2_B::_C_Section1::_C_Section1(void)
{
  Param101[0].initialize(4);
}

Test2::_C_Test2_B::_C_Section2::_C_Section2(void)
{
  Param[0].initialize(5);
  Param[1].initialize(6);
  Param[2].initialize(7);
  Param[3].initialize(8);
  Param[4].initialize(9);
  Param[5].initialize(10);
  Param[6].initialize(11);
  Param[7].initialize(12);
  Param[8].initialize(13);
  Param[9].initialize(14);
  Param[10].initialize(15);
  Param[11].initialize(16);
  Param[12].initialize(17);
  Param[13].initialize(18);
  Param[14].initialize(19);
  Param[15].initialize(20);
}

Test2::_C_Test2_B::_C_Section3::_C_Section3(void)
{
  Param.initialize(21);
}

void Test2::_C_Test2_B::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 22, readStatus);
}

void Test2::_C_Test2_B::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 22, writeStatus);
}

bool Test2::_C_Test2_B::_C_Section1::_C_Param101::validate(void * valuePtr)
{
  if ( !Test2::validateParam101(valuePtr) ) return false;
  return true;

}

const char * Test2::_C_Test2_C::_name[11] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param201[0]",
  "Param1[0]",
  "Param2[0]",
  "Param3[0]",
  "Param4[0]",
  "Param5[0]",
  "Param6[0]"
};

const char * Test2::_C_Test2_C::_sectionName[4] =
{
  "Version",
  "FileInfo",
  "Section1",
  "Section2"
};

Test2::_C_Test2_C::_C_Test2_C(Test2_C_Data & data)
  : Version(_dataMap), FileInfo(_dataMap), Section1(_dataMap), Section2(_dataMap)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TEnum, (void *)&data.Section1.Param201[0], _enumMap, NULL, &Test2::_C_Test2_C::_C_Section1::_C_Param201::validate);
  _dataMap[5].setMap(_sectionName[3], _name[5], TLong, (void *)&data.Section2.Param1[0], NULL, (RangeFunc *)&Test2::_C_Test2_C::_C_Section2::_C_Param1::getRange, NULL);
  _dataMap[6].setMap(_sectionName[3], _name[6], TLong, (void *)&data.Section2.Param2[0], NULL, (RangeFunc *)&Test2::_C_Test2_C::_C_Section2::_C_Param2::getRange, NULL);
  _dataMap[7].setMap(_sectionName[3], _name[7], TLong, (void *)&data.Section2.Param3[0], NULL, (RangeFunc *)&Test2::_C_Test2_C::_C_Section2::_C_Param3::getRange, NULL);
  _dataMap[8].setMap(_sectionName[3], _name[8], TLong, (void *)&data.Section2.Param4[0], NULL, (RangeFunc *)&Test2::_C_Test2_C::_C_Section2::_C_Param4::getRange, &Test2::_C_Test2_C::_C_Section2::_C_Param4::validate);
  _dataMap[9].setMap(_sectionName[3], _name[9], TLong, (void *)&data.Section2.Param5[0], NULL, (RangeFunc *)&Test2::_C_Test2_C::_C_Section2::_C_Param5::getRange, &Test2::_C_Test2_C::_C_Section2::_C_Param5::validate);
  _dataMap[10].setMap(_sectionName[3], _name[10], TLong, (void *)&data.Section2.Param6[0], NULL, (RangeFunc *)&Test2::_C_Test2_C::_C_Section2::_C_Param6::getRange, &Test2::_C_Test2_C::_C_Section2::_C_Param6::validate);
}

Test2::_C_Test2_C::_C_Version::_C_Version(const DataMap * dataMap)
{
  FormatVersion.initialize(dataMap, 0);
  DataVersion.initialize(dataMap, 1);
}

Test2::_C_Test2_C::_C_FileInfo::_C_FileInfo(const DataMap * dataMap)
{
  ReadOnly.initialize(dataMap, 2);
  FileName.initialize(dataMap, 3);
}

Test2::_C_Test2_C::_C_Section1::_C_Section1(const DataMap * dataMap)
{
  Param201[0].initialize(dataMap, 4);
}

Test2::_C_Test2_C::_C_Section2::_C_Section2(const DataMap * dataMap)
{
  Param1[0].initialize(dataMap, 5);
  Param2[0].initialize(dataMap, 6);
  Param3[0].initialize(dataMap, 7);
  Param4[0].initialize(dataMap, 8);
  Param5[0].initialize(dataMap, 9);
  Param6[0].initialize(dataMap, 10);
}

void Test2::_C_Test2_C::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 11, readStatus);
}

void Test2::_C_Test2_C::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 11, writeStatus);
}

bool Test2::_C_Test2_C::_C_Section1::_C_Param201::validate(void * valuePtr)
{
  if ( !Test2::validateParam201(valuePtr) ) return false;
  return true;

}

bool Test2::_C_Test2_C::_C_Section2::_C_Param4::validate(void * valuePtr)
{
  long value = *(long *)valuePtr;
return ( value <= 0 );

}

bool Test2::_C_Test2_C::_C_Section2::_C_Param5::validate(void * valuePtr)
{
  long value = *(long *)valuePtr;
return ( value > 0 );

}

bool Test2::_C_Test2_C::_C_Section2::_C_Param6::validate(void * valuePtr)
{
  long value = *(long *)valuePtr;
return ( value > 0 );

}

bool Test2::_C_Test2_C::_C_Section2::_C_Param2::getRange(long & min, long & max)
{
min=-2; max=-1; return true;

}

bool Test2::_C_Test2_C::_C_Section2::_C_Param3::getRange(long & min, long & max)
{
min=-2; max=-1; return false;

}

bool Test2::_C_Test2_C::_C_Section2::_C_Param5::getRange(long & min, long & max)
{
min=-2; max=2; return true;

}

bool Test2::_C_Test2_C::_C_Section2::_C_Param6::getRange(long & min, long & max)
{
min=-2; max=2; return false;

}

bool Test2::_C_Test2_C::_C_Version::_C_FormatVersion::set(const char * value)
{
  bool setOK = true;
  const char ** valuePtr = (const char **)_dataMap[_mapIdx]._value;
  if ( setOK )
  {
    if ( *valuePtr ) delete[] *valuePtr;
    *valuePtr = new char[strlen(value)+1];
    strcpy(*(char **)valuePtr, value);
  }
  return setOK;

}

bool Test2::_C_Test2_C::_C_Version::_C_DataVersion::set(const char * value)
{
  bool setOK = true;
  const char ** valuePtr = (const char **)_dataMap[_mapIdx]._value;
  if ( setOK )
  {
    if ( *valuePtr ) delete[] *valuePtr;
    *valuePtr = new char[strlen(value)+1];
    strcpy(*(char **)valuePtr, value);
  }
  return setOK;

}

bool Test2::_C_Test2_C::_C_FileInfo::_C_ReadOnly::set(bool value)
{
  bool setOK = true;
  bool * valuePtr = (bool *)_dataMap[_mapIdx]._value;
  if ( setOK ) *valuePtr = value;
  return setOK;

}

bool Test2::_C_Test2_C::_C_FileInfo::_C_FileName::set(const char * value)
{
  bool setOK = true;
  const char ** valuePtr = (const char **)_dataMap[_mapIdx]._value;
  if ( setOK )
  {
    if ( *valuePtr ) delete[] *valuePtr;
    *valuePtr = new char[strlen(value)+1];
    strcpy(*(char **)valuePtr, value);
  }
  return setOK;

}

bool Test2::_C_Test2_C::_C_Section1::_C_Param201::set(TParam201 value)
{
  bool setOK = true;
  TParam201 * valuePtr = (TParam201 *)_dataMap[_mapIdx]._value;
  if ( !validate((void *)&value) ) setOK = false;

  if ( setOK ) *valuePtr = value;
  return setOK;

}

bool Test2::_C_Test2_C::_C_Section2::_C_Param1::set(long value)
{
  bool setOK = true;
  long * valuePtr = (long *)_dataMap[_mapIdx]._value;
  if ( value < 1 || value > 2 ) setOK = false;

  if ( setOK ) *valuePtr = value;
  return setOK;

}

bool Test2::_C_Test2_C::_C_Section2::_C_Param2::set(long value)
{
  bool setOK = true;
  long * valuePtr = (long *)_dataMap[_mapIdx]._value;
  long min, max;
  if ( getRange(min, max) &&
       ( value < min || value > max )) setOK = false;

  if ( setOK ) *valuePtr = value;
  return setOK;

}

bool Test2::_C_Test2_C::_C_Section2::_C_Param3::set(long value)
{
  bool setOK = true;
  long * valuePtr = (long *)_dataMap[_mapIdx]._value;
  long min, max;
  if ( getRange(min, max) &&
       ( value < min || value > max )) setOK = false;

  if ( setOK ) *valuePtr = value;
  return setOK;

}

bool Test2::_C_Test2_C::_C_Section2::_C_Param4::set(long value)
{
  bool setOK = true;
  long * valuePtr = (long *)_dataMap[_mapIdx]._value;
  if ( value < -1 || value > 1 ) setOK = false;

  if ( !validate((void *)&value) ) setOK = false;

  if ( setOK ) *valuePtr = value;
  return setOK;

}

bool Test2::_C_Test2_C::_C_Section2::_C_Param5::set(long value)
{
  bool setOK = true;
  long * valuePtr = (long *)_dataMap[_mapIdx]._value;
  long min, max;
  if ( getRange(min, max) &&
       ( value < min || value > max )) setOK = false;

  if ( !validate((void *)&value) ) setOK = false;

  if ( setOK ) *valuePtr = value;
  return setOK;

}

bool Test2::_C_Test2_C::_C_Section2::_C_Param6::set(long value)
{
  bool setOK = true;
  long * valuePtr = (long *)_dataMap[_mapIdx]._value;
  long min, max;
  if ( getRange(min, max) &&
       ( value < min || value > max )) setOK = false;

  if ( !validate((void *)&value) ) setOK = false;

  if ( setOK ) *valuePtr = value;
  return setOK;

}

const char * Test2::_C_Test2_D::_name[5] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param1[0]"
};

const char * Test2::_C_Test2_D::_sectionName[3] =
{
  "Version",
  "FileInfo",
  "Section1"
};

Test2::_C_Test2_D::_C_Test2_D(Test2_D_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TLong, (void *)&data.Section1.Param1[0], NULL, (RangeFunc *)&Test2::_C_Test2_D::_C_Section1::_C_Param1::getRange, NULL);
}

Test2::_C_Test2_D::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test2::_C_Test2_D::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test2::_C_Test2_D::_C_Section1::_C_Section1(void)
{
  Param1[0].initialize(4);
}

void Test2::_C_Test2_D::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 5, readStatus);
}

void Test2::_C_Test2_D::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 5, writeStatus);
}

const char * Test2::_C_Test2_E::_name[5] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param1[0]"
};

const char * Test2::_C_Test2_E::_sectionName[3] =
{
  "Version",
  "FileInfo",
  "Section1"
};

Test2::_C_Test2_E::_C_Test2_E(Test2_E_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TLong, (void *)&data.Section1.Param1[0], NULL, (RangeFunc *)&Test2::_C_Test2_E::_C_Section1::_C_Param1::getRange, NULL);
}

Test2::_C_Test2_E::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test2::_C_Test2_E::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test2::_C_Test2_E::_C_Section1::_C_Section1(void)
{
  Param1[0].initialize(4);
}

void Test2::_C_Test2_E::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 5, readStatus);
}

void Test2::_C_Test2_E::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 5, writeStatus);
}

bool Test2::_C_Test2_E::_C_Section1::_C_Param1::getRange(long & min, long & max)
{
min=0; max=1; return true;

}

const char * Test2::_C_Test2_F::_name[5] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param1[0]"
};

const char * Test2::_C_Test2_F::_sectionName[3] =
{
  "Version",
  "FileInfo",
  "Section1"
};

Test2::_C_Test2_F::_C_Test2_F(Test2_F_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TLong, (void *)&data.Section1.Param1[0], NULL, NULL, &Test2::_C_Test2_F::_C_Section1::_C_Param1::validate);
}

Test2::_C_Test2_F::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test2::_C_Test2_F::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test2::_C_Test2_F::_C_Section1::_C_Section1(void)
{
  Param1[0].initialize(4);
}

void Test2::_C_Test2_F::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 5, readStatus);
}

void Test2::_C_Test2_F::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 5, writeStatus);
}

bool Test2::_C_Test2_F::_C_Section1::_C_Param1::validate(void * valuePtr)
{
  long value = *(long *)valuePtr;
return ( value == 2 );

}

const char * Test2::_C_Test2_G::_name[5] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param1[0]"
};

const char * Test2::_C_Test2_G::_sectionName[3] =
{
  "Version",
  "FileInfo",
  "Section1"
};

Test2::_C_Test2_G::_C_Test2_G(Test2_G_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TDouble, (void *)&data.Section1.Param1[0], NULL, (RangeFunc *)&Test2::_C_Test2_G::_C_Section1::_C_Param1::getRange, NULL);
}

Test2::_C_Test2_G::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test2::_C_Test2_G::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test2::_C_Test2_G::_C_Section1::_C_Section1(void)
{
  Param1[0].initialize(4);
}

void Test2::_C_Test2_G::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 5, readStatus);
}

void Test2::_C_Test2_G::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 5, writeStatus);
}

const char * Test2::_C_Test2_H::_name[5] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param1[0]"
};

const char * Test2::_C_Test2_H::_sectionName[3] =
{
  "Version",
  "FileInfo",
  "Section1"
};

Test2::_C_Test2_H::_C_Test2_H(Test2_H_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TDouble, (void *)&data.Section1.Param1[0], NULL, (RangeFunc *)&Test2::_C_Test2_H::_C_Section1::_C_Param1::getRange, NULL);
}

Test2::_C_Test2_H::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test2::_C_Test2_H::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test2::_C_Test2_H::_C_Section1::_C_Section1(void)
{
  Param1[0].initialize(4);
}

void Test2::_C_Test2_H::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 5, readStatus);
}

void Test2::_C_Test2_H::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 5, writeStatus);
}

bool Test2::_C_Test2_H::_C_Section1::_C_Param1::getRange(double & min, double & max)
{
min=0.0; max=0.1; return true;

}

const char * Test2::_C_Test2_I::_name[5] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param1[0]"
};

const char * Test2::_C_Test2_I::_sectionName[3] =
{
  "Version",
  "FileInfo",
  "Section1"
};

Test2::_C_Test2_I::_C_Test2_I(Test2_I_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TDouble, (void *)&data.Section1.Param1[0], NULL, NULL, &Test2::_C_Test2_I::_C_Section1::_C_Param1::validate);
}

Test2::_C_Test2_I::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test2::_C_Test2_I::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test2::_C_Test2_I::_C_Section1::_C_Section1(void)
{
  Param1[0].initialize(4);
}

void Test2::_C_Test2_I::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 5, readStatus);
}

void Test2::_C_Test2_I::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 5, writeStatus);
}

bool Test2::_C_Test2_I::_C_Section1::_C_Param1::validate(void * valuePtr)
{
  double value = *(double *)valuePtr;
return ( value < 1.4999 );

}

const char * Test2::_C_Test2_J::_name[6] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param1[0]",
  "Param2[0]"
};

const char * Test2::_C_Test2_J::_sectionName[3] =
{
  "Version",
  "FileInfo",
  "Section1"
};

Test2::_C_Test2_J::_C_Test2_J(Test2_J_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TString, (void *)&data.Section1.Param1[0], NULL, NULL, &Test2::_C_Test2_J::_C_Section1::_C_Param1::validate);
  _dataMap[5].setMap(_sectionName[2], _name[5], TString, (void *)&data.Section1.Param2[0], NULL, NULL, &Test2::_C_Test2_J::_C_Section1::_C_Param2::validate);
}

Test2::_C_Test2_J::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test2::_C_Test2_J::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test2::_C_Test2_J::_C_Section1::_C_Section1(void)
{
  Param1[0].initialize(4);
  Param2[0].initialize(5);
}

void Test2::_C_Test2_J::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 6, readStatus);
}

void Test2::_C_Test2_J::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 6, writeStatus);
}

bool Test2::_C_Test2_J::_C_Section1::_C_Param1::validate(void * valuePtr)
{
  const char * value = *(const char **)valuePtr;
return ( value[0] == 'a' );

}

bool Test2::_C_Test2_J::_C_Section1::_C_Param2::validate(void * valuePtr)
{
  const char * value = *(const char **)valuePtr;
return ( value[0] == 'b' );

}

const char * Test2::_C_Test2_K::_name[6] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param1[0]",
  "Param2[0]"
};

const char * Test2::_C_Test2_K::_sectionName[3] =
{
  "Version",
  "FileInfo",
  "Section1"
};

Test2::_C_Test2_K::_C_Test2_K(Test2_K_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TBool, (void *)&data.Section1.Param1[0], NULL, NULL, &Test2::_C_Test2_K::_C_Section1::_C_Param1::validate);
  _dataMap[5].setMap(_sectionName[2], _name[5], TBool, (void *)&data.Section1.Param2[0], NULL, NULL, &Test2::_C_Test2_K::_C_Section1::_C_Param2::validate);
}

Test2::_C_Test2_K::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test2::_C_Test2_K::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test2::_C_Test2_K::_C_Section1::_C_Section1(void)
{
  Param1[0].initialize(4);
  Param2[0].initialize(5);
}

void Test2::_C_Test2_K::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 6, readStatus);
}

void Test2::_C_Test2_K::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 6, writeStatus);
}

bool Test2::_C_Test2_K::_C_Section1::_C_Param1::validate(void * valuePtr)
{
  bool value = *(bool *)valuePtr;
return ( !value );

}

bool Test2::_C_Test2_K::_C_Section1::_C_Param2::validate(void * valuePtr)
{
  bool value = *(bool *)valuePtr;
return ( !value );

}

const char * Test2::_C_Test2_L::_name[6] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "ParamK[0]",
  "ParamK[0]"
};

const char * Test2::_C_Test2_L::_sectionName[4] =
{
  "Version",
  "FileInfo",
  "Section1",
  "Section2"
};

Test2::_C_Test2_L::_C_Test2_L(Test2_L_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TEnum, (void *)&data.Section1.ParamK[0], _enumMap, NULL, &Test2::_C_Test2_L::_C_Section1::_C_ParamK::validate);
  _dataMap[5].setMap(_sectionName[3], _name[5], TEnum, (void *)&data.Section2.ParamK[0], _enumMap, NULL, &Test2::_C_Test2_L::_C_Section2::_C_ParamK::validate);
}

Test2::_C_Test2_L::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test2::_C_Test2_L::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test2::_C_Test2_L::_C_Section1::_C_Section1(void)
{
  ParamK[0].initialize(4);
}

Test2::_C_Test2_L::_C_Section2::_C_Section2(void)
{
  ParamK[0].initialize(5);
}

void Test2::_C_Test2_L::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 6, readStatus);
}

void Test2::_C_Test2_L::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 6, writeStatus);
}

bool Test2::_C_Test2_L::_C_Section1::_C_ParamK::validate(void * valuePtr)
{
  if ( !Test2::validateParamK(valuePtr) ) return false;
  TParamK value = *(TParamK *)valuePtr;
return ( value == enum_k_id1 );

}

bool Test2::_C_Test2_L::_C_Section2::_C_ParamK::validate(void * valuePtr)
{
  if ( !Test2::validateParamK(valuePtr) ) return false;
  TParamK value = *(TParamK *)valuePtr;
return ( value == enum_k_id1 );

}

const char * Test2::_C_Test2_M::_name[6] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param1[0]",
  "Param2[0]"
};

const char * Test2::_C_Test2_M::_sectionName[3] =
{
  "Version",
  "FileInfo",
  "Section1"
};

Test2::_C_Test2_M::_C_Test2_M(Test2_M_Data & data)
  : Version(_dataMap), FileInfo(_dataMap), Section1(_dataMap)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TDouble, (void *)&data.Section1.Param1[0], NULL, NULL, NULL);
  _dataMap[5].setMap(_sectionName[2], _name[5], TDouble, (void *)&data.Section1.Param2[0], NULL, NULL, NULL);
}

Test2::_C_Test2_M::_C_Version::_C_Version(const DataMap * dataMap)
{
  FormatVersion.initialize(dataMap, 0);
  DataVersion.initialize(dataMap, 1);
}

Test2::_C_Test2_M::_C_FileInfo::_C_FileInfo(const DataMap * dataMap)
{
  ReadOnly.initialize(dataMap, 2);
  FileName.initialize(dataMap, 3);
}

Test2::_C_Test2_M::_C_Section1::_C_Section1(const DataMap * dataMap)
{
  Param1[0].initialize(dataMap, 4);
  Param2[0].initialize(dataMap, 5);
}

void Test2::_C_Test2_M::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 6, readStatus);
}

void Test2::_C_Test2_M::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 6, writeStatus);
}

bool Test2::_C_Test2_M::_C_Version::_C_FormatVersion::set(const char * value)
{
  bool setOK = true;
  const char ** valuePtr = (const char **)_dataMap[_mapIdx]._value;
  if ( setOK )
  {
    if ( *valuePtr ) delete[] *valuePtr;
    *valuePtr = new char[strlen(value)+1];
    strcpy(*(char **)valuePtr, value);
  }
  return setOK;

}

bool Test2::_C_Test2_M::_C_Version::_C_DataVersion::set(const char * value)
{
  bool setOK = true;
  const char ** valuePtr = (const char **)_dataMap[_mapIdx]._value;
  if ( setOK )
  {
    if ( *valuePtr ) delete[] *valuePtr;
    *valuePtr = new char[strlen(value)+1];
    strcpy(*(char **)valuePtr, value);
  }
  return setOK;

}

bool Test2::_C_Test2_M::_C_FileInfo::_C_ReadOnly::set(bool value)
{
  bool setOK = true;
  bool * valuePtr = (bool *)_dataMap[_mapIdx]._value;
  if ( setOK ) *valuePtr = value;
  return setOK;

}

bool Test2::_C_Test2_M::_C_FileInfo::_C_FileName::set(const char * value)
{
  bool setOK = true;
  const char ** valuePtr = (const char **)_dataMap[_mapIdx]._value;
  if ( setOK )
  {
    if ( *valuePtr ) delete[] *valuePtr;
    *valuePtr = new char[strlen(value)+1];
    strcpy(*(char **)valuePtr, value);
  }
  return setOK;

}

bool Test2::_C_Test2_M::_C_Section1::_C_Param1::set(double value)
{
  bool setOK = true;
  double * valuePtr = (double *)_dataMap[_mapIdx]._value;
  if ( setOK ) *valuePtr = value;
  return setOK;

}

