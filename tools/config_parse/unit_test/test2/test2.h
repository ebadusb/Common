/*
 * Auto-generated file, do not edit
 *
 * Source Files:
 * ./test2_a.cfg formatVersion=2a dataVersion=d2a
 * ./test2_b.cfg formatVersion=2b dataVersion=d2b
 * ./test2_c.cfg formatVersion=2c dataVersion=d2c
 * ./test2_d.cfg formatVersion=1 dataVersion=1
 * ./test2_e.cfg formatVersion=1 dataVersion=1
 * ./test2_f.cfg formatVersion=1 dataVersion=1
 * ./test2_g.cfg formatVersion=1 dataVersion=1
 * ./test2_h.cfg formatVersion=1 dataVersion=1
 * ./test2_i.cfg formatVersion=1 dataVersion=1
 * ./test2_j.cfg formatVersion=1 dataVersion=1
 * ./test2_k.cfg formatVersion=1 dataVersion=1
 * ./test2_l.cfg formatVersion=1 dataVersion=1
 * ./test2_m.cfg formatVersion=2m dataVersion=d2m
 *
 */

#ifndef _test2_INCLUDE
#define _test2_INCLUDE

#include <vxWorks.h>
#include <limits.h>

#include "base/datalog/taos_datalog.h"
#include "base/config/file_names.h"
#include "base/module_entry/common/config_helper.h"

#include  <vxWorks.h>
#include  "stdio.h"
#include  "stdlib.h"
class Test2 : public ConfigData
{
public:
  enum TParam1 { enum1_id1 = 0xd83faecf, enum1_id2 = 0x4136ff75, enum1_id3 = 0x3631cfe3 };
  static bool validateParam1(void *value)
  {
    return ( *(unsigned long *)value == 0xd83faecf ||
              *(unsigned long *)value == 0x4136ff75 ||
              *(unsigned long *)value == 0x3631cfe3 );
  }

  enum TParam2 { enum2_id1 = 0x9f9fd41f, enum2_id2 = 0x69685a5 };
  static bool validateParam2(void *value)
  {
    return ( *(unsigned long *)value == 0x9f9fd41f ||
              *(unsigned long *)value == 0x69685a5 );
  }

  enum TParam101 { enum3_id1 = 0xa2fffdaf, enum3_id2 = 0x3bf6ac15, enum3_id3 = 0x4cf19c83 };
  static bool validateParam101(void *value)
  {
    return ( *(unsigned long *)value == 0xa2fffdaf ||
              *(unsigned long *)value == 0x3bf6ac15 ||
              *(unsigned long *)value == 0x4cf19c83 );
  }

  enum TParam201 { enum4_id1 = 0x90df21bf, enum4_id2 = 0x89d67005, enum4_id3 = 0xfed14093 };
  static bool validateParam201(void *value)
  {
    return ( *(unsigned long *)value == 0x90df21bf ||
              *(unsigned long *)value == 0x89d67005 ||
              *(unsigned long *)value == 0xfed14093 );
  }

  enum TParamK { enum_k_id1 = 0xa6d60de6, enum_k_id2 = 0xbfdf5c5c, enum_k_id3 = 0xc8d86cca };
  static bool validateParamK(void *value)
  {
    return ( *(unsigned long *)value == 0xa6d60de6 ||
              *(unsigned long *)value == 0xbfdf5c5c ||
              *(unsigned long *)value == 0xc8d86cca );
  }


private:
  static const ConfigData::EnumMap _enumMap[15];

public:
  struct Test2_A_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      TParam1 Param1[2];
      TParam2 Param2[2];
    } Section1;
    struct
    {
      bool Param3[2];
      const char * Param4[2];
      long Param5[2];
      long Param6[2];
      long Param7[2];
      long Param8[2];
      double Param9[2];
      double Param10[2];
      double Param11[2];
      double Param12[2];
      double Param13[2];
    } Section2;
  Test2_A_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test2_A_Data & Test2_A(void) { return _Test2_A_Data; }

private:
  static Test2_A_Data _Test2_A_Data;

