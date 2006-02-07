// build_alarm_data.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <list>

using namespace std;

enum 
{
	LineBufferSize = 65000,
	MAX_PATH = 265
};

typedef list<string> StringList;

struct Buttons
{
	string ButtonName;
	string Available;
	string ResponseState;
	string ResponseType;
};

typedef list<Buttons> ButtonList;

struct ButtonGroup
{
	string GroupName;
	ButtonList buttonInfo;
	string Params;
};

typedef list<ButtonGroup> ButtonGroupList;

struct AlarmData
{
	string alarmID;
	string node;
	string layer;
	string alarmNamespace;
	string priority;
	string constraint;
	string response;
	string display;
	string screen;
	string alarmMsg;
	string alarmText;
	string buttonGroupName;
};

typedef list<AlarmData> AlarmDataList;

StringList			_stringInfoIDList;
ButtonGroupList		_buttonGroupList;
AlarmDataList		_alarmDataList;

// Loads in string IDs from a given string.info file
bool loadStringInfo(char *stringInfoPath)
{
	FILE *pFile = fopen(stringInfoPath, "r");
	if (pFile == NULL)
	{
		printf("ERROR: %s file not found\r\n", stringInfoPath);
		return false;
	}

	char lineBuffer[LineBufferSize];
	while ( fgets(lineBuffer, LineBufferSize, pFile) != NULL )
	{
		char *firstToken = strtok(lineBuffer, " \t\n");
		if (!firstToken || firstToken[0] == '#')
		{
			continue;
		}
		char guiStringID[MAX_PATH];
		memset(guiStringID, 0, MAX_PATH);
		strcpy(guiStringID, "\"");
		strcat(guiStringID, firstToken);
		strcat(guiStringID, "\"");
		_stringInfoIDList.push_back(guiStringID);
	}

	fclose(pFile);
	return true;
}

