/*
 * Auto-generated file, do not edit
 */

#include <vxWorks.h>
#include "test1.h"

Test1::Test1_A_Data Test1::_Test1_A_Data;
Test1::_C_Test1_A Test1::Test1_A_Access(_Test1_A_Data);

Test1::Test1_B_Data Test1::_Test1_B_Data;
Test1::_C_Test1_B Test1::Test1_B_Access(_Test1_B_Data);

Test1::Test1_C_Data Test1::_Test1_C_Data;
Test1::_C_Test1_C Test1::Test1_C_Access(_Test1_C_Data);

Test1::Test1_D_Data Test1::_Test1_D_Data;
Test1::_C_Test1_D Test1::Test1_D_Access(_Test1_D_Data);

Test1::Test1_E_Data Test1::_Test1_E_Data;
Test1::_C_Test1_E Test1::Test1_E_Access(_Test1_E_Data);

Test1::Test1_F_Data Test1::_Test1_F_Data;
Test1::_C_Test1_F Test1::Test1_F_Access(_Test1_F_Data);

Test1::Test1_G_Data Test1::_Test1_G_Data;
Test1::_C_Test1_G Test1::Test1_G_Access(_Test1_G_Data);

Test1::Test1_H_Data Test1::_Test1_H_Data;
Test1::_C_Test1_H Test1::Test1_H_Access(_Test1_H_Data);

Test1::Test1_I_Data Test1::_Test1_I_Data;
Test1::_C_Test1_I Test1::Test1_I_Access(_Test1_I_Data);

Test1::Test1_J_Data Test1::_Test1_J_Data;
Test1::_C_Test1_J Test1::Test1_J_Access(_Test1_J_Data);

Test1::Test1_K_Data Test1::_Test1_K_Data;
Test1::_C_Test1_K Test1::Test1_K_Access(_Test1_K_Data);

Test1::Test1_L_Data Test1::_Test1_L_Data;
Test1::_C_Test1_L Test1::Test1_L_Access(_Test1_L_Data);

Test1::Test1_M_Data Test1::_Test1_M_Data;
Test1::_C_Test1_M Test1::Test1_M_Access(_Test1_M_Data);

Test1::Test1_O_Data Test1::_Test1_O_Data;
Test1::_C_Test1_O Test1::Test1_O_Access(_Test1_O_Data);

Test1::Test1_P_Data Test1::_Test1_P_Data;
Test1::_C_Test1_P Test1::Test1_P_Access(_Test1_P_Data);

Test1::Test1_Q_Data Test1::_Test1_Q_Data;
Test1::_C_Test1_Q Test1::Test1_Q_Access(_Test1_Q_Data);

Test1::Test1_S_Data Test1::_Test1_S_Data;
Test1::_C_Test1_S Test1::Test1_S_Access(_Test1_S_Data);

ConfigFile * Test1::getConfigFileObj(int index)
{
  switch (index)
  {
  case 0: return (ConfigFile *)&Test1_A_Access;
  case 1: return (ConfigFile *)&Test1_B_Access;
  case 2: return (ConfigFile *)&Test1_C_Access;
  case 3: return (ConfigFile *)&Test1_D_Access;
  case 4: return (ConfigFile *)&Test1_E_Access;
  case 5: return (ConfigFile *)&Test1_F_Access;
  case 6: return (ConfigFile *)&Test1_G_Access;
  case 7: return (ConfigFile *)&Test1_H_Access;
  case 8: return (ConfigFile *)&Test1_I_Access;
  case 9: return (ConfigFile *)&Test1_J_Access;
  case 10: return (ConfigFile *)&Test1_K_Access;
  case 11: return (ConfigFile *)&Test1_L_Access;
  case 12: return (ConfigFile *)&Test1_M_Access;
  case 13: return (ConfigFile *)&Test1_O_Access;
  case 14: return (ConfigFile *)&Test1_P_Access;
  case 15: return (ConfigFile *)&Test1_Q_Access;
  case 16: return (ConfigFile *)&Test1_S_Access;
  default: return NULL;
  }
}

const ConfigData::EnumMap Test1::_enumMap[20] =
{
  { "enum1_id1", 0xd83faecf },
  { "enum1_id2", 0xc136ff75 },
  { "enum1_id3", 0xb631cfe3 },
  { "enum2_id1", 0x9f9fd41f },
  { "enum3_id1", 0xa2fffdaf },
  { "enum3_id2", 0xbbf6ac15 },
  { "enum3_id3", 0xccf19c83 },
  { "enum4_id1", 0x90df21bf },
  { "enum4_id2", 0x89d67005 },
  { "enum4_id3", 0xfed14093 },
  { "enum_k_id1", 0xa6d60de6 },
  { "enum_k_id2", 0xbfdf5c5c },
  { "enum_k_id3", 0xc8d86cca },
  { "enum_o_id1", 0xd356ab26 },
  { "enum_o_id2", 0xca5ffa9c },
  { "enum_o_id3", 0xbd58ca0a },
  { "enumP_id1", 0xfc6df032 },
  { "enumP_id2", 0xe564a188 },
  { "enumP_id3", 0x9263911e },
  { NULL, 0 }
};