public:
  struct Test2_B_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      TParam101 Param101[1];
    } Section1;
    struct
    {
      long Param[16];
    } Section2;
    struct
    {
      long Param;
    } Section3;
  Test2_B_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test2_B_Data & Test2_B(void) { return _Test2_B_Data; }

private:
  static Test2_B_Data _Test2_B_Data;

public:
  struct Test2_C_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      TParam201 Param201[1];
    } Section1;
    struct
    {
      long Param1[1];
      long Param2[1];
      long Param3[1];
      long Param4[1];
      long Param5[1];
      long Param6[1];
    } Section2;
  Test2_C_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test2_C_Data & Test2_C(void) { return _Test2_C_Data; }

private:
  static Test2_C_Data _Test2_C_Data;

public:
  struct Test2_D_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      long Param1[1];
    } Section1;
  Test2_D_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test2_D_Data & Test2_D(void) { return _Test2_D_Data; }

private:
  static Test2_D_Data _Test2_D_Data;

public:
  struct Test2_E_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      long Param1[1];
    } Section1;
  Test2_E_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test2_E_Data & Test2_E(void) { return _Test2_E_Data; }

private:
  static Test2_E_Data _Test2_E_Data;

public:
  struct Test2_F_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      long Param1[1];
    } Section1;
  Test2_F_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test2_F_Data & Test2_F(void) { return _Test2_F_Data; }

private:
  static Test2_F_Data _Test2_F_Data;

public:
  struct Test2_G_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      double Param1[1];
    } Section1;
  Test2_G_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test2_G_Data & Test2_G(void) { return _Test2_G_Data; }

private:
  static Test2_G_Data _Test2_G_Data;

public:
  struct Test2_H_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      double Param1[1];
    } Section1;
  Test2_H_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test2_H_Data & Test2_H(void) { return _Test2_H_Data; }

private:
  static Test2_H_Data _Test2_H_Data;

public:
  struct Test2_I_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      double Param1[1];
    } Section1;
  Test2_I_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test2_I_Data & Test2_I(void) { return _Test2_I_Data; }

private:
  static Test2_I_Data _Test2_I_Data;

public:
  struct Test2_J_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      const char * Param1[1];
      const char * Param2[1];
    } Section1;
  Test2_J_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test2_J_Data & Test2_J(void) { return _Test2_J_Data; }

private:
  static Test2_J_Data _Test2_J_Data;

public:
  struct Test2_K_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      bool Param1[1];
      bool Param2[1];
    } Section1;
  Test2_K_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test2_K_Data & Test2_K(void) { return _Test2_K_Data; }

private:
  static Test2_K_Data _Test2_K_Data;

public:
  struct Test2_L_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      TParamK ParamK[1];
    } Section1;
    struct
    {
      TParamK ParamK[1];
    } Section2;
  Test2_L_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test2_L_Data & Test2_L(void) { return _Test2_L_Data; }

private:
  static Test2_L_Data _Test2_L_Data;

public:
  struct Test2_M_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      double Param1[1];
      double Param2[1];
    } Section1;
  Test2_M_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test2_M_Data & Test2_M(void) { return _Test2_M_Data; }

private:
  static Test2_M_Data _Test2_M_Data;

