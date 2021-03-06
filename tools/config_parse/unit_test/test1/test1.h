/*
 * Auto-generated file, do not edit
 *
 * Source Files:
 * ./test1_a.cfg formatVersion=1a dataVersion=d1a
 * ./test1_b.cfg formatVersion=1b dataVersion=d1b
 * ./test1_c.cfg formatVersion=1c dataVersion=d1c
 * ./test1_d.cfg formatVersion=1 dataVersion=1
 * ./test1_e.cfg formatVersion=1 dataVersion=1
 * ./test1_f.cfg formatVersion=1 dataVersion=1
 * ./test1_g.cfg formatVersion=1 dataVersion=1
 * ./test1_h.cfg formatVersion=1 dataVersion=1
 * ./test1_i.cfg formatVersion=1 dataVersion=1
 * ./test1_j.cfg formatVersion=1 dataVersion=1
 * ./test1_k.cfg formatVersion=1 dataVersion=1
 * ./test1_l.cfg formatVersion=1 dataVersion=1
 * ./test1_m.cfg formatVersion=1m dataVersion=d1m
 * ./test1_n.cfg formatVersion= dataVersion=
 * ./test1_o.cfg formatVersion=1 dataVersion=1
 * ./test1_p.cfg formatVersion=1p dataVersion=d1p
 * ./test1_q.cfg formatVersion=1.1c dataVersion=1.1c
 * ./test1_s.cfg formatVersion=1 dataVersion=1
 *
 */

#ifndef _test1_INCLUDE
#define _test1_INCLUDE

#include <vxWorks.h>
#include <limits.h>

#include "datalog.h"
#include "..\..\..\..\..\base\config/file_names.h"
#include "..\..\..\..\config_file_support/config_helper.h"

#include "test1_enum.h"

#include <vxWorks.h>
#include "stdio.h"
#include "stdlib.h"
class Test1 : public ConfigData, public Test1Enum
{
public:
  enum TParam1 { enum1_id1 = 0xd83faecf, enum1_id2 = 0xc136ff75, enum1_id3 = 0xb631cfe3 };
  static bool validateParam1(void *value)
  {
    return ( *(unsigned long *)value == 0xd83faecf ||
              *(unsigned long *)value == 0xc136ff75 ||
              *(unsigned long *)value == 0xb631cfe3 );
  }

  enum TParam2 { enum2_id1 = 0x9f9fd41f };
  static bool validateParam2(void *value)
  {
    return ( *(unsigned long *)value == 0x9f9fd41f );
  }

  enum TParam101 { enum3_id1 = 0xa2fffdaf, enum3_id2 = 0xbbf6ac15, enum3_id3 = 0xccf19c83 };
  static bool validateParam101(void *value)
  {
    return ( *(unsigned long *)value == 0xa2fffdaf ||
              *(unsigned long *)value == 0xbbf6ac15 ||
              *(unsigned long *)value == 0xccf19c83 );
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

  static bool validateParamO(void *value)
  {
    return ( *(unsigned long *)value == 0xd356ab26 ||
              *(unsigned long *)value == 0xca5ffa9c ||
              *(unsigned long *)value == 0xbd58ca0a );
  }

  enum TParamP { enumP_id1 = 0xfc6df032, enumP_id2 = 0xe564a188, enumP_id3 = 0x9263911e };
  static bool validateParamP(void *value)
  {
    return ( *(unsigned long *)value == 0xfc6df032 ||
              *(unsigned long *)value == 0xe564a188 ||
              *(unsigned long *)value == 0x9263911e );
  }


private:
  static const ConfigData::EnumMap _enumMap[20];

public:
  struct Test1_A_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      bool ReadOnly;
      const char * FileName;
    } FileInfo;
    struct
    {
      TParam1 Param1;
      TParam2 Param2;
    } Section1;
    struct
    {
      bool Param3;
      bool Param4;
      const char * Param5;
      const char * Param6;
      long Param7;
      long Param8;
      long Param9;
      long Param10;
      long Param11;
      double Param12;
      double Param13;
      double Param14;
      double Param15;
      double Param16;
    } Section2;
  Test1_A_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test1_A_Data & Test1_A(void) { return _Test1_A_Data; }

private:
  static Test1_A_Data _Test1_A_Data;

public:
  struct Test1_B_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      bool ReadOnly;
      const char * FileName;
    } FileInfo;
    struct
    {
      TParam101 Param101;
    } Section1;
  Test1_B_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test1_B_Data & Test1_B(void) { return _Test1_B_Data; }

private:
  static Test1_B_Data _Test1_B_Data;

public:
  struct Test1_C_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      bool ReadOnly;
      const char * FileName;
    } FileInfo;
    struct
    {
      TParam201 Param201;
    } Section1;
    struct
    {
      long Param1;
      long Param2;
      long Param3;
      long Param4;
      long Param5;
      long Param6;
    } Section2;
  Test1_C_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test1_C_Data & Test1_C(void) { return _Test1_C_Data; }