const char * Test1::_C_Test1_A::_name[20] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param1",
  "Param2",
  "Param3",
  "Param4",
  "Param5",
  "Param6",
  "Param7",
  "Param8",
  "Param9",
  "Param10",
  "Param11",
  "Param12",
  "Param13",
  "Param14",
  "Param15",
  "Param16"
};

const char * Test1::_C_Test1_A::_sectionName[4] =
{
  "Version",
  "FileInfo",
  "Section1",
  "Section2"
};

Test1::_C_Test1_A::_C_Test1_A(Test1_A_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TEnum, (void *)&data.Section1.Param1, _enumMap, NULL, &Test1::_C_Test1_A::_C_Section1::_C_Param1::validate);
  _dataMap[5].setMap(_sectionName[2], _name[5], TEnum, (void *)&data.Section1.Param2, _enumMap, NULL, &Test1::_C_Test1_A::_C_Section1::_C_Param2::validate);
  _dataMap[6].setMap(_sectionName[3], _name[6], TBool, (void *)&data.Section2.Param3, NULL, NULL, NULL);
  _dataMap[7].setMap(_sectionName[3], _name[7], TBool, (void *)&data.Section2.Param4, NULL, NULL, NULL);
  _dataMap[8].setMap(_sectionName[3], _name[8], TString, (void *)&data.Section2.Param5, NULL, NULL, NULL);
  _dataMap[9].setMap(_sectionName[3], _name[9], TString, (void *)&data.Section2.Param6, NULL, NULL, NULL);
  _dataMap[10].setMap(_sectionName[3], _name[10], TLong, (void *)&data.Section2.Param7, NULL, NULL, NULL);
  _dataMap[11].setMap(_sectionName[3], _name[11], TLong, (void *)&data.Section2.Param8, NULL, (RangeFunc *)&Test1::_C_Test1_A::_C_Section2::_C_Param8::getRange, NULL);
  _dataMap[12].setMap(_sectionName[3], _name[12], TLong, (void *)&data.Section2.Param9, NULL, (RangeFunc *)&Test1::_C_Test1_A::_C_Section2::_C_Param9::getRange, NULL);
  _dataMap[13].setMap(_sectionName[3], _name[13], TLong, (void *)&data.Section2.Param10, NULL, (RangeFunc *)&Test1::_C_Test1_A::_C_Section2::_C_Param10::getRange, NULL);
  _dataMap[14].setMap(_sectionName[3], _name[14], TLong, (void *)&data.Section2.Param11, NULL, NULL, &Test1::_C_Test1_A::_C_Section2::_C_Param11::validate);
  _dataMap[15].setMap(_sectionName[3], _name[15], TDouble, (void *)&data.Section2.Param12, NULL, (RangeFunc *)&Test1::_C_Test1_A::_C_Section2::_C_Param12::getRange, NULL);
  _dataMap[16].setMap(_sectionName[3], _name[16], TDouble, (void *)&data.Section2.Param13, NULL, (RangeFunc *)&Test1::_C_Test1_A::_C_Section2::_C_Param13::getRange, NULL);
  _dataMap[17].setMap(_sectionName[3], _name[17], TDouble, (void *)&data.Section2.Param14, NULL, NULL, &Test1::_C_Test1_A::_C_Section2::_C_Param14::validate);
  _dataMap[18].setMap(_sectionName[3], _name[18], TDouble, (void *)&data.Section2.Param15, NULL, (RangeFunc *)&Test1::_C_Test1_A::_C_Section2::_C_Param15::getRange, &Test1::_C_Test1_A::_C_Section2::_C_Param15::validate);
  _dataMap[19].setMap(_sectionName[3], _name[19], TDouble, (void *)&data.Section2.Param16, NULL, (RangeFunc *)&Test1::_C_Test1_A::_C_Section2::_C_Param16::getRange, &Test1::_C_Test1_A::_C_Section2::_C_Param16::validate);
}

Test1::_C_Test1_A::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test1::_C_Test1_A::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test1::_C_Test1_A::_C_Section1::_C_Section1(void)
{
  Param1.initialize(4);
  Param2.initialize(5);
}

Test1::_C_Test1_A::_C_Section2::_C_Section2(void)
{
  Param3.initialize(6);
  Param4.initialize(7);
  Param5.initialize(8);
  Param6.initialize(9);
  Param7.initialize(10);
  Param8.initialize(11);
  Param9.initialize(12);
  Param10.initialize(13);
  Param11.initialize(14);
  Param12.initialize(15);
  Param13.initialize(16);
  Param14.initialize(17);
  Param15.initialize(18);
  Param16.initialize(19);
}

void Test1::_C_Test1_A::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 20, readStatus);
}

void Test1::_C_Test1_A::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 20, writeStatus);
}

bool Test1::_C_Test1_A::_C_Section1::_C_Param1::validate(void * valuePtr)
{
  if ( !Test1::validateParam1(valuePtr) ) return false;
  return true;

}

bool Test1::_C_Test1_A::_C_Section1::_C_Param2::validate(void * valuePtr)
{
  if ( !Test1::validateParam2(valuePtr) ) return false;
  return true;

}

bool Test1::_C_Test1_A::_C_Section2::_C_Param11::validate(void * valuePtr)
{
  long value = *(long *)valuePtr;
return ( value < 0 );

}

bool Test1::_C_Test1_A::_C_Section2::_C_Param14::validate(void * valuePtr)
{
  double value = *(double *)valuePtr;
return ( value < 1e20 );

}

