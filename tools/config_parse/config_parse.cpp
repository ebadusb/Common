// $Header: K:/BCT_Development/vxWorks/Common/tools/config_parse/rcs/config_parse.cpp 1.4 2006/10/19 20:01:02Z MS10234 Exp jl11312 $
//
// Configuration file parse tool
//
// $Log: config_parse.cpp $
// Revision 1.4  2006/10/19 20:01:02Z  MS10234
// 63 - Add logging of the config files
// Revision 1.3  2005/05/11 21:30:45Z  ms10234
// - added argument for file_names and config_helper header file locations
// Revision 1.2  2005/05/11 16:10:39Z  jl11312
// - corrected archive types
// Revision 1.4  2005/01/20 19:22:15Z  jl11312
// - added suport for export file for shared enumeration types
// Revision 1.3  2005/01/13 19:05:07Z  ms10234
// - changed location of config_helper.h
// Revision 1.2  2004/07/23 14:18:13Z  jl11312
// - corrected problem with routines to read data for all config files
// Revision 1.1  2004/07/09 16:57:09Z  jl11312
// Initial revision
//

#include "stdafx.h"

#pragma warning(disable:4786)
#include "config_parse.h"

#include <stdlib.h>
#include <string.h>

static vector<ConfigFile *> configFile;

static void generateHeaderFile(FILE * fp, const char * outputName, const char * exportOutputFile, const char * sourceFileBase, const char *fileNamesHeaderDir, const char *configHelperHeaderDir)
{
	fprintf(fp,
		"/*\n"
		" * Auto-generated file, do not edit\n"
		" *\n"
		" * Source Files:\n");

	unsigned int file;
	for ( file=0; file<configFile.size(); file++ )
	{
		fprintf(fp,
			" * %s formatVersion=%s dataVersion=%s\n",
			configFile[file]->fileName(), configFile[file]->formatVersion(), configFile[file]->dataVersion());
	}

	fprintf(fp,
		" *\n"
		" */\n\n"
		"#ifndef _%s_INCLUDE\n"
		"#define _%s_INCLUDE\n\n"
		"#include <vxWorks.h>\n"
		"#include <limits.h>\n\n"
		"#include \"datalog.h\"\n"
		"#include \"%s/file_names.h\"\n"
		"#include \"%s/config_helper.h\"\n\n",
		sourceFileBase, sourceFileBase, fileNamesHeaderDir, configHelperHeaderDir);

	if ( exportOutputFile )
	{
		fprintf(fp,
			"#include \"%s\"\n\n",
			exportOutputFile);
	}

	for ( file=0; file<configFile.size(); file++ ) configFile[file]->generateIncludeFileList(fp);

	fprintf(fp,
		"class %s : public ConfigData",
		outputName);

	if ( exportOutputFile )
	{
		fprintf(fp,
			", public %sEnum",
			outputName);
	}

	fprintf(fp,
		"\n"
		"{\n"
		);

	bool enumsExist = ( ConfigFile::enumCount() > 0 );
	if ( enumsExist )
	{
		fprintf(fp,
			"public:\n");

		ConfigFile::generateEnumList(fp, ConfigFile::Local);
	
		fprintf(fp,
			"\n"
			"private:\n"
			"  static const ConfigData::EnumMap _enumMap[%d];\n\n",
			ConfigFile::enumCount()+1);
	}

	for ( file=0; file<configFile.size(); file++ )
	{
		if ( configFile[file]->noOutputFile() ) continue;
		configFile[file]->generateHeaderDataStructure(fp);
	}
 
	for ( file=0; file<configFile.size(); file++ )
	{
		if ( configFile[file]->noOutputFile() ) continue;
		configFile[file]->generateHeaderAccessClass(fp);
   }

	fprintf(fp,
		"public:\n");

	for ( file=0; file<configFile.size(); file++ )
	{
		if ( configFile[file]->noOutputFile() ) continue;
		configFile[file]->generateHeaderAccessObj(fp);
	}

	fprintf(fp,
		"  static ConfigFile * getConfigFileObj(int index);\n"
		"};\n\n"
		"#endif /* ifndef _%s_INCLUDE */\n",
		sourceFileBase);
}