private:
  static Test1_C_Data _Test1_C_Data;

public:
  struct Test1_D_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      bool ReadOnly;
      const char * FileName;
    } FileInfo;
    struct
    {
      long Param1;
    } Section1;
  Test1_D_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test1_D_Data & Test1_D(void) { return _Test1_D_Data; }

private:
  static Test1_D_Data _Test1_D_Data;

public:
  struct Test1_E_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      bool ReadOnly;
      const char * FileName;
    } FileInfo;
    struct
    {
      long Param1;
    } Section1;
  Test1_E_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test1_E_Data & Test1_E(void) { return _Test1_E_Data; }

private:
  static Test1_E_Data _Test1_E_Data;

public:
  struct Test1_F_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      bool ReadOnly;
      const char * FileName;
    } FileInfo;
    struct
    {
      long Param1;
    } Section1;
  Test1_F_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test1_F_Data & Test1_F(void) { return _Test1_F_Data; }

private:
  static Test1_F_Data _Test1_F_Data;

public:
  struct Test1_G_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      bool ReadOnly;
      const char * FileName;
    } FileInfo;
    struct
    {
      double Param1;
    } Section1;
  Test1_G_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test1_G_Data & Test1_G(void) { return _Test1_G_Data; }

private:
  static Test1_G_Data _Test1_G_Data;

public:
  struct Test1_H_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      bool ReadOnly;
      const char * FileName;
    } FileInfo;
    struct
    {
      double Param1;
    } Section1;
  Test1_H_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test1_H_Data & Test1_H(void) { return _Test1_H_Data; }

private:
  static Test1_H_Data _Test1_H_Data;

public:
  struct Test1_I_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      bool ReadOnly;
      const char * FileName;
    } FileInfo;
    struct
    {
      double Param1;
    } Section1;
  Test1_I_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test1_I_Data & Test1_I(void) { return _Test1_I_Data; }

private:
  static Test1_I_Data _Test1_I_Data;

public:
  struct Test1_J_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      bool ReadOnly;
      const char * FileName;
    } FileInfo;
    struct
    {
      const char * Param1;
      const char * Param2;
    } Section1;
  Test1_J_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test1_J_Data & Test1_J(void) { return _Test1_J_Data; }

private:
  static Test1_J_Data _Test1_J_Data;

public:
  struct Test1_K_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      bool ReadOnly;
      const char * FileName;
    } FileInfo;
    struct
    {
      bool Param1;
      bool Param2;
    } Section1;
  Test1_K_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test1_K_Data & Test1_K(void) { return _Test1_K_Data; }

private:
  static Test1_K_Data _Test1_K_Data;

public:
  struct Test1_L_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      bool ReadOnly;
      const char * FileName;
    } FileInfo;
    struct
    {
      TParamK ParamK;
    } Section1;
    struct
    {
      TParamK ParamK;
    } Section2;
  Test1_L_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test1_L_Data & Test1_L(void) { return _Test1_L_Data; }

private:
  static Test1_L_Data _Test1_L_Data;

public:
  struct Test1_M_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      bool ReadOnly;
      const char * FileName;
    } FileInfo;
    struct
    {
      double Param1;
      double Param2;
    } Section1;
  Test1_M_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test1_M_Data & Test1_M(void) { return _Test1_M_Data; }

private:
  static Test1_M_Data _Test1_M_Data;

public:
  struct Test1_O_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      bool ReadOnly;
      const char * FileName;
    } FileInfo;
    struct
    {
      TParamO ParamO;
    } Section1;
  Test1_O_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test1_O_Data & Test1_O(void) { return _Test1_O_Data; }

private:
  static Test1_O_Data _Test1_O_Data;

public:
  struct Test1_P_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      bool ReadOnly;
      const char * FileName;
    } FileInfo;
    struct
    {
      TParamP P1;
      TParamP P2;
      TParamP P3;
    } Section1;
    struct
    {
      TParamP ParamP;
      TParamP P1;
      TParamP P2;
      TParamP P3;
    } Section2;
  Test1_P_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test1_P_Data & Test1_P(void) { return _Test1_P_Data; }

private:
  static Test1_P_Data _Test1_P_Data;

public:
  struct Test1_Q_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      bool ReadOnly;
      const char * FileName;
    } FileInfo;
    struct
    {
      long Param1;
    } Section2;
    struct
    {
      long Param2;
    } Section3;
    struct
    {
      long Param1;
    } Section1;
  Test1_Q_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test1_Q_Data & Test1_Q(void) { return _Test1_Q_Data; }

private:
  static Test1_Q_Data _Test1_Q_Data;