bool Test1::_C_Test1_A::_C_Section2::_C_Param15::validate(void * valuePtr)
{
  double value = *(double *)valuePtr;
return true;

}

bool Test1::_C_Test1_A::_C_Section2::_C_Param16::validate(void * valuePtr)
{
  double value = *(double *)valuePtr;
return true;

}

bool Test1::_C_Test1_A::_C_Section2::_C_Param10::getRange(long & min, long & max)
{
min=0; max=0x22222; return true;

}

bool Test1::_C_Test1_A::_C_Section2::_C_Param13::getRange(double & min, double & max)
{
min=0; max=0; return false;

}

const char * Test1::_C_Test1_B::_name[5] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param101"
};

const char * Test1::_C_Test1_B::_sectionName[3] =
{
  "Version",
  "FileInfo",
  "Section1"
};

Test1::_C_Test1_B::_C_Test1_B(Test1_B_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TEnum, (void *)&data.Section1.Param101, _enumMap, NULL, &Test1::_C_Test1_B::_C_Section1::_C_Param101::validate);
}

Test1::_C_Test1_B::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test1::_C_Test1_B::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test1::_C_Test1_B::_C_Section1::_C_Section1(void)
{
  Param101.initialize(4);
}

void Test1::_C_Test1_B::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 5, readStatus);
}

void Test1::_C_Test1_B::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 5, writeStatus);
}

bool Test1::_C_Test1_B::_C_Section1::_C_Param101::validate(void * valuePtr)
{
  if ( !Test1::validateParam101(valuePtr) ) return false;
  return true;

}

const char * Test1::_C_Test1_C::_name[11] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param201",
  "Param1",
  "Param2",
  "Param3",
  "Param4",
  "Param5",
  "Param6"
};

const char * Test1::_C_Test1_C::_sectionName[4] =
{
  "Version",
  "FileInfo",
  "Section1",
  "Section2"
};

Test1::_C_Test1_C::_C_Test1_C(Test1_C_Data & data)
  : Version(_dataMap), FileInfo(_dataMap), Section1(_dataMap), Section2(_dataMap)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TEnum, (void *)&data.Section1.Param201, _enumMap, NULL, &Test1::_C_Test1_C::_C_Section1::_C_Param201::validate);
  _dataMap[5].setMap(_sectionName[3], _name[5], TLong, (void *)&data.Section2.Param1, NULL, (RangeFunc *)&Test1::_C_Test1_C::_C_Section2::_C_Param1::getRange, NULL);
  _dataMap[6].setMap(_sectionName[3], _name[6], TLong, (void *)&data.Section2.Param2, NULL, (RangeFunc *)&Test1::_C_Test1_C::_C_Section2::_C_Param2::getRange, NULL);
  _dataMap[7].setMap(_sectionName[3], _name[7], TLong, (void *)&data.Section2.Param3, NULL, (RangeFunc *)&Test1::_C_Test1_C::_C_Section2::_C_Param3::getRange, NULL);
  _dataMap[8].setMap(_sectionName[3], _name[8], TLong, (void *)&data.Section2.Param4, NULL, (RangeFunc *)&Test1::_C_Test1_C::_C_Section2::_C_Param4::getRange, &Test1::_C_Test1_C::_C_Section2::_C_Param4::validate);
  _dataMap[9].setMap(_sectionName[3], _name[9], TLong, (void *)&data.Section2.Param5, NULL, (RangeFunc *)&Test1::_C_Test1_C::_C_Section2::_C_Param5::getRange, &Test1::_C_Test1_C::_C_Section2::_C_Param5::validate);
  _dataMap[10].setMap(_sectionName[3], _name[10], TLong, (void *)&data.Section2.Param6, NULL, (RangeFunc *)&Test1::_C_Test1_C::_C_Section2::_C_Param6::getRange, &Test1::_C_Test1_C::_C_Section2::_C_Param6::validate);
}

Test1::_C_Test1_C::_C_Version::_C_Version(const DataMap * dataMap)
{
  FormatVersion.initialize(dataMap, 0);
  DataVersion.initialize(dataMap, 1);
}

Test1::_C_Test1_C::_C_FileInfo::_C_FileInfo(const DataMap * dataMap)
{
  ReadOnly.initialize(dataMap, 2);
  FileName.initialize(dataMap, 3);
}

Test1::_C_Test1_C::_C_Section1::_C_Section1(const DataMap * dataMap)
{
  Param201.initialize(dataMap, 4);
}

Test1::_C_Test1_C::_C_Section2::_C_Section2(const DataMap * dataMap)
{
  Param1.initialize(dataMap, 5);
  Param2.initialize(dataMap, 6);
  Param3.initialize(dataMap, 7);
  Param4.initialize(dataMap, 8);
  Param5.initialize(dataMap, 9);
  Param6.initialize(dataMap, 10);
}

void Test1::_C_Test1_C::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 11, readStatus);
}

void Test1::_C_Test1_C::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 11, writeStatus);
}

bool Test1::_C_Test1_C::_C_Section1::_C_Param201::validate(void * valuePtr)
{
  if ( !Test1::validateParam201(valuePtr) ) return false;
  return true;

}

bool Test1::_C_Test1_C::_C_Section2::_C_Param4::validate(void * valuePtr)
{
  long value = *(long *)valuePtr;
return ( value <= 0 );

}