private:
  class _C_Test2_A : public ConfigFile
  {
    static const char * _sectionName[3];
    static const char * _name[28];
    DataMap _dataMap[28];

    class _C_Version
    {
      class _C_FormatVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_DataVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Version(void);
      static const char * name(void) { return _sectionName[0]; }

      _C_FormatVersion FormatVersion;
      _C_DataVersion DataVersion;
    };

    class _C_Section1
    {
      class _C_Param1
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param2
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Section1(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_Param1 Param1[2];
      _C_Param2 Param2[2];
    };

    class _C_Section2
    {
      class _C_Param3
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param4
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param5
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool getRange(long & min, long & max) { min=-100; max=100; return true; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param6
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool getRange(long & min, long & max) { min=0x00; max=0x7f; return true; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param7
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool getRange(long & min, long & max);
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param8
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        static bool getRange(long & min, long & max) { return false; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param9
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool getRange(double & min, double & max) { min=0; max=10.0; return true; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param10
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool getRange(double & min, double & max);
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param11
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        static bool getRange(double & min, double & max) { return false; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param12
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        static bool getRange(double & min, double & max) { min=-1000; max=0; return true; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param13
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        static bool getRange(double & min, double & max) { min=0.0; max=1000.0; return true; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Section2(void);
      static const char * name(void) { return _sectionName[2]; }

      _C_Param3 Param3[2];
      _C_Param4 Param4[2];
      _C_Param5 Param5[2];
      _C_Param6 Param6[2];
      _C_Param7 Param7[2];
      _C_Param8 Param8[2];
      _C_Param9 Param9[2];
      _C_Param10 Param10[2];
      _C_Param11 Param11[2];
      _C_Param12 Param12[2];
      _C_Param13 Param13[2];
    };

  public:
    _C_Test2_A(Test2_A_Data & data);
    const char * name(void) { return "Test2_A"; }
    const char * formatVersion(void) { return "2a"; }
    const char * fileName(void) { return TAOS_PATH "/subdir1a/subdir2/subdir3/file2_a.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 28, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 28, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 28, value);
    }

    ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
    {
      _logLevel = logLevel;
		 _errorLevel = errorLevel;
      return ConfigFile::readData(_dataMap, 28, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
    }

    _C_Version Version;
    _C_Section1 Section1;
    _C_Section2 Section2;
  };

private:
  class _C_Test2_B : public ConfigFile
  {
    static const char * _sectionName[4];
    static const char * _name[20];
    DataMap _dataMap[20];

    class _C_Version
    {
      class _C_FormatVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_DataVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Version(void);
      static const char * name(void) { return _sectionName[0]; }

      _C_FormatVersion FormatVersion;
      _C_DataVersion DataVersion;
    };

    class _C_Section1
    {
      class _C_Param101
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Section1(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_Param101 Param101[1];
    };

    class _C_Section2
    {
      class _C_Param
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool getRange(long & min, long & max) { return false; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Section2(void);
      static const char * name(void) { return _sectionName[2]; }

      _C_Param Param[16];
    };

    class _C_Section3
    {
      class _C_Param
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool getRange(long & min, long & max) { return false; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Section3(void);
      static const char * name(void) { return _sectionName[3]; }

      _C_Param Param;
    };

  public:
    _C_Test2_B(Test2_B_Data & data);
    const char * name(void) { return "Test2_B"; }
    const char * formatVersion(void) { return "2b"; }
    const char * fileName(void) { return CONFIG_PATH "/file2_b.dat"; }
    const char * crcFileName(void) { return CONFIG_PATH "/crc/file2_b.crc"; }
    const char * backupFileName(void) { return CONFIG_PATH "/backup/file2_b.dat"; }
    const char * backupCRCFileName(void) { return CONFIG_PATH "/backup/crc/file2_b.crc"; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 20, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 20, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 20, value);
    }

    ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
    {
      _logLevel = logLevel;
		 _errorLevel = errorLevel;
      return ConfigFile::readData(_dataMap, 20, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
    }

    WriteStatus writeFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
    {
      _logLevel = logLevel;
		 _errorLevel = errorLevel;
      return ConfigFile::writeData(_dataMap, 20, fileName(), crcFileName(), backupFileName(), backupCRCFileName());
    }

    _C_Version Version;
    _C_Section1 Section1;
    _C_Section2 Section2;
    _C_Section3 Section3;
  };

private:
  class _C_Test2_C : public ConfigFile
  {
    static const char * _sectionName[3];
    static const char * _name[9];
    DataMap _dataMap[9];

    class _C_Version
    {
      class _C_FormatVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }
        bool set(const char * value);
      private:
        const DataMap * _dataMap;
        int _mapIdx;
      };

      class _C_DataVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }
        bool set(const char * value);
      private:
        const DataMap * _dataMap;
        int _mapIdx;
      };

    public:
      _C_Version(const DataMap * dataMap);
      static const char * name(void) { return _sectionName[0]; }

      _C_FormatVersion FormatVersion;
      _C_DataVersion DataVersion;
    };

    class _C_Section1
    {
      class _C_Param201
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }
        bool set(TParam201 value);
      private:
        const DataMap * _dataMap;
        int _mapIdx;
      };

    public:
      _C_Section1(const DataMap * dataMap);
      static const char * name(void) { return _sectionName[1]; }

      _C_Param201 Param201[1];
    };

    class _C_Section2
    {
      class _C_Param1
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool getRange(long & min, long & max) { min=1; max=2; return true; }
        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }
        bool set(long value);
      private:
        const DataMap * _dataMap;
        int _mapIdx;
      };

      class _C_Param2
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool getRange(long & min, long & max);
        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }
        bool set(long value);
      private:
        const DataMap * _dataMap;
        int _mapIdx;
      };

      class _C_Param3
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool getRange(long & min, long & max);
        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }
        bool set(long value);
      private:
        const DataMap * _dataMap;
        int _mapIdx;
      };

      class _C_Param4
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        static bool getRange(long & min, long & max) { min=-1; max=1; return true; }
        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }
        bool set(long value);
      private:
        const DataMap * _dataMap;
        int _mapIdx;
      };

      class _C_Param5
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        static bool getRange(long & min, long & max);
        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }
        bool set(long value);
      private:
        const DataMap * _dataMap;
        int _mapIdx;
      };

      class _C_Param6
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        static bool getRange(long & min, long & max);
        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }
        bool set(long value);
      private:
        const DataMap * _dataMap;
        int _mapIdx;
      };

    public:
      _C_Section2(const DataMap * dataMap);
      static const char * name(void) { return _sectionName[2]; }

      _C_Param1 Param1[1];
      _C_Param2 Param2[1];
      _C_Param3 Param3[1];
      _C_Param4 Param4[1];
      _C_Param5 Param5[1];
      _C_Param6 Param6[1];
    };

  public:
    _C_Test2_C(Test2_C_Data & data);
    const char * name(void) { return "Test2_C"; }
    const char * formatVersion(void) { return "2c"; }
    const char * fileName(void) { return CONFIG_PATH "/file2_c.dat"; }
    const char * crcFileName(void) { return CONFIG_PATH "/crc/file2_c.crc"; }
    const char * backupFileName(void) { return CONFIG_PATH "/backup/file2_c.dat"; }
    const char * backupCRCFileName(void) { return CONFIG_PATH "/backup/crc/file2_c.crc"; }

    const char * defaultFileName(void) { return TAOS_PATH "/subdir1b/subdir2/subdir3/file2_c.dat"; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 9, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 9, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 9, value);
    }

    ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
    {
      _logLevel = logLevel;
		 _errorLevel = errorLevel;
      return ConfigFile::readData(_dataMap, 9, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
    }

    WriteStatus writeFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
    {
      _logLevel = logLevel;
		 _errorLevel = errorLevel;
      return ConfigFile::writeData(_dataMap, 9, fileName(), crcFileName(), backupFileName(), backupCRCFileName());
    }

    _C_Version Version;
    _C_Section1 Section1;
    _C_Section2 Section2;
  };