// Reads in alarm.info and sets up all of our link lists that 
// are needed to generate our alarm_id.h and alarm_config.cpp files.
// Also calls verifyString if the user gave us a string.info file path.
int readStringInfo(char *pStrPath)
{
	int retVal = 0;
	int lineNo = 0;
	char fileName[MAX_PATH];
	strcpy(fileName, pStrPath);
	strcat(fileName, "/alarm.info");

	FILE *pFile = fopen(fileName, "r");
	if (pFile == NULL)
	{
		printf("ERROR: %s file not found\r\n", fileName);
		return -1;
	}

	ButtonGroup btnGrp;
	bool bReadingButtons = false;
	bool bReadingAlarms = false;
	char lineBuffer[LineBufferSize];

	while ( fgets(lineBuffer, LineBufferSize, pFile) != NULL )
	{
		lineNo++;
		char *firstToken = strtok(lineBuffer, " \t\n");
		if ( !firstToken || firstToken[0] == '#' )
		{
			// empty line or a comment
			continue;
		}

		if (strcmp(firstToken, "BEGIN_DEFINE_BUTTONS") == 0)
		{
			// starting the button definition section
			bReadingButtons = true;
			continue;
		}

		if (strcmp(firstToken, "END_DEFINE_BUTTONS") == 0)
		{
			// done with the button definition section
			bReadingButtons = false;
			continue;
		}

		if (strcmp(firstToken, "BEGIN_ALARM_CONFIG") == 0)
		{
			// starting the alarm config section
			bReadingAlarms = true;
			continue;
		}

		if (strcmp(firstToken, "END_ALARM_CONFIG") == 0)
		{
			// done with the alarm config section and since that's the
			// last section in the file we're done reading the file too.
			break;
		}

		if (bReadingButtons)
		{
			// if '%' is found it's just an indication that this is a 
			// continuation of the alarm screen definition
			if (firstToken[0] == '%')
			{
				firstToken = strtok(NULL, " \t\n");
			}

			if (stricmp(firstToken, "Name") == 0)
			{
				firstToken = strtok(NULL, " \t\n");
				btnGrp.GroupName = firstToken;
				continue;
			}

			if (firstToken[0] == 'B')
			{
				// adding a button to the group
				Buttons buttonInfo;
				buttonInfo.ButtonName = strtok(NULL, " \t\n");
				if (buttonInfo.ButtonName != "NULL")
				{
					buttonInfo.Available = strtok(NULL, " \t\n");
					buttonInfo.ResponseState = strtok(NULL, " \t\n");
					buttonInfo.ResponseType = strtok(NULL, " \t\n");
				}
				btnGrp.buttonInfo.push_back(buttonInfo);
				continue;
			}

			if (stricmp(firstToken, "Params") == 0)
			{
				// Adding parameter info to the group.
				// parameters may have spaces in them therefore don't
				// include " " in the delimiter arg for strtok...
				btnGrp.Params = strtok(NULL, "\t\n");
				continue;
			}

			if (stricmp(firstToken, "End") == 0)
			{
				// done with this button group
				_buttonGroupList.push_back(btnGrp);
				btnGrp.buttonInfo.clear();
				continue;
			}
		}
		else if (bReadingAlarms)
		{
			AlarmData alarm;
			try
			{
				alarm.alarmID = firstToken;
				alarm.node = strtok(NULL, " \t\n");
				alarm.layer = strtok(NULL, " \t\n");
				alarm.alarmNamespace = strtok(NULL, " \t\n");
				alarm.priority = strtok(NULL, " \t\n");
				alarm.constraint = strtok(NULL, " \t\n");
				alarm.response = strtok(NULL, " \t\n");
				alarm.display = strtok(NULL, " \t\n");
				alarm.screen = strtok(NULL, " \t\n");
				alarm.alarmMsg = strtok(NULL, " \t\n");
				alarm.alarmText = strtok(NULL, " \t\n");
				alarm.buttonGroupName = strtok(NULL, " \t\n");
				_alarmDataList.push_back(alarm);
			}
			catch (...)
			{
				printf("ERROR: Bad syntax in alarm.info line %d", lineNo);
				retVal = -1;
				break;
			}

			// if we were given a string.info file then the user wants us
			// to verify that the alarmMsg and alarmText string actually exist
			// and are defined in a string.info file already.
			if (_stringInfoIDList.size() > 0)
			{
				bool bMsgOk = false;
				bool bTextOk = false;
				StringList::iterator iter;
				
				if (strcmp("\"\"", alarm.alarmMsg.c_str()) == 0)
				{
					bMsgOk = true;
				}
				
				if (strcmp("\"\"", alarm.alarmText.c_str()) == 0)
				{
					bTextOk = true;
				}

				for (iter = _stringInfoIDList.begin(); iter != _stringInfoIDList.end(); iter++)
				{
					if (!bMsgOk)
					{
						if ((*iter) == alarm.alarmMsg)
						{
							bMsgOk = true;
						}
					}

					if (!bTextOk)
					{
						if ((*iter) == alarm.alarmText)
						{
							bTextOk = true;
						}
					}

					if (bMsgOk && bTextOk)
					{
						break;
					}
				}

				if (!bMsgOk)
				{
					printf("ERROR: %s @ line(%d) not found in any string.info file\r\n", alarm.alarmMsg.c_str(), lineNo);
					retVal = -1;
					break;
				}
	
				if (!bMsgOk)
				{
					printf("ERROR: %s @ line(%d) not found in any string.info file\r\n", alarm.alarmText.c_str(), lineNo);
					retVal = -1;
					break;
				}

			}
		}

	}

	fclose(pFile);
	return retVal;
}