bool Test1::_C_Test1_C::_C_Section2::_C_Param5::validate(void * valuePtr)
{
  long value = *(long *)valuePtr;
return ( value > 0 );

}

bool Test1::_C_Test1_C::_C_Section2::_C_Param6::validate(void * valuePtr)
{
  long value = *(long *)valuePtr;
return ( value > 0 );

}

bool Test1::_C_Test1_C::_C_Section2::_C_Param2::getRange(long & min, long & max)
{
min=-2; max=-1; return true;

}

bool Test1::_C_Test1_C::_C_Section2::_C_Param3::getRange(long & min, long & max)
{
min=-2; max=-1; return false;

}

bool Test1::_C_Test1_C::_C_Section2::_C_Param5::getRange(long & min, long & max)
{
min=-2; max=2; return true;

}

bool Test1::_C_Test1_C::_C_Section2::_C_Param6::getRange(long & min, long & max)
{
min=-2; max=2; return false;

}

bool Test1::_C_Test1_C::_C_Version::_C_FormatVersion::set(const char * value)
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

bool Test1::_C_Test1_C::_C_Version::_C_DataVersion::set(const char * value)
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

bool Test1::_C_Test1_C::_C_FileInfo::_C_ReadOnly::set(bool value)
{
  bool setOK = true;
  bool * valuePtr = (bool *)_dataMap[_mapIdx]._value;
  if ( setOK ) *valuePtr = value;
  return setOK;

}

bool Test1::_C_Test1_C::_C_FileInfo::_C_FileName::set(const char * value)
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

bool Test1::_C_Test1_C::_C_Section1::_C_Param201::set(TParam201 value)
{
  bool setOK = true;
  TParam201 * valuePtr = (TParam201 *)_dataMap[_mapIdx]._value;
  if ( !validate((void *)&value) ) setOK = false;

  if ( setOK ) *valuePtr = value;
  return setOK;

}

bool Test1::_C_Test1_C::_C_Section2::_C_Param1::set(long value)
{
  bool setOK = true;
  long * valuePtr = (long *)_dataMap[_mapIdx]._value;
  if ( value < 1 || value > 2 ) setOK = false;

  if ( setOK ) *valuePtr = value;
  return setOK;

}

bool Test1::_C_Test1_C::_C_Section2::_C_Param2::set(long value)
{
  bool setOK = true;
  long * valuePtr = (long *)_dataMap[_mapIdx]._value;
  long min, max;
  if ( getRange(min, max) &&
       ( value < min || value > max )) setOK = false;

  if ( setOK ) *valuePtr = value;
  return setOK;

}

bool Test1::_C_Test1_C::_C_Section2::_C_Param3::set(long value)
{
  bool setOK = true;
  long * valuePtr = (long *)_dataMap[_mapIdx]._value;
  long min, max;
  if ( getRange(min, max) &&
       ( value < min || value > max )) setOK = false;

  if ( setOK ) *valuePtr = value;
  return setOK;

}

bool Test1::_C_Test1_C::_C_Section2::_C_Param4::set(long value)
{
  bool setOK = true;
  long * valuePtr = (long *)_dataMap[_mapIdx]._value;
  if ( value < -1 || value > 1 ) setOK = false;

  if ( !validate((void *)&value) ) setOK = false;

  if ( setOK ) *valuePtr = value;
  return setOK;

}

bool Test1::_C_Test1_C::_C_Section2::_C_Param5::set(long value)
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

bool Test1::_C_Test1_C::_C_Section2::_C_Param6::set(long value)
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

const char * Test1::_C_Test1_D::_name[5] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param1"
};

const char * Test1::_C_Test1_D::_sectionName[3] =
{
  "Version",
  "FileInfo",
  "Section1"
};

Test1::_C_Test1_D::_C_Test1_D(Test1_D_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TLong, (void *)&data.Section1.Param1, NULL, (RangeFunc *)&Test1::_C_Test1_D::_C_Section1::_C_Param1::getRange, NULL);
}

Test1::_C_Test1_D::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test1::_C_Test1_D::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test1::_C_Test1_D::_C_Section1::_C_Section1(void)
{
  Param1.initialize(4);
}

void Test1::_C_Test1_D::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 5, readStatus);
}

void Test1::_C_Test1_D::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 5, writeStatus);
}

const char * Test1::_C_Test1_E::_name[5] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param1"
};

const char * Test1::_C_Test1_E::_sectionName[3] =
{
  "Version",
  "FileInfo",
  "Section1"
};

Test1::_C_Test1_E::_C_Test1_E(Test1_E_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TLong, (void *)&data.Section1.Param1, NULL, (RangeFunc *)&Test1::_C_Test1_E::_C_Section1::_C_Param1::getRange, NULL);
}

Test1::_C_Test1_E::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test1::_C_Test1_E::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test1::_C_Test1_E::_C_Section1::_C_Section1(void)
{
  Param1.initialize(4);
}

void Test1::_C_Test1_E::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 5, readStatus);
}

void Test1::_C_Test1_E::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 5, writeStatus);
}

bool Test1::_C_Test1_E::_C_Section1::_C_Param1::getRange(long & min, long & max)
{
min=0; max=1; return true;

}

const char * Test1::_C_Test1_F::_name[5] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param1"
};