private:
  class _C_Test2_D : public ConfigFile
  {
    static const char * _sectionName[2];
    static const char * _name[3];
    DataMap _dataMap[3];

    class _C_Version
    {
      class _C_FormatVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_DataVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Version(void);
      static const char * name(void) { return _sectionName[0]; }

      _C_FormatVersion FormatVersion;
      _C_DataVersion DataVersion;
    };

    class _C_Section1
    {
      class _C_Param1
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool getRange(long & min, long & max) { min=-1; max=0; return true; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Section1(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_Param1 Param1[1];
    };

  public:
    _C_Test2_D(Test2_D_Data & data);
    const char * name(void) { return "Test2_D"; }
    const char * formatVersion(void) { return "1"; }
    const char * fileName(void) { return TAOS_PATH "/file2_d.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 3, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 3, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 3, value);
    }

    ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
    {
      _logLevel = logLevel;
		 _errorLevel = errorLevel;
      return ConfigFile::readData(_dataMap, 3, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
    }

    _C_Version Version;
    _C_Section1 Section1;
  };

private:
  class _C_Test2_E : public ConfigFile
  {
    static const char * _sectionName[2];
    static const char * _name[3];
    DataMap _dataMap[3];

    class _C_Version
    {
      class _C_FormatVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_DataVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Version(void);
      static const char * name(void) { return _sectionName[0]; }

      _C_FormatVersion FormatVersion;
      _C_DataVersion DataVersion;
    };

    class _C_Section1
    {
      class _C_Param1
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool getRange(long & min, long & max);
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Section1(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_Param1 Param1[1];
    };

  public:
    _C_Test2_E(Test2_E_Data & data);
    const char * name(void) { return "Test2_E"; }
    const char * formatVersion(void) { return "1"; }
    const char * fileName(void) { return TAOS_PATH "/file2_e.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 3, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 3, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 3, value);
    }

    ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
    {
      _logLevel = logLevel;
		 _errorLevel = errorLevel;
      return ConfigFile::readData(_dataMap, 3, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
    }

    _C_Version Version;
    _C_Section1 Section1;
  };

private:
  class _C_Test2_F : public ConfigFile
  {
    static const char * _sectionName[2];
    static const char * _name[3];
    DataMap _dataMap[3];

    class _C_Version
    {
      class _C_FormatVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_DataVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Version(void);
      static const char * name(void) { return _sectionName[0]; }

      _C_FormatVersion FormatVersion;
      _C_DataVersion DataVersion;
    };

    class _C_Section1
    {
      class _C_Param1
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        static bool getRange(long & min, long & max) { return false; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Section1(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_Param1 Param1[1];
    };

  public:
    _C_Test2_F(Test2_F_Data & data);
    const char * name(void) { return "Test2_F"; }
    const char * formatVersion(void) { return "1"; }
    const char * fileName(void) { return TAOS_PATH "/file2_f.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 3, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 3, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 3, value);
    }

    ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
    {
      _logLevel = logLevel;
		 _errorLevel = errorLevel;
      return ConfigFile::readData(_dataMap, 3, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
    }

    _C_Version Version;
    _C_Section1 Section1;
  };

private:
  class _C_Test2_G : public ConfigFile
  {
    static const char * _sectionName[2];
    static const char * _name[3];
    DataMap _dataMap[3];

    class _C_Version
    {
      class _C_FormatVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_DataVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Version(void);
      static const char * name(void) { return _sectionName[0]; }

      _C_FormatVersion FormatVersion;
      _C_DataVersion DataVersion;
    };

    class _C_Section1
    {
      class _C_Param1
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool getRange(double & min, double & max) { min=-0.1; max=0; return true; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Section1(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_Param1 Param1[1];
    };

  public:
    _C_Test2_G(Test2_G_Data & data);
    const char * name(void) { return "Test2_G"; }
    const char * formatVersion(void) { return "1"; }
    const char * fileName(void) { return TAOS_PATH "/file2_g.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 3, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 3, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 3, value);
    }

    ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
    {
      _logLevel = logLevel;
		 _errorLevel = errorLevel;
      return ConfigFile::readData(_dataMap, 3, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
    }

    _C_Version Version;
    _C_Section1 Section1;
  };

private:
  class _C_Test2_H : public ConfigFile
  {
    static const char * _sectionName[2];
    static const char * _name[3];
    DataMap _dataMap[3];

    class _C_Version
    {
      class _C_FormatVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_DataVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Version(void);
      static const char * name(void) { return _sectionName[0]; }

      _C_FormatVersion FormatVersion;
      _C_DataVersion DataVersion;
    };

    class _C_Section1
    {
      class _C_Param1
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool getRange(double & min, double & max);
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Section1(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_Param1 Param1[1];
    };

  public:
    _C_Test2_H(Test2_H_Data & data);
    const char * name(void) { return "Test2_H"; }
    const char * formatVersion(void) { return "1"; }
    const char * fileName(void) { return TAOS_PATH "/file2_h.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 3, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 3, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 3, value);
    }

    ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
    {
      _logLevel = logLevel;
		 _errorLevel = errorLevel;
      return ConfigFile::readData(_dataMap, 3, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
    }

    _C_Version Version;
    _C_Section1 Section1;
  };

private:
  class _C_Test2_I : public ConfigFile
  {
    static const char * _sectionName[2];
    static const char * _name[3];
    DataMap _dataMap[3];

    class _C_Version
    {
      class _C_FormatVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_DataVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Version(void);
      static const char * name(void) { return _sectionName[0]; }

      _C_FormatVersion FormatVersion;
      _C_DataVersion DataVersion;
    };

    class _C_Section1
    {
      class _C_Param1
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        static bool getRange(double & min, double & max) { return false; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Section1(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_Param1 Param1[1];
    };

  public:
    _C_Test2_I(Test2_I_Data & data);
    const char * name(void) { return "Test2_I"; }
    const char * formatVersion(void) { return "1"; }
    const char * fileName(void) { return TAOS_PATH "/file2_i.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 3, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 3, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 3, value);
    }

    ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
    {
      _logLevel = logLevel;
		 _errorLevel = errorLevel;
      return ConfigFile::readData(_dataMap, 3, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
    }

    _C_Version Version;
    _C_Section1 Section1;
  };

private:
  class _C_Test2_J : public ConfigFile
  {
    static const char * _sectionName[2];
    static const char * _name[4];
    DataMap _dataMap[4];

    class _C_Version
    {
      class _C_FormatVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_DataVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Version(void);
      static const char * name(void) { return _sectionName[0]; }

      _C_FormatVersion FormatVersion;
      _C_DataVersion DataVersion;
    };

    class _C_Section1
    {
      class _C_Param1
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param2
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Section1(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_Param1 Param1[1];
      _C_Param2 Param2[1];
    };

  public:
    _C_Test2_J(Test2_J_Data & data);
    const char * name(void) { return "Test2_J"; }
    const char * formatVersion(void) { return "1"; }
    const char * fileName(void) { return TAOS_PATH "/file2_j.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 4, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 4, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 4, value);
    }

    ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
    {
      _logLevel = logLevel;
		 _errorLevel = errorLevel;
      return ConfigFile::readData(_dataMap, 4, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
    }

    _C_Version Version;
    _C_Section1 Section1;
  };

private:
  class _C_Test2_K : public ConfigFile
  {
    static const char * _sectionName[2];
    static const char * _name[4];
    DataMap _dataMap[4];

    class _C_Version
    {
      class _C_FormatVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_DataVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Version(void);
      static const char * name(void) { return _sectionName[0]; }

      _C_FormatVersion FormatVersion;
      _C_DataVersion DataVersion;
    };

    class _C_Section1
    {
      class _C_Param1
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param2
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Section1(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_Param1 Param1[1];
      _C_Param2 Param2[1];
    };

  public:
    _C_Test2_K(Test2_K_Data & data);
    const char * name(void) { return "Test2_K"; }
    const char * formatVersion(void) { return "1"; }
    const char * fileName(void) { return TAOS_PATH "/file2_k.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 4, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 4, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 4, value);
    }

    ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
    {
      _logLevel = logLevel;
		 _errorLevel = errorLevel;
      return ConfigFile::readData(_dataMap, 4, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
    }

    _C_Version Version;
    _C_Section1 Section1;
  };

private:
  class _C_Test2_L : public ConfigFile
  {
    static const char * _sectionName[3];
    static const char * _name[4];
    DataMap _dataMap[4];

    class _C_Version
    {
      class _C_FormatVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_DataVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Version(void);
      static const char * name(void) { return _sectionName[0]; }

      _C_FormatVersion FormatVersion;
      _C_DataVersion DataVersion;
    };

    class _C_Section1
    {
      class _C_ParamK
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Section1(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_ParamK ParamK[1];
    };

    class _C_Section2
    {
      class _C_ParamK
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_Section2(void);
      static const char * name(void) { return _sectionName[2]; }

      _C_ParamK ParamK[1];
    };

  public:
    _C_Test2_L(Test2_L_Data & data);
    const char * name(void) { return "Test2_L"; }
    const char * formatVersion(void) { return "1"; }
    const char * fileName(void) { return TAOS_PATH "/file2_l.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 4, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 4, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 4, value);
    }

    ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
    {
      _logLevel = logLevel;
		 _errorLevel = errorLevel;
      return ConfigFile::readData(_dataMap, 4, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
    }

    _C_Version Version;
    _C_Section1 Section1;
    _C_Section2 Section2;
  };

private:
  class _C_Test2_M : public ConfigFile
  {
    static const char * _sectionName[2];
    static const char * _name[4];
    DataMap _dataMap[4];

    class _C_Version
    {
      class _C_FormatVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }
        bool set(const char * value);
      private:
        const DataMap * _dataMap;
        int _mapIdx;
      };

      class _C_DataVersion
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }
        bool set(const char * value);
      private:
        const DataMap * _dataMap;
        int _mapIdx;
      };

    public:
      _C_Version(const DataMap * dataMap);
      static const char * name(void) { return _sectionName[0]; }

      _C_FormatVersion FormatVersion;
      _C_DataVersion DataVersion;
    };

    class _C_Section1
    {
      class _C_Param1
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool getRange(double & min, double & max) { return false; }
        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }
        bool set(double value);
      private:
        const DataMap * _dataMap;
        int _mapIdx;
      };

      class _C_Param2
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool getRange(double & min, double & max) { return false; }
        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }
      private:
        const DataMap * _dataMap;
        int _mapIdx;
      };

    public:
      _C_Section1(const DataMap * dataMap);
      static const char * name(void) { return _sectionName[1]; }

      _C_Param1 Param1[1];
      _C_Param2 Param2[1];
    };

  public:
    _C_Test2_M(Test2_M_Data & data);
    const char * name(void) { return "Test2_M"; }
    const char * formatVersion(void) { return "2m"; }
    const char * fileName(void) { return CONFIG_PATH "/file2_m.dat"; }
    const char * crcFileName(void) { return CONFIG_PATH "/crc/file2_m.crc"; }
    const char * backupFileName(void) { return CONFIG_PATH "/backup/file2_m.dat"; }
    const char * backupCRCFileName(void) { return CONFIG_PATH "/backup/crc/file2_m.crc"; }

    const char * defaultFileName(void) { return TAOS_PATH "/file2_m.dat"; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 4, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 4, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 4, value);
    }

    ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
    {
      _logLevel = logLevel;
		 _errorLevel = errorLevel;
      return ConfigFile::readData(_dataMap, 4, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
    }

    WriteStatus writeFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
    {
      _logLevel = logLevel;
		 _errorLevel = errorLevel;
      return ConfigFile::writeData(_dataMap, 4, fileName(), crcFileName(), backupFileName(), backupCRCFileName());
    }

    _C_Version Version;
    _C_Section1 Section1;
  };

public:
  static _C_Test2_A Test2_A_Access;
  static _C_Test2_B Test2_B_Access;
  static _C_Test2_C Test2_C_Access;
  static _C_Test2_D Test2_D_Access;
  static _C_Test2_E Test2_E_Access;
  static _C_Test2_F Test2_F_Access;
  static _C_Test2_G Test2_G_Access;
  static _C_Test2_H Test2_H_Access;
  static _C_Test2_I Test2_I_Access;
  static _C_Test2_J Test2_J_Access;
  static _C_Test2_K Test2_K_Access;
  static _C_Test2_L Test2_L_Access;
  static _C_Test2_M Test2_M_Access;
  static ConfigFile * getConfigFileObj(int index);
};

#endif /* ifndef _test2_INCLUDE */