public:
  struct Test1_S_Data
  {
    struct
    {
      const char * FormatVersion;
      const char * DataVersion;
    } Version;
    struct
    {
      bool ReadOnly;
      const char * FileName;
    } FileInfo;
    struct
    {
      long Param1;
    } Section1;
  Test1_S_Data(void) { memset(this, 0, sizeof(*this)); }
  };

  static const Test1_S_Data & Test1_S(void) { return _Test1_S_Data; }

private:
  static Test1_S_Data _Test1_S_Data;

private:
  class _C_Test1_A : public ConfigFile
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

    class _C_FileInfo
    {
      class _C_ReadOnly
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_FileName
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_FileInfo(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_ReadOnly ReadOnly;
      _C_FileName FileName;
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
      static const char * name(void) { return _sectionName[2]; }

      _C_Param1 Param1;
      _C_Param2 Param2;
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
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param6
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param7
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool getRange(long & min, long & max) { return false; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param8
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool getRange(long & min, long & max) { min=-100; max=100; return true; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param9
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool getRange(long & min, long & max) { min=0x00; max=0x7f; return true; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param10
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool getRange(long & min, long & max);
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param11
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        static bool getRange(long & min, long & max) { return false; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param12
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool getRange(double & min, double & max) { min=0; max=10.0; return true; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param13
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool getRange(double & min, double & max);
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param14
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        static bool getRange(double & min, double & max) { return false; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param15
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        static bool getRange(double & min, double & max) { min=-1000; max=0; return true; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_Param16
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
      static const char * name(void) { return _sectionName[3]; }

      _C_Param3 Param3;
      _C_Param4 Param4;
      _C_Param5 Param5;
      _C_Param6 Param6;
      _C_Param7 Param7;
      _C_Param8 Param8;
      _C_Param9 Param9;
      _C_Param10 Param10;
      _C_Param11 Param11;
      _C_Param12 Param12;
      _C_Param13 Param13;
      _C_Param14 Param14;
      _C_Param15 Param15;
      _C_Param16 Param16;
    };

                void logData( DataLog_Level * level, ConfigFile::ReadStatus s );
                void logData( DataLog_Level * level, ConfigFile::WriteStatus s);

  public:
    _C_Test1_A(Test1_A_Data & data);
    const char * name(void) { return "Test1_A"; }
    const char * formatVersion(void) { return "1a"; }
    const char * fileName(void) { return TAOS_PATH "/subdir1a/subdir2/subdir3/file1_a.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

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
                ConfigFile::ReadStatus status;
                _logLevel = logLevel;
                _errorLevel = errorLevel;
                status = ConfigFile::readData(_dataMap, 20, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
                logData(&log_level_config_data_info, status);
                return status;
        }

    _C_Version Version;
    _C_FileInfo FileInfo;
    _C_Section1 Section1;
    _C_Section2 Section2;
  };

private:
  class _C_Test1_B : public ConfigFile
  {
    static const char * _sectionName[3];
    static const char * _name[5];
    DataMap _dataMap[5];

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

    class _C_FileInfo
    {
      class _C_ReadOnly
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_FileName
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_FileInfo(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_ReadOnly ReadOnly;
      _C_FileName FileName;
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
      static const char * name(void) { return _sectionName[2]; }

      _C_Param101 Param101;
    };

                void logData( DataLog_Level * level, ConfigFile::ReadStatus s );
                void logData( DataLog_Level * level, ConfigFile::WriteStatus s);

  public:
    _C_Test1_B(Test1_B_Data & data);
    const char * name(void) { return "Test1_B"; }
    const char * formatVersion(void) { return "1b"; }
    const char * fileName(void) { return CONFIG_PATH "/file1_b.dat"; }
    const char * crcFileName(void) { return CONFIG_PATH "/crc/file1_b.crc"; }
    const char * backupFileName(void) { return CONFIG_PATH "/backup/file1_b.dat"; }
    const char * backupCRCFileName(void) { return CONFIG_PATH "/backup/crc/file1_b.crc"; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 5, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 5, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 5, value);
    }

        ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
        {
                ConfigFile::ReadStatus status;
                _logLevel = logLevel;
                _errorLevel = errorLevel;
                status = ConfigFile::readData(_dataMap, 5, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
                logData(&log_level_config_data_info, status);
                return status;
        }

     WriteStatus writeFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
     {
             ConfigFile::WriteStatus status;
             _logLevel = logLevel;
             _errorLevel = errorLevel;
             status = ConfigFile::writeData(_dataMap, 5, fileName(), crcFileName(), backupFileName(), backupCRCFileName());
             logData(&log_level_config_data_info, status);
             return status;
     }

    _C_Version Version;
    _C_FileInfo FileInfo;
    _C_Section1 Section1;
  };

private:
  class _C_Test1_C : public ConfigFile
  {
    static const char * _sectionName[4];
    static const char * _name[11];
    DataMap _dataMap[11];

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

    class _C_FileInfo
    {
      class _C_ReadOnly
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }
        bool set(bool value);
      private:
        const DataMap * _dataMap;
        int _mapIdx;
      };

      class _C_FileName
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
      _C_FileInfo(const DataMap * dataMap);
      static const char * name(void) { return _sectionName[1]; }

      _C_ReadOnly ReadOnly;
      _C_FileName FileName;
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
      static const char * name(void) { return _sectionName[2]; }

      _C_Param201 Param201;
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
      static const char * name(void) { return _sectionName[3]; }

      _C_Param1 Param1;
      _C_Param2 Param2;
      _C_Param3 Param3;
      _C_Param4 Param4;
      _C_Param5 Param5;
      _C_Param6 Param6;
    };

                void logData( DataLog_Level * level, ConfigFile::ReadStatus s );
                void logData( DataLog_Level * level, ConfigFile::WriteStatus s);

  public:
    _C_Test1_C(Test1_C_Data & data);
    const char * name(void) { return "Test1_C"; }
    const char * formatVersion(void) { return "1c"; }
    const char * fileName(void) { return CONFIG_PATH "/file1_c.dat"; }
    const char * crcFileName(void) { return CONFIG_PATH "/crc/file1_c.crc"; }
    const char * backupFileName(void) { return CONFIG_PATH "/backup/file1_c.dat"; }
    const char * backupCRCFileName(void) { return CONFIG_PATH "/backup/crc/file1_c.crc"; }

    const char * defaultFileName(void) { return TAOS_PATH "/subdir1b/subdir2/subdir3/file1_c.dat"; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 11, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 11, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 11, value);
    }

        ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
        {
                ConfigFile::ReadStatus status;
                _logLevel = logLevel;
                _errorLevel = errorLevel;
                status = ConfigFile::readData(_dataMap, 11, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
                logData(&log_level_config_data_info, status);
                return status;
        }

     WriteStatus writeFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
     {
             ConfigFile::WriteStatus status;
             _logLevel = logLevel;
             _errorLevel = errorLevel;
             status = ConfigFile::writeData(_dataMap, 11, fileName(), crcFileName(), backupFileName(), backupCRCFileName());
             logData(&log_level_config_data_info, status);
             return status;
     }

    _C_Version Version;
    _C_FileInfo FileInfo;
    _C_Section1 Section1;
    _C_Section2 Section2;
  };

private:
  class _C_Test1_D : public ConfigFile
  {
    static const char * _sectionName[3];
    static const char * _name[5];
    DataMap _dataMap[5];

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

    class _C_FileInfo
    {
      class _C_ReadOnly
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_FileName
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_FileInfo(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_ReadOnly ReadOnly;
      _C_FileName FileName;
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
      static const char * name(void) { return _sectionName[2]; }

      _C_Param1 Param1;
    };

                void logData( DataLog_Level * level, ConfigFile::ReadStatus s );
                void logData( DataLog_Level * level, ConfigFile::WriteStatus s);

  public:
    _C_Test1_D(Test1_D_Data & data);
    const char * name(void) { return "Test1_D"; }
    const char * formatVersion(void) { return "1"; }
    const char * fileName(void) { return TAOS_PATH "/file1_d.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 5, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 5, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 5, value);
    }

        ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
        {
                ConfigFile::ReadStatus status;
                _logLevel = logLevel;
                _errorLevel = errorLevel;
                status = ConfigFile::readData(_dataMap, 5, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
                logData(&log_level_config_data_info, status);
                return status;
        }

    _C_Version Version;
    _C_FileInfo FileInfo;
    _C_Section1 Section1;
  };

private:
  class _C_Test1_E : public ConfigFile
  {
    static const char * _sectionName[3];
    static const char * _name[5];
    DataMap _dataMap[5];

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

    class _C_FileInfo
    {
      class _C_ReadOnly
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_FileName
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_FileInfo(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_ReadOnly ReadOnly;
      _C_FileName FileName;
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
      static const char * name(void) { return _sectionName[2]; }

      _C_Param1 Param1;
    };

                void logData( DataLog_Level * level, ConfigFile::ReadStatus s );
                void logData( DataLog_Level * level, ConfigFile::WriteStatus s);

  public:
    _C_Test1_E(Test1_E_Data & data);
    const char * name(void) { return "Test1_E"; }
    const char * formatVersion(void) { return "1"; }
    const char * fileName(void) { return TAOS_PATH "/file1_e.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 5, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 5, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 5, value);
    }

        ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
        {
                ConfigFile::ReadStatus status;
                _logLevel = logLevel;
                _errorLevel = errorLevel;
                status = ConfigFile::readData(_dataMap, 5, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
                logData(&log_level_config_data_info, status);
                return status;
        }

    _C_Version Version;
    _C_FileInfo FileInfo;
    _C_Section1 Section1;
  };

private:
  class _C_Test1_F : public ConfigFile
  {
    static const char * _sectionName[3];
    static const char * _name[5];
    DataMap _dataMap[5];

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

    class _C_FileInfo
    {
      class _C_ReadOnly
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_FileName
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_FileInfo(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_ReadOnly ReadOnly;
      _C_FileName FileName;
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
      static const char * name(void) { return _sectionName[2]; }

      _C_Param1 Param1;
    };

                void logData( DataLog_Level * level, ConfigFile::ReadStatus s );
                void logData( DataLog_Level * level, ConfigFile::WriteStatus s);

  public:
    _C_Test1_F(Test1_F_Data & data);
    const char * name(void) { return "Test1_F"; }
    const char * formatVersion(void) { return "1"; }
    const char * fileName(void) { return TAOS_PATH "/file1_f.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 5, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 5, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 5, value);
    }

        ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
        {
                ConfigFile::ReadStatus status;
                _logLevel = logLevel;
                _errorLevel = errorLevel;
                status = ConfigFile::readData(_dataMap, 5, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
                logData(&log_level_config_data_info, status);
                return status;
        }

    _C_Version Version;
    _C_FileInfo FileInfo;
    _C_Section1 Section1;
  };

private:
  class _C_Test1_G : public ConfigFile
  {
    static const char * _sectionName[3];
    static const char * _name[5];
    DataMap _dataMap[5];

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

    class _C_FileInfo
    {
      class _C_ReadOnly
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_FileName
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_FileInfo(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_ReadOnly ReadOnly;
      _C_FileName FileName;
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
      static const char * name(void) { return _sectionName[2]; }

      _C_Param1 Param1;
    };

                void logData( DataLog_Level * level, ConfigFile::ReadStatus s );
                void logData( DataLog_Level * level, ConfigFile::WriteStatus s);

  public:
    _C_Test1_G(Test1_G_Data & data);
    const char * name(void) { return "Test1_G"; }
    const char * formatVersion(void) { return "1"; }
    const char * fileName(void) { return TAOS_PATH "/file1_g.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 5, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 5, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 5, value);
    }

        ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
        {
                ConfigFile::ReadStatus status;
                _logLevel = logLevel;
                _errorLevel = errorLevel;
                status = ConfigFile::readData(_dataMap, 5, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
                logData(&log_level_config_data_info, status);
                return status;
        }

    _C_Version Version;
    _C_FileInfo FileInfo;
    _C_Section1 Section1;
  };

private:
  class _C_Test1_H : public ConfigFile
  {
    static const char * _sectionName[3];
    static const char * _name[5];
    DataMap _dataMap[5];

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

    class _C_FileInfo
    {
      class _C_ReadOnly
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_FileName
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_FileInfo(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_ReadOnly ReadOnly;
      _C_FileName FileName;
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
      static const char * name(void) { return _sectionName[2]; }

      _C_Param1 Param1;
    };

                void logData( DataLog_Level * level, ConfigFile::ReadStatus s );
                void logData( DataLog_Level * level, ConfigFile::WriteStatus s);

  public:
    _C_Test1_H(Test1_H_Data & data);
    const char * name(void) { return "Test1_H"; }
    const char * formatVersion(void) { return "1"; }
    const char * fileName(void) { return TAOS_PATH "/file1_h.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 5, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 5, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 5, value);
    }

        ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
        {
                ConfigFile::ReadStatus status;
                _logLevel = logLevel;
                _errorLevel = errorLevel;
                status = ConfigFile::readData(_dataMap, 5, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
                logData(&log_level_config_data_info, status);
                return status;
        }

    _C_Version Version;
    _C_FileInfo FileInfo;
    _C_Section1 Section1;
  };

private:
  class _C_Test1_I : public ConfigFile
  {
    static const char * _sectionName[3];
    static const char * _name[5];
    DataMap _dataMap[5];

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

    class _C_FileInfo
    {
      class _C_ReadOnly
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_FileName
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_FileInfo(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_ReadOnly ReadOnly;
      _C_FileName FileName;
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
      static const char * name(void) { return _sectionName[2]; }

      _C_Param1 Param1;
    };

                void logData( DataLog_Level * level, ConfigFile::ReadStatus s );
                void logData( DataLog_Level * level, ConfigFile::WriteStatus s);

  public:
    _C_Test1_I(Test1_I_Data & data);
    const char * name(void) { return "Test1_I"; }
    const char * formatVersion(void) { return "1"; }
    const char * fileName(void) { return TAOS_PATH "/file1_i.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 5, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 5, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 5, value);
    }

        ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
        {
                ConfigFile::ReadStatus status;
                _logLevel = logLevel;
                _errorLevel = errorLevel;
                status = ConfigFile::readData(_dataMap, 5, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
                logData(&log_level_config_data_info, status);
                return status;
        }

    _C_Version Version;
    _C_FileInfo FileInfo;
    _C_Section1 Section1;
  };

private:
  class _C_Test1_J : public ConfigFile
  {
    static const char * _sectionName[3];
    static const char * _name[6];
    DataMap _dataMap[6];

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

    class _C_FileInfo
    {
      class _C_ReadOnly
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_FileName
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_FileInfo(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_ReadOnly ReadOnly;
      _C_FileName FileName;
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
      static const char * name(void) { return _sectionName[2]; }

      _C_Param1 Param1;
      _C_Param2 Param2;
    };

                void logData( DataLog_Level * level, ConfigFile::ReadStatus s );
                void logData( DataLog_Level * level, ConfigFile::WriteStatus s);

  public:
    _C_Test1_J(Test1_J_Data & data);
    const char * name(void) { return "Test1_J"; }
    const char * formatVersion(void) { return "1"; }
    const char * fileName(void) { return TAOS_PATH "/file1_j.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 6, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 6, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 6, value);
    }

        ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
        {
                ConfigFile::ReadStatus status;
                _logLevel = logLevel;
                _errorLevel = errorLevel;
                status = ConfigFile::readData(_dataMap, 6, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
                logData(&log_level_config_data_info, status);
                return status;
        }

    _C_Version Version;
    _C_FileInfo FileInfo;
    _C_Section1 Section1;
  };

private:
  class _C_Test1_K : public ConfigFile
  {
    static const char * _sectionName[3];
    static const char * _name[6];
    DataMap _dataMap[6];

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

    class _C_FileInfo
    {
      class _C_ReadOnly
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_FileName
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_FileInfo(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_ReadOnly ReadOnly;
      _C_FileName FileName;
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
      static const char * name(void) { return _sectionName[2]; }

      _C_Param1 Param1;
      _C_Param2 Param2;
    };

                void logData( DataLog_Level * level, ConfigFile::ReadStatus s );
                void logData( DataLog_Level * level, ConfigFile::WriteStatus s);

  public:
    _C_Test1_K(Test1_K_Data & data);
    const char * name(void) { return "Test1_K"; }
    const char * formatVersion(void) { return "1"; }
    const char * fileName(void) { return TAOS_PATH "/file1_k.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 6, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 6, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 6, value);
    }

        ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
        {
                ConfigFile::ReadStatus status;
                _logLevel = logLevel;
                _errorLevel = errorLevel;
                status = ConfigFile::readData(_dataMap, 6, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
                logData(&log_level_config_data_info, status);
                return status;
        }

    _C_Version Version;
    _C_FileInfo FileInfo;
    _C_Section1 Section1;
  };

private:
  class _C_Test1_L : public ConfigFile
  {
    static const char * _sectionName[4];
    static const char * _name[6];
    DataMap _dataMap[6];

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

    class _C_FileInfo
    {
      class _C_ReadOnly
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_FileName
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_FileInfo(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_ReadOnly ReadOnly;
      _C_FileName FileName;
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
      static const char * name(void) { return _sectionName[2]; }

      _C_ParamK ParamK;
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
      static const char * name(void) { return _sectionName[3]; }

      _C_ParamK ParamK;
    };

                void logData( DataLog_Level * level, ConfigFile::ReadStatus s );
                void logData( DataLog_Level * level, ConfigFile::WriteStatus s);

  public:
    _C_Test1_L(Test1_L_Data & data);
    const char * name(void) { return "Test1_L"; }
    const char * formatVersion(void) { return "1"; }
    const char * fileName(void) { return TAOS_PATH "/file1_l.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 6, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 6, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 6, value);
    }

        ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
        {
                ConfigFile::ReadStatus status;
                _logLevel = logLevel;
                _errorLevel = errorLevel;
                status = ConfigFile::readData(_dataMap, 6, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
                logData(&log_level_config_data_info, status);
                return status;
        }

    _C_Version Version;
    _C_FileInfo FileInfo;
    _C_Section1 Section1;
    _C_Section2 Section2;
  };

private:
  class _C_Test1_M : public ConfigFile
  {
    static const char * _sectionName[3];
    static const char * _name[6];
    DataMap _dataMap[6];

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

    class _C_FileInfo
    {
      class _C_ReadOnly
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }
        bool set(bool value);
      private:
        const DataMap * _dataMap;
        int _mapIdx;
      };

      class _C_FileName
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
      _C_FileInfo(const DataMap * dataMap);
      static const char * name(void) { return _sectionName[1]; }

      _C_ReadOnly ReadOnly;
      _C_FileName FileName;
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
      static const char * name(void) { return _sectionName[2]; }

      _C_Param1 Param1;
      _C_Param2 Param2;
    };

                void logData( DataLog_Level * level, ConfigFile::ReadStatus s );
                void logData( DataLog_Level * level, ConfigFile::WriteStatus s);

  public:
    _C_Test1_M(Test1_M_Data & data);
    const char * name(void) { return "Test1_M"; }
    const char * formatVersion(void) { return "1m"; }
    const char * fileName(void) { return CONFIG_PATH "/file1_m.dat"; }
    const char * crcFileName(void) { return CONFIG_PATH "/crc/file1_m.crc"; }
    const char * backupFileName(void) { return CONFIG_PATH "/backup/file1_m.dat"; }
    const char * backupCRCFileName(void) { return CONFIG_PATH "/backup/crc/file1_m.crc"; }

    const char * defaultFileName(void) { return TAOS_PATH "/file1_m.dat"; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 6, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 6, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 6, value);
    }

        ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
        {
                ConfigFile::ReadStatus status;
                _logLevel = logLevel;
                _errorLevel = errorLevel;
                status = ConfigFile::readData(_dataMap, 6, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
                logData(&log_level_config_data_info, status);
                return status;
        }

     WriteStatus writeFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
     {
             ConfigFile::WriteStatus status;
             _logLevel = logLevel;
             _errorLevel = errorLevel;
             status = ConfigFile::writeData(_dataMap, 6, fileName(), crcFileName(), backupFileName(), backupCRCFileName());
             logData(&log_level_config_data_info, status);
             return status;
     }

    _C_Version Version;
    _C_FileInfo FileInfo;
    _C_Section1 Section1;
  };

private:
  class _C_Test1_O : public ConfigFile
  {
    static const char * _sectionName[3];
    static const char * _name[5];
    DataMap _dataMap[5];

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

    class _C_FileInfo
    {
      class _C_ReadOnly
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_FileName
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_FileInfo(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_ReadOnly ReadOnly;
      _C_FileName FileName;
    };

    class _C_Section1
    {
      class _C_ParamO
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
      static const char * name(void) { return _sectionName[2]; }

      _C_ParamO ParamO;
    };

                void logData( DataLog_Level * level, ConfigFile::ReadStatus s );
                void logData( DataLog_Level * level, ConfigFile::WriteStatus s);

  public:
    _C_Test1_O(Test1_O_Data & data);
    const char * name(void) { return "Test1_O"; }
    const char * formatVersion(void) { return "1"; }
    const char * fileName(void) { return TAOS_PATH "/file1_o.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 5, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 5, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 5, value);
    }

        ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
        {
                ConfigFile::ReadStatus status;
                _logLevel = logLevel;
                _errorLevel = errorLevel;
                status = ConfigFile::readData(_dataMap, 5, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
                logData(&log_level_config_data_info, status);
                return status;
        }

    _C_Version Version;
    _C_FileInfo FileInfo;
    _C_Section1 Section1;
  };

private:
  class _C_Test1_P : public ConfigFile
  {
    static const char * _sectionName[4];
    static const char * _name[11];
    DataMap _dataMap[11];

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

    class _C_FileInfo
    {
      class _C_ReadOnly
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }
        bool set(bool value);
      private:
        const DataMap * _dataMap;
        int _mapIdx;
      };

      class _C_FileName
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
      _C_FileInfo(const DataMap * dataMap);
      static const char * name(void) { return _sectionName[1]; }

      _C_ReadOnly ReadOnly;
      _C_FileName FileName;
    };

    class _C_Section1
    {
      class _C_P1
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }
        bool set(TParamP value);
      private:
        const DataMap * _dataMap;
        int _mapIdx;
      };

      class _C_P2
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }
      private:
        const DataMap * _dataMap;
        int _mapIdx;
      };

      class _C_P3
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }
        bool set(TParamP value);
      private:
        const DataMap * _dataMap;
        int _mapIdx;
      };

    public:
      _C_Section1(const DataMap * dataMap);
      static const char * name(void) { return _sectionName[2]; }

      _C_P1 P1;
      _C_P2 P2;
      _C_P3 P3;
    };

    class _C_Section2
    {
      class _C_ParamP
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }
        bool set(TParamP value);
      private:
        const DataMap * _dataMap;
        int _mapIdx;
      };

      class _C_P1
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }
      private:
        const DataMap * _dataMap;
        int _mapIdx;
      };

      class _C_P2
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }
        bool set(TParamP value);
      private:
        const DataMap * _dataMap;
        int _mapIdx;
      };

      class _C_P3
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        static bool validate(void * valuePtr);
        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }
        bool set(TParamP value);
      private:
        const DataMap * _dataMap;
        int _mapIdx;
      };

    public:
      _C_Section2(const DataMap * dataMap);
      static const char * name(void) { return _sectionName[3]; }

      _C_ParamP ParamP;
      _C_P1 P1;
      _C_P2 P2;
      _C_P3 P3;
    };

                void logData( DataLog_Level * level, ConfigFile::ReadStatus s );
                void logData( DataLog_Level * level, ConfigFile::WriteStatus s);

  public:
    _C_Test1_P(Test1_P_Data & data);
    const char * name(void) { return "Test1_P"; }
    const char * formatVersion(void) { return "1p"; }
    const char * fileName(void) { return TAOS_PATH "/file1_p.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 11, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 11, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 11, value);
    }

        ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
        {
                ConfigFile::ReadStatus status;
                _logLevel = logLevel;
                _errorLevel = errorLevel;
                status = ConfigFile::readData(_dataMap, 11, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
                logData(&log_level_config_data_info, status);
                return status;
        }

    _C_Version Version;
    _C_FileInfo FileInfo;
    _C_Section1 Section1;
    _C_Section2 Section2;
  };

private:
  class _C_Test1_Q : public ConfigFile
  {
    static const char * _sectionName[5];
    static const char * _name[7];
    DataMap _dataMap[7];

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

    class _C_FileInfo
    {
      class _C_ReadOnly
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_FileName
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_FileInfo(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_ReadOnly ReadOnly;
      _C_FileName FileName;
    };

    class _C_Section2
    {
      class _C_Param1
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

      _C_Param1 Param1;
    };

    class _C_Section3
    {
      class _C_Param2
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

      _C_Param2 Param2;
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
      static const char * name(void) { return _sectionName[4]; }

      _C_Param1 Param1;
    };

                void logData( DataLog_Level * level, ConfigFile::ReadStatus s );
                void logData( DataLog_Level * level, ConfigFile::WriteStatus s);

  public:
    _C_Test1_Q(Test1_Q_Data & data);
    const char * name(void) { return "Test1_Q"; }
    const char * formatVersion(void) { return "1.1c"; }
    const char * fileName(void) { return TAOS_PATH "/file1_q.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 7, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 7, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 7, value);
    }

        ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
        {
                ConfigFile::ReadStatus status;
                _logLevel = logLevel;
                _errorLevel = errorLevel;
                status = ConfigFile::readData(_dataMap, 7, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
                logData(&log_level_config_data_info, status);
                return status;
        }

    _C_Version Version;
    _C_FileInfo FileInfo;
    _C_Section2 Section2;
    _C_Section3 Section3;
    _C_Section1 Section1;
  };

private:
  class _C_Test1_S : public ConfigFile
  {
    static const char * _sectionName[3];
    static const char * _name[5];
    DataMap _dataMap[5];

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

    class _C_FileInfo
    {
      class _C_ReadOnly
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

      class _C_FileName
      {
      public:
        const char * name(void) { return _name[_mapIdx]; }
        void initialize(int mapIdx) { _mapIdx = mapIdx; }
      private:
        int _mapIdx;
      };

    public:
      _C_FileInfo(void);
      static const char * name(void) { return _sectionName[1]; }

      _C_ReadOnly ReadOnly;
      _C_FileName FileName;
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
      static const char * name(void) { return _sectionName[2]; }

      _C_Param1 Param1;
    };

                void logData( DataLog_Level * level, ConfigFile::ReadStatus s );
                void logData( DataLog_Level * level, ConfigFile::WriteStatus s);

  public:
    _C_Test1_S(Test1_S_Data & data);
    const char * name(void) { return "Test1_S"; }
    const char * formatVersion(void) { return "1"; }
    const char * fileName(void) { return TAOS_PATH "/file1_s.dat"; }
    const char * crcFileName(void) { return NULL; }
    const char * backupFileName(void) { return NULL; }
    const char * backupCRCFileName(void) { return NULL; }

    const char * defaultFileName(void) { return NULL; }

    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)
    {
      return ConfigFile::getParamValueByName(_dataMap, 5, sectionName, paramName, value);
    }

    inline const char * getParamNameByRef(void * value)
    {
      return ConfigFile::getParamNameByRef(_dataMap, 5, value);
    }

    inline const char * getSectionNameByRef(void * value)
    {
      return ConfigFile::getSectionNameByRef(_dataMap, 5, value);
    }

        ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)
        {
                ConfigFile::ReadStatus status;
                _logLevel = logLevel;
                _errorLevel = errorLevel;
                status = ConfigFile::readData(_dataMap, 5, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());
                logData(&log_level_config_data_info, status);
                return status;
        }

    _C_Version Version;
    _C_FileInfo FileInfo;
    _C_Section1 Section1;
  };

public:
  static _C_Test1_A Test1_A_Access;
  static _C_Test1_B Test1_B_Access;
  static _C_Test1_C Test1_C_Access;
  static _C_Test1_D Test1_D_Access;
  static _C_Test1_E Test1_E_Access;
  static _C_Test1_F Test1_F_Access;
  static _C_Test1_G Test1_G_Access;
  static _C_Test1_H Test1_H_Access;
  static _C_Test1_I Test1_I_Access;
  static _C_Test1_J Test1_J_Access;
  static _C_Test1_K Test1_K_Access;
  static _C_Test1_L Test1_L_Access;
  static _C_Test1_M Test1_M_Access;
  static _C_Test1_O Test1_O_Access;
  static _C_Test1_P Test1_P_Access;
  static _C_Test1_Q Test1_Q_Access;
  static _C_Test1_S Test1_S_Access;
  static ConfigFile * getConfigFileObj(int index);
};

#endif /* ifndef _test1_INCLUDE */