const char * Test1::_C_Test1_F::_sectionName[3] =
{
  "Version",
  "FileInfo",
  "Section1"
};

Test1::_C_Test1_F::_C_Test1_F(Test1_F_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TLong, (void *)&data.Section1.Param1, NULL, NULL, &Test1::_C_Test1_F::_C_Section1::_C_Param1::validate);
}

Test1::_C_Test1_F::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test1::_C_Test1_F::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test1::_C_Test1_F::_C_Section1::_C_Section1(void)
{
  Param1.initialize(4);
}

void Test1::_C_Test1_F::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 5, readStatus);
}

void Test1::_C_Test1_F::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 5, writeStatus);
}

bool Test1::_C_Test1_F::_C_Section1::_C_Param1::validate(void * valuePtr)
{
  long value = *(long *)valuePtr;
return ( value == 2 );

}

const char * Test1::_C_Test1_G::_name[5] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param1"
};

const char * Test1::_C_Test1_G::_sectionName[3] =
{
  "Version",
  "FileInfo",
  "Section1"
};

Test1::_C_Test1_G::_C_Test1_G(Test1_G_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TDouble, (void *)&data.Section1.Param1, NULL, (RangeFunc *)&Test1::_C_Test1_G::_C_Section1::_C_Param1::getRange, NULL);
}

Test1::_C_Test1_G::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test1::_C_Test1_G::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test1::_C_Test1_G::_C_Section1::_C_Section1(void)
{
  Param1.initialize(4);
}

void Test1::_C_Test1_G::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 5, readStatus);
}

void Test1::_C_Test1_G::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 5, writeStatus);
}

const char * Test1::_C_Test1_H::_name[5] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param1"
};

const char * Test1::_C_Test1_H::_sectionName[3] =
{
  "Version",
  "FileInfo",
  "Section1"
};

Test1::_C_Test1_H::_C_Test1_H(Test1_H_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TDouble, (void *)&data.Section1.Param1, NULL, (RangeFunc *)&Test1::_C_Test1_H::_C_Section1::_C_Param1::getRange, NULL);
}

Test1::_C_Test1_H::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test1::_C_Test1_H::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test1::_C_Test1_H::_C_Section1::_C_Section1(void)
{
  Param1.initialize(4);
}

void Test1::_C_Test1_H::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 5, readStatus);
}

void Test1::_C_Test1_H::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 5, writeStatus);
}

bool Test1::_C_Test1_H::_C_Section1::_C_Param1::getRange(double & min, double & max)
{
min=0.0; max=0.1; return true;

}

const char * Test1::_C_Test1_I::_name[5] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param1"
};

const char * Test1::_C_Test1_I::_sectionName[3] =
{
  "Version",
  "FileInfo",
  "Section1"
};

Test1::_C_Test1_I::_C_Test1_I(Test1_I_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TDouble, (void *)&data.Section1.Param1, NULL, NULL, &Test1::_C_Test1_I::_C_Section1::_C_Param1::validate);
}

Test1::_C_Test1_I::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test1::_C_Test1_I::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test1::_C_Test1_I::_C_Section1::_C_Section1(void)
{
  Param1.initialize(4);
}

void Test1::_C_Test1_I::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 5, readStatus);
}

void Test1::_C_Test1_I::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 5, writeStatus);
}

bool Test1::_C_Test1_I::_C_Section1::_C_Param1::validate(void * valuePtr)
{
  double value = *(double *)valuePtr;
return ( value < 1.4999 );

}

const char * Test1::_C_Test1_J::_name[6] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param1",
  "Param2"
};

const char * Test1::_C_Test1_J::_sectionName[3] =
{
  "Version",
  "FileInfo",
  "Section1"
};

Test1::_C_Test1_J::_C_Test1_J(Test1_J_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TString, (void *)&data.Section1.Param1, NULL, NULL, &Test1::_C_Test1_J::_C_Section1::_C_Param1::validate);
  _dataMap[5].setMap(_sectionName[2], _name[5], TString, (void *)&data.Section1.Param2, NULL, NULL, &Test1::_C_Test1_J::_C_Section1::_C_Param2::validate);
}

Test1::_C_Test1_J::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test1::_C_Test1_J::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test1::_C_Test1_J::_C_Section1::_C_Section1(void)
{
  Param1.initialize(4);
  Param2.initialize(5);
}

void Test1::_C_Test1_J::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 6, readStatus);
}

void Test1::_C_Test1_J::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 6, writeStatus);
}

bool Test1::_C_Test1_J::_C_Section1::_C_Param1::validate(void * valuePtr)
{
  const char * value = *(const char **)valuePtr;
return ( value[0] == 'a' );

}

bool Test1::_C_Test1_J::_C_Section1::_C_Param2::validate(void * valuePtr)
{
  const char * value = *(const char **)valuePtr;
return ( value[0] == 'b' );

}

const char * Test1::_C_Test1_K::_name[6] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param1",
  "Param2"
};

const char * Test1::_C_Test1_K::_sectionName[3] =
{
  "Version",
  "FileInfo",
  "Section1"
};

Test1::_C_Test1_K::_C_Test1_K(Test1_K_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TBool, (void *)&data.Section1.Param1, NULL, NULL, &Test1::_C_Test1_K::_C_Section1::_C_Param1::validate);
  _dataMap[5].setMap(_sectionName[2], _name[5], TBool, (void *)&data.Section1.Param2, NULL, NULL, &Test1::_C_Test1_K::_C_Section1::_C_Param2::validate);
}