// Generates alarm_id.h
int generateAlarmID(char *pStrPath)
{
	char fileName[MAX_PATH];
	strcpy(fileName, pStrPath);
	strcat(fileName, "/alarm_id.h");

	FILE *pFile = fopen(fileName, "w");
	if (pFile == NULL)
	{
		printf("ERROR: Could not create file %s.\n", fileName);
		return -1;
	}

	// file header
	fprintf(pFile, "// Do NOT edit this file.\n");
	fprintf(pFile, "// This is an auto-generated file from the build_alarm_data tool.\n");
	fprintf(pFile, "// Built from %s/alarm.info\n\n", pStrPath);
	
	AlarmDataList::iterator iter;
	for (iter = _alarmDataList.begin(); iter != _alarmDataList.end(); iter++)
	{
		// if the user does not want the alarmID to be placed in alarm_id.h
		// they will add a '_' to the beginning of the enum name....
		const char *pStr = (*iter).alarmID.c_str();
		if (pStr[0] == '_')
		{
			// don't add it to alarm_id.h and remove the '_' for 
			// generateAlarmConfig's sake...
			pStr++;
			(*iter).alarmID = pStr;
		}
		else
		{
			fprintf(pFile, "%s,\n", (*iter).alarmID.c_str());
		}
	}

	fclose(pFile);
	printf("alarm_id.h generated.\n");
	return 0;
}