static void generateExportHeaderFile(FILE * fp, const char * outputName, const char * sourceFileBase)
{
	fprintf(fp,
		"/*\n"
		" * Auto-generated file, do not edit\n"
		" *\n"
		" * Source Files:\n");

	unsigned int file;
	for ( file=0; file<configFile.size(); file++ )
	{
		fprintf(fp,
			" * %s formatVersion=%s dataVersion=%s\n",
			configFile[file]->fileName(), configFile[file]->formatVersion(), configFile[file]->dataVersion());
	}

	fprintf(fp,
		" *\n"
		" */\n\n"
		"#ifndef _%s_enum_INCLUDE\n"
		"#define _%s_enum_INCLUDE\n\n",
		sourceFileBase, sourceFileBase);

	for ( file=0; file<configFile.size(); file++ ) configFile[file]->generateIncludeFileList(fp);

	fprintf(fp,
		"class %sEnum\n"
		"{\n",
		outputName);

	bool enumsExist = ( ConfigFile::enumCount() > 0 );
	if ( enumsExist )
	{
		fprintf(fp,
			"public:\n");

		ConfigFile::generateEnumList(fp, ConfigFile::Exported);
	}

	fprintf(fp,
		"};\n\n"
		"#endif /* ifndef _%s_enum_INCLUDE */\n",
		sourceFileBase);
}

void generateSourceFile(FILE * fp, const char * outputName, const char * sourceFileBase)
{
	fprintf(fp,
		"/*\n"
		" * Auto-generated file, do not edit\n"
		" */\n\n"
		"#include <vxWorks.h>\n"
		"#include \"%s.h\"\n\n",
		sourceFileBase);

	unsigned int file;
	for ( file=0; file<configFile.size(); file++ )
	{
		if ( configFile[file]->noOutputFile() ) continue;

		fprintf(fp,
			"%s::%s_Data %s::_%s_Data;\n",
			outputName, configFile[file]->className(), outputName, configFile[file]->className());

		fprintf(fp,
			"%s::_C_%s %s::%s_Access(_%s_Data);\n\n",
			outputName, configFile[file]->className(), outputName, configFile[file]->className(), configFile[file]->className());
	}

	fprintf(fp,
		"ConfigFile * %s::getConfigFileObj(int index)\n"
		"{\n"
		"  switch (index)\n"
		"  {\n",
		outputName);

	int	accessIndex = 0;
	for ( file=0; file<configFile.size(); file++ )
	{
		if ( configFile[file]->noOutputFile() ) continue;

		fprintf(fp,
			"  case %d: return (ConfigFile *)&%s_Access;\n",
			accessIndex, configFile[file]->className());
	   accessIndex += 1;
	}

	fprintf(fp,
		"  default: return NULL;\n"
		"  }\n"
		"}\n\n");

	int enumCount = ConfigFile::enumCount();
	if ( enumCount > 0 )
	{
		fprintf(fp,
			"const ConfigData::EnumMap %s::_enumMap[%d] =\n"
			"{\n",
			outputName, enumCount+1);

		ConfigFile::generateEnumMap(fp);
	
		fprintf(fp,
			"  { NULL, 0 }\n"
			"};\n\n");
	}

	for ( file=0; file<configFile.size(); file++ )
	{
		if ( configFile[file]->noOutputFile() ) continue;

		configFile[file]->generateNameList(fp, outputName);
		configFile[file]->generateSectionNameList(fp, outputName);
		configFile[file]->generateConstructors(fp, outputName);
		configFile[file]->generateLogDataFunction(fp, outputName);
		configFile[file]->generateParameterValidateFunctions(fp, outputName);
		configFile[file]->generateParameterRangeFunctions(fp, outputName);
		configFile[file]->generateParameterSetFunctions(fp, outputName);
   }
}

void parseFileList(void)
{
	bool error = false;
	for (unsigned int file = 0; file < configFile.size(); file += 1)
	{
		FILE * cfg_file = fopen(configFile[file]->fileName(), "r");
		if ( !cfg_file )
		{
			fprintf(stderr, "Unable to access file \"%s\"\n", configFile[file]->fileName());
			error = true;
			continue;
		}

		error |= configFile[file]->parseFile(cfg_file);
		fclose(cfg_file);
	}

	if ( error ) exit(1);
}