Test1::_C_Test1_K::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test1::_C_Test1_K::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test1::_C_Test1_K::_C_Section1::_C_Section1(void)
{
  Param1.initialize(4);
  Param2.initialize(5);
}

void Test1::_C_Test1_K::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 6, readStatus);
}

void Test1::_C_Test1_K::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 6, writeStatus);
}

bool Test1::_C_Test1_K::_C_Section1::_C_Param1::validate(void * valuePtr)
{
  bool value = *(bool *)valuePtr;
return ( !value );

}

bool Test1::_C_Test1_K::_C_Section1::_C_Param2::validate(void * valuePtr)
{
  bool value = *(bool *)valuePtr;
return ( !value );

}

const char * Test1::_C_Test1_L::_name[6] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "ParamK",
  "ParamK"
};

const char * Test1::_C_Test1_L::_sectionName[4] =
{
  "Version",
  "FileInfo",
  "Section1",
  "Section2"
};

Test1::_C_Test1_L::_C_Test1_L(Test1_L_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TEnum, (void *)&data.Section1.ParamK, _enumMap, NULL, &Test1::_C_Test1_L::_C_Section1::_C_ParamK::validate);
  _dataMap[5].setMap(_sectionName[3], _name[5], TEnum, (void *)&data.Section2.ParamK, _enumMap, NULL, &Test1::_C_Test1_L::_C_Section2::_C_ParamK::validate);
}

Test1::_C_Test1_L::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test1::_C_Test1_L::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test1::_C_Test1_L::_C_Section1::_C_Section1(void)
{
  ParamK.initialize(4);
}

Test1::_C_Test1_L::_C_Section2::_C_Section2(void)
{
  ParamK.initialize(5);
}

void Test1::_C_Test1_L::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 6, readStatus);
}

void Test1::_C_Test1_L::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 6, writeStatus);
}

bool Test1::_C_Test1_L::_C_Section1::_C_ParamK::validate(void * valuePtr)
{
  if ( !Test1::validateParamK(valuePtr) ) return false;
  TParamK value = *(TParamK *)valuePtr;
return ( value == enum_k_id1 );

}

bool Test1::_C_Test1_L::_C_Section2::_C_ParamK::validate(void * valuePtr)
{
  if ( !Test1::validateParamK(valuePtr) ) return false;
  TParamK value = *(TParamK *)valuePtr;
return ( value == enum_k_id1 );

}

const char * Test1::_C_Test1_M::_name[6] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param1",
  "Param2"
};

const char * Test1::_C_Test1_M::_sectionName[3] =
{
  "Version",
  "FileInfo",
  "Section1"
};

Test1::_C_Test1_M::_C_Test1_M(Test1_M_Data & data)
  : Version(_dataMap), FileInfo(_dataMap), Section1(_dataMap)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TDouble, (void *)&data.Section1.Param1, NULL, NULL, NULL);
  _dataMap[5].setMap(_sectionName[2], _name[5], TDouble, (void *)&data.Section1.Param2, NULL, NULL, NULL);
}

Test1::_C_Test1_M::_C_Version::_C_Version(const DataMap * dataMap)
{
  FormatVersion.initialize(dataMap, 0);
  DataVersion.initialize(dataMap, 1);
}

Test1::_C_Test1_M::_C_FileInfo::_C_FileInfo(const DataMap * dataMap)
{
  ReadOnly.initialize(dataMap, 2);
  FileName.initialize(dataMap, 3);
}

Test1::_C_Test1_M::_C_Section1::_C_Section1(const DataMap * dataMap)
{
  Param1.initialize(dataMap, 4);
  Param2.initialize(dataMap, 5);
}

void Test1::_C_Test1_M::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 6, readStatus);
}

void Test1::_C_Test1_M::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 6, writeStatus);
}

bool Test1::_C_Test1_M::_C_Version::_C_FormatVersion::set(const char * value)
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

bool Test1::_C_Test1_M::_C_Version::_C_DataVersion::set(const char * value)
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

bool Test1::_C_Test1_M::_C_FileInfo::_C_ReadOnly::set(bool value)
{
  bool setOK = true;
  bool * valuePtr = (bool *)_dataMap[_mapIdx]._value;
  if ( setOK ) *valuePtr = value;
  return setOK;

}

bool Test1::_C_Test1_M::_C_FileInfo::_C_FileName::set(const char * value)
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

bool Test1::_C_Test1_M::_C_Section1::_C_Param1::set(double value)
{
  bool setOK = true;
  double * valuePtr = (double *)_dataMap[_mapIdx]._value;
  if ( setOK ) *valuePtr = value;
  return setOK;

}

const char * Test1::_C_Test1_O::_name[5] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "ParamO"
};

const char * Test1::_C_Test1_O::_sectionName[3] =
{
  "Version",
  "FileInfo",
  "Section1"
};

Test1::_C_Test1_O::_C_Test1_O(Test1_O_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TEnum, (void *)&data.Section1.ParamO, _enumMap, NULL, &Test1::_C_Test1_O::_C_Section1::_C_ParamO::validate);
}