// Generates alarm_config.cpp
int generateAlarmConfig(char *pStrPath)
{
	char fileName[MAX_PATH];
	strcpy(fileName, pStrPath);
	strcat(fileName, "/alarm_config.cpp");

	FILE *pFile = fopen(fileName, "w");
	if (pFile == NULL)
	{
		printf("ERROR: Could not create file %s.\n", fileName);
		return -1;
	}

	// file header
	fprintf(pFile, "// Do NOT edit this file.\n");
	fprintf(pFile, "// This is an auto-generated file from the build_alarm_data tool.\n");
	fprintf(pFile, "// Built from %s/alarm.info\n\n", pStrPath);

	// include files
	fprintf(pFile, "#include <string>\n\n");

	// structure for buttons in alarm response array
	fprintf(pFile, "struct ResponseArrayButtonsStruct\n");
	fprintf(pFile, "{\n");
	fprintf(pFile, "\tbool createMe;\n");
	fprintf(pFile, "\tBaseResponseButton::BaseButtonId buttonName;\n");
	fprintf(pFile, "\tbool available;\n");
	fprintf(pFile, "\tAlarmResponseState responseState;\n");
	fprintf(pFile, "\tAlarmResponseType responseType;\n");
	fprintf(pFile, "};\n\n");

	// structure for alarm response array table
	fprintf(pFile, "struct AlarmResponseArrayStruct\n");
	fprintf(pFile, "{\n");
	fprintf(pFile, "\tchar *arrayName;\n");
	fprintf(pFile, "\tResponseArrayButtonsStruct response1;\n");
	fprintf(pFile, "\tResponseArrayButtonsStruct response2;\n");
	fprintf(pFile, "\tResponseArrayButtonsStruct response3;\n");
	fprintf(pFile, "\tResponseArrayButtonsStruct response4;\n");
	fprintf(pFile, "\tResponseArrayButtonsStruct response5;\n");
	fprintf(pFile, "\tResponseArrayButtonsStruct response6;\n");
	fprintf(pFile, "\tResponseArrayButtonsStruct response7;\n");
	fprintf(pFile, "\tResponseArrayButtonsStruct response8;\n");
	fprintf(pFile, "\tResponseArrayButtonsStruct response9;\n");
	fprintf(pFile, "\tResponseArrayButtonsStruct response10;\n");
	fprintf(pFile, "\tchar *params;\n");
	fprintf(pFile, "};\n\n");

	// structure for alarm attribute table
	fprintf(pFile, "struct AlarmAttributesStruct\n");
	fprintf(pFile, "{\n");
	fprintf(pFile, "\tstring					_alarmName;		// string version of _alarmId\n");
	fprintf(pFile, "\tTaosNodeID				_nodeId;		// Node ID or source of the alarm (control or safety)\n");
	fprintf(pFile, "\tTaosLinkElement::Level	_moduleLevel;	// Module link level base, disposable, protocol\n");
	fprintf(pFile, "\tint						_alarmId;		// Alarm serial number (only unique for a module and node)\n");
	fprintf(pFile, "\tint						_priority;		// Display priority higher number is higher priority\n");
	fprintf(pFile, "\tConstraintGroup*			_constraintObj;	// Pointer to constraint group object. Pointer may be NULL.\n");
	fprintf(pFile, "\tAlarmResponse* 			_responseObj;	// Pointer to operator response. Pointer may be NULL.\n");
	fprintf(pFile, "\tstring					_responseMsgs;	// string that maps to data that will create the response message array.\n");
	fprintf(pFile, "\tAlarmDisplay* 			_displayObj;	// Pointer to display object. Pointer may be NULL.\n");
	fprintf(pFile, "\tActionScreenGenFunc		_actionGenerator;	// Function to generate a BaseActionScreen* for GUI object.\n");
	fprintf(pFile, "\tstring					_alarmMsg;\n");
	fprintf(pFile, "\tstring					_alarmText;\n");
	fprintf(pFile, "};\n\n");
	
	// alarm response array table
	fprintf(pFile, "AlarmResponseArrayStruct responseArrayTable[] =\n");
	fprintf(pFile, "{\n");
	ButtonGroupList::iterator iterBtnGrp;
	ButtonList::iterator iterBtn;
	bool bFirstTimeThrough = true;
	for (iterBtnGrp = _buttonGroupList.begin(); iterBtnGrp != _buttonGroupList.end(); iterBtnGrp++)
	{
		if (!bFirstTimeThrough)
		{
			fprintf(pFile, "},\n");
		}
		bFirstTimeThrough = false;

		fprintf(pFile, "{\"%s\"", (*iterBtnGrp).GroupName.c_str());
		int count = 0;
		for (iterBtn = (*iterBtnGrp).buttonInfo.begin(); iterBtn != (*iterBtnGrp).buttonInfo.end(); iterBtn ++)
		{
			count++;
			string buttonName = (*iterBtn).ButtonName;
			if (buttonName == "NULL")
			{
				fprintf(pFile, ",{false,BaseResponseButton::Continue,false,ResponsePending,NoSafetyAction}");
			}
			else
			{
				fprintf(pFile, ",{true,%s,%s,%s,%s}", 
					buttonName.c_str(), (*iterBtn).Available.c_str(), 
					(*iterBtn).ResponseState.c_str(), (*iterBtn).ResponseType.c_str());
			}
		}

		// fill in rest of buttons as NULL
		for (int i = count; i < 10; i++)
		{
			fprintf(pFile, ",{false,BaseResponseButton::Continue,false,ResponsePending,NoSafetyAction}");
		}
	
		// fill in params information
		fprintf(pFile, ",\"%s\"", (*iterBtnGrp).Params.c_str());
	}
	fprintf(pFile, "}\n");
	fprintf(pFile, "};\n\n");

	// alarm attributes table
	fprintf(pFile, "AlarmAttributesStruct attributesTable[] =\n");
	fprintf(pFile, "{\n");
	AlarmDataList::iterator alarmDataIter;
	bFirstTimeThrough = true;
	for (alarmDataIter = _alarmDataList.begin(); alarmDataIter != _alarmDataList.end(); alarmDataIter++)
	{
		if (!bFirstTimeThrough)
		{
			fprintf(pFile, "},\n");
		}
		bFirstTimeThrough = false;

		if ((*alarmDataIter).display == "NULL")
		{
			fprintf(pFile, "{\"%s\",%s,TaosLinkElement::%s,%s::%s,%s,&%s,&%s,\"%s\",%s,%s,%s,%s", 
				(*alarmDataIter).alarmID.c_str(), (*alarmDataIter).node.c_str(), (*alarmDataIter).layer.c_str(), 
				(*alarmDataIter).alarmNamespace.c_str(), (*alarmDataIter).alarmID.c_str(), (*alarmDataIter).priority.c_str(), 
				(*alarmDataIter).constraint.c_str(), (*alarmDataIter).response.c_str(), (*alarmDataIter).buttonGroupName.c_str(), 
				(*alarmDataIter).display.c_str(), (*alarmDataIter).screen.c_str(), (*alarmDataIter).alarmMsg.c_str(), (*alarmDataIter).alarmText.c_str());
		}
		else
		{
			fprintf(pFile, "{\"%s\",%s,TaosLinkElement::%s,%s::%s,%s,&%s,&%s,\"%s\",&%s,%s,%s,%s", 
				(*alarmDataIter).alarmID.c_str(), (*alarmDataIter).node.c_str(), (*alarmDataIter).layer.c_str(), 
				(*alarmDataIter).alarmNamespace.c_str(), (*alarmDataIter).alarmID.c_str(), (*alarmDataIter).priority.c_str(), 
				(*alarmDataIter).constraint.c_str(), (*alarmDataIter).response.c_str(), (*alarmDataIter).buttonGroupName.c_str(), 
				(*alarmDataIter).display.c_str(), (*alarmDataIter).screen.c_str(), (*alarmDataIter).alarmMsg.c_str(), (*alarmDataIter).alarmText.c_str());
		}
	}
	fprintf(pFile, "}\n");
	fprintf(pFile, "};\n\n");

	// code that uses the above structs and tables to init the alarms
	string layer = (*_alarmDataList.begin()).layer;
	fprintf(pFile, "class %s_AlarmConfig\n", layer.c_str());
	fprintf(pFile, "{\n");
	fprintf(pFile, "public:\n");
	fprintf(pFile, "\t%s_AlarmConfig(AlarmAttributesList *alarmSet);\n", layer.c_str());
	fprintf(pFile, "\t~%s_AlarmConfig(){};\n", layer.c_str());
	fprintf(pFile, "};\n\n");

	fprintf(pFile, "%s_AlarmConfig::%s_AlarmConfig(AlarmAttributesList *alarmSet)\n", layer.c_str(), layer.c_str());
	fprintf(pFile, "{\n");
	fprintf(pFile, "\tfor ( int i=0; i < sizeof(attributesTable)/sizeof(attributesTable[0]); i++ )\n");
	fprintf(pFile, "\t{\n");
	fprintf(pFile, "\t\t// create response array\n");
	fprintf(pFile, "\t\tstring responseName = attributesTable[i]._responseMsgs;\n");
	fprintf(pFile, "\t\tAlarmResponseArray* pResponseArray = NULL;\n");
	fprintf(pFile, "\t\tint len = attributesTable[i]._alarmMsg.size();\n");
	fprintf(pFile, "\t\tlen += attributesTable[i]._alarmText.size();\n");
	fprintf(pFile, "\t\tchar *parameterString = NULL;\n");
	fprintf(pFile, "\t\tfor (int y=0; y < sizeof(responseArrayTable)/sizeof(responseArrayTable[0]); y++)\n");
	fprintf(pFile, "\t\t{\n");
	fprintf(pFile, "\t\t\tif (strcmp(responseArrayTable[y].arrayName, responseName.c_str()) == 0)\n");
	fprintf(pFile, "\t\t\t{\n");
	fprintf(pFile, "\t\t\t\tpResponseArray = AlarmResponseArray::createResponseObj(\n");
	fprintf(pFile, "\t\t\t\t\t(responseArrayTable[y].response1.createMe) ? (new BaseResponseButton(responseArrayTable[y].response1.buttonName, responseArrayTable[y].response1.available, responseArrayTable[y].response1.responseState, responseArrayTable[y].response1.responseType)) : NULL,\n");
	fprintf(pFile, "\t\t\t\t\t(responseArrayTable[y].response2.createMe) ? (new BaseResponseButton(responseArrayTable[y].response2.buttonName, responseArrayTable[y].response2.available, responseArrayTable[y].response2.responseState, responseArrayTable[y].response2.responseType)) : NULL,\n");
	fprintf(pFile, "\t\t\t\t\t(responseArrayTable[y].response3.createMe) ? (new BaseResponseButton(responseArrayTable[y].response3.buttonName, responseArrayTable[y].response3.available, responseArrayTable[y].response3.responseState, responseArrayTable[y].response3.responseType)) : NULL,\n");
	fprintf(pFile, "\t\t\t\t\t(responseArrayTable[y].response4.createMe) ? (new BaseResponseButton(responseArrayTable[y].response4.buttonName, responseArrayTable[y].response4.available, responseArrayTable[y].response4.responseState, responseArrayTable[y].response4.responseType)) : NULL,\n");
	fprintf(pFile, "\t\t\t\t\t(responseArrayTable[y].response5.createMe) ? (new BaseResponseButton(responseArrayTable[y].response5.buttonName, responseArrayTable[y].response5.available, responseArrayTable[y].response5.responseState, responseArrayTable[y].response5.responseType)) : NULL,\n");
	fprintf(pFile, "\t\t\t\t\t(responseArrayTable[y].response6.createMe) ? (new BaseResponseButton(responseArrayTable[y].response6.buttonName, responseArrayTable[y].response6.available, responseArrayTable[y].response6.responseState, responseArrayTable[y].response6.responseType)) : NULL,\n");
	fprintf(pFile, "\t\t\t\t\t(responseArrayTable[y].response7.createMe) ? (new BaseResponseButton(responseArrayTable[y].response7.buttonName, responseArrayTable[y].response7.available, responseArrayTable[y].response7.responseState, responseArrayTable[y].response7.responseType)) : NULL,\n");
	fprintf(pFile, "\t\t\t\t\t(responseArrayTable[y].response8.createMe) ? (new BaseResponseButton(responseArrayTable[y].response8.buttonName, responseArrayTable[y].response8.available, responseArrayTable[y].response8.responseState, responseArrayTable[y].response8.responseType)) : NULL,\n");
	fprintf(pFile, "\t\t\t\t\t(responseArrayTable[y].response9.createMe) ? (new BaseResponseButton(responseArrayTable[y].response9.buttonName, responseArrayTable[y].response9.available, responseArrayTable[y].response9.responseState, responseArrayTable[y].response9.responseType)) : NULL,\n");
	fprintf(pFile, "\t\t\t\t\t(responseArrayTable[y].response10.createMe) ? (new BaseResponseButton(responseArrayTable[y].response10.buttonName, responseArrayTable[y].response10.available, responseArrayTable[y].response10.responseState, responseArrayTable[y].response10.responseType)) : NULL\n");
	fprintf(pFile, "\t\t\t\t);\n");
	fprintf(pFile, "\t\t\t\tlen += strlen(responseArrayTable[y].params) + 1;\n");
	fprintf(pFile, "\t\t\t\tparameterString = new char[len];\n");
	fprintf(pFile, "\t\t\t\tmemset(parameterString, 0, len);\n");
	fprintf(pFile, "\t\t\t\tstrcpy(parameterString, attributesTable[i]._alarmMsg.c_str());\n");
	fprintf(pFile, "\t\t\t\tstrcat(parameterString, \":\");\n");
	fprintf(pFile, "\t\t\t\tstrcat(parameterString, attributesTable[i]._alarmText.c_str());\n");
	fprintf(pFile, "\t\t\t\tstrcat(parameterString, responseArrayTable[y].params);\n");
    fprintf(pFile, "\t\t\t}\n");
	fprintf(pFile, "\t\t}\n\n");
	fprintf(pFile, "\t\t// add new alarm attribute object\n");
	fprintf(pFile, "\t\talarmSet->add(new AlarmAttributes(\n");
	fprintf(pFile, "\t\t\tattributesTable[i]._nodeId,\n");
	fprintf(pFile, "\t\t\tattributesTable[i]._moduleLevel,\n");
    fprintf(pFile, "\t\t\tattributesTable[i]._alarmId,\n");
	fprintf(pFile, "\t\t\tattributesTable[i]._priority,\n");
	fprintf(pFile, "\t\t\tattributesTable[i]._constraintObj,\n");
	fprintf(pFile, "\t\t\tattributesTable[i]._responseObj,\n");
	fprintf(pFile, "\t\t\tpResponseArray,\n");
	fprintf(pFile, "\t\t\tattributesTable[i]._displayObj,\n");
	fprintf(pFile, "\t\t\tattributesTable[i]._actionGenerator,\n");
	fprintf(pFile, "\t\t\tparameterString,\n");
	fprintf(pFile, "\t\t\tattributesTable[i]._alarmName.c_str()));\n");
	fprintf(pFile, "\t}\n");
	fprintf(pFile, "}\n\n");

	fclose(pFile);
	printf("alarm_config.cpp generated.\n");
	return 0;
}