void generateCode(const char * outputName, const char * exportOutputFile, const char * outputDir, const char *fileNamesHeaderDir, const char *configHelperHeaderDir)
{
	string fileName;
	string sourceFileBase;
	bool	 error = false;

	sourceFileBase.insert(sourceFileBase.end(), tolower(outputName[0]));
	const char * convertChar = &outputName[1];

	while ( *convertChar )
	{
		if ( *convertChar >= 'A' && *convertChar <= 'Z' )
		{
			sourceFileBase.insert(sourceFileBase.end(), '_');
		}

		sourceFileBase.insert(sourceFileBase.end(), tolower(*convertChar));
		convertChar++;
	}

	fileName = outputDir + string("/") + sourceFileBase + string(".cpp");
	FILE * cpp_file = fopen(fileName.c_str(), "w");
	if ( !cpp_file )
	{
		fprintf(stderr, "Can't open \"%s\"\n", fileName);
		error = true;
	}

	fileName = outputDir + string("/") + sourceFileBase + string(".h");
	FILE * h_file = fopen(fileName.c_str(), "w");
	if ( !h_file )
	{
		fprintf(stderr, "Can't open \"%s\"\n", fileName);
		error = true;
	}

	if ( !error )
	{
		generateHeaderFile(h_file, outputName, exportOutputFile, sourceFileBase.c_str(), fileNamesHeaderDir, configHelperHeaderDir);
		generateSourceFile(cpp_file, outputName, sourceFileBase.c_str());
	}

	if ( cpp_file ) fclose(cpp_file);
	if ( h_file ) fclose(h_file);
	if ( error ) exit(1);
}

void generateExportFile(const char * outputName, const char * exportFileName, const char * outputDir)
{
	string fileName;
	string sourceFileBase;
	bool	 error = false;

	sourceFileBase.insert(sourceFileBase.end(), tolower(outputName[0]));
	const char * convertChar = &outputName[1];

	while ( *convertChar )
	{
		if ( *convertChar >= 'A' && *convertChar <= 'Z' )
		{
			sourceFileBase.insert(sourceFileBase.end(), '_');
		}

		sourceFileBase.insert(sourceFileBase.end(), tolower(*convertChar));
		convertChar++;
	}

	fileName = outputDir + string("/") + exportFileName;
	FILE * h_file = fopen(fileName.c_str(), "w");
	if ( !h_file )
	{
		fprintf(stderr, "Can't open \"%s\"\n", fileName);
		error = true;
	}

	if ( !error )
	{
		generateExportHeaderFile(h_file, outputName, sourceFileBase.c_str());
	}

	if ( h_file ) fclose(h_file);
	if ( error ) exit(1);
}

void generateDataFiles(const char * dataFileDir)
{
	for ( unsigned int file=0; file<configFile.size(); file++ )
	{
		if ( configFile[file]->noOutputFile() ) continue;
		configFile[file]->generateDataFile(dataFileDir);
   }
}