Test1::_C_Test1_O::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test1::_C_Test1_O::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test1::_C_Test1_O::_C_Section1::_C_Section1(void)
{
  ParamO.initialize(4);
}

void Test1::_C_Test1_O::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 5, readStatus);
}

void Test1::_C_Test1_O::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 5, writeStatus);
}

bool Test1::_C_Test1_O::_C_Section1::_C_ParamO::validate(void * valuePtr)
{
  if ( !Test1::validateParamO(valuePtr) ) return false;
  return true;

}

const char * Test1::_C_Test1_P::_name[11] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "P1",
  "P2",
  "P3",
  "ParamP",
  "P1",
  "P2",
  "P3"
};

const char * Test1::_C_Test1_P::_sectionName[4] =
{
  "Version",
  "FileInfo",
  "Section1",
  "Section2"
};

Test1::_C_Test1_P::_C_Test1_P(Test1_P_Data & data)
  : Version(_dataMap), FileInfo(_dataMap), Section1(_dataMap), Section2(_dataMap)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TEnum, (void *)&data.Section1.P1, _enumMap, NULL, &Test1::_C_Test1_P::_C_Section1::_C_P1::validate);
  _dataMap[5].setMap(_sectionName[2], _name[5], TEnum, (void *)&data.Section1.P2, _enumMap, NULL, &Test1::_C_Test1_P::_C_Section1::_C_P2::validate);
  _dataMap[6].setMap(_sectionName[2], _name[6], TEnum, (void *)&data.Section1.P3, _enumMap, NULL, &Test1::_C_Test1_P::_C_Section1::_C_P3::validate);
  _dataMap[7].setMap(_sectionName[3], _name[7], TEnum, (void *)&data.Section2.ParamP, _enumMap, NULL, &Test1::_C_Test1_P::_C_Section2::_C_ParamP::validate);
  _dataMap[8].setMap(_sectionName[3], _name[8], TEnum, (void *)&data.Section2.P1, _enumMap, NULL, &Test1::_C_Test1_P::_C_Section2::_C_P1::validate);
  _dataMap[9].setMap(_sectionName[3], _name[9], TEnum, (void *)&data.Section2.P2, _enumMap, NULL, &Test1::_C_Test1_P::_C_Section2::_C_P2::validate);
  _dataMap[10].setMap(_sectionName[3], _name[10], TEnum, (void *)&data.Section2.P3, _enumMap, NULL, &Test1::_C_Test1_P::_C_Section2::_C_P3::validate);
}

Test1::_C_Test1_P::_C_Version::_C_Version(const DataMap * dataMap)
{
  FormatVersion.initialize(dataMap, 0);
  DataVersion.initialize(dataMap, 1);
}

Test1::_C_Test1_P::_C_FileInfo::_C_FileInfo(const DataMap * dataMap)
{
  ReadOnly.initialize(dataMap, 2);
  FileName.initialize(dataMap, 3);
}

Test1::_C_Test1_P::_C_Section1::_C_Section1(const DataMap * dataMap)
{
  P1.initialize(dataMap, 4);
  P2.initialize(dataMap, 5);
  P3.initialize(dataMap, 6);
}

Test1::_C_Test1_P::_C_Section2::_C_Section2(const DataMap * dataMap)
{
  ParamP.initialize(dataMap, 7);
  P1.initialize(dataMap, 8);
  P2.initialize(dataMap, 9);
  P3.initialize(dataMap, 10);
}

void Test1::_C_Test1_P::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 11, readStatus);
}

void Test1::_C_Test1_P::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 11, writeStatus);
}

bool Test1::_C_Test1_P::_C_Section1::_C_P1::validate(void * valuePtr)
{
  if ( !Test1::validateParamP(valuePtr) ) return false;
  TParamP value = *(TParamP *)valuePtr;
return true;

}

bool Test1::_C_Test1_P::_C_Section1::_C_P2::validate(void * valuePtr)
{
  if ( !Test1::validateParamP(valuePtr) ) return false;
  TParamP value = *(TParamP *)valuePtr;
return true;

}

bool Test1::_C_Test1_P::_C_Section1::_C_P3::validate(void * valuePtr)
{
  if ( !Test1::validateParamP(valuePtr) ) return false;
  return true;

}

bool Test1::_C_Test1_P::_C_Section2::_C_ParamP::validate(void * valuePtr)
{
  if ( !Test1::validateParamP(valuePtr) ) return false;
  TParamP value = *(TParamP *)valuePtr;
return true;

}

bool Test1::_C_Test1_P::_C_Section2::_C_P1::validate(void * valuePtr)
{
  if ( !Test1::validateParamP(valuePtr) ) return false;
  return true;

}

bool Test1::_C_Test1_P::_C_Section2::_C_P2::validate(void * valuePtr)
{
  if ( !Test1::validateParamP(valuePtr) ) return false;
  return true;

}

bool Test1::_C_Test1_P::_C_Section2::_C_P3::validate(void * valuePtr)
{
  if ( !Test1::validateParamP(valuePtr) ) return false;
  return true;

}

bool Test1::_C_Test1_P::_C_Version::_C_FormatVersion::set(const char * value)
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

bool Test1::_C_Test1_P::_C_Version::_C_DataVersion::set(const char * value)
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