char **ParseStringInfoArray(char *stringInfo)
{
	int arrayIndex = 0;
	char **pArray = new char *[MAX_PATH];
	pArray[arrayIndex] = new char[MAX_PATH];
	pArray[arrayIndex] = strtok(stringInfo, ",");
	while (pArray[arrayIndex] != NULL)
	{
		arrayIndex++;
		pArray[arrayIndex] = new char[MAX_PATH];
		pArray[arrayIndex] = strtok(NULL, ",");
	}

	return pArray;
}

// Main entry point for the application.
int _tmain(int argc, _TCHAR* argv[])
{
	// Arguments
	// -p path to alarm.info file
	// -s stringInfo (path to string.info file to use for verifying)
	if (argc < 3)
	{
		printf("Missing arguments.\r\n");
		printf("Arguments are:\r\n");
		printf("  -p path (path to alarm.info file)\r\n");
		printf("  [-s stringInfoFileArray] (array of string.info files comma separated without spaces)\r\n\r\n");
		printf("Example:\r\n");
		printf("   build_alarm_data -p ../../base/safety/ -s ../../base/gui/string.info,../../base/generic.info\r\n");

		return -1;
	}

	char path[MAX_PATH];
	char stringInfo[MAX_PATH];
	memset(&path, 0, MAX_PATH);
	memset(&stringInfo, 0, MAX_PATH);

	bool bHaveMandatory = false;
	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "-p") == 0)
		{
			bHaveMandatory = true;
			strcpy(path, argv[++i]);
		}
		else if (strcmp(argv[i], "-s") == 0)
		{
			strcpy(stringInfo, argv[++i]);
		}
	}

	if (!bHaveMandatory)
	{
		printf("Missing arguments.\r\n");
		printf("Arguments are:\r\n");
		printf("  -p path (path to alarm.info file)\r\n");
		printf("  [-s stringInfoFileArray] (array of string.info files comma separated without spaces)\r\n\r\n");
		printf("Example:\r\n");
		printf("   build_alarm_data -p ../../base/safety/ -s ../../base/gui/string.info,../../base/generic.info\r\n");
	}

	_stringInfoIDList.clear();
	int arrayIndex = 0;
	char **pFileName = ParseStringInfoArray(stringInfo);
	while (pFileName[arrayIndex] != NULL)
	{
		if (!loadStringInfo(pFileName[arrayIndex]))
		{
			printf("ERROR: %s file not found.\r\n", pFileName[arrayIndex]);
			return -1;
		}
		arrayIndex++;
	}

	int retVal = readStringInfo(path);
	if (retVal == 0)
	{
		// MUST BE CALLED IN THIS ORDER...
		// generateAlarmID(path);
		// generateAlarmConfig(path);
		retVal = generateAlarmID(path);
		if (retVal == 0)
		{
			retVal = generateAlarmConfig(path);
			// generate .dfile for vxworks makfile
			string dfilePath = path;
			if (dfilePath.rfind('/') == dfilePath.length())
				dfilePath += ".dfile/alarm_config_cpp.d";
			else
				dfilePath += "/.dfile/alarm_config_cpp.d";
			FILE *dfile = fopen(dfilePath.c_str(), "w");
			fprintf(dfile, "alarm_config.cpp alarm_id.h : alarm.info");
			fclose(dfile);
		}
	}
	
	return retVal;
}