int main(int argc, char* argv[])
{
	//
	// Usage:
	//   config_parse [options] [file list]
	//
	//   options:
	//			-output <name> - sets base file name for output, this also sets the associated class name
	//			-export_output <name> - sets the file name and class name for exported types
	//       -data_file_dir <name> - sets the base directory name for generated data files
	//
	//			-output_dir <name> - sets the directory name for output files
	//			-input_dir <name>	- sets the directory name for config files
	//
	//			-project <name> - set root project name (e.g. -project taos)
   //    
   //       -file_names <path> - sets the directory path, relative to the base directory, where "file_names.h" exists in the project
   //       -config_helper <path> - sets the directory path, relative to the base directory, where "config_helper.h" exists in the project
	//
	//	  file list:
	//	  		List of one or more file names (with no path information and no .cfg extension, these are
	//			added automatically.
	//
	int		argParse = 1;
	int		fileCount = 0;
	char *	outputName = NULL;
	char *   exportOutputFile = NULL;
	char *   dataFileDir = NULL;
	char *	outputDir = NULL;
	char *	inputDir = NULL;
	char *	projectName = NULL;
	char *	fileNamesHeaderDir = NULL;
	char *   configHelperHeaderDir = NULL;

	while ( argParse < argc )
	{
		if ( argv[argParse][0] == '-' )
		{
			//	Process command line option
			//
			if ( strcmp(argv[argParse], "-output") == 0 )
			{
				if ( argParse+1 >= argc )
				{
					fprintf(stderr, "Missing -output argument\n");
					exit(1);
				}

				outputName = argv[argParse+1];
				argParse += 2;
			}
			if ( strcmp(argv[argParse], "-export_output") == 0 )
			{
				if ( argParse+1 >= argc )
				{
					fprintf(stderr, "Missing -export_output argument\n");
					exit(1);
				}

				exportOutputFile = argv[argParse+1];
				argParse += 2;
			}
			else if ( strcmp(argv[argParse], "-data_file_dir") == 0 )
			{
				if ( argParse+1 >= argc )
				{
					fprintf(stderr, "Missing -data_file_dir argument\n");
					exit(1);
				}

				dataFileDir = argv[argParse+1];
				if ( dataFileDir[0] && 
						( dataFileDir[strlen(dataFileDir)-1] == '/' ||
						  dataFileDir[strlen(dataFileDir)-1] == '\\' ))
			   {
					dataFileDir[strlen(dataFileDir)-1] = '\0';
				}

				argParse += 2;
			}
			else if ( strcmp(argv[argParse], "-input_dir") == 0 )
			{
				if ( argParse+1 >= argc )
				{
					fprintf(stderr, "Missing -input_dir argument\n");
					exit(1);
				}

				inputDir = argv[argParse+1];
				if ( inputDir[0] && 
						( inputDir[strlen(inputDir)-1] == '/' ||
						  inputDir[strlen(inputDir)-1] == '\\' ))
			   {
					inputDir[strlen(inputDir)-1] = '\0';
				}

				argParse += 2;
			}
			else if ( strcmp(argv[argParse], "-project") == 0 )
			{
				if ( argParse+1 >= argc )
				{
					fprintf(stderr, "Missing -project argument\n");
					exit(1);
				}

			   projectName = argv[argParse+1];
				argParse += 2;
			}
			else if ( strcmp(argv[argParse], "-file_names") == 0 )
			{
				if ( argParse+1 >= argc )
				{
					fprintf(stderr, "Missing -file_names argument\n");
					exit(1);
				}

			   fileNamesHeaderDir = argv[argParse+1];
				argParse += 2;
			}
			else if ( strcmp(argv[argParse], "-config_helper") == 0 )
			{
				if ( argParse+1 >= argc )
				{
					fprintf(stderr, "Missing -config_helper argument\n");
					exit(1);
				}

			   configHelperHeaderDir = argv[argParse+1];
				argParse += 2;
			}
			else if ( strcmp(argv[argParse], "-output_dir") == 0 )
			{
				if ( argParse+1 >= argc )
				{
					fprintf(stderr, "Missing -output_dir argument\n");
					exit(1);
				}

				outputDir = argv[argParse+1];
				if ( outputDir[0] && 
						( outputDir[strlen(outputDir)-1] == '/' ||
						  outputDir[strlen(outputDir)-1] == '\\' ))
			   {
					outputDir[strlen(outputDir)-1] = '\0';
				}

				argParse += 2;
			}
			else
			{
				fprintf(stderr, "Unrecoginized option \"%s\"\n", argv[argParse]);
				exit(1);
			}
		}
		else
		{
			string fileName = (inputDir) ? inputDir : ".";
			fileName.insert(fileName.end(), '/');
			fileName.insert(fileName.size(), argv[argParse]);
			fileName.insert(fileName.size(), ".cfg");
			
			configFile.push_back(new ConfigFile(fileName.c_str()));
			argParse += 1;
		}
	}
 
	if ( !outputName && !dataFileDir )
	{
		fprintf(stderr, "Must specify \"-output\" or \"-data_file_dir\" option\n");
		exit(1);
	}

	if ( !projectName )
	{
		fprintf(stderr, "Must specify \"-project\" option\n");
		exit(1);
	}
	else if ( !fileNamesHeaderDir )
	{
		fprintf(stderr, "Must specify \"-file_names\" option\n");
		exit(1);
	}
	else if ( !configHelperHeaderDir )
	{
		fprintf(stderr, "Must specify \"-config_helper\" option\n");
		exit(1);
	}
	else
	{
		for ( unsigned int file=0; file<configFile.size(); file++ )
		{
			configFile[file]->setProjectName(projectName);
		}
	}

	if ( configFile.size() == 0 )
	{
		fprintf(stderr, "Must specify at least one file name\n");
		exit(1);
	}

	if ( !outputDir )
	{
		outputDir = ".";
	}

	parseFileList();
	if ( outputName ) generateCode(outputName, exportOutputFile, outputDir, fileNamesHeaderDir, configHelperHeaderDir);
	if ( exportOutputFile ) generateExportFile(outputName, exportOutputFile, outputDir);
	if ( dataFileDir ) generateDataFiles(dataFileDir);

	return 0;
}