bool Test1::_C_Test1_P::_C_FileInfo::_C_ReadOnly::set(bool value)
{
  bool setOK = true;
  bool * valuePtr = (bool *)_dataMap[_mapIdx]._value;
  if ( setOK ) *valuePtr = value;
  return setOK;

}

bool Test1::_C_Test1_P::_C_FileInfo::_C_FileName::set(const char * value)
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

bool Test1::_C_Test1_P::_C_Section1::_C_P1::set(TParamP value)
{
  bool setOK = true;
  TParamP * valuePtr = (TParamP *)_dataMap[_mapIdx]._value;
  if ( !validate((void *)&value) ) setOK = false;

  if ( setOK ) *valuePtr = value;
  return setOK;

}

bool Test1::_C_Test1_P::_C_Section1::_C_P3::set(TParamP value)
{
  bool setOK = true;
  TParamP * valuePtr = (TParamP *)_dataMap[_mapIdx]._value;
  if ( !validate((void *)&value) ) setOK = false;

  if ( setOK ) *valuePtr = value;
  return setOK;

}

bool Test1::_C_Test1_P::_C_Section2::_C_ParamP::set(TParamP value)
{
  bool setOK = true;
  TParamP * valuePtr = (TParamP *)_dataMap[_mapIdx]._value;
  if ( !validate((void *)&value) ) setOK = false;

  if ( setOK ) *valuePtr = value;
  return setOK;

}

bool Test1::_C_Test1_P::_C_Section2::_C_P2::set(TParamP value)
{
  bool setOK = true;
  TParamP * valuePtr = (TParamP *)_dataMap[_mapIdx]._value;
  if ( !validate((void *)&value) ) setOK = false;

  if ( setOK ) *valuePtr = value;
  return setOK;

}

bool Test1::_C_Test1_P::_C_Section2::_C_P3::set(TParamP value)
{
  bool setOK = true;
  TParamP * valuePtr = (TParamP *)_dataMap[_mapIdx]._value;
  if ( !validate((void *)&value) ) setOK = false;

  if ( setOK ) *valuePtr = value;
  return setOK;

}

const char * Test1::_C_Test1_Q::_name[7] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param1",
  "Param2",
  "Param1"
};

const char * Test1::_C_Test1_Q::_sectionName[5] =
{
  "Version",
  "FileInfo",
  "Section2",
  "Section3",
  "Section1"
};

Test1::_C_Test1_Q::_C_Test1_Q(Test1_Q_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TLong, (void *)&data.Section2.Param1, NULL, NULL, NULL);
  _dataMap[5].setMap(_sectionName[3], _name[5], TLong, (void *)&data.Section3.Param2, NULL, NULL, NULL);
  _dataMap[6].setMap(_sectionName[4], _name[6], TLong, (void *)&data.Section1.Param1, NULL, (RangeFunc *)&Test1::_C_Test1_Q::_C_Section1::_C_Param1::getRange, NULL);
}

Test1::_C_Test1_Q::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test1::_C_Test1_Q::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test1::_C_Test1_Q::_C_Section2::_C_Section2(void)
{
  Param1.initialize(4);
}

Test1::_C_Test1_Q::_C_Section3::_C_Section3(void)
{
  Param2.initialize(5);
}

Test1::_C_Test1_Q::_C_Section1::_C_Section1(void)
{
  Param1.initialize(6);
}

void Test1::_C_Test1_Q::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 7, readStatus);
}

void Test1::_C_Test1_Q::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 7, writeStatus);
}

const char * Test1::_C_Test1_S::_name[5] =
{
  "FormatVersion",
  "DataVersion",
  "ReadOnly",
  "FileName",
  "Param1"
};

const char * Test1::_C_Test1_S::_sectionName[3] =
{
  "Version",
  "FileInfo",
  "Section1"
};

Test1::_C_Test1_S::_C_Test1_S(Test1_S_Data & data)
{
  _dataMap[0].setMap(_sectionName[0], _name[0], TString, (void *)&data.Version.FormatVersion, NULL, NULL, NULL);
  _dataMap[1].setMap(_sectionName[0], _name[1], TString, (void *)&data.Version.DataVersion, NULL, NULL, NULL);
  _dataMap[2].setMap(_sectionName[1], _name[2], TBool, (void *)&data.FileInfo.ReadOnly, NULL, NULL, NULL);
  _dataMap[3].setMap(_sectionName[1], _name[3], TString, (void *)&data.FileInfo.FileName, NULL, NULL, NULL);
  _dataMap[4].setMap(_sectionName[2], _name[4], TLong, (void *)&data.Section1.Param1, NULL, (RangeFunc *)&Test1::_C_Test1_S::_C_Section1::_C_Param1::getRange, NULL);
}

Test1::_C_Test1_S::_C_Version::_C_Version(void)
{
  FormatVersion.initialize(0);
  DataVersion.initialize(1);
}

Test1::_C_Test1_S::_C_FileInfo::_C_FileInfo(void)
{
  ReadOnly.initialize(2);
  FileName.initialize(3);
}

Test1::_C_Test1_S::_C_Section1::_C_Section1(void)
{
  Param1.initialize(4);
}

void Test1::_C_Test1_S::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )
{
   ConfigFile::logData(level, _dataMap, 5, readStatus);
}

void Test1::_C_Test1_S::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )
{
   ConfigFile::logData(level, _dataMap, 5, writeStatus);
}

